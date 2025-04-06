#pragma once

#include "types.h"

void *zbufGetAllocation(void);
void zbufReset(int stagenum);
void zbufAllocate(void);
Gfx *zbufConfigureRdp(Gfx *gdl);
Gfx *zbufDrawArtifactsOffscreen(Gfx *gdl);
