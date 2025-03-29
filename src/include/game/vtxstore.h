#ifndef _IN_GAME_GAME_129210_H
#define _IN_GAME_GAME_129210_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void vtxstoreReset(void);

void vtxstoreFixRefs(void *find, void *replacement);
void vtxstoreTick(void);
void *vtxstoreAllocate(int count, int index, struct modelnode *node, int level);
void vtxstoreFree(int type, void *arg1);

#endif
