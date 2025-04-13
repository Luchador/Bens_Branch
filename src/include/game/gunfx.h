#pragma once

#include "data.h"
#include "types.h"

void boltbeamsReset(void);
void lasersightsReset(void);

void beamCreate(struct beam *beam, int weaponnum, struct coord *from, struct coord *to);
void beamCreateForHand(int handnum);
Gfx *beamRenderGeneric(Gfx *gdl, struct textureconfig *arg1, float arg2, struct coord *arg3, uint32_t colour1, float arg5, struct coord *arg6, uint32_t colour2);
Gfx *beamRender(Gfx *gdl, struct beam *beam, bool arg2, uint8_t arg3);
void beamTick(struct beam *beam);

struct casing *casingCreate(struct modeldef *modeldef, Mtxf *mtx);
void casingCreateForHand(int handnum, float ground, Mtx *mtx);
void casingRender(struct casing *casing, Gfx **gdlptr);
void casingsRender(Gfx **gdlptr);

int boltbeamFindByProp(struct prop *prop);
int boltbeamCreate(struct prop *prop);
void boltbeamSetHeadPos(int beamnum, struct coord *pos);
void boltbeamSetTailPos(int beamnum, struct coord *pos);
void boltbeamIncrementHeadPos(int beamnum, float newlength, bool arg2);
void boltbeamSetAutomatic(int beamnum, float speed);
Gfx *boltbeamsRender(Gfx *gdl);
void boltbeamsTick(void);

bool lasersightExists(int id, int *index);
Gfx *lasersightRenderDot(Gfx *gdl);
Gfx *lasersightRenderBeam(Gfx *gdl);
void lasersightSetBeam(int id, int arg1, struct coord *near, struct coord *far);
void lasersightSetDot(int arg0, struct coord *pos, struct coord *rot);
void lasersightFree(int arg0);
