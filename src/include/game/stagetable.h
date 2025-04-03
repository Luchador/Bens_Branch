#ifndef _IN_GAME_STAGETABLE_H
#define _IN_GAME_STAGETABLE_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

struct stagetableentry *stageGetCurrent(void);
int stageGetIndex(int stagenum);

#endif
