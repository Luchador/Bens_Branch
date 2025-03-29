#ifndef IN_GAME_SHARDS_H
#define IN_GAME_SHARDS_H
#include "data.h"
#include "types.h"

void shardsCreate(struct coord *pos, float *rotx, float *roty, float *rotz, float xmin, float xmax, float ymin, float ymax, int type, struct prop *prop);
void shardsReset(void);
Gfx *shardsRender(Gfx *gdl);
void shardsStop(void);
void shardsTick(void);

#endif
