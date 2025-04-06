#pragma once

#include "data.h"
#include "types.h"

struct weapon *weaponFindById(int itemid);
struct weaponfunc *weaponGetFunctionById(uint32_t weaponnum, uint32_t which);
struct weaponfunc *gsetGetWeaponFunction2(struct gset *gset);
struct weaponfunc *gsetGetWeaponFunction(struct gset *gset);
struct weaponfunc *weaponGetFunction(struct gset *gset, int which);
struct weaponfunc *currentPlayerGetWeaponFunction(uint32_t hand);
struct invaimsettings *gsetGetAimSettings(struct gset *gset);
struct inventory_ammo *weaponGetAmmoByFunction(uint32_t weaponnum, uint32_t funcnum);
float handGetXShift(int handnum);
float func0f0b131c(int handnum);
float currentPlayerGetGunZoomFov(void);
void currentPlayerZoomOut(float fovpersec);
void currentPlayerZoomIn(float fovpersec);
bool weaponHasFlag(int itemid, uint32_t flag);
bool weaponHasAimFlag(int weaponnum, uint32_t flag);
bool weaponHasAmmoFlag(int weaponnum, int funcnum, uint32_t flag);
int currentPlayerGetDeviceState(int weaponnum);
void currentPlayerSetDeviceActive(int weaponum, bool active);
uint16_t weaponGetFileNum(int weaponnum);
uint16_t weaponGetFileNum2(int weaponnum);
void gsetPopulateFromCurrentPlayer(int handnum, struct gset *gset);
struct inventory_ammo *gsetGetAmmoDefinition(struct gset *gset);
uint8_t gsetGetSinglePenetration(struct gset *gset);
float gsetGetImpactForce(struct gset *gset);
float gsetGetDamage(struct gset *gset);
uint8_t gsetGetFireslotDuration(struct gset *gset);
uint16_t gsetGetSingleShootSound(struct gset *gset);
bool gsetHasFunctionFlags(struct gset *gset, uint32_t flags);
int8_t weaponGetNumTicksPerShot(uint32_t weaponnum, uint32_t funcindex);
uint32_t currentPlayerGetSight(void);
void gsetGetNoiseSettings(struct gset *gset, struct noisesettings *settings);
struct guncmd *gsetGetPriToSecAnim(struct gset *gset);
struct guncmd *gsetGetSecToPriAnim(struct gset *gset);
void Lerp2D(float *param_1, float *param_2, float scale, float *dst);
void ScaleVector2D(float *a, float *b, float *dst);
