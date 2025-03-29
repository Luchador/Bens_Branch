#ifndef IN_GAME_GFXREPLACE_H
#define IN_GAME_GFXREPLACE_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void gfxReplaceGbiCommands(Gfx *gdl, Gfx *endgdl, int type);
void gfxReplaceGbiCommandsRecursively(struct roomblock *arg0, int type);

#endif
