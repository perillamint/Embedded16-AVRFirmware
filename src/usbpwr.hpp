#ifndef __USBPWR_HPP
#define __USBPWR_HPP

#ifndef __cplusplus
#error USBPWR class requires C++.
#endif

#define USB_PWR_DDR DDRC
#define USB_PWR_PORT PORTC
#define USB_PWR0_PIN 6
#define USB_PWR1_PIN 7

class USBPWR
{
private:
public:
  USBPWR();
  ~USBPWR();
  int init();
  int set_pwr(uint8_t portno, bool pwren);
};

#endif
