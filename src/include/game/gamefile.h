#pragma once

#include "data.h"
#include "types.h"

uint32_t gamefileHasFlag(uint32_t value);
void gamefileSetFlag(uint32_t value);
void gamefileUnsetFlag(uint32_t value);
void gamefileApplyOptions(struct gamefile *file);
void gamefileLoadDefaults(struct gamefile *file);
int gamefileLoad(int device);
int gamefileSave(int device, int filenum, uint16_t deviceserial);
void gamefileGetOverview(char *arg0, char *name, uint8_t *stage, uint8_t *difficulty, uint32_t *time);
void gamefileUnlockEverything(void);