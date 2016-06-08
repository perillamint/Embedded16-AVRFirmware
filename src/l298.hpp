#ifndef __L298_HPP
#define __L298_HPP

#ifndef __cplusplus
#error L298 class requires C++.
#endif

#define PWM_TIMER_TCCRA TCCR1A
#define PWM_TIMER_TCCRB TCCR1B
#define PWM_TIMER_OCR_MOT1 OCR1B
#define PWM_TIMER_OCR_MOT2 OCR1A
#define PWM_TIMER_BITSIZE 16

#define PWM_OUTPUT_PIN_MASK _BV(4) | _BV(5)
#define PWM_OUTPUT_PORT_DDR DDRD

#define MOTOR_DIR_PORT_DDR DDRB
#define MOTOR_DIR_PORT PORTB
#define MOT1F_PORT 0
#define MOT1R_PORT 1
#define MOT2F_PORT 2
#define MOT2R_PORT 3

typedef enum
  {
    MOT_FORWARD,
    MOT_REVERSE,
    MOT_STOP
  } motor_dir_t;

class L298
{
private:
public:
  L298();
  ~L298();
  int init(uint32_t freq);
  int set_motor_dir(uint8_t motor, motor_dir_t dir);
  int set_motor_speed(uint8_t motor, uint8_t duty);
};

#endif
