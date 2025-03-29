#ifndef _IN_GAME_SPLAT_H
#define _IN_GAME_SPLAT_H
#include "data.h"
#include "types.h"

void splatTickChr(struct prop *prop);
void splatsCreateForChrHit(struct prop *prop, struct shotdata *arg1, struct coord *arg2, struct coord *arg3, int arg4, int arg5, struct chrdata *arg6);
int splatsCreate(int qty, float arg1, struct prop *prop, struct shotdata *arg3, struct coord *arg4, struct coord *arg5, bool isskedar, int arg7, int arg8, struct chrdata *arg9, int arg10);
void splatResetChr(struct chrdata *chr);

#endif
