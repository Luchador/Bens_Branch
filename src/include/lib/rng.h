#ifndef _IN_LIB_RNG_H
#define _IN_LIB_RNG_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

uint32_t rngRandom(void);
uint32_t rngRotateSeed(uint64_t *value);

#endif
