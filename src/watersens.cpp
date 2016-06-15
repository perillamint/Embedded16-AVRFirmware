#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "watersens.hpp"

WaterSensor::WaterSensor()
{
  //
}

WaterSensor::~WaterSensor()
{
  //
}

int WaterSensor::init()
{
  WSENS_DIR &= ~(_BV(WATER_SENS_1) | _BV(WATER_SENS_2));
  return 0;
}

int WaterSensor::get_data(uint16_t *water_tank)
{
  uint8_t tmp;

  *water_tank = 0x0000;

  tmp = WSENS_PORT & _BV(WATER_SENS_1);
  *water_tank |= tmp?0x01:0x00;

  tmp = WSENS_PORT & _BV(WATER_SENS_2);
  *water_tank |= tmp?0x02:0x00;

  return 0;
}
