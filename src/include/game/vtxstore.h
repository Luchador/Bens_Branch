#pragma once

#include "data.h"
#include "types.h"

void vtxstoreReset(void);

void vtxstoreFixRefs(void *find, void *replacement);
void vtxstoreTick(void);
void *vtxstoreAllocate(int count, int index, struct modelnode *node, int level);
void vtxstoreFree(int type, void *arg1);

