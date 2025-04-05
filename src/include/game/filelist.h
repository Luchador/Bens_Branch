#pragma once

#include "data.h"
#include "types.h"

void filelistUnload(void);
void filelistCreate(int listnum, uint8_t filetype);
int filelistFindOrCreate(uint8_t filetype);
void filelistInvalidatePak(int device);
void filelistsTick(void);
void filelistUpdate(struct filelist *list);