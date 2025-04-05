#pragma once

#include "data.h"
#include "types.h"

bool botroomFindPos(RoomNum room, struct coord *pos, float *angleptr, int *padnumptr, int *covernumptr);

bool botIsDizzy(struct chrdata *chr);
void botReset(struct chrdata *chr, uint8_t respawning);
void botSpawn(struct chrdata *chr, uint8_t full);
void botSpawnAll(void);
uint32_t add87654321(uint32_t value);
uint32_t botPickupProp(struct prop *prop, struct chrdata *chr);
bool botTestPropForPickup(struct prop *prop, struct chrdata *chr);
void botCheckPickups(struct chrdata *chr);
int botGuessCrouchPos(struct chrdata *chr);
bool botApplyMovement(struct chrdata *chr);
bool botIsAboutToAttack(struct chrdata *chr, bool arg1);
int botTick(struct prop *prop);
float botCalculateMaxSpeed(struct chrdata *chr);
void botUpdateSmoothedMovement(struct chrdata *chr, float *move, int numupdates, float arg3);
void botDisarm(struct chrdata *chr, struct prop *attacker);
void botSetTarget(struct chrdata *chr, int propnum);
bool botIsTargetInvisible(struct chrdata *botchr, struct chrdata *otherchr);
bool botHasGround(struct chrdata *chr);
void bot0f192a74(struct chrdata *chr);
bool botPassesPeaceCheck(struct chrdata *botchr, struct chrdata *otherchr);
bool botPassesCowardCheck(struct chrdata *botchr, struct chrdata *otherchr);
void botChooseGeneralTarget(struct chrdata *chr);
void botScheduleReload(struct chrdata *chr, int handnum);
struct prop *botFindPickup(struct chrdata *chr, int criteria);
int botGetNumOpponentsInHill(struct chrdata *chr);
void botTickUnpaused(struct chrdata *chr);
int botIsObjCollectable(struct defaultobj *obj);
int botGetWeaponNum(struct chrdata *chr);
uint8_t botGetTargetsWeaponNum(struct chrdata *chr);
char *botGetCommandName(int command);
void botApplyAttack(struct chrdata *chr, struct prop *prop);
void botApplyFollow(struct chrdata *chr, struct prop *prop);
void botApplyProtect(struct chrdata *chr, struct prop *prop);
void botApplyDefend(struct chrdata *chr, struct coord *pos, RoomNum *room, float angle);
void botApplyHold(struct chrdata *chr, struct coord *pos, RoomNum *room, float angle);
void botApplyScenarioCommand(struct chrdata *chr, uint32_t arg1);
bool botCanFollow(struct chrdata *leader, struct chrdata *follower);
int botFindTeammateToFollow(struct chrdata *chr, float range);
bool botCanDoCriticalPickup(struct chrdata *chr);
struct prop *botFindDefaultPickup(struct chrdata *chr);
struct prop *botFindAnyPickup(struct chrdata *chr);
int botGetTeamSize(struct chrdata *chr);
int botGetCountInTeamDoingCommand(struct chrdata *self, uint32_t command, bool includeself);
int botIsChrsCtcTokenHeld(struct chrdata *chr);
bool botShouldReturnCtcToken(struct chrdata *chr);
int botGetNumTeammatesDefendingHill(struct chrdata *bot);
void botCheckFetch(struct chrdata *chr);