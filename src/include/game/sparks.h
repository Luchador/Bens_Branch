#pragma once

#include "data.h"
#include "types.h"

void sparksReset(void);

void sparksTick(void);

void sparkCreate(struct coord *pos, struct sparktype *type);
void sparkgroupEnsureFreeSparkSlot(struct sparkgroup *group);
void sparksCreate(int room, struct prop *prop, struct coord *pos, struct coord *arg3, struct coord *dir, int type);
Gfx *sparksRender(Gfx *gdl);
