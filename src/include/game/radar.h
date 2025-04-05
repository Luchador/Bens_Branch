#pragma once

#include "data.h"
#include "types.h"

Gfx *radarRenderBackground(Gfx *gdl, struct textureconfig *tconfig, int arg2, int arg3, int arg4);
int radarGetTeamIndex(int team);
Gfx *radarDrawDot(Gfx *gdl, struct prop *prop, struct coord *dist, uint32_t colour1, uint32_t colour2, bool swapcolours);
Gfx *radarRender(Gfx *gdl);
Gfx *radarRenderRTrackedProps(Gfx *gdl);
