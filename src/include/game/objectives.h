#ifndef IN_GAME_OBJECTIVES_H
#define IN_GAME_OBJECTIVES_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void objectivesStop(void);

void objectivesReset(void);
void tagInsert(struct tag *tag);
void briefingInsert(struct briefingobj *obj);
void objectiveInsert(struct objective *objective);
void objectiveAddRoomEnteredCriteria(struct criteria_roomentered *criteria);
void objectiveAddThrowInRoomCriteria(struct criteria_throwinroom *criteria);
void objectiveAddHolographCriteria(struct criteria_holograph *criteria);

uint32_t xorBaffbeff(uint32_t value);
uint32_t xorBabeffff(uint32_t value);
uint32_t xorBoobless(uint32_t value);
void tagsReset(void);
int objGetTagNum(struct defaultobj *obj);
int objectiveGetCount(void);
uint32_t objectiveGetDifficultyBits(int index);
int objectiveCheck(int index);
bool objectiveIsAllComplete(void);
void objectivesDisableChecking(void);
void objectivesShowHudmsg(char *buffer, int hudmsgtype);
void objectivesCheckAll(void);
void objectiveCheckRoomEntered(int currentroom);
void objectiveCheckThrowInRoom(int arg0, RoomNum *requiredrooms);
void objectiveCheckHolograph(float sqdist);
struct prop *chopperGetTargetProp(struct chopperobj *heli);
struct defaultobj *objFindByTagId(int tag_id);
struct tag *tagFindById(int tag_id);

#endif
