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
#include "util.h"

/* Local data */

/* Application threads' TCBs */
static ATOM_TCB main_tcb;

/* Main thread's stack area */
static uint8_t main_thread_stack[MAIN_STACK_SIZE_BYTES];

/* Idle thread's stack area */
static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];

/* STDIO stream */
//static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


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
  //Hello, loop!

  DDRB = 0xFF;
  PORTB = 0x00;

  int cnt = 0;
  for(;;)
    {
      cnt++;
      PORTB ^= 0xFF;
      printf_P(PSTR("Hello, thread! count = %d\n"), cnt);
      atomTimerDelay(msec_to_ticks(500));
    }
}
