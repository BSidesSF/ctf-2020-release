#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <wincrypt.h>
#include <wininet.h>

#include "pstdint.h"
#include "my_getopt.h"

#define SERVER "chameleon-82f4b6ab.challenges.bsidessf.net"
#define HTTPS

//#define SERVER "detritus"
//#undef HTTPS

#define STORE "/api/store"
#define RETRIEVE "/api/retrieve"

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wininet.lib")

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

#if 0
void print_hex(char *title, uint8_t *str, size_t length) {
  size_t i;

  fprintf(stderr, "%s: ", title);
  for(i = 0; i < length; i++) {
    fprintf(stderr, "%02x", str[i]);
  }
  fprintf(stderr, " (length: %d)\n\n", length);
}
#endif

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

void generate_key(uint8_t buffer[DES_KEY_SIZE]) {
  size_t i;

  mysrand((unsigned int)time(NULL));
  for(i = 0; i < DES_KEY_SIZE; i++) {
    buffer[i] = ((uint8_t)myrand() & 0x0FF) ^ 0x55;
  }

  //print_hex("test", buffer, DES_KEY_SIZE);
}

void to_hex(uint8_t in[DES_KEY_SIZE], char out[DES_KEY_SIZE * 2]) {
  size_t i;
  for(i = 0; i < DES_KEY_SIZE; i++) {
    sprintf(out + (i*2), "%02x", in[i]);
  }
}

void from_hex(char in[DES_KEY_SIZE * 2], uint8_t out[DES_KEY_SIZE]) {
  size_t i;
  char buffer[3];
  buffer[2] = '\0';

  for(i = 0; i < DES_KEY_SIZE; i++) {
    memcpy(buffer, &in[i * 2], 2);
    out[i] = (uint8_t)strtol(buffer, NULL, 16);
  }
}

void store_key(uint8_t key[DES_KEY_SIZE]) {
  char buffer[1024];
  DWORD bytes_read;
  char key_hex[DES_KEY_SIZE * 2 + 1];

  HINTERNET hInternet = InternetOpenA(
    "Chameleon",                             // LPCSTR lpszAgent,
    INTERNET_OPEN_TYPE_DIRECT,               // DWORD  dwAccessType,
    NULL,                                    // LPCSTR lpszProxy,
    NULL,                                    // LPCSTR lpszProxyBypass,
    0                                        // DWORD  dwFlags
  );
  if(!hInternet) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  HINTERNET hConnect = InternetConnect(
    hInternet,             // HINTERNET     hInternet,
    SERVER,                // LPCSTR        lpszServerName,
#ifdef HTTPS
    443,                   // INTERNET_PORT nServerPort,
#else
    4567,                  // INTERNET_PORT nServerPort,
#endif
    "",                    // LPCSTR        lpszUserName,
    "",                    // LPCSTR        lpszPassword,
    INTERNET_SERVICE_HTTP, // DWORD         dwService,
    0,                     // DWORD         dwFlags,
    NULL                   // DWORD_PTR     dwContext
  );
  if(!hConnect) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  //HttpOpenRequest
  HINTERNET hRequest = HttpOpenRequest(
    hConnect, // HINTERNET hConnect,
    "POST",   // LPCSTR    lpszVerb,
    STORE,    // LPCSTR    lpszObjectName,
    NULL,     // LPCSTR    lpszVersion,
    NULL,     // LPCSTR    lpszReferrer,
    NULL,     // LPCSTR    *lplpszAcceptTypes,
#ifdef HTTPS
    INTERNET_FLAG_SECURE, // DWORD     dwFlags,
#else
    0,                    // DWORD     dwFlags,
#endif
    NULL      // DWORD_PTR dwContext
  );
  if(!hRequest) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  DWORD ignore_revocation = SECURITY_FLAG_IGNORE_REVOCATION;
  if(!InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &ignore_revocation, sizeof(DWORD))) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  to_hex(key, key_hex);

  if(!HttpSendRequest(
    hRequest,        // HINTERNET hRequest,
    NULL,            // LPCSTR    lpszHeaders,
    0,               // DWORD     dwHeadersLength,
    key_hex,         // LPVOID    lpOptional,
    DES_KEY_SIZE * 2 // DWORD     dwOptionalLength
  )) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  char status[10];
  DWORD buffer_length = 10;
  if(!HttpQueryInfo(
    hRequest,               // HINTERNET hRequest,
    HTTP_QUERY_STATUS_CODE, // DWORD     dwInfoLevel,
    &status,                // LPVOID    lpBuffer,
    &buffer_length,         // LPDWORD   lpdwBufferLength,
    0                       // LPDWORD   lpdwIndex
  )) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  if(!InternetReadFile(
    hRequest,   // HINTERNET hFile,
    buffer,     // LPVOID    lpBuffer,
    1023,       // DWORD     dwNumberOfBytesToRead,
    &bytes_read
  )) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }
  buffer[bytes_read] = '\0';

  if(!strcmp(status, "200")) {
    fprintf(stderr, "Encrypted with id %s\n", buffer);
  } else {
    fprintf(stderr, "Uh oh!\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "HTTP %s: %s\n", status, buffer);
    exit(1);
  }
}

