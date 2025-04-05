#pragma once

#include "data.h"
#include "types.h"

int botactGetAmmoTypeByFunction(int weaponnum, int funcnum);
int botactGetClipCapacityByFunction(int weaponnum, uint32_t funcnum);
void botactReload(struct chrdata *chr, int handnum, bool withsound);
int botactGetAmmoQuantityByWeapon(struct aibot *aibot, int weaponnum, int funcnum, bool include_equipped);
int botactGetAmmoQuantityByType(struct aibot *aibot, int ammotype, bool include_equipped);
int botactTryRemoveAmmoFromReserve(struct aibot *aibot, int weaponnum, int funcnum, int qty);
void botactGiveAmmoByWeapon(struct aibot *aibot, int weaponnum, int funcnum, int qty);
void botactGiveAmmoByType(struct aibot *aibot, uint32_t ammotype, int quantity);
bool botactShootFarsight(struct chrdata *chr, int arg1, struct coord *arg2, struct coord *arg3);
bool botactIsWeaponThrowable(int weaponnum, bool is_secondary);
uint32_t botactGetProjectileThrowInterval(uint32_t weapon);
int botactGetWeaponByAmmoType(int ammotype);
void botactThrow(struct chrdata *chr);
int botactGetShootInterval60(int weaponnum, int funcnum);
bool botactFindRocketRoute(struct chrdata *chr, struct coord *frompos, struct coord *topos, RoomNum *fromrooms, RoomNum *torooms, struct projectile *projectile);
void botactGetRocketNextStepPos(uint16_t padnum, struct coord *pos);
void botactCreateSlayerRocket(struct chrdata *chr);

