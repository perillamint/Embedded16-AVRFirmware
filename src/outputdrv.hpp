#ifndef __OUTPUTDRV_HPP
#define __OUTPUTDRV_HPP

#ifndef __cplusplus
#error Outputdrv class requires C++.
#endif

#define OUTDRV_STACK_SIZE_BYTES 256

class Outputdrv
{
private:
  ATOM_TCB tcb;
  static void thread_func(uint32_t data);
public:
  Outputdrv();
  ~Outputdrv();
  int init();
  int start_thread();
  int set_light(bool light);
};

#endif
