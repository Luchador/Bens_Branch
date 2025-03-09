#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/inv.h"
#include "game/bondgun.h"
#include "game/weaponutils.h"
#include "game/player.h"
#include "game/hudmsg.h"
#include "game/playermgr.h"
#include "game/mplayer/setup.h"
#include "game/botcmd.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "game/debug.h"
#include "bss.h"
#include "data.h"
#include "types.h"

struct weapon *weaponGetByRank(u16 rank)
{
	if(rank >= ARRAYCOUNT(g_Weapons)) {
		return NULL;
	}

	u16 i;

	for(i = 0; i < ARRAYCOUNT(g_Weapons); i++) {
		if(g_Weapons[i]->rank == rank) {
			return g_Weapons[i];
		}
	}

	return NULL;
}

u16 weaponGetRank(u16 rank)
{
	if(rank >= ARRAYCOUNT(g_Weapons)) {
		return 0;
	}

	u16 i;

	for(i = 0; i < ARRAYCOUNT(g_Weapons); i++) {
		if(g_Weapons[i]->rank == rank) {
			return g_Weapons[i]->rank;
		}
	}

	return 0;
}

struct weapon *weaponMatchEnum(u16 num)
{
	if(num >= ARRAYCOUNT(g_Weapons)) {
		return NULL;
	}

