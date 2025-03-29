#ifndef _IN_GAME_TIMING_H
#define _IN_GAME_TIMING_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void frametimeInit(void);
void frametimeApply(int diffframe60, int diffframe240, int frametime);
void frametimeCalculate(void);
void func0f16cf94(void);

#endif
