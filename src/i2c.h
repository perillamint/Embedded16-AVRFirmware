#ifndef __I2C_H
#define __I2C_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  int i2c_init(uint32_t clock_freq);
  int i2c_start(char sla, bool w);
  void i2c_stop(void);
  int i2c_write(void *buf, int len);
  int i2c_read(void *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
