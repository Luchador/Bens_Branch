#ifndef _IN_MOD_H
#define _IN_MOD_H

#include <stdint.h>

#define MOD_CONFIG_FNAME "modconfig.txt"

struct animtableentry;

int modConfigLoad(const char *path);

int modTextureLoad(uint16_t num, void *dst, uint32_t dstSize);

int modAnimationLoadDescriptor(uint16_t num, struct animtableentry *anim);
void *modAnimationLoadData(uint16_t num);

void *modSequenceLoad(uint16_t num, uint32_t *outSize);

#endif
