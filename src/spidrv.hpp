#ifndef __SPIDRV_HPP
#define __SPIDRV_HPP

#ifndef __cplusplus
#error SPI driver class requires C++.
#endif

#define SPIDRV_STACK_SIZE_BYTES 256

class SPIdrv
{
private:
  ATOM_TCB tcb;
  static void thread_func(uint32_t data);
public:
  SPIdrv();
  ~SPIdrv();
  int init();
  int start_thread();
};

#endif
