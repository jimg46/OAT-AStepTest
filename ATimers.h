/*
  Based on TimerInterrupt.h
  For Arduino boards (UNO, Nano, Mega, etc. )
  Khoi Hoang https://github.com/khoih-prog/TimerInterrupt
   (which is mainly to obtain long times)
  Mods:
  Interrupt routine(s) in main Arduino .ino file set a flag read and reset in loop()
   Miniumum delay in int routine.
 
  ATimers is inherited by (and hence instantiated with )the Stepper code which: 
    - loads the OCRA register with the next count
    - includes all the acceleration code
    - clears the interrupt flag
    - updates position.
*/
#ifndef ATimers_h
#define ATimers_h

#include <avr/interrupt.h>  //check which of these are still needed
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "pins_arduino.h"
#include "ATimerDefs.h"  //timers to use


// 16 bit timers only
// Timer 1 in UNO, Timers 1,3,4,5 in MEGA


class ATimers
{
  private:


  public:
    int _timer;
    int _stepPin;
    unsigned long _stepcount;
   //ATimers(int timer);
    unsigned int _NextStep;
    void InitTimer(int timer); //timer allocated in constructor
    void StopTimer();
    void StartTimer( unsigned int count, int prescale_index);
    void set_OCR( unsigned int count);

}; // end class def
#endif   //ATimers_h
