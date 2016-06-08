#ifndef __OUTPUTDRV_HPP
#define __OUTPUTDRV_HPP

#ifndef __cplusplus
#error Outputdrv class requires C++.
#endif

class Outputdrv
{
private:
public:
  Outputdrv();
  ~Outputdrv();
  int init();
  int set_light(bool light);
};

#endif
