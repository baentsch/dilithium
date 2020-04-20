#include <stdint.h>
#include "../sign.h"
#include "../poly.h"
#include "../polyvec.h"
#include "../params.h"
#include "cpucycles.h"
#include "speed_print.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NTESTS 3000

uint64_t t[NTESTS];

static int read_file(const char* name, unsigned char* data, unsigned int len) {
    FILE *fp = fopen(name, "r");
    if (!fp) { printf("fp not set!!\n"); return -1; }
    //printf("Reading %d bytes to %s\n", len, name);
    if (fread(data, 1, len, fp) != len) {
      printf("reading mismatch\n"); fclose(fp); return -1;
    }
    fclose(fp);
    return 0;
}

/*
static void write_file(const char* name, unsigned char* data, int len) {
    FILE *fp = fopen(name, "w");
if (!fp) printf("fp not set!!\n");
printf("Writing %d bytes to %s\n", len, name);
printf("Written: %ld\n", fwrite(data, 1, len, fp));
    fclose(fp);
}
*/

typedef struct {
   unsigned long cycles;
   unsigned char message[CRYPTO_BYTES + CRHBYTES];
} CRT; 

int main(int argc, char* argv[])
{
  unsigned int i;
  unsigned long long mlen, smlen;
  unsigned char pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char sk[CRYPTO_SECRETKEYBYTES];
  unsigned char sm[CRYPTO_BYTES + CRHBYTES] = {0};
  unsigned char *m = sm;
  unsigned char *m2 = sk;
  uint8_t *seed = sm;
  polyveck vk;
  polyvecl mat[K];
  poly *a = &mat[0].vec[0];
  poly *b = &mat[0].vec[1];
  poly *c = &mat[0].vec[2];
  unsigned long max_t;
  CRT crts[NTESTS];

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    expand_mat(mat, seed);
  }
  print_results("expand_mat:", t, NTESTS, NULL);


/*
  unsigned int j;
  polyvecl *vl = &mat[0];
  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    for(j = 0; j < L; ++j)
      poly_uniform_eta(&vl->vec[j], seed, j);
  }
  print_results("polyvecl_uniform_eta:", t, NTESTS, NULL);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    for(j = 0; j < L; ++j)
      poly_uniform_gamma1m1(&vl->vec[j], seed, j);
  }
  print_results("polyvecl_uniform_gamma1m1:", t, NTESTS, NULL);
*/

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    poly_ntt(a);
  }
  print_results("poly_ntt:", t, NTESTS, NULL);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    poly_invntt_tomont(a);
  }
  print_results("poly_invntt_tomont:", t, NTESTS, NULL);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    poly_pointwise_montgomery(c, a, b);
  }
  print_results("poly_pointwise_montgomery:", t, NTESTS, NULL);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    challenge(c, seed, &vk);
  }
  print_results("challenge:", t, NTESTS, NULL);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    crypto_sign_keypair(pk, sk);
  }
  print_results("Keypair:", t, NTESTS, NULL);

  if (argc>2) {
      printf("Loading keys from %s and %s\n", argv[1], argv[2]);
      if (read_file(argv[1], pk, CRYPTO_PUBLICKEYBYTES) || read_file(argv[2], sk, CRYPTO_SECRETKEYBYTES))
         exit(-1);
  }

  for(i = 0; i < NTESTS; ++i) {
    read_file("maxmsg-1193-worst.bin", m, CRYPTO_BYTES + CRHBYTES);
    //memset(m, 0, CRYPTO_BYTES + CRHBYTES);
    t[i] = cpucycles();
    crypto_sign(m, &smlen, m, CRHBYTES, sk);
  //  crts[i].cycles=t[i];
  //  memcpy(&crts[i].message,m,CRYPTO_BYTES + CRHBYTES);
  }
  max_t = print_results("Sign:", t, NTESTS, (argc>3?argv[3]:NULL));
  for(i = 0; i < NTESTS-1; ++i) {
     if (crts[i+1].cycles-crts[i].cycles >= max_t) {
        printf("Index for max_t = %d\n", i); 
        //write_file("maxmsg--1.bin", crts[i-1].message, CRYPTO_BYTES + CRHBYTES);
        //write_file("maxmsg-0.bin", crts[i].message, CRYPTO_BYTES + CRHBYTES);
        //write_file("maxmsg-1.bin", crts[i+1].message, CRYPTO_BYTES + CRHBYTES);
     }
  }
  printf("max_t = %ld\n", max_t);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    crypto_sign_open(m2, &mlen, m, smlen, pk);
  }
  print_results("Verify:", t, NTESTS, NULL);

  return 0;
}
