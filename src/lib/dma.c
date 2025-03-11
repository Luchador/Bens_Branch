#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/crash.h"
#include "lib/dma.h"
#include "data.h"
#include "types.h"

volatile u32 g_DmaNumSlotsBusy;
OSIoMesg g_DmaIoMsgs[32];
volatile u8 g_DmaSlotsBusy[32];
OSMesg g_DmaMesgs[32];
OSMesgQueue g_DmaMesgQueue;

u8 g_LoadType = 0;

void dmaInit(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_DmaSlotsBusy); i++) {
		g_DmaSlotsBusy[i] = 0;
	}

	g_DmaNumSlotsBusy = 0;

	osCreateMesgQueue(&g_DmaMesgQueue, g_DmaMesgs, ARRAYCOUNT(g_DmaMesgs));
}

void dmaStart(void *memaddr, romptr_t romaddr, u32 len, bool priority)
{
	bcopy((const void *)romaddr, memaddr, len);
}

void dmaExec(void *memaddr, romptr_t romaddr, u32 len)
{
	dmaStart(memaddr, romaddr, len, false);
}

void dmaExecHighPriority(void *memaddr, romptr_t romaddr, u32 len)
{
	dmaStart(memaddr, romaddr, len, true);
}

/**
 * DMA data from ROM to RAM with automatic alignment.
 *
 * The destination memory address is aligned to 0x10.
 *
 * The ROM address is aligned to 2 bytes (ie. subtract 1 if ROM address is odd).
 * If this is done then the returned memory pointer is bumped forwards by one
 * to compensate. The length of data to be transferred is also increased by one
 * to make it 2-byte aligned.
 *
 * It is assumed that the passed len is 2-byte aligned (ie. an even number).
 *
 * If a length of zero is passed, no DMA is done. This can be used to retrieve
 * the memory address that would have been returned.
 */
void *dmaExecWithAutoAlign(void *memaddr, romptr_t romaddr, u32 len)
{
	uintptr_t alignedrom = ALIGN2(romaddr);
	uintptr_t alignedmem = ALIGN16((uintptr_t) memaddr);
	u32 offset = romaddr - alignedrom; // 0 or 1
	u32 alignedlen = ALIGN16(offset + len);

	if (len == 0) {
		return (void *)(alignedmem + offset);
	}

	dmaExec((void *)alignedmem, alignedrom, alignedlen);

	return (void *)(alignedmem + offset);
}
