#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "i2c.h"
#include "errno.h"
#include "tsl2561.hpp"

TSL2561::TSL2561(char addr)
{
  this -> addr = addr;
  //TODO: Constructor.
}

TSL2561::~TSL2561()
{
  //TODO: Destructor.
}

int TSL2561::init()
{
  int ret;

  ret = i2c_start(this -> addr, true);
  if(ret < 0) //There is error.
    {
      return -EIO;
    }

  ret = i2c_putchar(TSL2561_REGISTER_ID);
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
      return 0;
    }

  return -1;
}
