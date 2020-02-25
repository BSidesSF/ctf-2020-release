#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

int nibble(char c);

// urldecode in place
size_t urldecode(char *p) {
  char *start = p;
  char *s = p;
  int u, l;
  char c, d;
  while(*s) {
    switch (*s) {
      case '+':
        s++;
        *p++ = ' ';
        break;
      case '%':
        s++;
        if (!*s) {
          *p++ = '%';
          goto done;
        }
        c = *s++;
        u = nibble(c);
        if (!*s) {
          *p++ = '%';
          *p++ = c;
          goto done;
        }
        d = *s++;
        l = nibble(d);
        if (u == -1 || l == -1) {
          *p++ = '%';
          *p++ = c;
          *p++ = d;
        } else {
          *p++ = (char)((u << 4) | l);
        }
        break;
      default:
        *p++ = *s++;
        break;
    }
  }
done:
    *p = '\0';
    return p-start;
}

int nibble(char c) {
  if ((c >= '0') && (c <= '9'))
    return c - '0';
  if ((c >= 'a') && (c <= 'f'))
    return c - 'a' + 10;
  if ((c >= 'A') && (c <= 'F'))
    return c - 'A' + 10;
  return -1;
}

#ifdef URLDECODE_TEST
void compare_decoded_raw(char *full, char *dec, size_t s) {
  char *out = strdup(full);
  size_t sz = urldecode(out);
  if (strcmp(out, dec)) {
    printf("%s decoded to %s, not %s\n", full, out, dec);
  }
  if (sz != s) {
    printf("Expected %d, got %d.\n", s, sz);
  }
  free(out);
}

#define compare_decoded(a, b) compare_decoded_raw(a, b, strlen(b))

int main(int argc, char **argv) {
  compare_decoded("ABC", "ABC");
  compare_decoded("%41%41%42%42%41", "AABBA");
  compare_decoded("%YYZ", "%YYZ");
  compare_decoded("%%%", "%%%");
  compare_decoded("%A", "%A");
  compare_decoded("%41", "A");
  compare_decoded("123%", "123%");
  compare_decoded_raw("00%0033", "00", 5);
  return 0;
}
#endif
