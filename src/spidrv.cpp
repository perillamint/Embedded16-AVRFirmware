
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
  int ret;

  for(;;)
    {
      while(!spi_rx_flag) //Delay until we get spi flag.
        {
          atom_delay_ms(10);
        }

      memcpy(&spi_rx_packet, spi_rx_buf, sizeof(spi_packet_t));
      spi_rx_flag = false;

      printf_P(PSTR("SPI packet received!, version = %d, write = %d, reserved = %d\n"
                    "parity = %d, rid = 0x%X, did = 0x%X, data = 0x%X\n"),
               spi_rx_packet.version, spi_rx_packet.write, spi_rx_packet.reserved,
               spi_rx_packet.parity, spi_rx_packet.rid, spi_rx_packet.did, spi_rx_packet.data);

      parity = do_parity(spi_rx_buf, 4, true);

      if(parity) //Faulty!
        {
          printf_P(PSTR("Bad parity!\n"));
          spi_tx_packet.write = 1;
          spi_tx_packet.data = EINVAL;
        }
      else //Okay.
        {
          spi_tx_packet.write = 0; // OK flag here.
          ret = do_command(&spi_rx_packet, &spi_tx_packet);

          if(ret < 0)
            {
              spi_tx_packet.write = 1;
              spi_tx_packet.data = -ret;
            }
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

int SPIdrv::do_command(spi_packet_t *rx_packet, spi_packet_t *tx_packet)
{
  spimmap_t idx;
  switch(rx_packet -> did)
    {
    case SYSTEM_ALIVE:
      if(0 == rx_packet -> write)
        {
          tx_packet -> data = 0x0000;
        }
      else
        {
          printf_P(PSTR("RESET!\n"));
          do_reset();
        }
      break;
    case AIR_SENSOR_AVAIL:
      if(0 == rx_packet -> write)
        {
          //Hardcode. two sensors available.
          tx_packet -> data = 0x0006;

          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case AIR_HUMIDITY:
      if(0 == rx_packet -> write)
        {
          tx_packet -> data = spi_mem[HUMID_AIR].uint16;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      return 0;
      break;
    case AIR_TEMPERATURE:
      if(0 == rx_packet -> write)
        {
          tx_packet -> data = spi_mem[THERMAL_AIR].uint16;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case SOIL_SENSOR_AVAIL:
      if(0 == rx_packet -> write)
        {
          //Hardcode. One SHT10 available.
          tx_packet -> data = 0x0003;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case SOIL_HUMIDITY:
      if(0 == rx_packet -> write)
        {
          tx_packet -> data = spi_mem[HUMID_SOIL].uint16;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case LIGHT_SENS_AVAIL:
      if(0 == rx_packet -> write)
        {
          //Hardcode. one sensor is available.
          //Uh-oh. we burnt our light sensor during dev.
          //Disable it now...
          tx_packet -> data = 0x0001;

          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case LIGHT_INTENSITY:
      if(0 == rx_packet -> write)
        {
          tx_packet -> data = spi_mem[LUMINOSITY].uint16;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case WATER_SENS_AVAIL:
      if(0 == rx_packet -> write)
        {
          //Hardcode. two sensors are available.
          tx_packet -> data = 0x0006;

          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case WATER_TANK_LEVEL:
    case SAUCER_TANK_LEVEL:
      if(0 == rx_packet -> write)
        {
          uint16_t value = spi_mem[WATER_LEVEL].uint16;

          if(WATER_TANK_LEVEL == rx_packet -> did)
            {
              value &= 0x0001;
            }
          else
            {
              value &= 0x0002;
              value >>= 1;
            }

          if(value)
            {
              value = 0xFFFF;
            }

          tx_packet -> data = value;
          return 0;
        }
      else
        {
          return -EPERM;
        }
      break;
    case SYSTEM_TICK:
      break;
    case WATER_SPRAY_MOTOR:
    case WATER_PUMP_MOTOR:
      if(WATER_SPRAY_MOTOR == rx_packet -> did)
        {
          idx = PUMP1_TIME;
        }
      else
        {
          idx = PUMP2_TIME;
        }

      if(1 == rx_packet -> write)
        {
          if(spi_mem[idx].uint16 & 0x8000)
            {
              return -EBUSY;
            }

          if(rx_packet -> data & 0x8000)
            {
              return -EINVAL;
            }

          spi_mem[idx].uint16 = rx_packet -> data;
          tx_packet -> data = 0x0000;
          return 0;
        }
      else
        {
          tx_packet -> data = spi_mem[idx].uint16;
          return 0;
        }
    case OTHER_MOTIVATOR_AVAIL:
      return 0;
      break;
    case LAMP_PRIMARY:
    case LAMP_SECONDARY:
      if(LAMP_PRIMARY == rx_packet -> did)
        {
          idx = LED1_PWR;
        }
      else
        {
          idx = LED2_PWR;
        }

      if(1 == rx_packet -> write)
        {
          spi_mem[idx].uint16 = rx_packet -> data;
          return 0;
        }
      else
        {
          tx_packet -> data = spi_mem[idx].uint16;
          return 0;
        }
      break;
    default:
      tx_packet -> data = 0x0000;
      return -EADDRNOTAVAIL;
    }
  return 0;
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
  static bool tx_start = false;
  char buf;

  switch(status)
    {
    case 0: //RX
      buf = SPDR;
      SPDR = 0x00;

      if(0x00 != buf || tx_start)
        {
          tx_start = true;
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
          tx_start = false;
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
