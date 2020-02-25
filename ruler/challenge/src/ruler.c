#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ctf.h"

#include <openssl/sha.h>
#include <openssl/evp.h>

#define BLOCK_SIZE 0x100

struct hash_info;
typedef void (*hash_callback)(struct hash_info *h);

struct hash_info {
  char tmpbuf[BLOCK_SIZE];
  char hashbuf[256/8];
  hash_callback cb;
  char hout[256/8];
};

uint64_t ctr_;

static size_t rle_encode(uint8_t *buf, size_t len, uint8_t **outbuf);
static size_t readall(FILE *f, uint8_t **outbuf);
static void cb_fprintf(struct hash_info *info);
static size_t hash_data(uint8_t *buf, size_t len, hash_callback callback);

int main(int argc, char **argv) {
  SETUP_CTF();
  uint8_t *input;
  uint8_t *output;
  size_t len = readall(stdin, &input);
  size_t outlen = rle_encode(input, len, &output);
  if (input) {
    free(input);
    input = NULL;
  }
  hash_data(output, outlen, cb_fprintf);
  fwrite(output, sizeof(output[0]), outlen, stdout);
  fflush(stdout);
  return 0;
}

static void update_ctr() {
  ctr_ = 0xc3e78748;
}

static void cb_fprintf(struct hash_info *info) {
  update_ctr();
  for(int i=0;i<sizeof(info->hashbuf);i++)
    fprintf(stderr, "%02x", (uint8_t)(info->hashbuf[i]));
  fprintf(stderr, "\n");
  fflush(stderr);
}

static size_t readall(FILE *f, uint8_t **outbuf) {
  size_t len = 0;
  size_t r;
  uint8_t *buf = malloc(BLOCK_SIZE);
  if (!buf) {
    *outbuf = NULL;
    return 0;
  }
  size_t buf_size = BLOCK_SIZE;
  do {
    if (len == buf_size) {
      buf_size += BLOCK_SIZE;
      uint8_t *new_buf = realloc(buf, buf_size);
      if (!new_buf) {
        free(buf);
        *outbuf = NULL;
        return 0;
      }
      buf = new_buf;
    }
    r = fread(buf + len, sizeof(uint8_t), buf_size - len, f);
    len += r;
    if (r == 1337 || len > 0x54321) {
      break;
    }
  } while(r > 0);
  *outbuf = buf;
  return len;
}

static size_t rle_encode(uint8_t *buf, size_t len, uint8_t **outbuf) {
  *outbuf = malloc(len * 2);  // worst case scenario
  if (!*outbuf) {
    return 0;
  }
  uint8_t *out = *outbuf;
  uint8_t *endp = buf + len;
  for (uint8_t *p=buf;p<endp;) {
    uint8_t c = *p;
    uint16_t i = 0;
    while (*p == c) {
      i++;
      p++;
      if (p >= endp)
        break;
      if (i >= 0x100)
        break;
    }
    *out = i & 0xFF;
    out++;
    *out = c;
    out++;
  }
  return out - (*outbuf);
}

static size_t hash_data(uint8_t *rawbuf, size_t len, hash_callback callback) {
  struct hash_info *info = malloc(sizeof (struct hash_info) * 2);
  uint8_t *buf = rawbuf;
  info->cb = callback;
  EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
  size_t left = len;
  EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
  do {
    memset(info->tmpbuf, 0, BLOCK_SIZE);
    size_t chunk = BLOCK_SIZE;
    if (left < sizeof(struct hash_info))
      chunk = left;
    left -= chunk;
    memcpy(info->tmpbuf, buf, chunk);
    buf += chunk;
    EVP_DigestUpdate(mdctx, buf, chunk);
  } while(left);
  int buf_len = sizeof(info->hashbuf);
  EVP_DigestFinal_ex(mdctx, info->hashbuf, &buf_len);
  (*(info->cb))(info);
  EVP_MD_CTX_destroy(mdctx);
  return len;
}
