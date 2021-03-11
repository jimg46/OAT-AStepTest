/* ========================================================================== */
/*  ATStepper.cpp
  Step TMC 2208/9 stepper with acceleration
  TMCStepper used to drive the motor under software control (UART mode)
  Uses AVR 16 bit timers
  ISR routine steps motor and loads next count
  Step pin state used as a flag for calculation of next step
  /*                                                                            */
/* ========================================================================== */

#include "ATStepper.h"
//constructor determines the motor driver type

//NEMA ED, BED, TMC trad.
ATStepper::ATStepper(int Timer, int StepPin, int DirPin ) //TMC 2208/9
{
  InitTimer(Timer); //stores timer in the inherited class
  _StepPin = StepPin;
  _DirPin = DirPin;
  pinMode(_StepPin, OUTPUT);
  pinMode(_DirPin, OUTPUT);
}

//set nsteps 0 to run continuously
void ATStepper::Move( float v, int accel, long nsteps, int dir) {
  //Serial.println("mv");
  stopped = true;
  _nsteps = nsteps;
  stepcount = 0;
  float t0; //time for first step

  if (accel > 0) {
    //v^2 = 2aS -> nsteps
    float x = 0.5 * v * v / accel; //steps to reach v
    _nAccel =  int(x + 0.5);  // integer acceleration steps
    t0 =   sqrt(2. / accel);     //S=at^2/2 -> t0
    _amode = 1;
    if ( nsteps > 0 &&  _nAccel >= nsteps / 2) {
      _nAccel = nsteps / 2;
    }
    _astp = 1;
    _DecelCount = _nsteps -  _nAccel;
  }
  else {
    t0 = 1. / v; //fixed time between steps
    _amode = 0;
    _nAccel = 0;
  }

  //form timer count and  prescale index
  float fclks =  16000000. * t0;
  if (accel > 0) {
    fclks *= 0.683; //compensates for first step error and integer truncation;
  }
  float clkcnt;
  for (_pindex = 1; _pindex < 6; _pindex++) {
    clkcnt = fclks / _prescale[_pindex];
    //Serial.println(String(_pindex) + "; " + String(clkcnt, 0)); //show first count settings
    
    if (clkcnt < 32767.) {   //may need to leave headroom for guider adjustment
      break;
    }
  }

  OCRValue = int(clkcnt);
  int FirstCount = OCRValue;
  SetNextCount();
   _ms = millis();
  StepMotor(); //first step
  StartTimer(FirstCount, _pindex);
  stopped = false;
}
void ATStepper::StopMotor() {
  if (_nAccel == 0) {
    StopTimer();
    stopped = true;
  }
  else {  //decelerate
    _amode = -1;
  }
}

// load count for subsequent step into NextCount
//this is called from loop if the int flag is set

void ATStepper::SetNextCount(void)
{
  digitalWrite(_StepPin, 0); //reset step pin  clears interrupt
  if (_nsteps > 0) {
    stepcount++;
    if (stepcount == _nsteps) {
      StopTimer();
      _ms = millis() - _ms;
      Serial.println("Run time: " + String(_ms));
      stopped = true;
    }
    if (stepcount == _DecelCount) {
      _amode = -1;
    }
  }
  if ( _amode == 1) {//accelerating
    if ( _astp ==  _nAccel) {
      _amode = 0;    //coast
    }
    else {
      //note order to prevent overflowing 15 bits
      OCRValue -= (OCRValue  / (4 * _astp + 1)) * 2 ;
    }
    _astp++;
  }
  if ( _amode == -1) {//decelerating. Stop handled on entry
    OCRValue +=  (OCRValue  / (4 *  _astp + 1)) * 2;
    _astp--;
    if (_astp < 0) {
      StopTimer();
      stopped = true;
    }
  }
  //NextCount  unchanged if amode = 0
}


//Step motor outside the interrupt
void ATStepper::StepMotor() {
  digitalWrite(_StepPin, !digitalRead(_StepPin));
  delayMicroseconds(100);
  digitalWrite(_StepPin, !digitalRead(_StepPin));
  _stepcount++;
}
