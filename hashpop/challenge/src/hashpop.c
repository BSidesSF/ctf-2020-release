#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>
#include "ctf.h"

#define HASH_SIZE (128/8)
#define BUF_SIZE (1<<16)

typedef void (printer)(char *);

#define print_func(N, SZ) void print_ ## N (char *h) {\
  for (int i=0; i<SZ/(sizeof(char)*8); i++) \
    printf("%02hhx", (int)(h[i]) & 0xFF); \
  printf("\n"); \
}

#define print_b64_func(N, SZ) void print_b64_ ## N (char *h) {\
  b64_print(h, SZ/8);\
}

struct hash_runner {
  char *input;
  size_t input_len;
  char *hash_name;
  char *op_format;
  const EVP_MD *which_hash;
  unsigned char output[HASH_SIZE];
  printer *print_func;
};

/* Prototypes */
void set_hash_and_printer(struct hash_runner *h);
int parse_request(struct hash_runner *h, char *buf);
size_t urldecode(char *p);
char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
static void b64_print(char *what, size_t len);
void exec_hashing(struct hash_runner *h);
void print_hash(struct hash_runner *h);
void print_header();
void validate_request(struct hash_runner *h);
void rstrip(char *s);


print_func(md5, 128);
print_func(sha1, 160);
print_func(sha256, 256);
print_func(sha512, 512);
print_b64_func(md5, 128);
print_b64_func(sha1, 160);
print_b64_func(sha256, 256);
print_b64_func(sha512, 512);

#undef print_func
#undef print_b64_func

int main(int argc, char **argv) {
  SETUP_CTF();
  int rv;
  struct hash_runner *h = malloc(sizeof(struct hash_runner));
  char *buf = malloc(BUF_SIZE);
  if (!h || !buf) {
    exit(1);
  }
  size_t len = fread(buf, sizeof(buf[0]), BUF_SIZE, stdin);
  if (!len) {
    exit(1);
  }
  buf[len] = '\0';
  rstrip(buf);
  h->hash_name = "md5";
  h->op_format = "hex";
  if ((rv = parse_request(h, buf)) != 0) {
    exit(rv);
  }
  validate_request(h);
  h->which_hash = NULL;
  set_hash_and_printer(h);
  if (!h->which_hash)
    exit(1);
  exec_hashing(h);
  print_hash(h);
  return 0;
}

void set_hash_and_printer(struct hash_runner *h) {
#define choose(FN, ALT) if (!strcmp(h->hash_name, #FN) || !strcmp(h->hash_name, #ALT)) {\
  h->which_hash = EVP_ ## FN(); \
  if (h->op_format && (!strcmp(h->op_format, "base64") || !strcmp(h->op_format, "b64"))) { \
    h->print_func = print_b64_ ## FN; \
  } else { \
    h->print_func = print_ ## FN; \
  } \
  return; \
}
  choose(md5, MD5)
  choose(sha1, SHA1)
  choose(sha256, SHA256)
  choose(sha512, SHA512)
#undef choose
}

void exec_hashing(struct hash_runner *h) {
  static unsigned int md_len = 0;
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, h->which_hash, NULL);
  EVP_DigestUpdate(mdctx, h->input, h->input_len);
  EVP_DigestFinal_ex(mdctx, h->output, &md_len);
  EVP_MD_CTX_free(mdctx);
}

void print_hash(struct hash_runner *h) {
  print_header();
  h->print_func((char *)h->output);
  fflush(stdout);
}

static void b64_print(char *what, size_t len) {
  size_t out_len;
  char *encoded = base64_encode((unsigned char *)what, len, &out_len);
  printf("%s\n", encoded);
  free(encoded);
}

struct field {
  char *key;
  char *val;
  size_t len;
};

#define MAX_FIELDS 10

int parse_request(struct hash_runner *h, char *buf) {
  int n_fields = 0;
  int rv = 1;
  struct field *fields = calloc(MAX_FIELDS, sizeof(struct field));
  if (!fields)
    return 1;
  fields[0].key = buf;
  char *tok_next = strtok(buf, "&");
  n_fields++;
  while (tok_next) {
    if (n_fields >= MAX_FIELDS)
      goto parse_done;
    tok_next = strtok(NULL, "&");
    if (tok_next)
      fields[n_fields++].key = tok_next;
  }
  for (int i=0; i<n_fields; i++) {
    char *pch = strchr(fields[i].key, '=');
    if (!pch)
      continue;
    *pch++ = '\0';
    fields[i].val = pch;
    // urldecode in place
    urldecode(fields[i].key);
    fields[i].len = urldecode(fields[i].val);
    if (!strcmp(fields[i].key, "output")) {
      h->op_format = fields[i].val;
    } else if (!strcmp(fields[i].key, "input")) {
      h->input = fields[i].val;
      h->input_len = fields[i].len;
    } else if (!strcmp(fields[i].key, "hash")) {
      h->hash_name = fields[i].val;
    } else if (!strcmp(fields[i].key, "blog")) {
      printf("X-WWW-Blog: https://systemoverlord.com/\r\n");
    }
  }
  rv = 0;

parse_done:
  free(fields);
  return rv;
} // end parse_request

void validate_request(struct hash_runner *h) {
  if (!h->input)
    exit(1);
}

void print_header() {
  printf("Content-type: text/plain\r\n\r\n");
}

FILE *open_flag() {
  FILE *f;
#define TRY(p) if ((f = fopen(p, "r"))) return f
  TRY("/home/ctf/flag.txt");
  TRY("flag.txt");
  TRY("../flag.txt");
  TRY("/home/matir/flag.txt");
  TRY("/root/flag.txt");
#undef TRY
  return NULL;
}

void print_flag() {
  print_header();
  FILE *f = open_flag();
  if (!f)
    return;
  char buf[256];
  int r;
  if ((r = fread(buf, sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]), f)) == 0) {
    fclose(f);
    return;
  }
  fclose(f);
  buf[r] = '\0';
  printf("%s\n", buf);
}

void print_flag_json() {
  FILE *f = open_flag();
  if (!f)
    return;
  char buf[256];
  int r;
  if ((r = fread(buf, sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]), f)) == 0) {
    fclose(f);
    return;
  }
  fclose(f);
  buf[r] = '\0';
  printf("Content-type: application/json\r\n\r\n{\"flag\": \"%s\"}\n", buf);
}

void print_flag_xml() {
  FILE *f = open_flag();
  if (!f) return;
  char buf[256];
  int r;
  if ((r = fread(buf, sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]), f)) == 0) {
    fclose(f);
    return;
  }
  fclose(f);
  buf[r] = '\0';
  printf("Content-type: application/xml\r\n\r\n<flag>%s</flag>\n", buf);
}

void rstrip(char *s) {
  char *b=s;
  while(*s++);
  while(s-- > b) {
    if (*s == '\r' || *s == '\n')
      *s = '\0';
  }
}
