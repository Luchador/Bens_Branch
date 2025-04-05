#pragma once

#include "data.h"
#include "types.h"

void weatherReset(void);

void weatherTick(void);

Gfx *weatherRender(Gfx *gdl);
void weatherSetBoundaries(struct weatherparticledata *data, int index, float min, float max);
struct weatherparticledata *weatherAllocateParticles(void);
void weatherRollLightning(struct weatherdata *weather);
void func0f131678(int arg0);
void weatherSetIntensity(int intensity);
void weatherTickRain(struct weatherdata *weather);
void weatherTickSnow(struct weatherdata *weather);
void weatherConfigureRain(uint32_t intensity);
void weatherConfigureSnow(uint32_t intensity);
bool weatherIsRoomWeatherProof(int room);
Gfx *weatherRenderRain(Gfx *gdl, struct weatherdata *weather, int arg2);
Gfx *weatherRenderSnow(Gfx *gdl, struct weatherdata *weather, int arg2);
void weatherStop(void);
