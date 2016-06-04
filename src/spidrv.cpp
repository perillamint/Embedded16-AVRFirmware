#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "errno.h"

#include "spidrv.hpp"

static bool is_running = false;
static uint8_t thread_stack[SPIDRV_STACK_SIZE_BYTES];

SPIdrv::SPIdrv()
{
  //
}

SPIdrv::~SPIdrv()
{
  //
}

void SPIdrv::thread_func(uint32_t data)
{
  //TODO: SPI stuff
  printf_P(PSTR("TODO: Put SPI stuff here.\n"));
  for(;;)
    {
      atom_delay_ms(1000);
    }
}

int SPIdrv::init()
{
  return 0;
}

int SPIdrv::start_thread()
{
  uint8_t status;

  if(is_running)
    {
      return -EBUSY;
    }

  status = atomThreadCreate(&(this -> tcb), DEFAULT_THREAD_PRIO + 1,
                            this -> thread_func, 0,
                            thread_stack,
                            SPIDRV_STACK_SIZE_BYTES, TRUE);

  if(ATOM_OK == status)
    {
      is_running = true;
      return 0;
    }
  else
    {
      return atom_err_to_errno(status);
    }
}
