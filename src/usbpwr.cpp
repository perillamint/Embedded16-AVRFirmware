#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include "util.h"
#include "errno.h"
#include "usbpwr.hpp"

#include "dumpcode.h"

USBPWR::USBPWR()
{
  //
}

USBPWR::~USBPWR()
{
  //
}

int USBPWR::init()
{
  //Set output.
  USB_PWR_DDR |= _BV(USB_PWR0_PIN) | _BV(USB_PWR1_PIN);
  return 0;
}

int USBPWR::set_pwr(uint8_t portno, bool pwren)
{
  switch(portno)
    {
    case 0:
      if(pwren)
        {
          USB_PWR_PORT |= _BV(USB_PWR0_PIN);
        }
      else
        {
          USB_PWR_PORT &= !(_BV(USB_PWR0_PIN));
        }
      break;
    case 1:
      if(pwren)
        {
          USB_PWR_PORT |= _BV(USB_PWR1_PIN);
        }
      else
        {
          USB_PWR_PORT &= !(_BV(USB_PWR1_PIN));
        }
      break;
    default:
      return -EADDRNOTAVAIL;
      break;
    }
  return 0;
}
