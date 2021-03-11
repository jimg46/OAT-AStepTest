/*  AStepTest
 *   Test OAT steppers, including StallGuard with limit stops
  test  accel code and variable sharing
  This version for TMC2209 and AVR 16 bit timers
  
*/
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "ATimers.h"
#include "ATStepper.h"
#include "ATimerDefs.h"


int LED = 13;         //on-board LED for testing
int timer = 1;
int SubStep = 4;
int StallThr = 0;
int Current = 300;
float vMax = 500;//s20;    //steps/sec   20/10 means na=20
float accel = 500;//10;  //steps/sec^2
long nsteps = 0;
bool mrunning = false;
bool ShowSGresult = false;
int dirn = fwd;
volatile int stall = LOW;  //will be used in loop for immediate stop

/*
  driver setup code from OAT
*/

#include <TMCStepper.h>

//TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driver(SW_RX, SW_TX, R_SENSE, DRIVER_ADDRESS);

ATStepper stepper(timer, STEP_PIN, DIR_PIN);

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(DIAG_PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);   //used as indicator
  digitalWrite(DIR_PIN, fwd);
  digitalWrite(EN_PIN, LOW);      // Enable motor power
  digitalWrite(LED, LOW);  //off

  attachInterrupt(0, ShowStall, RISING); //pin2 = int0
 
  Serial.begin(115200);
  // Enable one according to your setup
  //SERIAL_PORT.begin(115200);      // HW UART drivers
  driver.beginSerial(19200);     // SW UART drivers

  driver.begin();
  driver.toff(4);
  driver.blank_time(24);
  driver.rms_current(Current);
  driver.microsteps(SubStep);   
  //driver.fclktrim(4);
  driver.TCOOLTHRS(0xFFFFF);  //0xFFFFF);  //sets min velocity for diag output
  //    //driver.ihold(1); // its safe to assume that the only time RA stands still is during parking and the current can be limited to a minimum
  //    driver.semin(2);  
  //    driver.semax(5);
  //    driver.sedn(0b01);
  driver.SGTHRS(StallThr); //stallguard threshold 

  stepper.InitTimer(timer);  //store timer# 

  Serial.println(F(" TMC2209 diagnostics."));
  Serial.println(F(" Commands are:"));
  Serial.println(F("  V n - set max velocity to n steps/sec."));
  Serial.println(F("  A n - set acceleration steps/sec^2."));
  Serial.println(F("  M n - set microsteps {0,4,16,64,256}. "));
  Serial.println(F("  I n - set current."));
  Serial.println(F("  S - toggle start/stop."));
  Serial.println(F("  F n - set fwd, nsteps (optional, retained)."));
  Serial.println(F("  R n - set reverse, nsteps (ditto)."));
  Serial.println(F("    nsteps set to 0 (continuous running) at reset."));
  Serial.println(F("  G - Toggle show SGResult when stepping"));
  Serial.println(F("  T n - set stall threshold."));
  Serial.println(F("  P - show current settings."));
  Serial.println(F("  X - Cancel LED stall detection."));
  Serial.println();
  Pcmd();  //show default settings
}

#ifdef UseTimer1
ISR(TIMER1_COMPA_vect) {
  digitalWrite(STEP_PIN, 1);  //flag int by stepping motor
  OCR1A = stepper.OCRValue;
}
#endif

void ShowStall() {
  stall = HIGH;
  digitalWrite(LED,HIGH);
}


void loop() {
  static uint32_t last_time = 0;  //timer to read SG value
  uint32_t ms = millis();
  if (!stepper.stopped && (digitalRead(STEP_PIN) == 1)) {
    stepper.SetNextCount();
  }
  if (mrunning && stepper.stopped) {
    mrunning = false;
    Serial.println("Stopped.");
  }

  char inByte;
  int n;
  if (Serial.available() > 0)
  { inByte = Serial.read()  & ~(0x20);
    switch (inByte)
    {
      case 'M':
        n = Serial.parseInt();
        Serial.println("Microsteps set to " + String(n));
        driver.microsteps(n);   // no checking
        break;
      case 'I':
        n = Serial.parseInt();
        Serial.println("Current set to " + String(n));
        driver.rms_current(n);
        break;
      case 'V':   // set max velocity
        vMax = Serial.parseInt();
        Serial.println("vMax set to " + String(vMax));
        break;
      case 'A':   // set accel
        accel = (float) Serial.parseInt();
        Serial.println("Accel set to " + String(accel));
        break;
      case 'S':  //Start/stop
        if (mrunning) {
          stepper.StopMotor(); //will decel if required
                               //takes vMax/( 2* (Vmax/accel) ) steps to stop
          Serial.println("Stopping ..");
          // separate check in loop() for motor stopped
        }
        else {
          Serial.println("Starting ..");
          stepper.Move( vMax, accel, nsteps, dirn);
          mrunning = true;
        }
        break;

      case 'F': //set n_steps fwd
        n = Serial.parseInt(); //check if available!
        //if (n != 0) {
        nsteps = n;
        dirn = fwd;
        Serial.println("fwd, nsteps set to " + String(nsteps));
        digitalWrite(DIR_PIN, fwd);
        //}
        break;

      case 'R': //set n_steps reverse
        n = Serial.parseInt();
        //if (n != 0) {
        nsteps = n;
        dirn = !fwd;
        Serial.println("Reverse, nsteps set to " + String(nsteps));
        digitalWrite(DIR_PIN, !fwd);
        //}
        break;

      case 'G':  //toggle show SGresult when stepping
        ShowSGresult = !ShowSGresult;
        Serial.print("ShowSGresult ");
        if (ShowSGresult) {
          Serial.println( "enabled");  
        }
        else {
          Serial.println( "disabled");  
        }
        break;

      case 'T': //Set stall threshold
        StallThr = Serial.parseInt();
        Serial.println("Stall threshold set to " + String(StallThr));
        driver.SGTHRS(StallThr);
        break;
      
      case 'P':  //show params
        Pcmd();
        break;
        
      case 'X':  //turn LED off
        digitalWrite(LED,LOW);
        break;
    }
    Serial.flush();

    delay(100);
  
  }// end command loop
  
  if (ShowSGresult && mrunning && (ms - last_time) > 100) { //run every 0.1s
    last_time = ms;
    Serial.print("SGR ");
    Serial.println(driver.SG_RESULT(), DEC);
  }
} // end loop

void Pcmd() { //show current parameters
  Serial.print("Microsteps: "); Serial.print(driver.microsteps());
  Serial.print(", Current: "); Serial.println(driver.rms_current());
  Serial.print("Vmax: "); Serial.print(vMax);
  Serial.print(", Accel: "); Serial.println(accel);
  Serial.print("Stall threshold: "); Serial.println(StallThr);
  
}
