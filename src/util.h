#ifndef __UTIL_H
#define __UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

  int msec_to_ticks(int msec);
  void atom_delay_ms(int msec);
  int atom_err_to_errno(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif
