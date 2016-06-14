#include <inttypes.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"

int msec_to_ticks(int msec)
{
  int32_t ret = msec;
  ret *= SYSTICKS_PER_SEC;
  ret /= 1000;

  return ret;
}

void atom_delay_ms(int msec)
{
  atomTimerDelay(msec_to_ticks(msec));
}

int atom_err_to_errno(uint8_t status)
{
  //TODO: impl this.
  return status;
}

int popcount(uint64_t x)
{
  static const uint64_t m1  = 0x5555555555555555; //binary: 0101...
  static const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
  static const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
  x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
  x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
  x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
  x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
  x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
  x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
  return x & 0x7f;
}

bool do_parity(void* data, int size, bool odd)
{
  uint64_t csum = 0;
  uint8_t *uint8_data = (uint8_t*)data;
  int pop;

  for(int i = 0; i < 4; i++)
    {
      csum |= uint8_data[i];
      csum <<= 8;
    }

  pop = popcount(csum);

  if(odd) //Odd parity
    {
      return (pop % 2 == 0);
    }
  else
    {
      return !(pop % 2 == 0);
    }
}
