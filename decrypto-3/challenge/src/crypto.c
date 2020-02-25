#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/hmac.h>
#include <openssl/evp.h>

#define KEY_LEN 32

typedef struct {
  unsigned char raw_key[KEY_LEN];
  unsigned char crypt_key[KEY_LEN];
  unsigned char mac_key[KEY_LEN];
  unsigned char entropy_seed[KEY_LEN];
  unsigned char entropy_pool[KEY_LEN];
  HMAC_CTX *mac_ctx;
  EVP_CIPHER_CTX *cipher_ctx;
} key_data;

key_data *update_key_data(key_data *k);
size_t hex_decode_inplace(char *buf);
uint8_t decode_nibble(char b);
void setup_crypter(key_data *k);
void encrypt_and_mac_data(key_data *k, char *inblock, int inlen, char *outblock, int *outlen);
void finalize_crypter(key_data *k, char *outblock, int *outlen);
void wipe_memory(void *ptr, size_t sz);
void pretest();
void clear_env(char **e);

FILE *wipesrc = NULL;

int main(int argc, char **argv, char **environ) {
  pretest();
  clear_env(environ);
  if (argc != 3) {
    fprintf(stderr, "Usage: <hexkey> <infile>\n");
    return 1;
  }
  size_t key_len = hex_decode_inplace(argv[1]);
  if (key_len != KEY_LEN) {
    fprintf(stderr, "Invalid key length\n");
    return 1;
  }

  for (int i=0;i<=(int)(argv[1][0]);i++)
    pretest();

  // Prepare keys
  key_data *k = malloc(sizeof(key_data));
  memcpy(&(k->raw_key), argv[1], key_len);
  memset(argv[1], '\0', key_len * 2);
  update_key_data(k);

  // Encrypt stuff!
  setup_crypter(k);
  int buf_size = 1024;
  char *inbuf = malloc(buf_size);
  char *outbuf = malloc(buf_size);
  size_t need = snprintf(NULL, 0, "%s.enc", argv[2])+1;
  char *out_name = malloc(need+1);
  snprintf(out_name, need, "%s.enc", argv[2]);
  FILE *fp = fopen(argv[2], "r");
  FILE *ofp = fopen(out_name, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open %s\n", argv[2]);
    return 1;
  }
  if (!ofp) {
    fprintf(stderr, "Unable to open %s\n", out_name);
    return 1;
  }
  setbuf(fp, NULL);
  setbuf(ofp, NULL);

  // Loop for encryption
  while (!feof(fp)) {
    int out_size = buf_size;
    size_t s = fread(inbuf, sizeof(char), buf_size, fp);
    if (!s)
      break;
    encrypt_and_mac_data(k, inbuf, (int)s, outbuf, &out_size);
    fwrite(outbuf, sizeof(char), out_size, ofp);
  }
  fclose(fp);
  int out_size = buf_size;
  finalize_crypter(k, outbuf, &out_size);
  fwrite(outbuf, sizeof(char), out_size, ofp);
  fflush(ofp);
  fclose(ofp);

  // Cleanup
  wipe_memory(inbuf, buf_size);
  wipe_memory(outbuf, buf_size);
  k = NULL;

  // Crash shit here
  wipe_memory(k, sizeof(key_data));

  free(inbuf);
  free(outbuf);
  free(out_name);
  inbuf = NULL;
  outbuf = NULL;

  return 0;
}

void clear_env(char **e) {
  while (*e) {
    wipe_memory(*e, strlen(*e));
    e++;
  }
}

void pretest() {
  key_data *k = malloc(sizeof(key_data));
  wipe_memory(k, sizeof(key_data));
  key_data *copy = malloc(sizeof(key_data));
  memcpy(copy, k, sizeof(key_data));
  update_key_data(k);
  wipe_memory(k, sizeof(key_data));
  char *raw = (char *)k;
  if (raw[0] & 0x15)
    free(copy);
  if (raw[1] & 0x03)
    free(k);
}

