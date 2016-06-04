#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "i2c.h"
#include "util.h"
#include "errno.h"
#include "tsl2561.hpp"

TSL2561::TSL2561(uint8_t addr)
{
  this -> addr = addr;
  this -> initialized = false;
  this -> sleeping = false;
  this -> integration = TSL2561_INTEGRATIONTIME_13MS;
  this -> gain = TSL2561_GAIN_16X;
}

TSL2561::~TSL2561()
{
  //TODO: Destructor.
}

int TSL2561::write_reg(uint8_t reg, uint8_t data)
{
  int ret;
  char cmd[2] = {reg, data};

  ret = i2c_start(this -> addr, true);
  if(ret < 0)
    {
      return -EIO;
    }

  ret = i2c_write(cmd, 2);
  if(ret < 0)
    {
      i2c_stop();
      return -EIO;
    }

  i2c_stop();

  return 0;
}

int TSL2561::read_reg(uint8_t reg, uint16_t *data)
{
  int ret;
  uint8_t recv[2];

  ret = i2c_start(this -> addr, true);
  if(ret < 0)
    {
      return -EIO;
    }

  ret = i2c_putchar(reg);
  if(ret < 0)
    {
      i2c_stop();
      return -EIO;
    }

  i2c_stop();

  ret = i2c_start(this -> addr, false);
  if(ret < 0)
    {
      i2c_stop();
      return -EIO;
    }

  ret = i2c_read(recv, 2);
  if(ret < 0)
    {
      i2c_stop();
      return -EIO;
    }

  i2c_stop();

  *data = recv[1];
  *data = *data << 8;
  *data |= recv[0];

  return 0;
}

int TSL2561::init()
{
  int ret;

  ret = i2c_start(this -> addr, true);
  if(ret < 0) //There is error.
    {
      return -EIO;
    }

  ret = i2c_putchar(TSL2561_REG_ID);
  if(ret != 0)
    {
      i2c_stop();
      return -EIO; 
    }

  ret = i2c_start(this -> addr, false);
  if(ret < 0)
    {
      return -EIO;
    }

  char res;
  ret = i2c_read(&res, 1);
  if(ret < 0)
    {
      i2c_stop();
      return -EIO;
    }

  i2c_stop();

  if(res & 0x0A) //Found TSL2561
    {
      this -> initialized = true;
      return 0;
    }

  return -EAGAIN;
}

int TSL2561::enable()
{
  int ret;

  if(!(this -> initialized))
    {
      ret = this -> init();
      if(ret < 0)
        {
          return ret;
        }
    }

  ret = this -> write_reg(TSL2561_COMMAND_BIT | TSL2561_REG_CONTROL,
                          TSL2561_CTRL_POWERON);
  if(ret < 0)
    {
      return ret;
    }

  this -> sleeping = false;

  return 0;
}

int TSL2561::disable()
{
  int ret;

  if(!(this -> initialized))
    {
      ret = this -> init();
      if(ret < 0)
        {
          return ret;
        }
    }

  ret = this -> write_reg(TSL2561_COMMAND_BIT | TSL2561_REG_CONTROL,
                          TSL2561_CTRL_POWEROFF);
  if(ret < 0)
    {
      return ret;
    }

  this -> sleeping = true;

  return 0;
}

int TSL2561::set_gain(tsl2561Gain_t gain)
{
  int ret;

  if(this -> sleeping)
    {
      return -EAGAIN;
    }

  ret = this -> write_reg(TSL2561_COMMAND_BIT | TSL2561_REG_TIMING,
                          this -> integration | gain);
  if(ret < 0)
    {
      return ret;
    }

  this -> gain = gain;

  return 0;
}

int TSL2561::set_timing(tsl2561IntegrationTime_t integration)
{
  int ret;

  if(this -> sleeping)
    {
      return -EAGAIN;
    }

  ret = this -> write_reg(TSL2561_COMMAND_BIT | TSL2561_REG_TIMING,
                          integration | this -> gain);
  if(ret < 0)
    {
      return ret;
    }

  this -> integration = integration;

  return 0;
}

int TSL2561::get_raw_full_luminosity(uint32_t *result)
{
  int ret;

  if(this -> sleeping)
    {
      return -EAGAIN;
    }

  switch(this -> integration)
    {
    case TSL2561_INTEGRATIONTIME_13MS:
      atom_delay_ms(14);
      break;
    case TSL2561_INTEGRATIONTIME_101MS:
      atom_delay_ms(102);
      break;
    default:
      atom_delay_ms(403);
      break;
    }

  uint32_t luminosity;
  uint16_t buf;
  ret = this -> read_reg(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REG_CH1_LOW,
                         &buf);
  if(ret < 0)
    {
      return ret;
    }

  luminosity = buf;
  luminosity <<= 16;
  ret = this -> read_reg(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REG_CH0_LOW,
                         &buf);
  if(ret < 0)
    {
      return ret;
    }

  luminosity |= buf;

  *result = luminosity;

  return 0;
}

