#ifndef _IN_ROMDATA_H
#define _IN_ROMDATA_H

#include <stdint.h>

extern uint8_t *g_RomFile;
extern uint32_t g_RomFileSize;

int romdataInit(void);

uint8_t *romdataFileLoad(int fileNum, uint32_t *outSize);
void romdataFilePreprocess(int fileNum, int loadType, uint8_t *data, uint32_t size, uint32_t *outSize);
void romdataFileFree(int fileNum);
const char *romdataFileGetName(int fileNum);

uint8_t *romdataFileGetData(int fileNum);
int romdataFileGetSize(int fileNum);

int romdataFileGetNumForName(const char *name);

uint8_t *romdataSegGetData(const char *segName);
uint8_t *romdataSegGetDataEnd(const char *segName);
uint32_t romdataSegGetSize(const char *segName);
uint32_t romdataFileGetEstimatedSize(const uint32_t size, const uint32_t loadtype);

int romdataCheckGbcRom(void);

#endif
