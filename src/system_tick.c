#include <avr/interrupt.h>
#include <atom.h>
#include <atomport.h>

#include "system_tick.h"
#include "constants.h"

void init_system_tick(void)
{
  //Initialize Timer 0 for scheduler tick.
  OCR0A = (F_CPU / 1024 / SYSTICKS_PER_SEC);

  //Enable compare-match interrupt.
#ifdef TIMSK
  TIMSK = _BV(OCIE0A);
#else
  TIMSK0 = _BV(OCIE0A);
#endif

  //Set prescaler: 1024
  TCCR0B = _BV(CS00) | _BV(CS02) | _BV(WGM02);
}

//Atom scheduler ISR.
ISR (TIMER0_COMPA_vect)
{
  atomIntEnter();
  atomTimerTick();
  atomIntExit(TRUE);
}

ISR (BADISR_vect)
{
  //Do nothing.
}