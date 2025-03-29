#ifndef IN_GAME_FILELIST_H
#define IN_GAME_FILELIST_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void func0f110bf8(void);
void filelistCreate(int listnum, uint8_t filetype);
int filelistFindOrCreate(uint8_t filetype);
void filelistInvalidatePak(int device);
void filelistsTick(void);
void filelistUpdate(struct filelist *list);

#endif
