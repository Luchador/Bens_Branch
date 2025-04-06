#include <ultra64.h>
#include <string.h>
#include "constants.h"
#include "lib/dma.h"
#include "data.h"
#include "types.h"

uint8_t g_LoadType = 0;

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
void *dmaExecWithAutoAlign(void *memaddr, romptr_t romaddr, uint32_t len)
{
	uintptr_t alignedrom = ALIGN2(romaddr);
	uintptr_t alignedmem = ALIGN16((uintptr_t) memaddr);
	uint32_t offset = romaddr - alignedrom; // 0 or 1
	uint32_t alignedlen = ALIGN16(offset + len);

	if (len == 0) {
		return (void *)(alignedmem + offset);
	}

	memcpy((void *)alignedmem, (const void *) alignedrom, alignedlen);

	return (void *)(alignedmem + offset);
}
