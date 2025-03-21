#include <ultra64.h>
#include "constants.h"
#include "game/file.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/rzip.h"
#include "data.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/debug.h"
#ifndef PLATFORM_N64
#include "system.h"
#endif

/**
 * This file contains functions relating to ROM asset files.
 *
 * Asset files include:
 * - BG segment files
 * - Lang files
 * - MP3 files
 * - Model files
 * - Pad files
 * - Stage setup files
 * - Tile files
 *
 * The following are not implemented as asset files and are therefore not
 * managed here:
 * - Animations
 * - Music
 * - Textures
 *
 * The file system does not keep track of which files are loaded, nor does it
 * maintain a list of pointers to loaded file data. All load operations either
 * require the caller to pass a destination pointer, or the file system can make
 * its own allocation and return the pointer. It does not store the pointer in
 * either case.
 *
 * Most file types are compressed. This is abstracted away, so from the caller's
 * perspect they just call a load function and they receive an inflated file.
 * Exceptions to this are:
 * - BG files, which contain multiple compressed parts. The caller uses
 *   fileLoadPartToAddr which loads a slice of the file without inflation.
 * - MP3 files, which are not compressed. The caller retrieves the ROM start and
 *   end addresses from the file system, then gives that to the MP3 system which
 *   does its own DMA operations.
 *
 * It is likely that during development files could be alternatively loaded
 * from the host computer. This code no longer exists, but there are some unused
 * functions that support this theory.
 */

/**
 * Currently the port just takes the filename from the filename[] array and feeds it
 * into its own FS API that just loads it from disk. The whole file number/slot system
 * is still mostly intact as a kludge. This will probably be removed later and reworked
 * to just use filenames.
 */

struct fileinfo g_FileInfo[NUM_FILES];

uintptr_t g_FileTable[NUM_FILES + 1]; // TODO: this is only used to get the filenum, remove this

romptr_t fileGetRomAddress(s32 filenum)
{
	return (romptr_t) romdataFileGetData(filenum);
}

u32 fileGetRomSizeByTableAddress(uintptr_t *filetableaddr)
{
	const s32 size = romdataFileGetSize(filetableaddr - g_FileTable);
	return (size < 0) ? 0 : size;
}

s32 fileGetRomSize(s32 filenum)
{
	return fileGetRomSizeByTableAddress((uintptr_t*)&g_FileTable[filenum]);
}

u32 file0f166ea8(uintptr_t *filetableaddr)
{
	return 0;
}

void fileLoad(u8 *dst, u32 allocationlen, romptr_t *romaddrptr, struct fileinfo *info)
{
	// load the file first
	const s32 filenum = (uintptr_t *)romaddrptr - g_FileTable;
	u32 romsize = 0;
	u8 *filedata = romdataFileLoad(filenum, &romsize);
	if (!filedata) {
		return;
	}
	romaddrptr = (romptr_t *)&filedata;

	u8 buffer[5 * 1024];

	if (allocationlen == 0) {
		// DMA with no inflate
		dmaExec(dst, *romaddrptr, romsize);
	} else {
		// DMA the compressed data to scratch space then inflate
		u8 *scratch = (dst + allocationlen) - ((romsize + 7) & (uintptr_t)~7);

		if ((uintptr_t)scratch - (uintptr_t)dst < 8) {
			info->loadedsize = 0;
		} else {
			s32 result;

			dmaExec(scratch, *romaddrptr, romsize);
			result = rzipInflate(scratch, dst, buffer);

			result = ALIGN16(result);

			info->loadedsize = result;
		}
	}

	// byteswap/preprocess file according to g_LoadType right after inflating it
	const u32 dstsize = allocationlen ? info->loadedsize : romsize; 
	romdataFilePreprocess(filenum, g_LoadType, dst, dstsize, &info->loadedsize);
	g_LoadType = LOADTYPE_NONE;
}

void filesInit(void)
{
	s32 i;
	s32 j = 0;

	for (i = 1, j = 0; i < NUM_FILES; i++) {
		struct fileinfo *info = g_FileInfo + i;
		j = i;

		info->loadedsize = 0;
		info->allocsize = 0;

		if (g_FileTable);
		if (g_FileInfo);
	}
}

