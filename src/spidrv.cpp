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
static spimemdata_t spi_mem[MAX_SPIMEM_SIZE];

//Buffers for ISR.
static uint8_t spi_rx_buf[4];
static uint8_t spi_tx_buf[4];
static bool spi_tx_flag, spi_rx_flag;

/**
 * Brief SPI protocol description:
 * Packet MUST NOT start with null.
 * Packet MAY contain null in its data and 4-byte fixed length.
 * Transmit 0x00 when idling.
 */

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
  for(;;)
    {
      while(!spi_rx_flag) //Delay until we get spi flag.
        {
          atom_delay_ms(10);
        }

      printf_P(PSTR("SPI RX done!, buf = 0x%X 0x%X 0x%X 0x%X\n"),
               spi_rx_buf[0], spi_rx_buf[1], spi_rx_buf[2], spi_rx_buf[3]);

      spi_rx_flag = false;

      //TODO: Set TX flag.
      spi_tx_buf[0] = 'A';
      spi_tx_buf[1] = 'B';
      spi_tx_buf[2] = 'C';
      spi_tx_buf[3] = 'D';

      spi_tx_flag = true;

      while(spi_tx_flag)
        {
          atom_delay_ms(10);
        }

      printf_P(PSTR("SPI TX done!\n"));
    }
}

int SPIdrv::init()
{
  DDRB |= _BV(6);
  SPCR = _BV(SPE) | _BV(SPIE);
  SPDR = 0x00;
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

spimemdata_t *SPIdrv::get_spi_mem()
{
  return spi_mem;
}

int SPIdrv::write_memory(spimmap_t addr, uint16_t value)
{
  if(addr >= MAX_SPIMEM_SIZE)
    {
      return -EFAULT;
    }

  spi_mem[addr].uint16 = value;
  return 0;
}

int SPIdrv::write_memory(spimmap_t addr, int16_t value)
{
  if(addr >= MAX_SPIMEM_SIZE)
    {
      return -EFAULT;
    }

  spi_mem[addr].int16 = value;
  return 0;
}

int SPIdrv::read_memory(spimmap_t addr, uint16_t *value)
{
  if(addr >= MAX_SPIMEM_SIZE)
    {
      return -EFAULT;
    }

  *value = spi_mem[addr].uint16;
  return 0;
}

int SPIdrv::read_memory(spimmap_t addr, int16_t *value)
{
  if(addr >= MAX_SPIMEM_SIZE)
    {
      return -EFAULT;
    }

  *value = spi_mem[addr].int16;
  return 0;
}

ISR (SPI_STC_vect)
{
  static int idx = 0;
  static int status = 0; // 0 = rx, 1 = tx.
  char buf;

  switch(status)
    {
    case 0: //RX
      buf = SPDR;
      SPDR = 0x00;

      if(0x00 != buf)
        {
          spi_rx_buf[idx] = buf;
          idx ++;
        }

      break;
    case 1:
      if(spi_tx_flag)
        {
          SPDR = spi_tx_buf[idx];
          idx ++;
        }
      else
        {
          SPDR = 0x00; //Idle until txflag set.
        }
      break;
    default:
      printf_P(PSTR("Uh-oh, it shouldn't be here! DEBUG UR SPI_STC_vect.\n"));
      break;
    }

  if(idx >= 4)
    {
      switch(status)
        {
        case 0:
          spi_rx_flag = true;
          break;
        case 1:
          spi_tx_flag = false;
          break;
        default:
          printf_P(PSTR("Uh-oh, it shouldn't be here! DEBUG UR SPI_STC_vect.\n"));
          break;
        }

      status = (status + 1) % 2;
      idx = 0;
    }
}
