#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#define INP_BUF_SIZE 4096

char *prompt_user(const char *prompt);
char *prompt_user_key(const char *prompt);
void trim_inplace(char *buf);
