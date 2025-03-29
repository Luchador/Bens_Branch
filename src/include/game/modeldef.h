#ifndef IN_GAME_MODELDEF_H
#define IN_GAME_MODELDEF_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void modeldef0f1a7560(struct modeldef *modeldef, uint16_t filenum, uint32_t arg2, struct modeldef *modeldef2, struct texpool *texpool, bool arg5);
void modelPromoteTypeToPointer(struct modeldef *modeldef);
struct modeldef *modeldefLoad(uint16_t fileid, uint8_t *arg1, int arg2, struct texpool *arg3);
struct modeldef *modeldefLoadToNew(uint16_t fileid);

#endif