		switch(num) {
		case WEAPON_NONE:             return g_Weapons[0];
		case WEAPON_UNARMED:          return g_Weapons[1];
		case WEAPON_FALCON2:          return g_Weapons[2];
		case WEAPON_FALCON2_SILENCER: return g_Weapons[3];
		case WEAPON_FALCON2_SCOPE:    return g_Weapons[4];
		case WEAPON_FALCON2_SANDS:    return g_Weapons[5];
		case WEAPON_MAGSEC4:          return g_Weapons[6];
		case WEAPON_MAULER:           return g_Weapons[7];
		case WEAPON_PHOENIX:          return g_Weapons[8];
		case WEAPON_DY357MAGNUM:      return g_Weapons[9];
		case WEAPON_DY357LX:          return g_Weapons[10];
		case WEAPON_CMP150:           return g_Weapons[11];
		case WEAPON_CYCLONE:          return g_Weapons[12];
		case WEAPON_CALLISTO:         return g_Weapons[13];
		case WEAPON_RCP120:           return g_Weapons[14];
		case WEAPON_LAPTOPGUN:        return g_Weapons[15];
		case WEAPON_DRAGON:           return g_Weapons[16];
		case WEAPON_K7AVENGER:        return g_Weapons[17];
		case WEAPON_AR34:             return g_Weapons[18];
		case WEAPON_SUPERDRAGON:      return g_Weapons[19];
		case WEAPON_SHOTGUN:          return g_Weapons[20];
		case WEAPON_REAPER:           return g_Weapons[21];
		case WEAPON_SNIPERRIFLE:      return g_Weapons[22];
		case WEAPON_FARSIGHT:         return g_Weapons[23];
		case WEAPON_DEVASTATOR:       return g_Weapons[24];
		case WEAPON_ROCKETLAUNCHER:   return g_Weapons[25];
		case WEAPON_SLAYER:           return g_Weapons[26];
		case WEAPON_COMBATKNIFE:      return g_Weapons[27];
		case WEAPON_CROSSBOW:         return g_Weapons[28];
		case WEAPON_TRANQUILIZER:     return g_Weapons[29];
		case WEAPON_LASER:            return g_Weapons[30];
		case WEAPON_GRENADE:          return g_Weapons[31];
		case WEAPON_NBOMB:            return g_Weapons[32];
		case WEAPON_TIMEDMINE:        return g_Weapons[33];
		case WEAPON_PROXIMITYMINE:    return g_Weapons[34];
		case WEAPON_REMOTEMINE:       return g_Weapons[35];
		case WEAPON_COMBATBOOST:      return g_Weapons[36];
		case WEAPON_PP9I:             return g_Weapons[37];
		case WEAPON_CC13:             return g_Weapons[38];
		case WEAPON_KL01313:          return g_Weapons[39];
		case WEAPON_KF7SPECIAL:       return g_Weapons[40];
		case WEAPON_ZZT:              return g_Weapons[41];
		case WEAPON_DMC:              return g_Weapons[42];
		case WEAPON_AR53:             return g_Weapons[43];
		case WEAPON_RCP45:            return g_Weapons[44];
		case WEAPON_PSYCHOSISGUN:     return g_Weapons[45];
		case WEAPON_NIGHTVISION:      return g_Weapons[46];
		case WEAPON_EYESPY:           return g_Weapons[47];
		case WEAPON_XRAYSCANNER:      return g_Weapons[48];
		case WEAPON_IRSCANNER:        return g_Weapons[49];
		case WEAPON_CLOAKINGDEVICE:   return g_Weapons[50];
		case WEAPON_HORIZONSCANNER:   return g_Weapons[51];
		case WEAPON_TESTER:           return g_Weapons[52];
		case WEAPON_ROCKETLAUNCHER_34:return g_Weapons[53];
		case WEAPON_ECMMINE:          return g_Weapons[54];
		case WEAPON_DATAUPLINK:       return g_Weapons[55];
		case WEAPON_RTRACKER:         return g_Weapons[56];
		case WEAPON_PRESIDENTSCANNER: return g_Weapons[57];
		case WEAPON_DOORDECODER:      return g_Weapons[58];
		case WEAPON_AUTOSURGEON:      return g_Weapons[59];
		case WEAPON_EXPLOSIVES:       return g_Weapons[60];
		case WEAPON_SKEDARBOMB:       return g_Weapons[61];
		case WEAPON_COMMSRIDER:       return g_Weapons[62];
		case WEAPON_TRACERBUG:        return g_Weapons[63];
		case WEAPON_TARGETAMPLIFIER:  return g_Weapons[64];
		case WEAPON_DISGUISE40:       return g_Weapons[65];
		case WEAPON_DISGUISE41:       return g_Weapons[66];
		case WEAPON_FLIGHTPLANS:      return g_Weapons[67];
		case WEAPON_RESEARCHTAPE:     return g_Weapons[68];
		case WEAPON_BACKUPDISK:       return g_Weapons[69];
		case WEAPON_KEYCARD45:        return g_Weapons[70];
		case WEAPON_KEYCARD46:        return g_Weapons[71];
		case WEAPON_KEYCARD47:        return g_Weapons[72];
		case WEAPON_KEYCARD48:        return g_Weapons[73];
		case WEAPON_KEYCARD49:        return g_Weapons[74];
		case WEAPON_KEYCARD4A:        return g_Weapons[75];
		case WEAPON_KEYCARD4B:        return g_Weapons[76];
		case WEAPON_KEYCARD4C:        return g_Weapons[77];
		case WEAPON_SUITCASE:         return g_Weapons[78];
		case WEAPON_BRIEFCASE:        return g_Weapons[79];
		case WEAPON_SHIELDTECHITEM:   return g_Weapons[80];
		case WEAPON_NECKLACE:         return g_Weapons[81];
		case WEAPON_HAMMER:           return g_Weapons[82];
		case WEAPON_SCREWDRIVER:      return g_Weapons[83];
		case WEAPON_ROCKET:           return g_Weapons[84];
		case WEAPON_HOMINGROCKET:     return g_Weapons[85];
		case WEAPON_GRENADEROUND:     return g_Weapons[86];
		case WEAPON_BOLT:             return g_Weapons[87];
		case WEAPON_BRIEFCASE2:       return g_Weapons[88];
		case WEAPON_SKROCKET:         return g_Weapons[89];
		case WEAPON_CHOPPERGUN:       return g_Weapons[90];
		case WEAPON_WATCHLASER:       return g_Weapons[91];
		case WEAPON_MPSHIELD:         return g_Weapons[0];
		case WEAPON_DISABLED:         return g_Weapons[0];
		case WEAPON_SUICIDEPILL:      return g_Weapons[93];
		default:
			return g_Weapons[1]; // Unarmed
	}

	return NULL;
}

s32 weaponGetHighestRank()
{ 
	s32 i;
	s32 highest = 0;

	for(i = 0; i < ARRAYCOUNT(g_Weapons); i++) {
		if(g_Weapons[i]->rank > highest) {
			highest = g_Weapons[i]->rank;
		}
	}

	return highest;
}

