#ifndef __SHT10_HPP
#define __SHT10_HPP

#ifndef __cplusplus
#error SHT10 class requires C++.
#endif

#define SHT10_DDR DDRD
#define SHT10_PORT PORTD
#define SHT10_PIN PIND

#define SHT10_CLK 7
#define SHT10_DATA 6

#define SHT10_PULSE_DELAY_US 5

typedef enum sht10_type
  {
    TEMP,
    HUMID
  } sht10_type_t;

class SHT10
{
private:
  int shift_out(bool msbfirst, uint8_t data);
  void shift_in(bool msbfirst, uint8_t *data, bool ack);
  void data_mode(bool output);
  void wait_for_data_ready();
  int start(uint8_t sla, bool w);
  int read(void *data, int len);
  int write(void *data, int len);
public:
  SHT10();
  ~SHT10();
  int init();
  int get_raw_data(sht10_type_t type, uint16_t *raw_data);
  int get_calculated_data(sht10_type_t type, int16_t *data);
};

#endif
