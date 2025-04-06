#pragma once

#include <stdint.h>
#include "data.h"
#include "types.h"
#include "romdata.h"

romptr_t fileGetRomAddress(int filenum);
int fileGetRomSize(int filenum);
void filesInit(void);
void fileLoadPartToAddr(uint16_t filenum, void *memaddr, int offset, unsigned int len);
unsigned int fileGetInflatedSize(int filenum, unsigned int loadtype);
void *fileLoadToNew(int filenum, unsigned int method, unsigned int loadtype);
void *fileLoadToAddr(int filenum, int method, uint8_t *ptr, unsigned int size);
unsigned int fileGetLoadedSize(int filenum);
unsigned int fileGetAllocationSize(int filenum);
void fileSetSize(int filenum, void *ptr, unsigned int size, bool reallocate);
void filesStop(uint8_t arg0);
TextData* loadFileIntoMemory(const char *filename);
char* buildDynamicPath(const char *directory, const char *filename);