struct weapon *weaponFindById(s32 rank)
{
	if (rank < 0) {
		return NULL;
	}

	if (rank >= weaponGetHighestRank()) {
		return NULL;
	}

	return weaponGetByRank(rank);
}

struct weaponfunc *weaponGetFunctionById(u32 weaponnum, u32 which)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (weapon) {
		return weapon->functions[which];
	}

	return NULL;
}

struct weaponfunc *gsetGetWeaponFunction2(struct gset *gset)
{
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (weapon) {
		return weapon->functions[gset->weaponfunc];
	}

	return NULL;
}

struct weaponfunc *gsetGetWeaponFunction(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		// SuperDragon grenades have FUNC_2, so this can happen
		if (gset->weaponfunc > FUNC_SECONDARY) {
			return NULL;
		}
		return weapon->functions[gset->weaponfunc];
	}

	return NULL;
}

struct weaponfunc *weaponGetFunction(struct gset *gset, s32 which)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->functions[which];
	}

	return NULL;
}

struct weaponfunc *currentPlayerGetWeaponFunction(u32 hand)
{
	struct weapon *weapon = weaponFindById(g_Vars.currentplayer->hands[hand].gset.weaponnum);

	if (weapon) {
		return weapon->functions[g_Vars.currentplayer->hands[hand].gset.weaponfunc];
	}

	return NULL;
}

u32 weaponGetNumFunctions(u32 weaponnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return 0;
	}

	for (i = 0; i < 2; i++) {
		if (weapon->functions[i] == NULL) {
			return i;
		}
	}

	return 2;
}

struct invaimsettings *gsetGetAimSettings(struct gset *gset)
{
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (weapon) {
		return weapon->aimsettings;
	}

	return &invaimsettings_default;
}

struct inventory_ammo *weaponGetAmmoByFunction(u32 weaponnum, u32 funcnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	struct weaponfunc *func = weaponGetFunctionById(weaponnum, funcnum);

	if (func && weapon && func->ammoindex >= 0) {
		return weapon->ammos[func->ammoindex];
	}

	return NULL;
}

void currentPlayerGetWeaponPos(struct coord *pos)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));

	if (weapon) {
		pos->x = weapon->posx;
		pos->y = weapon->posy;
		pos->z = weapon->posz;
	}
}

void currentPlayerSetWeaponPos(struct coord *pos)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));

	if (weapon) {
		weapon->posx = pos->x;
		weapon->posy = pos->y;
		weapon->posz = pos->z;
	}
}

f32 handGetXShift(s32 handnum)
{
	return g_Vars.currentplayer->hands[handnum].xshift;
}

f32 func0f0b131c(s32 hand)
{
	f32 x;
	struct weapon *weapon;

	if (hand == 0) {
		weapon = weaponFindById(bgunGetWeaponNum(0));
		x = weapon->posx;

		if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			x -= 3.5f;

			if (g_Vars.currentplayernum == 0) {
				x += 2.0f;
			}
		}
	} else {
		weapon = weaponFindById(bgunGetWeaponNum(1));
		x = -weapon->posx;

		if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			x += 3.5f;

			if (g_Vars.currentplayernum == 0) {
				x += 2.0f;
			}
		}
	}

	return x;
}

f32 currentPlayerGetGunZoomFov(void)
{
	s32 index = -1;
	struct weapon *weapon;

	u16 rank = weaponGetRank(bgunGetWeaponNum(0));
	if(rank == weaponMatchEnum(WEAPON_SNIPERRIFLE)->rank) {
		index = 0;
	}
	else if(rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
		index = 1;
	}
	else if(rank == weaponMatchEnum(WEAPON_HORIZONSCANNER)->rank) {
		index = 2;
	}

	if (index >= 0) {
		return g_Vars.currentplayer->gunzoomfovs[index];
	}

	weapon = weaponGetByRank(weaponGetRank(g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum));

	if (weapon) {
		f32 fov = weapon->aimsettings->zoomfov;
		return ADJUST_ZOOM_FOV(fov);
	}

	return 0;
}

