#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

#include <atom.h>
#include <atomtimer.h>

#include "constants.h"
#include "util.h"
#include "errno.h"
#include "l298.hpp"

L298::L298()
{
  //
}

L298::~L298()
{
  //
}

int L298::init(uint32_t freq)
{
  //Mark our PWM ports as output
  PWM_OUTPUT_PORT_DDR |= PWM_OUTPUT_PIN_MASK;
  //Mark our motor direction ports as output
  MOTOR_DIR_PORT_DDR |= _BV(MOT1F_PORT) | _BV(MOT1R_PORT) | _BV(MOT2F_PORT) | _BV(MOT2R_PORT);
  //Turn off all motor dir pins.
  MOTOR_DIR_PORT &= !(_BV(MOT1F_PORT) | _BV(MOT1R_PORT) | _BV(MOT2F_PORT) | _BV(MOT2R_PORT));
  return 0;
}

int L298::set_motor_dir(uint8_t motor, motor_dir_t dir)
{
  //CAUTION!! CAUTION!! CAUTION!!
  //DO NOT TURN ON FORWARD AND REVERSE AT SAME TIME.
  //IT WILL SHORT H-BRIDGE WITHOUT PROTECTION CIRCUIT
  //AND LET BLUE MAGIC SMOKE OUT!
  //(Although L298 has protection logic, but let's play safe.)

  switch(motor)
    {
    case 0: //MOT1(F|R)
      //Turn off it first.
      MOTOR_DIR_PORT &= !(_BV(MOT1F_PORT) | _BV(MOT1R_PORT));

      if(MOT_FORWARD == dir)
        {
          MOTOR_DIR_PORT |= _BV(MOT1F_PORT);
        }
      else if(MOT_REVERSE == dir)
        {
          MOTOR_DIR_PORT |= _BV(MOT1R_PORT);
        }
      else //MOT_STOP
        {
          //Do nothing.
        }
      break;
    case 1: //MOT2(F|R)
      //Turn off it first.
      MOTOR_DIR_PORT &= !(_BV(MOT2F_PORT) | _BV(MOT2R_PORT));

      if(MOT_FORWARD == dir)
        {
          MOTOR_DIR_PORT |= _BV(MOT2F_PORT);
        }
      else if(MOT_REVERSE == dir)
        {
          MOTOR_DIR_PORT |= _BV(MOT2R_PORT);
        }
      else //MOT_STOP
        {
          //Do nothing.
        }
      break;
    default:
      return -EADDRNOTAVAIL;
    }

  return 0;
}
