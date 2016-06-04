#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include <atom.h>
#include <atommutex.h>

#include "i2c.h"
#include "errno.h"

//I2C semaphore
static ATOM_MUTEX i2c_mutex;


int i2c_init(uint32_t clock_freq)
{
  TWSR = 0x00;
  TWBR = (F_CPU / clock_freq - 16) / 2;
  //TWCR = _BV(TWEN);

  DDRC = 0x00;
  PORTC = 0x18;

  if(atomMutexCreate(&i2c_mutex) != ATOM_OK)
    {
      return -1;
    }

  return 0;
}

int i2c_start(char sla, bool w)
{
  static bool i2c_started = false;
  uint8_t mutex_result = 0;

  //Lock mutex
  if(ATOM_OK == (mutex_result = atomMutexGet(&i2c_mutex, 0)))
    {
      TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
      loop_until_bit_is_set(TWCR, TWINT);

      if(!i2c_started && 0x08 != TWSR)
        {
          //start signal is not sent but TWSR is not 0x08.
          //It indicates failure.
          atomMutexPut(&i2c_mutex);
          return -EIO;
        }
      else if (i2c_started && 0x10 != TWSR)
        {
          //start signal is not sent but TWSR is not 0x08.
          //It indicates failure.
          atomMutexPut(&i2c_mutex);
          return -EIO;
        }
      else
        {
          //Set started flag.
          i2c_started = true;
        }

      //Send SLA+W
      char data = (sla << 1) | (w ? 0x00 : 0x01);
      TWDR = data;
      TWCR = _BV(TWINT) | _BV(TWEN);
      loop_until_bit_is_set(TWCR, TWINT);

      //Check TWSR status.
      switch(TWSR)
        {
        case 0x18: //SLA+W has been transmitted; ACK received.
          if(!w)
            {
              return -EIO;
            }

          return 0;
        case 0x20: //SLA+W has been transmitted; NACK received.
          return -EADDRNOTAVAIL;
        case 0x40: //SLA+R has been transmitted; ACK received.
          if(w)
            {
              return -EIO;
            }

          return 0;
        case 0x48: //SLA+R has been transmitted; NACK received.
          return -EADDRNOTAVAIL;
        default:   //Unknown case. Maybe I/O error.
          return -EIO;
        }
    }
  else
    {
      switch(mutex_result)
        {
        case ATOM_TIMEOUT:
          return -ETIMEDOUT;
        case ATOM_WOULDBLOCK:
          return -EBUSY;
        default:
          return -EFAULT;
        }
    }
}

void i2c_stop(void)
{
  //Release mutex.
  atomMutexPut(&i2c_mutex);

  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

void i2c_write(char data)
{
  TWDR = data;
  TWCR = _BV(TWINT) | _BV(TWEN);
  loop_until_bit_is_set(TWCR, TWINT);
}

char i2c_read(bool with_ack)
{
  char retval = 0;
  
  if(atomMutexGet(&i2c_mutex, 0) == ATOM_OK)
    {
      TWCR = _BV(TWINT) | _BV(TWEN) | (with_ack ? _BV(TWEA) : 0);
      loop_until_bit_is_set(TWCR, TWINT);

      retval = TWDR;
      atomMutexPut(&i2c_mutex);
    }

  return retval;
}
