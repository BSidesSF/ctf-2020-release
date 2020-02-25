#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "ctf.h"

#define BUF_SIZE 4096

#define KEY_LEN_MIN 1
#define KEY_LEN_MAX 256
#define RC4_SZ 256

#define TYPE_PLAIN 1
#define TYPE_ENCODE 2
#define TYPE_ENCRYPT 3

#define PROT_MASK (PROT_READ|PROT_WRITE|PROT_EXEC)
#define PROT_RW (PROT_MASK|PROT_READ|PROT_WRITE)

#define CHECK_TYPE(ws, t) ((ws->type & t) == t)
#define CMD_IS(c) (!strcmp(cmd, c))

typedef struct {
  int type;
  char *input_buf;
  size_t buf_len;
  union {
    void (*print_encoded)(const char *, size_t);
    char *enc_state;
  };
} workspace_t;

char *prompt(const char *prompt_str);
unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
void *secure_malloc(size_t sz);
void do_encrypt(workspace_t *ws);
void print_base64(const char *buf, size_t len);
void print_hex(const char *buf, size_t len);
void print_menu(int type);
void print_state(workspace_t *ws);
void secure_free(void *ptr);
char *secure_strdup(const char *s);
void set_encoding(workspace_t *ws);
void set_input(workspace_t *ws);
void set_key(workspace_t *ws);

int main(int argc, char **argv) {
  SETUP_CTF();
  workspace_t *ws = secure_malloc(sizeof(workspace_t));
  memset(ws, 0, sizeof(workspace_t));
  char *type = prompt("type (plain, encoded, encrypted)");
  if (!strcmp("plain", type)) {
    ws->type = TYPE_PLAIN;
  } else if (!strcmp("encoded", type)) {
    ws->type = TYPE_ENCODE;
  } else if (!strcmp("encrypted", type)) {
    ws->type = TYPE_ENCRYPT;
  } else {
    printf("Invalid type!\n");
    return 1;
  }
  while (1) {
    print_menu(ws->type);
    char *cmd = prompt("command");
    if (!cmd) {
      printf("Exiting!\n");
      return 0;
    }
    if (CMD_IS("quit") || CMD_IS("exit")) {
      return 0;
    } else if (CMD_IS("set_input")) {
      set_input(ws);
      continue;
    } else if (CMD_IS("set_encoding")) {
      if (CHECK_TYPE(ws, TYPE_ENCRYPT)) {
        printf("No encoding for encrypted type!\n");
        continue;
      }
      set_encoding(ws);
      continue;
    } else if (CMD_IS("print")) {
      print_state(ws);
      continue;
    } else if (CMD_IS("set_key")) {
      if (!CHECK_TYPE(ws, TYPE_ENCRYPT)) {
        printf("Can only set key for encrypted type.\n");
        continue;
      }
      set_key(ws);
      continue;
    } else if (CMD_IS("encrypt")) {
      if (!CHECK_TYPE(ws, TYPE_ENCRYPT)) {
        printf("Can only encrypt for encrypted type.\n");
        continue;
      }
      do_encrypt(ws);
      continue;
    }
  }
}

static void print_menu_line(const char *cmd, const char *desc) {
  printf("%-12s: %s\n", cmd, desc);
}

void print_menu(int type) {
  puts("");
  print_menu_line("set_input", "Set the input value");
  if (type == TYPE_PLAIN) {
    print_menu_line("print", "Print the output value");
  } else if (type == TYPE_ENCODE) {
    print_menu_line("print", "Print the output value");
    print_menu_line("set_encoding", "Set the encoding scheme.");
  } else if (type == TYPE_ENCRYPT) {
    print_menu_line("set_key", "Set the RC4 key.");
    print_menu_line("encrypt", "Perform encryption.");
  }
  print_menu_line("quit", "Quit the Program");
  puts("");
}

void set_key(workspace_t *ws) {
  char *key_data = prompt("key (hex)");
  if (!key_data) {
    printf("No key!\n");
    return;
  }
  size_t key_len = strlen(key_data);
  if (key_len > (KEY_LEN_MAX * 2) ||
      key_len < (KEY_LEN_MIN * 2) ||
      key_len % 2) {
    printf("Invalid key length!\n");
    return;
  }
  for (int i=0;i<key_len;i+=2) {
    int byte;
    int rv = sscanf(&key_data[i], "%02x", &byte);
    if (rv != 1) {
      printf("Invalid key data!\n");
      return;
    }
    key_data[i/2] = (char)(byte & 0xFF);
  }
  key_len = key_len >> 1;
  // Allocate key data
  if (ws->enc_state) {
    secure_free(ws->enc_state);
    ws->enc_state = NULL;
  }
  ws->enc_state = secure_malloc(RC4_SZ);
  // Initial data
  for (int i=0;i<RC4_SZ;i++) {
    ws->enc_state[i] = i;
  }
  // Permute from key
  int j=0;
  for (int i=0;i<RC4_SZ;i++) {
    j = (j + ws->enc_state[i] + key_data[i % key_len]) & 0xFF;
    char tmp = ws->enc_state[i];
    ws->enc_state[i] = ws->enc_state[j];
    ws->enc_state[j] = tmp;
  }
  memset(key_data, 0, key_len * 2);
  printf("Key has been set.\n");
}

