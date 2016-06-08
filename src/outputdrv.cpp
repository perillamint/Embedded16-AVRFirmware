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
#include "l298.hpp"
#include "outputdrv.hpp"

#include "dumpcode.h"

static USBPWR usbpwr;
static L298 l298;

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

  l298.init(10);
  l298.set_motor_dir(0, MOT_STOP);
  l298.set_motor_dir(1, MOT_STOP);
  return 0;
}

int Outputdrv::set_light(bool light)
{
  return usbpwr.set_pwr(1, light);
}
