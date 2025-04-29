#pragma once

#include "data.h"
#include "types.h"

struct distfadesettings {
    float opaperc;
    float xluperc;
    float refdist;
};

struct environment *envGetCurrent(void);
float envGetSquaredFogMax(void);
void envTick(void);
void envApplyEnvironment(struct environment *sky);
void envChooseAndApply(int stagenum);
//void envApplyTransitionFrac(float arg0);
Gfx *envStartFog(Gfx *gdl);
Gfx *envStopFog(Gfx *gdl);
bool envIsWithinFogRenderDistance(struct coord *pos, float tolerance);
struct distfadesettings *envGetDistFadeSettings(void);
int envGetObjShadeMode(struct prop *prop, float arg1[4]);