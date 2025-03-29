#ifndef IN_GAME_INV_H
#define IN_GAME_INV_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

extern struct noisesettings invnoisesettings_silent;
extern struct invaimsettings invaimsettings_default;
extern struct weapon *g_Weapons[WEAPON_SUICIDEPILL + 1];

void invReset(void);
void invInit(int numdoubles);

void invClear(void);
void invSortItem(struct invitem *item);
void invInsertItem(struct invitem *item);
void invRemoveItem(struct invitem *item);
struct invitem *invFindUnusedSlot(void);
void invSetAllGuns(bool enable);
struct invitem *invFindSingleWeapon(int weaponnum);
bool invHasSingleWeaponExcAllGuns(int weaponnum);
struct invitem *invFindDoubleWeapon(int weapon1, int weapon2);
bool invHasDoubleWeaponExcAllGuns(int weapon1, int weapon2);
bool invHasSingleWeaponOrProp(int weaponnum);
bool invHasSingleWeaponIncAllGuns(int weaponnum);
bool invHasDoubleWeaponIncAllGuns(int weapon1, int weapon2);
bool invGiveSingleWeapon(int weaponnum);
bool invGiveDoubleWeapon(int weapon1, int weapon2);
void invRemoveItemByNum(int weaponnum);
bool invGiveProp(struct prop *prop);
void invRemoveProp(struct prop *prop);
int invGiveWeaponsByProp(struct prop *prop);
void invChooseCycleForwardWeapon(int *weaponnum1, int *weaponnum2, bool arg2);
void invChooseCycleBackWeapon(int *weaponnum1, int *weaponnum2, bool arg2);
bool invHasKeyFlags(uint32_t wantkeyflags);
bool invHasBriefcase(void);
bool invHasDataUplink(void);
bool invHasProp(struct prop *prop);
int invGetCount(void);
struct invitem *invGetItemByIndex(int index);
struct textoverride *invGetTextOverrideForObj(struct defaultobj *obj);
struct textoverride *invGetTextOverrideForWeapon(int weaponnum);
int invGetWeaponNumByIndex(int index);
uint16_t invGetNameIdByIndex(int index);
char *invGetNameByIndex(int index);
char *invGetShortNameByIndex(int index);
void invInsertTextOverride(struct textoverride *override);
uint32_t invGetCurrentIndex(void);
void invSetCurrentIndex(uint32_t item);
void invCalculateCurrentIndex(void);
char *invGetPickupTextByObj(struct defaultobj *obj);
char *invGetPickupTextByWeaponNum(int weaponnum);
void invIncrementHeldTime(int param_1, int param_2);
void invGetWeaponOfChoice(int *weapon1, int *weapon2);

#endif