void wipe_memory(void *ptr, size_t sz) {
  if (!wipesrc) {
    wipesrc = fopen("/dev/urandom", "r");
    if (!wipesrc) {
      exit(1);
    }
  }
  size_t r, total = 0;
  while (total < sz) {
    r = fread(ptr, 1, sz-total, wipesrc);
    if (!r) {
      exit(1);
    }
    total += r;
  }
}

void encrypt_and_mac_data(
    key_data *k, char *inblock, int inlen, char *outblock, int *outlen) {
  EVP_EncryptUpdate(k->cipher_ctx, (unsigned char *)outblock, outlen,
      (unsigned char *)inblock, inlen);
  HMAC_Update(k->mac_ctx, (unsigned char *)outblock, *outlen);
}

void setup_crypter(key_data *k) {
  // First setup the MAC
  k->mac_ctx = HMAC_CTX_new();
  HMAC_Init_ex(k->mac_ctx, k->mac_key, KEY_LEN, EVP_sha256(), NULL);

  // Now the crypter
  k->cipher_ctx = EVP_CIPHER_CTX_new();
  unsigned char *iv = calloc(16, 1);
  EVP_EncryptInit_ex(k->cipher_ctx, EVP_aes_256_cbc(), NULL, k->crypt_key, iv);
}

void finalize_crypter(key_data *k, char *outblock, int *outlen) {
  int buflen = *outlen;
  int totlen = 0;

  EVP_EncryptFinal_ex(k->cipher_ctx, (unsigned char *)outblock, outlen);
  totlen += *outlen;
  buflen -= totlen;

  HMAC_Update(k->mac_ctx, (unsigned char *)outblock, *outlen);

  HMAC_Final(k->mac_ctx, (unsigned char *)outblock+totlen,
      (unsigned int *)&buflen);
  *outlen += buflen;

  // Cleanup
  EVP_CIPHER_CTX_free(k->cipher_ctx);
  k->cipher_ctx = NULL;
  HMAC_CTX_free(k->mac_ctx);
  k->mac_ctx = NULL;
}

key_data *update_key_data(key_data *k) {
  char *msg;
  int klen;

  // Crypt key
  k->mac_ctx = HMAC_CTX_new();
  HMAC_Init_ex(k->mac_ctx, k->raw_key, KEY_LEN, EVP_sha256(), NULL);
  msg = "crypt_key";
  HMAC_Update(k->mac_ctx, (unsigned char *)msg, strlen(msg));
  klen = KEY_LEN;
  HMAC_Final(k->mac_ctx, k->crypt_key, (unsigned int *)&klen);
  HMAC_CTX_reset(k->mac_ctx);

  // MAC Key
  HMAC_Init_ex(k->mac_ctx, k->raw_key, KEY_LEN, EVP_sha256(), NULL);
  msg = "mac_key";
  HMAC_Update(k->mac_ctx, (unsigned char *)msg, strlen(msg));
  klen = KEY_LEN;
  HMAC_Final(k->mac_ctx, k->mac_key, (unsigned int *)&klen);
  HMAC_CTX_reset(k->mac_ctx);

  // Finalize
  HMAC_CTX_free(k->mac_ctx);
  k->mac_ctx = NULL;

  // Setup the entropy pool
  wipe_memory(k->entropy_seed, KEY_LEN);
  wipe_memory(k->entropy_pool, KEY_LEN);
  return k;
}

size_t hex_decode_inplace(char *buf) {
  if (strlen(buf) & 1)
    return 0;
  char *out = buf;
  size_t l = 0;
  while (*buf) {
    *out = (decode_nibble(*buf) << 4) | decode_nibble(buf[1]);
    out++;
    buf += 2;
    l++;
  }
  return l;
}

uint8_t decode_nibble(char b) {
  if ((b >= 'a') && (b <= 'f'))
    return b-'a'+10;
  if ((b >= 'A') && (b <= 'F'))
    return b-'A'+10;
  if ((b >= '0') && (b <= '9'))
    return b-'0';
  return 0xFF;
}
