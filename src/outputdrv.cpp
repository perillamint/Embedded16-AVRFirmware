#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "errno.h"
#include "spidrv.hpp"
#include "usbpwr.hpp"
#include "l298.hpp"
#include "outputdrv.hpp"

#include "dumpcode.h"

static SPIdrv spidrv;
static USBPWR usbpwr;
static L298 l298;

static bool is_running = false;
static uint8_t thread_stack[OUTDRV_STACK_SIZE_BYTES];

Outputdrv::Outputdrv()
{
  //
}

Outputdrv::~Outputdrv()
{
  //
}

void Outputdrv::thread_func(uint32_t data)
{
  uint16_t pump_time[2] = {0, 0};
  int pump_smalltick[2] = {0, 0};
  bool is_running[2] = {false, false};
  static const spimmap_t pump_idx[2] = {PUMP1_TIME, PUMP2_TIME};

  for(;;)
    {
      //Get memory if we are not running.
      for(int i = 0; i < 2; i++)
        {
          if(!is_running[i])
            {
              //Not running. read it.
              spidrv.read_memory(pump_idx[i], &pump_time[i]);

              if(0 != (pump_time[i] & 0x8000))
                {
                  //Clear busy flag.
                  spidrv.write_memory(pump_idx[i], (uint16_t)0x0000);
                }
              else if(0 != pump_time[i])
                {
                  //Start motor.
                  l298.set_motor_speed(i, 255);
                  l298.set_motor_dir(i, MOT_FORWARD);
                  spidrv.write_memory(pump_idx[i], (uint16_t)0x8000);

                  is_running[i] = true;
                }
            }

          if(is_running[i] && 0 >= pump_time[i] && 0 >= pump_smalltick[i])
            {
              //Stop motor and get user data.
              l298.set_motor_speed(i, 0);
              l298.set_motor_dir(i, MOT_STOP);

              is_running[i] = false;
            }

          if(is_running[i])
            {
              //Working!
              if(0 < pump_time[i])
                {
                  pump_time[i] --;
                }
            }
        }

      atom_delay_ms(1000); //Sleep 1sec.
    }
}

int Outputdrv::init()
{
  usbpwr.init();
  usbpwr.set_pwr(0, true); //Turn on RasPi.

  l298.init(10);
  /*
  l298.set_motor_speed(0, 255);
  l298.set_motor_speed(1, 255);
  l298.set_motor_dir(0, MOT_FORWARD);
  l298.set_motor_dir(1, MOT_FORWARD);
  */
  return 0;
}

int Outputdrv::start_thread()
{
  uint8_t status;

  if(is_running)
    {
      return -EBUSY;
    }

  status = atomThreadCreate(&(this -> tcb), DEFAULT_THREAD_PRIO - 1,
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

int Outputdrv::set_light(bool light)
{
  return usbpwr.set_pwr(1, light);
}
