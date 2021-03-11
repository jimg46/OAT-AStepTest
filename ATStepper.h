/* ========================================================================== */
/*  ATStepper.h
  Step TMC 2208/9 stepper with acceleration
  TMCStepper used to drive the motor under software control (UART mode)
  Uses AVR 16 bit timers in conjunction with ATimers.h
  ISR routine steps motor and loads next count
  Step pin state used as a flag for calculation of next step
  Substepping managed elsewhere
  /*                                                                            */
/* ========================================================================== */
#ifndef ATStepper_h
#define ATStepper_h

#include <Arduino.h>
#include "ATimers.h"

class ATStepper : public ATimers //subclass inheriting public methods from ATimers
{
  private:
    int _timer;
    int _amode;
    int _astp;
    int  _nAccel;
    long _nsteps;
    long _DecelCount;
    int _prescale[6] = {0, 1, 8, 64, 256, 1024};
    int _pindex;
    int _StepPin;
    int _DirPin;
    uint32_t _ms;
  public:
    //Handles stepping and direction only
    ATStepper(int Timer, int StepPin, int DirPin); //ED, BED,TMC trad.
    //set nsteps 0 to run continuously
    void StepMotor();
    void Move( float v, int accel, long nsteps, int dir);
    void SetNextCount();
    void StopMotor();
    long stepcount;    // count is for current substep setting
    //int substep;
    volatile unsigned int OCRValue;
    volatile int stopped;

};
#endif
