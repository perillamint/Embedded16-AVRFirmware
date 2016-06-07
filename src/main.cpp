#include <inttypes.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//AtomOS headers
#include <atom.h>
#include <atomtimer.h>

//iPot headers
#include "system_tick.h"
#include "constants.h"
#include "uart.h"
#include "i2c.h"
#include "util.h"

#include "sensordrv.hpp"
#include "spidrv.hpp"

#include "dumpcode.h"

/* Local data */

/* Application threads' TCBs */
static ATOM_TCB main_tcb;

/* Main thread's stack area */
static uint8_t main_thread_stack[MAIN_STACK_SIZE_BYTES];

/* Idle thread's stack area */
static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];

/* Forward declarations */
static void main_thread_func (uint32_t data);

int main()
{
  //Initialize UART.
  if (uart_init(115200) != 0)
    {
      //Uh-oh.
      //TODO: Blink ERROR LED.
    }

  //Initialize I2C. SCK freq = 400kHz.
  i2c_init(400000);

  //Initialize RTOS.
  int8_t status;
  
  /**
   * Reuse part of the idle thread's stack for the stack required
   * during this startup function.
   */
  SP = (int)&idle_thread_stack[(IDLE_STACK_SIZE_BYTES/2) - 1];

  //Initialize atomOS before creating our threads.
  status = atomOSInit(&idle_thread_stack[0], IDLE_STACK_SIZE_BYTES, FALSE);

  if(ATOM_OK == status)
    {
      // Enable system timer.
      init_system_tick();

      status = atomThreadCreate(&main_tcb, DEFAULT_THREAD_PRIO,
                                main_thread_func, 0,
                                &main_thread_stack[0],
                                MAIN_STACK_SIZE_BYTES, TRUE);

      if(ATOM_OK == status)
        {
          atomOSStart();
        }
    }
  
  
  return 0;
}

static void main_thread_func(uint32_t data)
{
  printf_P(PSTR("Thread TEST!\n"));

  int ret = -1;

  Sensordrv sensordrv;
  SPIdrv spidrv;

  ret = sensordrv.init();
  ret = sensordrv.start_thread();
  printf_P(PSTR("thread ret = %d\n"), ret);

  ret = spidrv.init();
  ret = spidrv.start_thread();
  printf_P(PSTR("thread ret = %d\n"), ret);

  for(;;)
    {
      atom_delay_ms(1000);
    }
}
