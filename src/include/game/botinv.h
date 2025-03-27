#ifndef IN_GAME_BOTINV_H
#define IN_GAME_BOTINV_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void botinvClear(struct chrdata *chr);
struct invitem *botinvGetFreeSlot(struct chrdata *chr);
struct invitem *botinvGetItem(struct chrdata *chr, int weaponnum);
void botinvRemoveItem(struct chrdata *chr, int weaponnum);
uint32_t botinvGetItemType(struct chrdata *chr, uint32_t weaponnum);
bool botinvGiveSingleWeapon(struct chrdata *chr, uint32_t weaponnum);
void botinvGiveDualWeapon(struct chrdata *chr, uint32_t weaponnum);
int16_t botinvGetWeaponPad(struct chrdata *chr, uint32_t weaponnum);
bool botinvGiveProp(struct chrdata *chr, struct prop *prop);
void botinvScoreAllWeapons(struct chrdata *chr, int *weaponnums, int *scores1, int *scores2);
bool mpHasShield(void);
int mpGetWeaponSlotByWeaponNum(int weaponnum);
void botinvScoreWeapon(struct chrdata *chr, int weaponnum, int funcnum, int arg3, bool arg4, int *dst1, int *dst2, bool arg7, bool arg8);
void botinvScoreWeaponAgainstTarget(struct chrdata *chr, int weaponnum, int funcnum, int arg3, bool arg4, int *dst1, int *dst2);
void botinvScoreWeaponByItself(struct chrdata *chr, int weaponnum, int funcnum, int arg3, bool arg4, int *dst1, int *dst2);
int botinvGetDistConfig(int weaponnum, int funcnum);
bool botinvAllowsWeapon(struct chrdata *chr, int weaponnum, int funcnum);
void botinvTick(struct chrdata *chr);
bool botinvSwitchToWeapon(struct chrdata *chr, int weaponnum, int funcnum);
void botinvDrop(struct chrdata *chr, int weaponnum, uint8_t dropall);
void botinvDropAll(struct chrdata *chr, uint32_t weaponnum);
void botinvDropOne(struct chrdata *chr, uint32_t weaponnum);

#endif
