#ifndef __SENSORDRV_HPP
#define __SENSORDRV_HPP

#ifndef __cplusplus
#error Sensor driver class requires C++.
#endif

#ifndef __TSL2561_HPP
#include "tsl2561.hpp"
#endif

#define SENSORDRV_STACK_SIZE_BYTES 256

class Sensordrv
{
private:
  ATOM_TCB tcb;
  static void thread_func(uint32_t data);
public:
  Sensordrv();
  ~Sensordrv();
  int init();
  int start_thread();
};

#endif
