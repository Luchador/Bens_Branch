#ifndef IN_GAME_PAD_H
#define IN_GAME_PAD_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void padUnpack(int padnum, uint32_t fields, struct pad *pad);
bool padHasBboxData(int padnum);
void padGetCentre(int padnum, struct coord *coord);
void padRotateForDoor(int padnum);
void padCopyBboxFromPad(int padnum, struct pad *src);
void padSetFlag(int padnum, uint32_t flag);
void padUnsetFlag(int padnum, uint32_t flag);
int coverGetCount(void);
bool coverUnpack(int covernum, struct cover *cover);
bool coverIsInUse(int covernum);
void coverSetInUse(int covernum, bool enable);
void coverSetFlag(int covernum, uint32_t flag);
void coverUnsetFlag(int covernum, uint32_t flag);
void coverSetOutOfSight(int covernum, bool enable);
bool coverIsSpecial(struct cover *cover);

#endif
