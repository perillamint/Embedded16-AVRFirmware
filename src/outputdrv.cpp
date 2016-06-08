#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "errno.h"
#include "usbpwr.hpp"
#include "outputdrv.hpp"

#include "dumpcode.h"

static USBPWR usbpwr;

Outputdrv::Outputdrv()
{
  //
}

Outputdrv::~Outputdrv()
{
  //
}

int Outputdrv::init()
{
  usbpwr.init();
  usbpwr.set_pwr(0, true); //Turn on RasPi.
  return 0;
}

int Outputdrv::set_light(bool light)
{
  return usbpwr.set_pwr(1, light);
}
