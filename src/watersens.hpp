#ifndef __WATERSENS_HPP
#define __WATERSENS_HPP

#ifndef __cplusplus
#error Water sensor driver class requires C++.
#endif

#define WSENS_DIR DDRD
#define WSENS_PORT PIND
#define WATER_SENS_1 2
#define WATER_SENS_2 3

class WaterSensor {
private:
public:
  WaterSensor();
  ~WaterSensor();
  int init();
  int get_data(uint16_t *water_tank);
};

#endif