void set_input(workspace_t *ws) {
  if (ws->input_buf) {
    secure_free(ws->input_buf);
    ws->input_buf = NULL;
    ws->buf_len = 0;
  }
  char *inp = prompt("input");
  if (!inp) {
    printf("No input!");
    return;
  }
  ws->input_buf = secure_strdup(inp);
  printf("Input is %lu bytes long.\n", strlen(ws->input_buf));
  ws->buf_len = strlen(ws->input_buf);
}

void set_encoding(workspace_t *ws) {
  char *enc = prompt("encoding (base64, hex)");
  if (!strcasecmp("base64", enc)) {
    ws->print_encoded = print_base64;
  } else if (!strcasecmp("hex", enc)) {
    ws->print_encoded = print_hex;
  } else {
    printf("Invalid encoding!\n");
  }
}

void print_state(workspace_t *ws) {
  if (CHECK_TYPE(ws, TYPE_ENCODE)) {
    if (!ws->print_encoded) {
      printf("Must use set_encoding first.\n");
      return;
    }
    ws->print_encoded(ws->input_buf, ws->buf_len);
  } else if (CHECK_TYPE(ws, TYPE_PLAIN)) {
    printf("%s\n", ws->input_buf);
  } else {
    printf("Printing not supported for encrypted data.\n");
  }
}

char *prompt(const char *prompt_str) {
  static char buf[BUF_SIZE];
  printf("%s> ", prompt_str);
  char *rv = fgets(buf, BUF_SIZE, stdin);
  if (!rv) {
    return NULL;
  }
  size_t l = strlen(rv);
  if (rv[l-1] == '\n')
    rv[l-1] = '\0';
  return rv;
}

void print_hex(const char *buf, size_t buflen) {
  for (int i=0; i<buflen; i++) {
    printf("%02x", ((int)buf[i]) & 0xFF);
  }
  printf("\n");
}

void print_base64(const char *buf, size_t buflen) {
  unsigned char *encoded = base64_encode((unsigned char *)buf, buflen, NULL);
  if (!encoded)
    return;
  printf("%s\n", encoded);
  free(encoded);
}

void do_encrypt(workspace_t *ws) {
  // PRGA keystream
  int i=0, j=0;
  for(int k=0;k<ws->buf_len;k++) {
    i += 1;
    i &= 0xFF;
    j += ws->enc_state[i];
    j &= 0xFF;
    int tmp = ws->enc_state[i];
    ws->enc_state[i] = ws->enc_state[j];
    ws->enc_state[j] = tmp;
    tmp = ws->enc_state[(ws->enc_state[i] + ws->enc_state[j]) & 0xFF];
    ws->input_buf[k] ^= tmp;
  }
  printf("Buffer encrypted!\n");
}

// Allocate memory with guard pages to protect against heap overflow.
// Memory is also locked to avoid swapping out.
// Exits if unable to allocate.
void *secure_malloc(size_t sz) {
  size_t pg_size = (size_t)sysconf(_SC_PAGESIZE);
  // Round size up
  sz = (sz + pg_size-1) & ~(pg_size-1);
  // Add guard pages
  sz += (pg_size << 1);
  void *ptr = mmap(NULL, sz, PROT_RW, MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED, -1, 0);
  if (ptr == MAP_FAILED) {
    printf("Unable to allocate memory!\n\n");
    printf("Error: %s\n", strerror(errno));
    _exit(1);
  }
  *(size_t *)ptr = sz;
  // Lower guard page
  mprotect(ptr, pg_size, PROT_NONE);
  // Upper guard page
  mprotect((char *)ptr+sz-pg_size, pg_size, PROT_NONE);
  return (char *)ptr+pg_size;
}

// Free memory after zeroing to remove any artifacts.
void secure_free(void *ptr) {
  size_t pg_size = (size_t)sysconf(_SC_PAGESIZE);
  char *base = (char *)ptr - pg_size;
  mprotect(base, pg_size, PROT_READ);
  size_t sz = *(size_t *)base;
  memset(ptr, 0, sz-(pg_size<<1));
  munmap(base, sz);
}

char *secure_strdup(const char *s) {
  size_t len = strlen(s) + 1;
  char *buf = secure_malloc(len);
  memcpy(buf, s, len+1);
  return buf;
}
