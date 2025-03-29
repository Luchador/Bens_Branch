#ifndef IN_GAME_ZBUF_H
#define IN_GAME_ZBUF_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void *zbufGetAllocation(void);
void zbufReset(int stagenum);
void zbufAllocate(void);
Gfx *zbufConfigureRdp(Gfx *gdl);
Gfx *zbufClear(Gfx *gdl);
Gfx *zbufDrawArtifactsOffscreen(Gfx *gdl);

#endif
