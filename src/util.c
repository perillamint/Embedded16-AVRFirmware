#include <inttypes.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"

int msec_to_ticks(int msec)
{
  return (uint32_t)msec * SYSTICKS_PER_SEC / 1000;
}

void atom_delay_ms(int msec)
{
  atomTimerDelay(msec_to_ticks(msec));
}