void fileLoadPartToAddr(u16 filenum, void *memaddr, s32 offset, u32 len)
{

	if (fileGetRomSizeByTableAddress((uintptr_t*)&g_FileTable[filenum])) {
		const u8 *src = romdataFileGetData(filenum);
		if (src) {
			dmaExec(memaddr, (uintptr_t) src + offset, len);
		}
		// this intentionally does not execute romdataFilePreprocess,
		// because bg files are loaded and inflated in parts
	}
}

u32 fileGetInflatedSize(s32 filenum, u32 loadtype)
{
	u8 *ptr;
	u8 buffer[0x50];
	uintptr_t *romaddrptr;
	uintptr_t romaddr;

	romaddrptr = &g_FileTable[filenum];

	romaddr = (uintptr_t)romdataFileGetData(filenum);
	ptr = (u8 *) ((uintptr_t) &buffer[0x10] & ~0xf);

	if (romaddr == 0) {
	} else {
		dmaExec(ptr, romaddr, 0x40);
	}

	if (rzipIs1173(ptr)) {
		return romdataFileGetEstimatedSize((ptr[2] << 16) | (ptr[3] << 8) | ptr[4], loadtype);
	}

	return 0;
}

void *fileLoadToNew(s32 filenum, u32 method, u32 loadtype)
{
	struct fileinfo *info = &g_FileInfo[filenum];
	void *ptr;

	if (method == FILELOADMETHOD_EXTRAMEM || method == FILELOADMETHOD_DEFAULT) {
		if (info->loadedsize == 0) {
			info->loadedsize = (fileGetInflatedSize(filenum, loadtype) + 0x20) & 0xfffffff0;

			if (method == FILELOADMETHOD_EXTRAMEM) {
				info->loadedsize += 0x8000;
			}
		}

		ptr = mempAlloc(info->loadedsize, MEMPOOL_STAGE);
		info->allocsize = info->loadedsize;
		fileLoad(ptr, info->loadedsize, (uintptr_t*)&g_FileTable[filenum], info);

		if (method != FILELOADMETHOD_EXTRAMEM) {
			mempRealloc(ptr, info->loadedsize, MEMPOOL_STAGE);
		}
	} else {
		while (1);
	}

	return ptr;
}

void fileRemove(s32 filenum)
{
	g_FileTable[filenum] = 0;
	romdataFileFree(filenum);
}

void *fileLoadToAddr(s32 filenum, s32 method, u8 *ptr, u32 size)
{
	struct fileinfo *info = &g_FileInfo[filenum];

	if (method == FILELOADMETHOD_EXTRAMEM || method == FILELOADMETHOD_DEFAULT) {
		info->allocsize = size;
		fileLoad(ptr, size, (uintptr_t*)&g_FileTable[filenum], info);
	} else {
		while (1);
	}

	return ptr;
}

u32 fileGetLoadedSize(s32 filenum)
{
	return g_FileInfo[filenum].loadedsize;
}

u32 fileGetAllocationSize(s32 filenum)
{
	return g_FileInfo[filenum].allocsize;
}

void fileSetSize(s32 filenum, void *ptr, u32 size, bool reallocate)
{
	g_FileInfo[filenum].loadedsize = size;
	g_FileInfo[filenum].allocsize = size;

	if (reallocate) {
		mempRealloc(ptr, g_FileInfo[filenum].loadedsize, MEMPOOL_STAGE);
	}
}

void filesStop(u8 arg0)
{
	s32 i;

	// Minus 1 because the last entry in the file table is just a marker
	for (i = 1; i < ARRAYCOUNT(g_FileTable) - 1; i++) {
		if (arg0 == 4) {
			g_FileInfo[i].loadedsize = 0;
		}
	}
}

TextData* loadFileIntoMemory(const char *filename) {
	FILE *file = fopen(filename, "r");
	if(!file) {
		debug_log("Error opening file\n", 0);
		return NULL;
	}

	TextData *filedata = malloc(sizeof(TextData));
	filedata->lines = NULL;
	filedata-> count = 0;

	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, sizeof(buffer), file)) {
		filedata->count++;

		filedata->lines = realloc(filedata->lines, filedata->count * sizeof(char *));
		filedata->lines[filedata->count - 1] = strdup(buffer);
	}

	fclose(file);
	return filedata;
}

char* buildDynamicPath(const char *directory, const char *filename) {
    size_t len = strlen(directory) + strlen(filename) + 2; // +1 for `/`, +1 for `\0`
    char *path = malloc(len);
    if (!path) return NULL;

    snprintf(path, len, "%s/%s", directory, filename);
    return path;
}