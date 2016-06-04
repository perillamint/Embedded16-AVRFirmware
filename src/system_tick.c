#include <avr/interrupt.h>
#include <atom.h>
#include <atomport.h>

#include "system_tick.h"
#include "constants.h"

void init_system_tick(void)
{
#ifndef __AVR_ATmega128__
  //Initialize Timer 0 for scheduler tick.
  OCR0A = (F_CPU / 1024 / SYSTICKS_PER_SEC);

  //Enable compare-match interrupt.
  TIMSK0 = _BV(OCIE0A);

  //Timer0 CTC mode, prescaler = 1024
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS00) | _BV(CS02);
#else
  //Initialize Timer 0 for scheduler tick.
  OCR0 = (F_CPU / 1024 / SYSTICKS_PER_SEC);

  //Enable compare-match interrupt.
  TIMSK = _BV(OCIE0);

  //Timer0 CTC mode, prescaler = 1024
  TCCR0 = _BV(WGM01) | _BV(CS00) | _BV(CS01) | _BV(CS02);
#endif

  sei();
}

//Atom scheduler ISR.
#ifndef __AVR_ATmega128__
ISR (TIMER0_COMPA_vect)
#else
ISR (TIMER0_COMP_vect)
#endif
{
  atomIntEnter();
  atomTimerTick();
  atomIntExit(TRUE);
}

ISR (BADISR_vect)
{
  //Do nothing.
}
