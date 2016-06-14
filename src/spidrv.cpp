#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "errno.h"

#include "spidrv.hpp"

#include "dumpcode.h"

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
  spi_packet_t spi_rx_packet, spi_tx_packet;
  bool parity;

  for(;;)
    {
      while(!spi_rx_flag) //Delay until we get spi flag.
        {
          atom_delay_ms(10);
        }

      memcpy(&spi_rx_packet, spi_rx_buf, sizeof(spi_packet_t));

      spi_rx_flag = false;

      dumpcode(&spi_rx_packet, sizeof(spi_packet_t));

      printf_P(PSTR("SPI RX done!, version = %d, write = %d, reserved = %d, parity = %d\n"
                    "rid = 0x%X, did = 0x%X, data = 0x%X\n"),
               spi_rx_packet.version, spi_rx_packet.write, spi_rx_packet.reserved,
               spi_rx_packet.parity, spi_rx_packet.rid, spi_rx_packet.did, spi_rx_packet.data);

      parity = do_parity(spi_tx_buf, 4, true);

      if(!spi_rx_packet.parity) //Faulty!
        {
          printf_P(PSTR("Bad parity!\n"));
          spi_tx_packet.write = false;
          spi_tx_packet.data = EINVAL;
        }
      else //Okay.
        {
          //TODO: Query data from memory.
          spi_tx_packet.write = true; // OK flag here.
          spi_tx_packet.data = 42;
        }

      spi_tx_packet.version = 0;  // Must be zero.
      spi_tx_packet.reserved = 0; // Must be zero.
      spi_tx_packet.parity = false;
      spi_tx_packet.rid = spi_rx_packet.rid;
      spi_tx_packet.did = spi_rx_packet.did;

      parity = do_parity(&spi_tx_packet, sizeof(spi_packet_t), true);
      spi_tx_packet.parity = parity;

      memcpy(spi_tx_buf, &spi_tx_packet, sizeof(spi_packet_t));
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