void currentPlayerZoomOut(f32 fovpersec)
{
	s32 index = -1;

	u16 rank = weaponGetRank(bgunGetWeaponNum(0));
	if(rank == weaponMatchEnum(WEAPON_SNIPERRIFLE)->rank) {
		index = 0;
	}
	else if(rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
		index = 1;
	}
	else if(rank == weaponMatchEnum(WEAPON_HORIZONSCANNER)->rank) {
		index = 2;
	}

	if (index >= 0) {
		f32 amount = fovpersec * 0.25f * LVUPDATE60FREAL();

		if (rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
			amount *= 0.5f;
		}

		g_Vars.currentplayer->gunzoomfovs[index] *= 1.0f + amount * 0.1f;

		if (g_Vars.currentplayer->gunzoomfovs[index] > ADJUST_ZOOM_FOV(60)) {
			g_Vars.currentplayer->gunzoomfovs[index] = ADJUST_ZOOM_FOV(60);
		}
	}
}

void currentPlayerZoomIn(f32 fovpersec)
{
	s32 index = -1;

	u16 rank = weaponGetRank(bgunGetWeaponNum(0));
	if(rank == weaponMatchEnum(WEAPON_SNIPERRIFLE)->rank) {
		index = 0;
	}
	else if(rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
		index = 1;
	}
	else if(rank == weaponMatchEnum(WEAPON_HORIZONSCANNER)->rank) {
		index = 2;
	}

	if (index >= 0) {
		f32 amount = fovpersec * 0.25f * LVUPDATE60FREAL();

		if (rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
			amount *= 0.5f;
		}

		g_Vars.currentplayer->gunzoomfovs[index] /= 1 + amount * 0.1f;

		if (g_Vars.currentplayer->gunzoomfovs[index] < ADJUST_ZOOM_FOV(2)) {
			g_Vars.currentplayer->gunzoomfovs[index] = ADJUST_ZOOM_FOV(2);
		}
	}
}

bool weaponHasFlag(s32 itemid, u32 flag)
{
	struct weapon *weapon = weaponFindById(itemid);

	if (!weapon) {
		return false;
	}

	// always dual-wieldable if cheat is enabled
	if (cheatIsActive(CHEAT_DUALWIELDALLGUNS) && (flag == WEAPONFLAG_DUALWIELD)) {
		return true;
	}

	return (weapon->flags & flag) != 0;
}

bool weaponHasAimFlag(s32 weaponnum, u32 flag)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (!weapon) {
		return false;
	}

	return (weapon->aimsettings->flags & flag) != 0;
}

bool weaponHasAmmoFlag(s32 weaponnum, s32 funcnum, u32 flag)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	struct inventory_ammo *ammo;

	if (weapon == NULL) {
		return false;
	}

	ammo = weapon->ammos[funcnum];

	if (ammo) {
		return (ammo->flags & flag) != 0;
	}

	return false;
}

s32 currentPlayerGetDeviceState(s32 weaponnum)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return DEVICESTATE_UNEQUIPPED;
	}

	for (i = 0; i < ARRAYCOUNT(weapon->functions); i++) {
		if (weapon->functions[i]) {
			struct weaponfunc_device *devicefunc = weapon->functions[i];

			if ((devicefunc->base.type & 0xff) == INVENTORYFUNCTYPE_DEVICE) {
				if ((g_Vars.currentplayer->devicesactive & devicefunc->device) == 0) {
					return DEVICESTATE_INACTIVE;
				}

				return DEVICESTATE_ACTIVE;
			}
		}
	}

	return DEVICESTATE_UNEQUIPPED;
}

void currentPlayerSetDeviceActive(s32 weaponnum, bool active)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	s32 i;

	if (!weapon) {
		return;
	}

	for (i = 0; i < ARRAYCOUNT(weapon->functions); i++) {
		if (weapon->functions[i]) {
			struct weaponfunc_device *devicefunc = weapon->functions[i];

			if ((devicefunc->base.type & 0xff) == INVENTORYFUNCTYPE_DEVICE) {
				if (active) {
					if (devicefunc->device & (DEVICE_NIGHTVISION | DEVICE_XRAYSCANNER | DEVICE_EYESPY | DEVICE_IRSCANNER)) {
						g_Vars.currentplayer->devicesactive &= ~(DEVICE_NIGHTVISION | DEVICE_XRAYSCANNER | DEVICE_EYESPY | DEVICE_IRSCANNER);
					}

					g_Vars.currentplayer->devicesactive |= devicefunc->device;
					return;
				}

				g_Vars.currentplayer->devicesactive &= ~devicefunc->device;
				return;
			}
		}
	}
}

