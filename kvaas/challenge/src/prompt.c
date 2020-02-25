#include "prompt.h"
#include "nodes.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char *_inp_buf = NULL;
static char *_key_buf = NULL;

char *prompt_user(const char *prompt) {
  if (!_inp_buf)
    _inp_buf = malloc(INP_BUF_SIZE);
  printf("%s> ", prompt);
  if (!fgets(_inp_buf, INP_BUF_SIZE, stdin))
    return NULL;
  trim_inplace(_inp_buf);
  return _inp_buf;
}

char *prompt_user_key(const char *key_prompt) {
  if (key_prompt == NULL)
    key_prompt = "key name";
  if (!_key_buf)
    _key_buf = malloc(NODE_NAME_SZ+1);
  printf("%s> ", key_prompt);
  if (!fgets(_key_buf, NODE_NAME_SZ+1, stdin))
    return NULL;
  trim_inplace(_key_buf);
  return _key_buf;
}

void trim_inplace(char *buf) {
  char *end = buf + strlen(buf) - 1;
  while(isspace(*end)){
    end--;
  }
  end++;
  char *begin = buf;
  while(isspace(*begin)) {
    begin++;
  }
  if (end <= begin) {
    buf[0] = '\0';
    return;
  }
  size_t len = end-begin;
  if (buf != begin)
    memmove(buf, begin, len);
  buf[len] = '\0';
}
