#ifndef __PASSWORD_H__
#define __PASSWORD_H__

#include <stdint.h>

// We can store 24 bits of data
void print_password(uint8_t a, uint8_t b, uint8_t c);
uint8_t read_password(uint8_t *a, uint8_t *b, uint8_t *c);
#endif
