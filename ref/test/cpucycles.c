#include <stdint.h>
#include "cpucycles.h"
#include <string.h>
#include "../params.h"

uint64_t cpucycles_overhead(void) {
  uint64_t t0, t1, overhead = -1LL;
  unsigned int i;
  unsigned char m[CRYPTO_BYTES + CRHBYTES]; 

  for(i=0;i<100000;i++) {
    t0 = cpucycles();
    __asm__ volatile("");
    memset(m,1,CRYPTO_BYTES + CRHBYTES);
    t1 = cpucycles();
    if(t1 - t0 < overhead)
      overhead = t1 - t0;
  }

  return overhead;
}
