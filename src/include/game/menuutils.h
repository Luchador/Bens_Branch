#ifndef IN_GAME_MENUUTILS_H
#define IN_GAME_MENUUTILS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

unsigned int colourBlend(unsigned int a, unsigned int b, unsigned int weight);
void menuTickTimers(void);
float menuGetSinOscFrac(float freq);
float menuGetCosOscFrac(float freq);
float menuGetLinearIntervalFrac(float freq);
float menuGetLinearOscPauseFrac(float frac);

#endif
