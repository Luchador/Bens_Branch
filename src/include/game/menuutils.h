#ifndef IN_GAME_MENUUTILS_H
#define IN_GAME_MENUUTILS_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

uint32_t colourBlend(uint32_t a, uint32_t b, uint32_t weight);
void menuTickTimers(void);
float menuGetSinOscFrac(float freq);
float menuGetCosOscFrac(float freq);
float menuGetLinearIntervalFrac(float freq);
float menuGetLinearOscPauseFrac(float frac);

#endif
