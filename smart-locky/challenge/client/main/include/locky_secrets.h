#include <sys/types.h>

#define SECRET_PSK 1
#define SECRET_PASSWD 3
#define SECRETS _esp32_def_ptr

#ifndef _SECRET_HOLDER_
extern void *SECRETS[];
#endif