void retrieve_key(uint8_t key[DES_KEY_SIZE], char *id) {
  char buffer[1024];
  DWORD bytes_read;

  HINTERNET hInternet = InternetOpenA(
    "Chameleon",                             // LPCSTR lpszAgent,
    INTERNET_OPEN_TYPE_DIRECT,               // DWORD  dwAccessType,
    NULL,                                    // LPCSTR lpszProxy,
    NULL,                                    // LPCSTR lpszProxyBypass,
    0                                        // DWORD  dwFlags
  );
  if(!hInternet) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  HINTERNET hConnect = InternetConnect(
    hInternet,             // HINTERNET     hInternet,
    SERVER,                // LPCSTR        lpszServerName,
#ifdef HTTPS
    443,                   // INTERNET_PORT nServerPort,
#else
    4567,                  // INTERNET_PORT nServerPort,
#endif
    "",                    // LPCSTR        lpszUserName,
    "",                    // LPCSTR        lpszPassword,
    INTERNET_SERVICE_HTTP, // DWORD         dwService,
    0,                     // DWORD         dwFlags,
    NULL                   // DWORD_PTR     dwContext
  );
  if(!hConnect) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  //HttpOpenRequest
  HINTERNET hRequest = HttpOpenRequest(
    hConnect, // HINTERNET hConnect,
    "POST",   // LPCSTR    lpszVerb,
    RETRIEVE, // LPCSTR    lpszObjectName,
    NULL,     // LPCSTR    lpszVersion,
    NULL,     // LPCSTR    lpszReferrer,
    NULL,     // LPCSTR    *lplpszAcceptTypes,
#ifdef HTTPS
    INTERNET_FLAG_SECURE, // DWORD     dwFlags,
#else
    0,                    // DWORD     dwFlags,
#endif
    NULL      // DWORD_PTR dwContext
  );
  if(!hRequest) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  if(!HttpSendRequest(
    hRequest,        // HINTERNET hRequest,
    NULL,            // LPCSTR    lpszHeaders,
    0,               // DWORD     dwHeadersLength,
    id,              // LPVOID    lpOptional,
    strlen(id)       // DWORD     dwOptionalLength
  )) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  char status[10];
  DWORD buffer_length = 10;
  if(!HttpQueryInfo(
    hRequest,               // HINTERNET hRequest,
    HTTP_QUERY_STATUS_CODE, // DWORD     dwInfoLevel,
    &status,                // LPVOID    lpBuffer,
    &buffer_length,         // LPDWORD   lpdwBufferLength,
    0                       // LPDWORD   lpdwIndex
  )) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  if(!InternetReadFile(
    hRequest,    // HINTERNET hFile,
    buffer,      // LPVOID    lpBuffer,
    1024,        // DWORD     dwNumberOfBytesToRead,
    &bytes_read
  )) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }
  buffer[bytes_read] = '\0';

  if(!strcmp(status, "200")) {
    fprintf(stderr, "We found your key!\n");
  } else {
    fprintf(stderr, "Uh oh!\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "HTTP %s: %s\n", status, buffer);
    exit(1);
  }

  from_hex(buffer, key);
}

uint8_t *read_file(char *filename, uint32_t *len) {
  DWORD file_size;
  HANDLE f;

  if(filename) {
    f = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(f == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "Could not open the file for reading\n");
      exit(1);
    }
  } else {
    f = GetStdHandle(STD_INPUT_HANDLE);
  }

  file_size = GetFileSize(f, NULL);

  uint8_t *data = (uint8_t*)malloc(file_size);
  if(!ReadFile(f, data, file_size, len, 0)) {
    fprintf(stderr, "Could not read the file\n");
    exit(1);
  }

  return data;
}

void write_file(char *filename, uint8_t *data, size_t length) {
  DWORD bytes_written;
  HANDLE f;

  if(filename) {
    f = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  } else {
    f = GetStdHandle(STD_OUTPUT_HANDLE);
  }


  /* Write to a file */
  if(f == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Could not open the file for writing\n");
  }

  WriteFile(f, data, length, &bytes_written, NULL);
}

void do_encrypt(char *in_file, char *out_file) {
  uint8_t key[DES_KEY_SIZE];

  /* Cryptographic context */
  HCRYPTPROV hProv;

  /* Key blob */
  DESKEYBLOB keyBlob;

  /* The key that we're gonna use. */
  HCRYPTKEY hKey;

  /* Length of the file data. */
  uint32_t data_len;

  /* The data we're gonna encrypt. */
  uint8_t *data = read_file(in_file, &data_len);

  /* Make the buffer bigger so we can use it for encryption. */
  data = (uint8_t *)realloc(data, data_len + 16);

  /* Get a handle to the cryptographic library. */
  if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  /* Build the key blob */
  generate_key(key);

  keyBlob.hdr.bType = PLAINTEXTKEYBLOB;
  keyBlob.hdr.bVersion = CUR_BLOB_VERSION;
  keyBlob.hdr.reserved = 0;
  keyBlob.hdr.aiKeyAlg = CALG_DES;
  keyBlob.dwKeySize = DES_KEY_SIZE;
  memcpy(keyBlob.rgbKeyData, key, DES_KEY_SIZE);

  /* Convert the keyBlob to a key object */
  if(!CryptImportKey(hProv, (byte*) &keyBlob, sizeof(DESKEYBLOB), 0, CRYPT_EXPORTABLE, &hKey)) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  /* Perform the actual encyption. */
  if(!CryptEncrypt(hKey, 0, 1, 0, data, &data_len, data_len + 8)) {
    fprintf(stderr, "Encryption failed\n");
    exit(1);
  }

  store_key(key);

  write_file(out_file, data, data_len);
  free(data);
}

