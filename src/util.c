#include <inttypes.h>

#include "constants.h"
#include "util.h"

int msec_to_ticks(int msec)
{
  return (uint32_t)msec * SYSTICKS_PER_SEC / 1000;
}