u16 weaponGetFileNum(s32 weaponnum)
{
	struct weapon *weapon = NULL;

	if (weaponnum != -1) {
		weapon = weaponGetByRank(weaponnum);
	}

	if (weapon) {
		return weapon->hi_model;
	}

	return 0;
}

void gsetPopulateFromCurrentPlayer(s32 handnum, struct gset *gset)
{
	gset->weaponnum = g_Vars.currentplayer->gunctrl.weaponnum;
	u16 rank = weaponGetRank(gset->weaponnum);
	gset->weaponfunc = g_Vars.currentplayer->hands[handnum].gset.weaponfunc;
	gset->unk063a = g_Vars.currentplayer->hands[handnum].gset.unk063a;
	gset->gsetrank = g_Vars.currentplayer->hands[handnum].gset.gsetrank;

	if (rank == weaponMatchEnum(WEAPON_MAULER)->rank) {
		gset->unk063a = g_Vars.currentplayer->hands[handnum].matmot1 * 10.0f;
	}

	if (rank == weaponMatchEnum(WEAPON_LASER)->rank) {
		gset->unk063a = g_Vars.currentplayer->hands[handnum].burstbullets & 0xff;
	}
}

struct inventory_ammo *gsetGetAmmoDefinition(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	struct weapon *weapon = weaponFindById(gset->weaponnum);

	if (func && func->ammoindex >= 0) {
		return weapon->ammos[func->ammoindex];
	}

	return NULL;
}

u8 gsetGetSinglePenetration(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->penetration;
	}

	return 0;
}

s32 handGetCasingEject(struct gset *gset)
{
	s32 result = 0;
	struct inventory_ammo *ammo = gsetGetAmmoDefinition(gset);

	if (ammo) {
		result = ammo->casingeject;
	}

	return result;
}

f32 gsetGetImpactForce(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	f32 result = 0;

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		result = funcshoot->impactforce;
	}

	return result;
}

void Lerp2D(f32 *a, f32 *b, f32 scale, f32 *dst)
{
	dst[0] = (b[0] - a[0]) * scale + a[0];
	dst[1] = (b[1] - a[1]) * scale + a[1];
}

void ScaleVector2D(f32 *a, f32 *b, f32 *dst)
{
	dst[0] = b[0] * a[0];
	dst[1] = b[1] * a[1];
}

f32 gsetGetDamage(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	f32 damage = 0;

	if (func) {
		if ((func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
			struct weaponfunc_shoot *shootfunc = (struct weaponfunc_shoot *)func;
			damage = shootfunc->damage;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			struct weaponfunc_melee *meleefunc = (struct weaponfunc_melee *)func;
			damage = meleefunc->damage;

			if (weaponGetRank(gset->weaponnum) == weaponMatchEnum(WEAPON_REAPER)->rank) {
				damage *= LVUPDATE60FREAL();
			}
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_THROW) {
			struct weaponfunc_throw *throwfunc = (struct weaponfunc_throw *)func;
			damage = throwfunc->damage;
		}
	}

	if (weaponGetRank(gset->weaponnum) == weaponMatchEnum(WEAPON_MAULER)->rank) {
		damage = (gset->unk063a / 3.0f + 1.0f) * damage;
	}

	if (bgunIsFiring(HAND_LEFT) && bgunIsFiring(HAND_RIGHT)) {
		damage += damage;
	}

	return damage;
}

u8 gsetGetFireslotDuration(struct gset *gset)
{
/*#if VERSION >= VERSION_PAL_FINAL
	struct weaponfunc *func = gsetGetWeaponFunction(gset);
	u8 result = 0;

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		result = funcshoot->duration60;
	}

	if (result >= 4) {
		result = TICKS(result);
	}

	return result;
#else*/
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->duration60;
	}

	return 0;
//#endif
}

u16 gsetGetSingleShootSound(struct gset *gset)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *funcshoot = (struct weaponfunc_shoot *)func;
		return funcshoot->shootsound;
	}

	return 0;
}

bool gsetHasFunctionFlags(struct gset *gset, u32 flags)
{
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func) {
		return (func->flags & flags) == flags;
	}

	return false;
}