void do_decrypt(char *in_file, char *out_file, char *id) {
  uint8_t key[DES_KEY_SIZE];

  /* Cryptographic context */
  HCRYPTPROV hProv;

  /* Key blob */
  DESKEYBLOB keyBlob;

  /* The key that we're gonna use. */
  HCRYPTKEY hKey;

  /* Length of the file data. */
  uint32_t data_len;

  /* The data we're gonna encrypt. */
  uint8_t *data = read_file(in_file, &data_len);

  /* Get a handle to the cryptographic library. */
  if(!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  /* Build the key blob */
  retrieve_key(key, id);

  keyBlob.hdr.bType = PLAINTEXTKEYBLOB;
  keyBlob.hdr.bVersion = CUR_BLOB_VERSION;
  keyBlob.hdr.reserved = 0;
  keyBlob.hdr.aiKeyAlg = CALG_DES;
  keyBlob.dwKeySize = DES_KEY_SIZE;
  memcpy(keyBlob.rgbKeyData, key, DES_KEY_SIZE);

  /* Convert the keyBlob to a key object */
  if(!CryptImportKey(hProv, (byte*) &keyBlob, sizeof(DESKEYBLOB), 0, CRYPT_EXPORTABLE, &hKey)) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  /* Perform the actual decryption. */
  if(!CryptDecrypt(hKey, 0, 1, 0, data, &data_len)) {
    fprintf(stderr, "Decryption failed\n");
    exit(1);
  }

  fprintf(stderr, "File successfully decrypted!\n");

  write_file(out_file, data, data_len);
}

void usage(char *name)
{
  fprintf(stderr, "Usage: %s [--encrypt|<--decrypt --id=<id>>] <infile> <outfile>\n", name);
  fprintf(stderr, "\n");
  fprintf(stderr, "Example: %s --encrypt plaintext.txt ciphertext.txt\n", name);
  fprintf(stderr, "Example: %s --decrypt --id=abcd1234 ciphertext.txt plaintext.txt\n", name);
  exit(0);
}

int main(int argc, char *argv[]) {
  struct option long_options[] =
  {
    /* General options */
    {"help",    no_argument,       0, 0}, /* Help */
    {"h",       no_argument,       0, 0},
    {"version", no_argument,       0, 0}, /* Version */

    {"encrypt", no_argument,       0, 0}, /* Encrypt */
    {"decrypt", no_argument,       0, 0}, /* Decrypt */
    {"id",      required_argument, 0, 0}, /* id */

    /* Sentry */
    {0,         0,                 0, 0}  /* End */
  };

  int               c;
  int               option_index;
  const char       *option_name;

  int encrypt = 0;
  int decrypt = 0;
  char *id = NULL;
  char *in_file = 0;
  char *out_file = 0;

  /* Parse the command line options. */
  opterr = 0;
  while((c = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1)
  {
    switch(c)
    {
      case 0:
        option_name = long_options[option_index].name;

        /* General options */
        if(!strcmp(option_name, "help") || !strcmp(option_name, "h"))
        {
          usage(argv[0]);
        }
        else if(!strcmp(option_name, "encrypt"))
        {
          encrypt = 1;
        }
        else if(!strcmp(option_name, "decrypt"))
        {
          decrypt = 1;
        }
        else if(!strcmp(option_name, "id")) {
          id = optarg;
        }
        else
        {
          fprintf(stderr, "Unknown option\n\n");
          usage(argv[0]);
        }
        break;

      case '?':
      default:
        fprintf(stderr, "Unknown option\n\n");
        usage(argv[0]);
        break;
    }
  }

  // Optional arguments
  if(optind < argc) {
    in_file = argv[optind++];
  } else {
    fprintf(stderr, "\n* WARNING: You're reading from stdin. That only partially works, use at your own risk!\n\n");
  }

  if(optind < argc) {
    out_file = argv[optind++];
  }

  if(!(decrypt ^ encrypt)) {
    fprintf(stderr, "** Please pick --encrypt or --decrypt!\n\n");
    usage(argv[0]);
  }

  if(decrypt && !id) {
    fprintf(stderr, "** You need an --id to --decrypt!\n\n");
    usage(argv[0]);
  }

  if(encrypt) {
    do_encrypt(in_file, out_file);
  } else {
    do_decrypt(in_file, out_file, id);
  }
}
