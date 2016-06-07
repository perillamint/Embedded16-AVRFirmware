#ifndef __ETH01D_HPP
#define __ETH01D_HPP

#ifndef __cplusplus
#error ETH01D class requires C++.
#endif

#define ETH01D_DEFAULT_ADDR 0x44

class ETH01D
{
private:
  uint8_t addr;
public:
  ETH01D(uint8_t addr = ETH01D_DEFAULT_ADDR);
  ~ETH01D();
  int get_raw_data(uint16_t *humid, uint16_t *temp);
  int get_calculated_data(int16_t *humid, int16_t *temp);
};

#endif
