#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include <atom.h>
#include <atommutex.h>

#include "i2c.h"

//I2C semaphore
static ATOM_MUTEX i2c_mutex;

int i2c_init(uint32_t clock_freq)
{
  TWSR = 0x00;
  TWBR = (F_CPU / clock_freq - 16) / 2;
  TWCR = _BV(TWEN);

  return 0;
}

int i2c_start(void)
{
  //Lock mutex
  if(atomMutexCreate(&i2c_mutex) != ATOM_OK)
    {
      return -1;
    }

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  loop_until_bit_is_clear(TWCR, TWINT);

  return 0;
}

void i2c_stop(void)
{
  //Release mutex.
  atomMutexPut(&i2c_mutex);

  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
  loop_until_bit_is_clear(TWCR, TWINT);
}

void i2c_write(char data)
{
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);
  loop_until_bit_is_clear(TWCR, TWINT);
}

char i2c_read(bool with_ack)
{
  TWCR = _BV(TWINT) | _BV(TWEN) | (with_ack ? _BV(TWEA) : 0);
  loop_until_bit_is_clear(TWCR, TWINT);
  return TWDR;
}
