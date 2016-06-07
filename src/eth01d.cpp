#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>

#include "i2c.h"
#include "util.h"
#include "errno.h"
#include "eth01d.hpp"

#include "dumpcode.h"

ETH01D::ETH01D(uint8_t addr)
{
  this -> addr = addr;
}

ETH01D::~ETH01D()
{
  //
}

int ETH01D::get_raw_data(uint16_t *humid, uint16_t *temp)
{
  int ret;
  uint8_t read[4];

  ret = i2c_start(0x44, true);
  if(ret < 0)
    {
      return ret;
    }

  //Wait until sensor collects data.
  atom_delay_ms(40);
  ret = i2c_start(0x44, false);
  if(ret < 0)
    {
      return ret;
    }

  ret = i2c_read(read, 4);
  if(ret < 0)
    {
      return ret;
    }

  i2c_stop();

  *humid = 0;
  *humid |= read[0];
  *humid &= 0x3F;
  *humid <<= 8;
  *humid |= read[1];

  *temp = 0;
  *temp |= read[2];
  *temp <<= 8;
  *temp |= read[3];
  *temp |= 0xF6;

  dumpcode(read, 4);
  dumpcode((unsigned char*)humid, 2);
  dumpcode((unsigned char*)temp, 2);

  return 0;
}

int ETH01D::get_calculated_data(int16_t *humid, int16_t *temp)
{
  int ret;
  uint16_t humid_raw, temp_raw;
  int32_t tmp;

  ret = this -> get_raw_data(&humid_raw, &temp_raw);
  if(ret < 0)
    {
      return ret;
    }

  tmp = humid_raw;
  tmp *= 1000;     //100 * 10
  tmp /= 0x3FFF;   //(2^14 - 1)

  *humid = (uint16_t) tmp;

  tmp = temp_raw;
  printf_P(PSTR("raw tmp = %lu\n"), tmp);
  tmp *= 1650;     //165 * 10
  tmp /= 4;        //See ETH01D datasheet.
  tmp /= 0x3FFF;   //(2^14 - 1)
  tmp -= 400;      //40 * 10

  *temp = (uint16_t) tmp;

  return 0;
}
