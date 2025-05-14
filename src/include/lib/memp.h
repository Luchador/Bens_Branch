#pragma once

#include "data.h"
#include "types.h"

void mempSetHeap(uint8_t *heapstart, uint32_t heaplen);
void *mempGetNextStageAllocation(void);
void *mempAlloc(uint32_t len, uint8_t pool);
int mempRealloc(void *allocation, int newsize, uint8_t poolnum);
uint32_t mempGetPoolFree(uint8_t poolnum, uint32_t bank);
void mempResetPool(uint8_t pool);
void mempDisablePool(uint8_t pool);
void *mempAllocFromRight(uint32_t len, uint8_t pool);
