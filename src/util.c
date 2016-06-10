#include <inttypes.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"

int msec_to_ticks(int msec)
{
  int32_t ret = msec;
  ret *= SYSTICKS_PER_SEC;
  ret /= 1000;

  return ret;
}

void atom_delay_ms(int msec)
{
  atomTimerDelay(msec_to_ticks(msec));
}

int atom_err_to_errno(uint8_t status)
{
  //TODO: impl this.
  return status;
}
