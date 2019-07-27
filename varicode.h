#ifndef VARICODE_H
#define VARICODE_H

#include <inttypes.h>

// Returns 0 on failure.
uint16_t asc2varicode(char ch);

// Returns -1 on failure.
int varicode2asc(uint16_t vc);

#endif
