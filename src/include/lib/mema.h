#ifndef _IN_LIB_MEMA_H
#define _IN_LIB_MEMA_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void memaDefrag(void);
void memaReset(void *ptr, uint64_t size);
void memaPrint(void);
void *memaAlloc(uint64_t size);
uintptr_t memaGrow(uintptr_t addr, uint64_t amount);
void _memaFree(uintptr_t addr, uint64_t size);
void memaFree(void *addr, uint64_t size);

#endif
