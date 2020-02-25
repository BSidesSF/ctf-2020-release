#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ctf.h"

const char *banner = ("The earliest computers were thought of as nothing more than\n"
    "fancy adding machines.  We pay homage to that notion today by bringing\n"
    "you a literal adding machine.  All it does is add numbers.\n\n"
    "You can find the flag at %s.\n\n");

const char *flag_path = "/home/ctf/flag.txt";

void debug(const char *func, void *rbp, void *rsp);
void hexdump_line(void *addr);
long get_long(char *prompt);
void get_data(long *array, unsigned char l);
long add_nums(long *array, unsigned char l);
void print_asciiart(const char *path);

struct {
  const char *func_name;
  void *rsp;
  void *rbp;
} debug_info;

#define STORE_DEBUG() \
  do { \
    asm("mov %0, rbp": "=mr" (debug_info.rbp)); \
    asm("mov %0, rsp": "=mr" (debug_info.rsp)); \
    debug_info.func_name = __func__; \
  } while(0)

#define CALL_LOCAL_DEBUG() \
  do { \
    void *rsp; \
    void *rbp; \
    asm("mov %0, rbp": "=mr" (rbp)); \
    asm("mov %0, rsp": "=mr" (rsp)); \
    debug(__func__, rbp, rsp); \
  } while(0)

#define CALL_DEBUG() \
  do { \
    debug(debug_info.func_name, debug_info.rbp, debug_info.rsp); \
  } while(0)

int main(int argc, char **argv) {
  SETUP_CTF();
  long scratch[CHAR_MAX];
  print_asciiart("./asciiart.txt");
  printf(banner, flag_path);
  STORE_DEBUG();
  long num_ints = get_long("Number of numbers to add");
  if (num_ints > CHAR_MAX) {
    printf("Error, can only handle up to %d (CHAR_MAX) values!\n", CHAR_MAX);
    return 1;
  }
  get_data(scratch, (unsigned char)num_ints);
  long total = add_nums(scratch, (unsigned char)num_ints);
  printf("Total is %ld\n", total);
  return 0;
}

long get_long(char *prompt) {
  char buf[32];
  printf("%s> ", prompt);
  if (!fgets(buf, sizeof(buf), stdin)) {
    exit(0);
  }
  buf[strcspn(buf, "\n")] = '\0';
  if (!strcmp("quit", buf)) {
    exit(0);
  }
  if (!strcmp("debug", buf)) {
    CALL_DEBUG();
    return get_long(prompt);
  }
  if (!strcmp("localdebug", buf)) {
    CALL_LOCAL_DEBUG();
    return get_long(prompt);
  }
  return atol(buf);
}

void get_data(long *array, unsigned char l) {
  int i = 0;
  char pbuf[32];
  while (i < l) {
    snprintf(pbuf, sizeof(pbuf), "long %d", i);
    *array = get_long(pbuf);
    array++;
    i++;
  }
}

long add_nums(long *array, unsigned char l) {
  long total = 0;
  for (int i=0;i<l;i++) {
    total += array[i];
  }
  return total;
}

void debug(const char *func, void *rbp, void *rsp) {
  void *p;
  printf("Debug in %s\n", func);
  printf("  main @ %p\n", &main);
  printf("  printf @ %p\n", &printf);
  printf("  malloc @ %p\n", &malloc);
  printf("\n");
  printf("RSP is %p, RBP is %p\n", rsp, rbp);
  printf("\n");
  for (p = rsp; p <= rbp; p += 0x10) {
    hexdump_line(p);
  }
}

void hexdump_line(void *addr) {
  char *p = addr;
  printf("%p | ", addr);
  for (int i=0;i<16;i++) {
    printf("%02x ", p[i] & 0xFF);
    if (i == 7)
      printf(" ");
  }
  printf("|");
  for (int i=0;i<16;i++) {
    char c = p[i];
    if (c >= 0x20 && c <= 0x7E)
      printf("%c", c);
    else
      printf(".");
  }
  printf("|\n");
}

void print_asciiart(const char *path) {
  FILE *fp=fopen(path, "r");
  if (!fp)
    return;
  char buf[64];
  while(fgets(buf, sizeof(buf), fp))
    printf("%s", buf);
  fclose(fp);
}
