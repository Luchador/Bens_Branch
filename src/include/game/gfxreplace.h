#pragma once

#include "data.h"
#include "types.h"

void gfxReplaceGbiCommands(Gfx *gdl, Gfx *endgdl, int type);
void gfxReplaceGbiCommandsRecursively(struct roomblock *arg0, int type);

