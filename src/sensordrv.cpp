#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "i2c.h"
#include "errno.h"
#include "tsl2561.hpp"
#include "eth01d.hpp"

#include "sensordrv.hpp"

static bool is_running = false;
static uint8_t thread_stack[SENSORDRV_STACK_SIZE_BYTES];
static TSL2561 tsl2561(0x39);
static ETH01D eth01d;

Sensordrv::Sensordrv()
{
}

Sensordrv::~Sensordrv()
{
}

void Sensordrv::thread_func(uint32_t data)
{
  int ret;
  uint32_t res;
  int16_t humid, temp;
  int cnt = 0;

  for(;;)
    {
      cnt ++;
      ret = tsl2561.get_luminosity(&res);
      printf_P(PSTR("tsl2561.get_luminosity() returned %d, res = %lu, cnt = %d\n"), ret, res, cnt);
      ret = eth01d.get_calculated_data(&humid, &temp);
      printf_P(PSTR("eth01d.get_calculated_data() returned %d, humid = %d, temp = %d\n"), ret, humid, temp);
      atom_delay_ms(500);
    }
}

int Sensordrv::init()
{
  int ret = -1;

  while (ret >= 0)
    {
      ret = tsl2561.init();
      printf_P(PSTR("tsl2561.init() returned %d\n"), ret);
      if(ret < 0)
        {
          printf_P(PSTR("TSL2561 init failure. try it again after 1msec.\n"));
          atom_delay_ms(1);
        }
    }

  ret = tsl2561.enable();
  printf_P(PSTR("tsl2561.enable() returned %d\n"), ret);

  //Initialize sensor args.
  ret = tsl2561.set_gain(TSL2561_GAIN_1X);
  if(ret < 0)
    {
      printf("ERR! no gain!\n");
    }

  ret = tsl2561.set_timing(TSL2561_INTEGRATIONTIME_13MS);
  if(ret < 0)
    {
      printf("ERR! no timing!\n");
    }
  return 0;
}

int Sensordrv::start_thread()
{
  uint8_t status;

  if(is_running)
    {
      return -EBUSY;
    }

  status = atomThreadCreate(&(this -> tcb), DEFAULT_THREAD_PRIO,
                            this -> thread_func, 0,
                            thread_stack,
                            SENSORDRV_STACK_SIZE_BYTES, TRUE);

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
