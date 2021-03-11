
#include "ATimers.h"
// 16 bit timers only
// only Timer1 and Timer3 currently included
// All instances of the class contain code for all target 16-bit timers
//  -#define UseTimer1 etc doesn't include the code at runtime

void ATimers::InitTimer(int timer)
{
  noInterrupts();
  _timer = timer;
  switch (_timer)
  {
#ifdef UseTimer1
    //Timer1 present in UNO and MEGA
    case 1:
      TCCR1A &= B11111100; //clear WGM1 bits 1,0
      TCCR1B = (TCCR1B & B11100000) | 0x1 << 3 ; // set WMG1 bits 3,2
      TCNT1 = 0;
      break;
#endif

#ifdef UseTimer3
    case 3:
      TCCR3A &= B11111100; //clear WGM1 bits 1,0
      TCCR3B = (TCCR3B & B11100000) | 0x1 << 3 ; // set WMG1 bits 3,2
      TCNT3 = 0;
      break;
#endif

#ifdef UseTimer4
    case 4:
      TCCR4A &= B11111100; //clear WGM1 bits 1,0
      TCCR4B = (TCCR4B & B11100000) | 0x1 << 3 ; // set WMG1 bits 3,2
      TCNT4 = 0;
      break;
#endif

#ifdef UseTimer5
    case 5:
      TCCR5A &= B11111100; //clear WGM1 bits 1,0
      TCCR5B = (TCCR5B & B11100000) | 0x1 << 3 ; // set WMG1 bits 3,2
      TCNT5 = 0;
      break;
#endif
  }
  interrupts();
}  //init



void ATimers::StopTimer()
{
  noInterrupts();
  switch (_timer)
  {
#ifdef UseTimer1
    case 1:
      //bitWrite(TIMSK1, OCIE1A, 0);
      TIMSK1 = (TIMSK1 & B11111000);    //disable timer interrupt
      break;
#endif

#ifdef UseTimer3
    case 3:
      TIMSK3 = (TIMSK3 & B11111000);   //disable timer interrupt
      break;
#endif
  }

  interrupts();
}

void ATimers::StartTimer( unsigned int count, int index)
{ 
  noInterrupts();
  switch (_timer)
  {
#ifdef UseTimer1
    case 1:

      TCCR1B = (TCCR1B & B11111000) | index; //_prescaleIndex;
      //bitWrite(TIMSK1, OCIE1A, 1);
      OCR1A = count;
      TIMSK1 = (TIMSK1 & B11111000) | 0x02;    //Start. CMPA interrupt enabled
      break;
#endif

#ifdef UseTimer3
      TCCR3B = (TCCR1B & B11111000) | index; //_prescaleIndex;
      //bitWrite(TIMSK1, OCIE1A, 1);
      OCR3A = count;
      TIMSK3 = (TIMSK3 & B11111000) | 0x02;    //Start. CMPA interrupt enabled
      break;
#endif
  }
  interrupts();
}

void ATimers::set_OCR( unsigned int count)
{
  noInterrupts();
  switch (_timer)
  {
 #ifdef UseTimer1
   case 1:

      OCR1A = count;
      break;
#endif

#ifdef UseTimer3
      OCR3A = count;
      break;
#endif

      interrupts();
  }

}
