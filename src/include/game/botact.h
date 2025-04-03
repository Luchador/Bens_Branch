#ifndef IN_GAME_BOTACT_H
#define IN_GAME_BOTACT_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

int botactGetAmmoTypeByFunction(int weaponnum, int funcnum);
int botactGetClipCapacityByFunction(int weaponnum, u32 funcnum);
void botactReload(struct chrdata *chr, int handnum, bool withsound);
int botactGetAmmoQuantityByWeapon(struct aibot *aibot, int weaponnum, int funcnum, bool include_equipped);
int botactGetAmmoQuantityByType(struct aibot *aibot, int ammotype, bool include_equipped);
int botactTryRemoveAmmoFromReserve(struct aibot *aibot, int weaponnum, int funcnum, int qty);
void botactGiveAmmoByWeapon(struct aibot *aibot, int weaponnum, int funcnum, int qty);
void botactGiveAmmoByType(struct aibot *aibot, u32 ammotype, int quantity);
bool botactShootFarsight(struct chrdata *chr, int arg1, struct coord *arg2, struct coord *arg3);
bool botactIsWeaponThrowable(int weaponnum, bool is_secondary);
u32 botactGetProjectileThrowInterval(u32 weapon);
int botactGetWeaponByAmmoType(int ammotype);
void botactThrow(struct chrdata *chr);
int botactGetShootInterval60(int weaponnum, int funcnum);
bool botactFindRocketRoute(struct chrdata *chr, struct coord *frompos, struct coord *topos, RoomNum *fromrooms, RoomNum *torooms, struct projectile *projectile);
void botactGetRocketNextStepPos(u16 padnum, struct coord *pos);
void botactCreateSlayerRocket(struct chrdata *chr);

#endif