s8 weaponGetNumTicksPerShot(u32 weaponnum, u32 funcindex)
{
	u32 stack[2];
	s32 result = 0;
	struct weapon *weapon = weaponFindById(weaponnum);
	struct weaponfunc *func = weapon->functions[funcindex];

	if (func && func->type == INVENTORYFUNCTYPE_SHOOT_AUTOMATIC) {
		struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *)func;

		result = 3600.0f / autofunc->maxrpm;
	}

	// PAL beta removes this check, only for it to be added back in PAL final
	if (result > 3) {
		result = TICKS(result);
	}

	return result;
}

u32 currentPlayerGetSight(void)
{
	struct weaponfunc *func = weaponGetFunctionById(
			g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum,
			g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc);

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
		return SIGHT_NONE;
	}

	if (cheatIsActive(CHEAT_CLASSICSIGHT)) {
		return SIGHT_CLASSIC;
	}

	u16 rank = weaponGetRank(g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum);

	if(		rank == weaponMatchEnum(WEAPON_FALCON2_SCOPE)->rank ||
			//rank == weaponMatchEnum(WEAPON_FALCON2_SANDS)->rank ||
			rank == weaponMatchEnum(WEAPON_MAGSEC4)->rank ||
			rank == weaponMatchEnum(WEAPON_SNIPERRIFLE)->rank ||
			rank == weaponMatchEnum(WEAPON_LAPTOPGUN)->rank ||
			rank == weaponMatchEnum(WEAPON_DRAGON)->rank ||
			rank == weaponMatchEnum(WEAPON_K7AVENGER)->rank ||
			rank == weaponMatchEnum(WEAPON_AR34)->rank ||
			rank == weaponMatchEnum(WEAPON_SUPERDRAGON)->rank){
			return SIGHT_ZOOM;
		}
	else if(rank == weaponMatchEnum(WEAPON_MAULER)->rank ||
	        rank == weaponMatchEnum(WEAPON_REAPER)->rank) {
			return SIGHT_SKEDAR;
	}
	else if(rank == weaponMatchEnum(WEAPON_PHOENIX)->rank ||
			rank == weaponMatchEnum(WEAPON_CALLISTO)->rank ||
			rank == weaponMatchEnum(WEAPON_FARSIGHT)->rank) {
		return SIGHT_MAIAN;
	}
	else if(
			rank == weaponMatchEnum(WEAPON_PP9I)->rank ||
			rank == weaponMatchEnum(WEAPON_CC13)->rank ||
			rank == weaponMatchEnum(WEAPON_KL01313)->rank ||
			rank == weaponMatchEnum(WEAPON_KF7SPECIAL)->rank ||
			rank == weaponMatchEnum(WEAPON_ZZT)->rank ||
			rank == weaponMatchEnum(WEAPON_DMC)->rank ||
			rank == weaponMatchEnum(WEAPON_AR53)->rank ||
			rank == weaponMatchEnum(WEAPON_RCP45)->rank) {
	   return SIGHT_CLASSIC;
	}

	return SIGHT_DEFAULT;
}

void gsetGetNoiseSettings(struct gset *gset, struct noisesettings *dst)
{
	struct noisesettings *settings = NULL;
	struct weaponfunc *func = gsetGetWeaponFunction(gset);

	if (func != NULL) {
		settings = func->noisesettings;
	}

	if (settings == NULL) {
		settings = &invnoisesettings_silent;
	}

	dst->minradius = settings->minradius;
	dst->maxradius = settings->maxradius;
	dst->incradius = settings->incradius;
	dst->decbasespeed = settings->decbasespeed;
	dst->decremspeed = settings->decremspeed;
}

struct guncmd *handGetEquipAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->equip_animation;
	}

	return NULL;
}

struct guncmd *handGetUnequipAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->unequip_animation;
	}

	return NULL;
}

struct guncmd *gsetGetPriToSecAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->pritosec_animation;
	}

	return NULL;
}

struct guncmd *gsetGetSecToPriAnim(struct gset *gset)
{
	struct weapon *weapon = g_Weapons[gset->weaponnum];

	if (weapon) {
		return weapon->sectopri_animation;
	}

	return NULL;
}
