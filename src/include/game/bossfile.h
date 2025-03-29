#ifndef IN_GAME_BOSSFILE_H
#define IN_GAME_BOSSFILE_H
#include "data.h"
#include "types.h"

bool bossfileLoadFull(void);
uint32_t bossfileFindFileId(void);
void bossfileLoad(void);
void bossfileSave(void);
void bossfileSetDefaults(void);

#endif
