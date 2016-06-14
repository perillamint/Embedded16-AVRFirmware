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
    LED1_PWR     = 0x08,
    LED2_PWR     = 0x09
  } spimmap_t;

typedef enum spidid
  {
    SYSTEM_ALIVE          = 0x00,
    WATER_TANK_LEVEL      = 0x01,
    SAUCER_TANK_LEVEL     = 0x02,
    AIR_SENSOR_AVAIL      = 0x10,
    AIR_HUMIDITY          = 0x11,
    AIR_TEMPERATURE       = 0x12,
    SOIL_SENSOR_AVAIL     = 0x20,
    SOIL_HUMIDITY         = 0x21,
    OTHER_SENS_AVAIL      = 0x30,
    LIGHT_INTENSITY       = 0x31,
    SYSTEM_TICK           = 0x80,
    AIR_MOTIVATOR_AVAIL   = 0x90,
    WATER_SPRAY_MOTOR     = 0x91,
    SOIL_MOTIVATOR_AVAIL  = 0xA0,
    WATER_PUMP_MOTOR      = 0xA1,
    OTHER_MOTIVATOR_AVAIL = 0xB0,
    LAMP                  = 0xB1
  } spidid_t;

typedef union spimemdata
{
  uint16_t uint16;
  int16_t int16;
} spimemdata_t;

typedef struct spi_packet
{
  uint8_t   rid      :4; // 5-8th bit.
  bool      parity   :1; // 4th bit.
  bool      reserved :1; // 3th bit.
  bool      write    :1; // 2nd bit.
  bool      version  :1; // 1st bit. (MSB)
  uint8_t   did      :8;
  uint16_t  data     :16;
} __attribute__ ((__packed__)) spi_packet_t;

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
