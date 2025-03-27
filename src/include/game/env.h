#ifndef IN_GAME_ENV_H
#define IN_GAME_ENV_H
#include <ultra64.h>
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
void envApplyFogEnvironment(struct fogenvironment *sky);
void envApplyNoFogEnvironment(struct nofogenvironment *sky);
void envChooseAndApply(int stagenum);
void envApplyTransitionFrac(float arg0);
Gfx *envStartFog(Gfx *gdl, bool xlupass);
Gfx *envStopFog(Gfx *gdl);
bool envIsPosInFogMaxDistance(struct coord *pos, float tolerance);
struct distfadesettings *envGetDistFadeSettings(void);
int envGetObjShadeMode(struct prop *prop, float arg1[4]);

#endif
