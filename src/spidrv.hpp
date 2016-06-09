#ifndef __SPIDRV_HPP
#define __SPIDRV_HPP

#ifndef __cplusplus
#error SPI driver class requires C++.
#endif

#define SPIDRV_STACK_SIZE_BYTES 256
#define MAX_SPIMEM_SIZE 0x10

typedef enum spimmap
  {
    THERMAL_AIR  = 0x00,
    HUMID_AIR    = 0x01,
    THERMAL_SOIL = 0x02,
    HUMID_SOIL   = 0x03,
    LUMINOSITY   = 0x04,
    WATER_LEVEL  = 0x05,
    PUMP1_TIME   = 0x06,
    PUMP2_TIME   = 0x07,
    LED_PWR      = 0x08
  } spimmap_t;

typedef union spimemdata
{
  uint16_t uint16;
  int16_t int16;
} spimemdata_t;

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
  spimemdata_t *get_spi_mem();
  int write_memory(spimmap_t addr, uint16_t value);
  int write_memory(spimmap_t addr, int16_t value);
  int read_memory(spimmap_t addr, uint16_t *value);
  int read_memory(spimmap_t addr, int16_t *value);
};

#endif
