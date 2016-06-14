#ifndef __UTIL_H
#define __UTIL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  int msec_to_ticks(int msec);
  void atom_delay_ms(int msec);
  int atom_err_to_errno(uint8_t status);
  int popcount(uint64_t x);
  bool do_parity(void* data, int size, bool odd);

#ifdef __cplusplus
}
#endif

#endif