int TSL2561::get_raw_luminosity(uint8_t channel, uint16_t *result)
{
  int ret;
  uint32_t full_luminosity;

  if(channel != 0 && channel != 1)
    {
      return -EINVAL;
    }

  ret = this -> get_raw_full_luminosity(&full_luminosity);
  if(ret < 0)
    {
      return ret;
    }

  switch(channel)
    {
    case 0:
      *result = (full_luminosity & 0xFFFF);
      break;
    case 1:
      *result = (full_luminosity >> 16);
      break;
    default:
      return -EINVAL;
    }

  return 0;
}

int TSL2561::get_luminosity(uint32_t *result)
{
  int ret;
  unsigned long ch_scale;
  unsigned long ch0;
  unsigned long ch1;

  switch(this -> integration)
    {
    case TSL2561_INTEGRATIONTIME_13MS:
      ch_scale = TSL2561_LUX_CHSCALE_TINT0;
      break;
    case TSL2561_INTEGRATIONTIME_101MS:
      ch_scale = TSL2561_LUX_CHSCALE_TINT1;
      break;
    default:
      ch_scale = (1 << TSL2561_LUX_CHSCALE);
      break;
    }

  switch(this -> gain)
    {
    case TSL2561_GAIN_16X:
      break;
    default:
      ch_scale = ch_scale << 4;
      break;
    }

  //Fetch ch0 and ch1.
  uint32_t raw_luminosity;

  ret = get_raw_full_luminosity(&raw_luminosity);
  if(ret < 0)
    {
      return ret;
    }

  ch0 = raw_luminosity & 0xFFFF;
  ch1 = raw_luminosity >> 16;

  ch0 = (ch0 * ch_scale) >> TSL2561_LUX_CHSCALE;
  ch1 = (ch1 * ch_scale) >> TSL2561_LUX_CHSCALE;

  unsigned long ratio, ratio1 = 0;
  if(ch0 != 0)
    {
      ratio1 = (ch1 << (TSL2561_LUX_RATIOSCALE + 1)) / ch0;
    }

  ratio = (ratio1 + 1) >> 1;

  unsigned int b, m;

#ifdef TSL2561_PACKAGE_CS
  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1C))
    {b=TSL2561_LUX_B1C; m=TSL2561_LUX_M1C;}
  else if (ratio <= TSL2561_LUX_K2C)
    {b=TSL2561_LUX_B2C; m=TSL2561_LUX_M2C;}
  else if (ratio <= TSL2561_LUX_K3C)
    {b=TSL2561_LUX_B3C; m=TSL2561_LUX_M3C;}
  else if (ratio <= TSL2561_LUX_K4C)
    {b=TSL2561_LUX_B4C; m=TSL2561_LUX_M4C;}
  else if (ratio <= TSL2561_LUX_K5C)
    {b=TSL2561_LUX_B5C; m=TSL2561_LUX_M5C;}
  else if (ratio <= TSL2561_LUX_K6C)
    {b=TSL2561_LUX_B6C; m=TSL2561_LUX_M6C;}
  else if (ratio <= TSL2561_LUX_K7C)
    {b=TSL2561_LUX_B7C; m=TSL2561_LUX_M7C;}
  else if (ratio > TSL2561_LUX_K8C)
    {b=TSL2561_LUX_B8C; m=TSL2561_LUX_M8C;}
#else
  if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
    {b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
  else if (ratio <= TSL2561_LUX_K2T)
    {b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
  else if (ratio <= TSL2561_LUX_K3T)
    {b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
  else if (ratio <= TSL2561_LUX_K4T)
    {b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
  else if (ratio <= TSL2561_LUX_K5T)
    {b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
  else if (ratio <= TSL2561_LUX_K6T)
    {b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
  else if (ratio <= TSL2561_LUX_K7T)
    {b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
  else if (ratio > TSL2561_LUX_K8T)
    {b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}
#endif

  unsigned long tmp;

  tmp = (ch0 * b) - (ch1 * m);

  if(tmp < 0)
    {
      tmp = 0;
    }

  tmp += (1 << (TSL2561_LUX_LUXSCALE - 1));

  *result = tmp >> TSL2561_LUX_LUXSCALE;

  return 0;
}
