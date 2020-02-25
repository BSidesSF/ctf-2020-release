#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <wincrypt.h>
#include <wininet.h>

#include "pstdint.h"

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wininet.lib")

#define FILE "f:\\ctf-2020\\challenges\\chameleon\\dist\\flag.png.enc"
#define DES_KEY_SIZE 8

#define N 351
#define M 175
#define R 19
#define TEMU 11
#define TEMS 7
#define TEMT 15
#define TEML 17
#define MATRIX_A 0xE4BD75F5
#define TEMB     0x655E5280
#define TEMC     0xFFD58000
static unsigned long mt[N];                 // state vector
static int mti=N;

void print_hex(char *title, uint8_t *str, DWORD length) {
  size_t i;

  fprintf(stderr, "%s: ", title);
  for(i = 0; i < length; i++) {
    fprintf(stderr, "%02x", str[i]);
  }
  fprintf(stderr, " (length: %d)\n\n", length);
}

void mysrand (int seed) {
  unsigned long s = (unsigned long)seed;
  for (mti=0; mti<N; mti++) {
    s = s * 29945647 - 1;
    mt[mti] = s;}
  return;
}

int myrand () {
  // generate 32 random bits
  unsigned long y;

  if (mti >= N) {
    // generate N words at one time
    const unsigned long LOWER_MASK = (1u << R) - 1; // lower R bits
    const unsigned long UPPER_MASK = -1 << R;       // upper 32-R bits
    int kk, km;
    for (kk=0, km=M; kk < N-1; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[km] ^ (y >> 1) ^ (-(signed long)(y & 1) & MATRIX_A);
      if (++km >= N) km = 0;}

    y = (mt[N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ (-(signed long)(y & 1) & MATRIX_A);
    mti = 0;}

  y = mt[mti++];

  // Tempering (May be omitted):
  y ^=  y >> TEMU;
  y ^= (y << TEMS) & TEMB;
  y ^= (y << TEMT) & TEMC;
  y ^=  y >> TEML;

  return y & 0x0ff;
}

typedef struct {
    BLOBHEADER hdr;
    DWORD dwKeySize;
    BYTE rgbKeyData[DES_KEY_SIZE];
} DESKEYBLOB;

void generate_key(time_t seed, uint8_t buffer[DES_KEY_SIZE]) {
  size_t i;

  mysrand(seed);
  for(i = 0; i < DES_KEY_SIZE; i++) {
    buffer[i] = ((uint8_t)myrand() & 0x0FF) ^ 0x55;
  }
}

void print_hex(uint8_t *str, size_t length) {
  size_t i;

  for(i = 0; i < length; i++) {
    printf("%02x", str[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  time_t start = time(NULL);

  int seed;
  for(seed = start; seed > (start - 86400); seed--) {
    uint8_t buffer[8];
    generate_key(seed, buffer);
    print_hex(buffer, 8);
  }
}
