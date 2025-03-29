#ifndef _IN_LIB_DMA_H
#define _IN_LIB_DMA_H
#include "data.h"
#include "types.h"

void dmaInit(void);
void dmaStart(void *memaddr, romptr_t romaddr, uint32_t len, bool priority);
void dmaExec(void *memaddr, romptr_t romaddr, uint32_t len);
void dmaExecHighPriority(void *memaddr, romptr_t romaddr, uint32_t len);
void *dmaExecWithAutoAlign(void *memaddr, romptr_t romaddr, uint32_t len);

#endif
