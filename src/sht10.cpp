#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <util/delay.h>

#include "util.h"
#include "errno.h"
#include "sht10.hpp"

#include "dumpcode.h"

SHT10::SHT10()
{
  //
}

SHT10::~SHT10()
{
  //
}

int SHT10::shift_out(bool msbfirst, uint8_t data)
{
  int ret = 0;

  for(int i = 0; i < 8; i++)
    {
      //Set data.
      if((msbfirst ? data & 0x80 : data & 0x01))
        {
          //Out 1
          SHT10_PORT |= _BV(SHT10_DATA);
        }
      else
        {
          //Out 0
          SHT10_PORT &= ~(_BV(SHT10_DATA));
        }

      _delay_us(SHT10_PULSE_DELAY_US);
      SHT10_PORT |= _BV(SHT10_CLK);
      _delay_us(SHT10_PULSE_DELAY_US);
      SHT10_PORT &= ~(_BV(SHT10_CLK));
      _delay_us(SHT10_PULSE_DELAY_US);

      SHT10_PORT &= ~(_BV(SHT10_DATA));
      _delay_us(SHT10_PULSE_DELAY_US);

      if(msbfirst)
        {
          data <<= 1;
        }
      else
        {
          data >>= 1;
        }
    }

  SHT10_PORT |= _BV(SHT10_CLK);
  SHT10_PORT &= ~(_BV(SHT10_DATA));
  _delay_us(SHT10_PULSE_DELAY_US);
  data_mode(false);
  if(SHT10_PIN & _BV(SHT10_DATA))
    {
      //NACK
      ret = -EIO;
    }
  else
    {
      //ACK
      ret = 0;
    }
  data_mode(true);
  SHT10_PORT &= ~(_BV(SHT10_CLK));
  _delay_us(SHT10_PULSE_DELAY_US);

  return ret;
}

void SHT10::shift_in(bool msbfirst, uint8_t *data, bool ack)
{
  *data = 0x00;

  data_mode(false);

  for(int i = 0; i < 8; i++)
    {
      SHT10_PORT |= _BV(SHT10_CLK);
      _delay_us(SHT10_PULSE_DELAY_US);

      //Read data.
      if(SHT10_PIN & _BV(SHT10_DATA))
        {
          //Logic 1.
          *data |= (msbfirst ? 0x01 : 0x80);
        }
      else
        {
          //Logic 0.
          *data |= 0x00;
        }

      //Shift it.
      if(i != 7)
        {
          if(msbfirst)
            {
              *data <<= 1;
            }
          else
            {
              *data >>= 1;
            }
        }

      SHT10_PORT &= ~(_BV(SHT10_CLK));
      _delay_us(SHT10_PULSE_DELAY_US);
    }

  //Send (N)ACK.
  data_mode(true);
  if(ack)
    {
      SHT10_PORT &= ~(_BV(SHT10_DATA));
    }
  else
    {
      SHT10_PORT |= _BV(SHT10_DATA);
    }

  SHT10_PORT |= _BV(SHT10_CLK);
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT &= ~(_BV(SHT10_CLK));
  _delay_us(SHT10_PULSE_DELAY_US);
  data_mode(false);
}

void SHT10::data_mode(bool output)
{
  if(!output)
    {
      SHT10_DDR &= ~(_BV(SHT10_DATA));
    }
  else
    {
      SHT10_DDR |= _BV(SHT10_DATA);
    }
}

void SHT10::wait_for_data_ready()
{
  data_mode(false);
  atom_delay_ms(10);
  while(SHT10_PIN & _BV(SHT10_DATA))
    {
      atom_delay_ms(10);
    }
}

int SHT10::start(uint8_t sla, bool w)
{
  uint8_t data = (sla << 1) | (w ? 0x00 : 0x01);

  data_mode(true);

  SHT10_PORT &= ~(_BV(SHT10_CLK));
  SHT10_PORT &= ~(_BV(SHT10_DATA));
  _delay_us(SHT10_PULSE_DELAY_US);

  //Start sequence.
  SHT10_PORT |= _BV(SHT10_DATA);
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT |= _BV(SHT10_CLK);
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT &= ~(_BV(SHT10_DATA));
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT &= ~(_BV(SHT10_CLK));
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT |= _BV(SHT10_CLK);
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT |= _BV(SHT10_DATA);
  _delay_us(SHT10_PULSE_DELAY_US);
  SHT10_PORT &= ~(_BV(SHT10_CLK));
  _delay_us(SHT10_PULSE_DELAY_US);

  //Send data.
  shift_out(true, data);
  
  return 0;
}

int SHT10::read(void *data, int len)
{
  uint8_t *dat = (uint8_t*)data;

  for(int i = 0; i < len; i++)
    {
      shift_in(true, &dat[i], true);
    }

  return 0;
}

int SHT10::write(void *data, int len)
{
  return 0;
}

int SHT10::init()
{
  //Clock output mode.
  SHT10_DDR |= _BV(SHT10_CLK);
  data_mode(true);

  SHT10_PORT &= ~(_BV(SHT10_CLK));
  SHT10_PORT &= ~(_BV(SHT10_DATA));

  start(0x0F, true);
  atom_delay_ms(20);
  return 0;
}

int SHT10::get_raw_data(sht10_type_t type, uint16_t *raw_data)
{
  uint8_t sla = 0x00;
  uint8_t buf[3];
  int ret;

  switch(type)
    {
    case TEMP:
      sla = 0x01;
      break;
    case HUMID:
      sla = 0x02;
      break;
    default:
      return -EINVAL;
    }

  start(sla, false);
  wait_for_data_ready();
  ret = read(buf, 3);

  if(ret < 0)
    {
      return ret;
    }

  //TODO: Check parity.

  *raw_data = 0x0000;
  *raw_data |= buf[0];
  *raw_data <<= 8;
  *raw_data |= buf[1];

  return 0;
}

int SHT10::get_calculated_data(sht10_type_t type, int16_t *data)
{
  static const double c1 = -4.0;
  static const double c2 = 0.0405;
  static const double c3 = -0.0000028;
  static const double d1 = -39.6;
  static const double d2 = 0.01;

  uint16_t buf;
  double calcbuf = 0.0;

  get_raw_data(type, &buf);

  switch(type)
    {
    case TEMP:
      calcbuf += d1;
      calcbuf += d2 * ((double)buf);
      calcbuf *= 10;
      if(calcbuf > 1000)
        {
          calcbuf = 1000;
        }

      *data = (int16_t)calcbuf;
      break;
    case HUMID:
      calcbuf += c1;
      calcbuf += c2 * ((double)buf);
      calcbuf += c3 * (((double)buf) * ((double)buf));
      calcbuf *= 10;
      *data = (int16_t)calcbuf;
      break;
    default:
      return -EINVAL;
    }

  return 0;
}
