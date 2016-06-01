/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART implementation
 *
 * $Id: uart.c,v 1.1 2005/12/28 21:38:59 joerg_wunsch Exp $
 *
 * This source code is came from atomthread AVR port.
 */

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include <atom.h>
#include <atommutex.h>

#include "uart.h"

/*
 * Set register-access macros for the first UART across various Atmega devices.
 */
#ifdef UCSRA
/* Device with single UART */
#define REG_UCSRA	UCSRA
#define REG_UCSRB	UCSRB
#define REG_UCSRC	UCSRC
#define REG_UBRRH	UBRRH
#define REG_UBRRL	UBRRL
#define REG_UDR		UDR
#define BIT_TXEN    TXEN
#define BIT_RXEN    RXEN
#define BIT_UDRE    UDRE
#else
/* Device with multiple UARTs */
#define REG_UCSRA	UCSR0A
#define REG_UCSRB	UCSR0B
#define REG_UCSRC	UCSR0C
#define REG_UBRRH	UBRR0H
#define REG_UBRRL	UBRR0L
#define REG_UDR		UDR0
#define BIT_TXEN    TXEN0
#define BIT_RXEN    RXEN0
#define BIT_UDRE    UDRE0
#endif


/* Local data */

/*
 * Semaphore for single-threaded access to UART device
 */
static ATOM_MUTEX uart_mutex;

//Std(in|out)
static FILE uart_stdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/*
 * Initialize the UART to requested baudrate, tx/rx, 8N1.
 */
int uart_init(uint32_t baudrate)
{
  int status;
  
  /* Set up the UART device with the selected baudrate */
  REG_UBRRH = 0x00;
#if F_CPU < 2000000UL && defined(U2X)
  REG_UCSRA = _BV(U2X);             /* improve baud rate error by using 2x clk */
  REG_UBRRL = (F_CPU / (8UL * baudrate)) - 1;
#else
  REG_UBRRL = (F_CPU / (16UL * baudrate));
#endif
  REG_UCSRB = _BV(BIT_TXEN) | _BV(BIT_RXEN); /* tx/rx enable */
  REG_UCSRC = 0x06; // 8N1

  DDRD = 0xFE;

  /* Create a mutex for single-threaded putchar() access */
  if (atomMutexCreate (&uart_mutex) != ATOM_OK)
    {
      status = -1;
    }
  else
    {
      status = 0;
    }

  stdin = &uart_stdin;
  stdout = &uart_stdout;

  /* Finished */
  return (status);
}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int uart_putchar(char c, FILE *stream)
{

  /* Block on private access to the UART */
  if (atomMutexGet(&uart_mutex, 0) == ATOM_OK)
    {
      /* Convert \n to \r\n */
      if (c == '\n')
        uart_putchar('\r', stream);

      /* Wait until the UART is ready then send the character out */
      loop_until_bit_is_set(REG_UCSRA, BIT_UDRE);
      REG_UDR = c;

      /* Return mutex access */
      atomMutexPut(&uart_mutex);
    }

  return 0;
}

//TODO: Think better way to do I/O.
//I/O thread?
int uart_getchar(FILE *stream)
{
  char ret = 0;

  if (atomMutexGet(&uart_mutex, 0) == ATOM_OK)
    {
      loop_until_bit_is_set(REG_UCSRA, RXC0);
      ret = REG_UDR;

      atomMutexPut(&uart_mutex);
    }

  return ret;
}
