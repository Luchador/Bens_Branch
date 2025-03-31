#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/bondmove.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/inv.h"
#include "game/menuutils.h"
#include "game/chr.h"
#include "game/bg.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/gunfx.h"
#include "game/weaponutils.h"
#include "game/modeldef.h"
#include "game/modelmgr.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/gfxmemory.h"
#include "game/sight.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/smoke.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/texdecompress.h"
#include "game/zbuf.h"
#include "game/training.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/options.h"
#include "game/propobj.h"
#include "game/objectives.h"
#include "bss.h"
#include "lib/collision.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/lib_317f0.h"
#include "lib/lib_17ce0.h"
#include "data.h"
#include "types.h"
#include "game/stagetable.h"
#include "video.h"
#include "platform.h"
#include "game/debug.h"

#define GUNLOADSTATE_FLUX     0
#define GUNLOADSTATE_MODEL    1
#define GUNLOADSTATE_TEXTURES 2
#define GUNLOADSTATE_DLS      3
#define GUNLOADSTATE_LOADED   4

#define MASTERLOADSTATE_FLUX   0
#define MASTERLOADSTATE_HANDS  1
#define MASTERLOADSTATE_GUN    2
#define MASTERLOADSTATE_CARTS  3
#define MASTERLOADSTATE_LOADED 4

// Max downwards pitch when changing guns or reloading a classic gun
#define MAX_PITCH 0.87252569198608f

int var8009d0dc;
int var8009d0f0[3];
float var8009d140;
struct hand *var8009d144;
int var8009d148;
struct sndstate *g_CasingAudioHandles[2];
int g_TimeToNextCasingSound;
struct sndstate *g_BgunAudioHandles[MAX_PLAYERS];
struct fireslot g_Fireslots[20];
uint32_t fill2[1];
bool g_CloseToWall = 0;
int g_CloseToWallTimer = 0;

Lights1 g_GunLight = gdSPDefLights1(0x96, 0x96, 0x96, 0xff, 0xff, 0xff, 0xb2, 0x4d, 0x2e);

#ifdef PLATFORM_64BIT
uint32_t g_BgunGunMemBaseSizeDefault = 150 * 1024 * 2; // #TODO adjust these values properly
#else
uint32_t g_BgunGunMemBaseSizeDefault = 150 * 1024;
#endif

uint16_t g_CartFileNums[] = {
	FILE_GCARTRIDGE,
	FILE_GCARTRIFLE,
	FILE_GCARTBLUE,
	FILE_GCARTSHELL,
};

int g_BgunGeMuzzleFlashes = false;

void bgunRumble(int handnum, int weaponnum)
{
	int contpadtouse1;
	int contpadtouse2;
	bool singlewield = false;
	int contpad1;
	int contpad2;
	int contpad1hasrumble;
	int contpad2hasrumble;

	joyGetContpadNumsForPlayer(g_Vars.currentplayernum, &contpad1, &contpad2);

	if (optionsGetControlMode(g_Vars.currentplayerstats->mpindex) >= CONTROLMODE_21
			&& contpad1 >= 0 && contpad2 >= 0) {
		contpad1hasrumble = pakGetType(contpad1) == PAKTYPE_RUMBLE;
		contpad2hasrumble = pakGetType(contpad2) == PAKTYPE_RUMBLE;

		if (!weaponHasFlag(weaponnum, WEAPONFLAG_DUALWIELD)) {
			singlewield = true;
		}

		if (contpad1hasrumble && contpad2hasrumble) {
			if (singlewield) {
				pakRumble(contpad1, 0.2f, 2, 4);
				pakRumble(contpad2, 0.2f, 2, 4);
			} else {
				int contpadtouse1 = contpad1;

				if (handnum == HAND_LEFT) {
					contpadtouse1 = contpad2;
				}

				pakRumble(contpadtouse1, 0.2f, 2, 4);
			}
		} else {
			int contpadtouse2 = contpad1;

			if (contpad2hasrumble) {
				contpadtouse2 = contpad2;
			}

			pakRumble(contpadtouse2, 0.2f, 2, 4);
		}
	} else {
		if (contpad1 >= 0) {
			pakRumble(contpad1, 0.2f, 2, 4);
		}
	}
}

int bgunGetUnequippedReloadIndex(int weaponnum)
{
	if (weaponnum == WEAPON_CROSSBOW) {
		return 0;
	}

	if (weaponnum == WEAPON_SHOTGUN) {
		return 1;
	}

	if (weaponnum == WEAPON_DY357MAGNUM) {
		return 2;
	}

	if (weaponnum == WEAPON_DY357LX) {
		return 3;
	}

	return -1;
}

/**
 * The magnums, shotgun and crossbow are special because that the game remembers
 * how much ammo is loaded in their clips when the weapon is not being used.
 * Their clips are gradually reloaded while the weapon is not in use, and that
 * gradual reloading is handled by this function.
 *
 * The gunroundsspent value is actually a countdown timer,
 * not the number of rounds as the name suggests.
 */
void bgunTickUnequippedReload(void)
{
	int i;
	int j;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < ARRAYCOUNT(g_Vars.currentplayer->hands[i].gunroundsspent); j++) {
			uint16_t spent = g_Vars.currentplayer->hands[i].gunroundsspent[j];

			if (spent > g_Vars.lvupdate60) {
				spent -= g_Vars.lvupdate60;
			} else {
				spent = 0;
			}

			g_Vars.currentplayer->hands[i].gunroundsspent[j] = spent;
		}
	}
}

bool bgunTestGunVisCommand(struct gunviscmd *cmd, struct hand *hand)
{
	bool result = true;

	switch (cmd->type) {
	case GUNVISCMD_CHECKUPGRADE:
		if (((hand->gset.unk0639 >> cmd->param) & 1) == 0) {
			result = false;
		}
		break;
	case GUNVISCMD_CHECKINLEFTHAND:
		if (hand != &g_Vars.currentplayer->hands[HAND_LEFT]) {
			result = false;
		}
		break;
	case GUNVISCMD_CHECKINRIGHTHAND:
		if (hand != &g_Vars.currentplayer->hands[HAND_RIGHT]) {
			result = false;
		}
		break;
	}

	return result;
}

void bgunSetPartVisible(int16_t partnum, bool visible, struct hand *hand, struct modeldef *modeldef)
{
	struct modelnode *node;

	if (partnum == MODELPART_HAND_LEFT || partnum == MODELPART_HAND_RIGHT) {
		if (g_Vars.currentplayer->gunctrl.handmodeldef) {
			node = modelGetPart(g_Vars.currentplayer->gunctrl.handmodeldef, partnum);

			if (node) {
				struct modelrodata_toggle *rodata = &node->rodata->toggle;
				uint32_t *ptr = &hand->handsavedata[rodata->rwdataindex];
				*ptr = visible;
			}
		}
	} else {
		node = modelGetPart(modeldef, partnum);

		if (node) {
			struct modelrodata_toggle *rodata = &node->rodata->toggle;
			uint32_t *ptr = &hand->unk0a6c[rodata->rwdataindex];
			*ptr = visible;
		}
	}
}

void bgunExecuteGunVisCommands(struct hand *hand, struct modeldef *modeldef, struct gunviscmd *commands)
{
	struct gunviscmd *cmd = commands;
	bool done = false;

	if (cmd == NULL) {
		return;
	}

	while (!done) {
		if (bgunTestGunVisCommand(cmd, hand)) {
			if (cmd->op == GUNVISOP_IFTRUE_SETVISIBLE) {
				bgunSetPartVisible(cmd->partnum, true, hand, modeldef);
			}

			if (cmd->op == GUNVISOP_IFTRUE_SETHIDDEN) {
				bgunSetPartVisible(cmd->partnum, false, hand, modeldef);
			}

			if (cmd->op == GUNVISOP_SETVISIBILITY) {
				bgunSetPartVisible(cmd->partnum, true, hand, modeldef);
			}
		} else {
			if (cmd->op == GUNVISOP_SETVISIBILITY) {
				bgunSetPartVisible(cmd->partnum, false, hand, modeldef);
			}
		}

		cmd++;

		if (cmd->type == GUNVISCMD_END) {
			done = true;
		}
	}
}

void bgun0f098030(struct hand *hand, struct modeldef *modeldef)
{
	struct weapon *weapon = weaponFindById(hand->gset.weaponnum);
	int i;
	int j;

	bgunExecuteGunVisCommands(hand, modeldef, weapon->gunviscmds);
	bgunSetPartVisible(MODELPART_0042, false, hand, modeldef);

	for (i = 0; i < 2; i++) {
		if (weapon->ammos[i] && (weapon->ammos[i]->flags & AMMOFLAG_QTYAFFECTSPARTVIS)) {
			for (j = 0; j < hand->clipsizes[i]; j++) {
				if (j >= hand->loadedammo[i]) {
					bgunSetPartVisible(j + 100, false, hand, modeldef);
				} else {
					bgunSetPartVisible(j + 100, true, hand, modeldef);
				}
			}
		}
	}
}

float bgun0f09815c(struct hand *hand)
{
	if (hand->animmode == HANDANIMMODE_BUSY && hand->unk0ce8 != NULL) {
		if (hand->unk0ce8->unk04 < 0) {
			return modelGetNumAnimFrames(&hand->gunmodel) - modelGetCurAnimFrame(&hand->gunmodel);
		}

		return modelGetCurAnimFrame(&hand->gunmodel);
	}

	return 0;
}

void bgun0f0981e8(struct hand *hand, struct modeldef *modeldef)
{
	int s2;
	int s4;
	struct guncmd *cmd;
	float animspeed;
	bool done;
	float animspeedmult;
	int partnums[15];
	bool partsvisible[15];
	int partframes[15];
	int s0;
	int index;

	hand->unk0cc8_04 = false;

	if (hand->animmode == HANDANIMMODE_BUSY && bgun0f09815c(hand) >= modelGetNumAnimFrames(&hand->gunmodel) - 1) {
		hand->animmode = HANDANIMMODE_IDLE;
	}

	// This condition looks like a bug (using | instead of ||), but it happens
	// to make no difference anyway. Brackets added for clarity.
	if ((hand->animmode == (uint32_t)HANDANIMMODE_BUSY) || (hand->animload >= 0)) { // Ben's comment: fixing it anyway
		if (hand->gangstarot > 0.0f) {
			hand->animframeinc = 0;
		}

		if (hand->animload >= 0) {
			animspeedmult = 1.0f;
			animspeed = hand->unk0ce8->unk04 / 10000.0f;

			if (hand->unk0d0e_07 && g_Vars.currentplayer->hands[HAND_LEFT].inuse) {
				animspeedmult = RANDOMFRAC() * 0.77f + 0.7f;
			}

			if (hand->unk0ce8 && animspeed < 0.0f) {
				modelSetAnimation(&hand->gunmodel, hand->animload, false, 0.0f, animspeedmult * animspeed, 0.0f);
				modelSetAnimFrame(&hand->gunmodel, modelGetNumAnimFrames(&hand->gunmodel));
			} else {
				modelSetAnimation(&hand->gunmodel, hand->animload, false, 0.0f, animspeedmult * animspeed, 0.0f);
			}

			hand->animload = -1;
			hand->animmode = HANDANIMMODE_BUSY;
		}

		if (hand->unk0cc8_02) {
			hand->animframeinc = 0;
		}

		s4 = bgun0f09815c(hand);
		s2 = hand->animframeinc + s4;

		if (s4 == 0 && s2 > 0) {
			s4--;
		}

		if (hand->unk0ce8) {
			done = false;
			cmd = hand->unk0ce8;

			if (cmd) {
				s0 = 0;

				do {
					if (cmd->type == GUNCMD_END) {
						done = true;
					} else if (cmd->type == GUNCMD_SHOWPART || cmd->type == GUNCMD_HIDEPART) {
						if (s2 >= cmd->unk02) {
							int i;
							index = -1;

							for (i = 0; i < s0; i++) {
								if (cmd->unk04 == partnums[i]) {
									index = i;
								}
							}

							if (index == -1) {
								index = s0;
								s0++;

								partnums[index] = cmd->unk04;
								partframes[index] = -1;
							}

							if (cmd->unk02 > partframes[index]) {
								partframes[index] = cmd->unk02;
								partsvisible[index] = cmd->type == GUNCMD_SHOWPART ? true : false;
							}
						}
					} else {
						switch (cmd->type) {
						case GUNCMD_WAITFORZRELEASED:
							if (hand->unk0cc8_01) {
								if (s2 >= cmd->unk02 && s4 < cmd->unk02 && s4 < s2) {
									int tmp = cmd->unk02 - (int) bgun0f09815c(hand);
									tmp /= 2;

									if (hand->animframeinc > tmp) {
										hand->animframeinc = tmp;
									}

									s2 = hand->animframeinc + s4;
								}
							}
							break;
						case GUNCMD_REPEATUNTILFULL:
							if (hand->incrementalreloading && s2 >= cmd->unk02 && s4 < cmd->unk02 && s4 < s2) {
								int sp78 = cmd->unk04 + (((int)s2 - cmd->unk02) % ((cmd->unk02 - cmd->unk04) + 1));
								s4 = sp78;
								hand->animframeinc = 0;
								modelSetAnimFrame(&hand->gunmodel, sp78);
								hand->animloopcount++;
								s2 = sp78;
							}
							break;
						}
					}

					cmd++;
				} while (!done);

				if (s0 > 0) {
					int i;

					for (i = 0; i < s0; i++) {
						bgunSetPartVisible(partnums[i], partsvisible[i], hand, modeldef);
					}
				}
			}
		}
		modelTickAnim(&hand->gunmodel, hand->animframeinc, true);

		s2 = bgun0f09815c(hand);

		if (hand->unk0ce8) {
			bool done = false;
			struct guncmd *cmd = hand->unk0ce8;
			float speed = 1.0f;
			bool hasspeed = false;

			if (cmd) {
				do {
					if (cmd->type == GUNCMD_END) {
						done = true;
					} else {
						if (s2 >= cmd->unk02 && s4 < cmd->unk02 && s4 < s2) {
							switch (cmd->type) {
							case GUNCMD_PLAYSOUND:
								if (hasspeed) {
									snd00010718(0, 0, AL_VOL_FULL, AL_PAN_CENTER, cmd->unk04, speed, 1, -1, 1);
									hasspeed = false;
								} else {
									snd00010718(0, 0, AL_VOL_FULL, AL_PAN_CENTER, cmd->unk04, 1.0f, 1, -1, 1);
								}
								break;
							case GUNCMD_SETSOUNDSPEED:
								speed = cmd->unk04 / 1000.0f;
								hasspeed = true;
								break;
							case GUNCMD_POPOUTSACKOFPILLS:
								hand->unk0cc8_04++;
								break;
							}
						}
					}

					cmd++;
				} while (!done);
			}
		}
	}
}

bool bgun0f098884(struct guncmd *cmd, struct gset *gset)
{
	int result = false;

	if (cmd->unk01 == 0) {
		return true;
	}

	if (cmd->unk01 == 1 && g_Vars.currentplayer->hands[HAND_LEFT].inuse == true) {
		result = true;
	}

	if (cmd->unk01 == 2 && gset->weaponfunc == FUNC_SECONDARY) {
		result = true;
	}

	return result;
}

void bgunStartAnimation(struct guncmd *cmd, int handnum, struct hand *hand)
{
	if (cmd->type != GUNCMD_PLAYANIMATION) {
		struct guncmd *loopcmd = cmd;
		int done = false;
		uint32_t rand = rngRandom() % 100;

		while (loopcmd->type != GUNCMD_END) {
			if (bgun0f098884(loopcmd, &hand->gset) && !done) {
				if (loopcmd->type == GUNCMD_INCLUDE) {
					done = true;
					bgunStartAnimation((struct guncmd *)loopcmd->unk04, handnum, hand);
				} else if (loopcmd->type == GUNCMD_RANDOM) {
					if ((struct guncmd *)loopcmd->unk04 != hand->unk0d80 && loopcmd->unk02 > rand) {
						done = true;
						bgunStartAnimation((struct guncmd *)loopcmd->unk04, handnum, hand);
					}
				}
			}

			loopcmd++;
		}
	} else {
		hand->animload = cmd->unk02;
		hand->animmode = HANDANIMMODE_IDLE;
		hand->unk0cc8_01 = 0;
		hand->incrementalreloading = false;
		hand->unk0ce8 = cmd;
		hand->animloopcount = 0;
		hand->unk0cc8_02 = 0;
		hand->unk0d0e_07 = false;
		hand->unk0d80 = cmd;
	}
}

bool bgun0f098a44(struct hand *hand, int time)
{
	struct guncmd *cmd = hand->unk0ce8;
	int waittimekeyframe = -1;
	int zreleasekeyframe = -1;

	if (hand->animmode == HANDANIMMODE_IDLE) {
		return (hand->animload == -1);
	}

	while (cmd->type != GUNCMD_END && waittimekeyframe == -1) {
		if (cmd->type == GUNCMD_WAITFORZRELEASED) {
			zreleasekeyframe = cmd->unk02;
		}

		if (cmd->type == GUNCMD_WAITTIME && time == cmd->unk04) {
			waittimekeyframe = cmd->unk02;
		}

		cmd++;
	}

	if (waittimekeyframe >= 0) {
		if (hand->unk0cc8_01 && (int)bgun0f09815c(hand) <= zreleasekeyframe) {
			return false;
		}

		return (bgun0f09815c(hand) + hand->animframeinc >= waittimekeyframe);
	}

	return true;
}

int bgun0f098b80(struct hand *hand, int arg1)
{
	struct guncmd *cmd = hand->unk0ce8;
	int keyframe = -1;

	if (hand->animmode == HANDANIMMODE_IDLE) {
		return 0;
	}

	while (cmd->type != GUNCMD_END && keyframe == -1) {
		if (cmd->type == GUNCMD_WAITTIME) {
			if (cmd->unk04 == arg1) {
				keyframe = cmd->unk02;
			}
		}

		cmd++;
	}

	if (keyframe == -1) {
		keyframe = 0;
	}

	return keyframe;
}

bool bgunIsAnimBusy(struct hand *hand)
{
	return hand->animmode != HANDANIMMODE_IDLE;
}

void bgunResetAnim(struct hand *hand)
{
	hand->animload = -1;
	hand->animmode = HANDANIMMODE_IDLE;
	hand->unk0cc8_01 = false;
	hand->incrementalreloading = false;
	hand->unk0ce8 = NULL;
	hand->animloopcount = 0;
	hand->unk0cc8_02 = false;
	hand->unk0d0e_07 = false;
}

void bgunGetWeaponInfo(struct handweaponinfo *info, int handnum)
{
	int weaponnum = bgunGetWeaponNum2(handnum);

	info->weaponnum = weaponnum;
	info->definition = g_Weapons[weaponnum];
	info->gunctrl = &g_Vars.currentplayer->gunctrl;
}

/**
 * Return values:
 * -1 = gun function doesn't exist or ammo fully depleted
 * 0 = trigger reload
 * 1 = has ammo in clip and ammo in reserve
 * 2 = has ammo in clip but none in reserve
 * 3 = gun doesn't use ammo or clip is full
 */
int bgunDetermineReloadType(int funcnum, struct handweaponinfo *info, struct hand *hand)
{
	int result = 3;
	struct weaponfunc *func = weaponGetFunction(&hand->gset, funcnum);

	if (!func) {
		return -1;
	}

	if (func->ammoindex != -1) {
		int ammoindex = func->ammoindex;

		if (info->gunctrl->ammotypes[ammoindex] >= 0
				&& hand->loadedammo[ammoindex] < hand->clipsizes[ammoindex]) {
			int minqty = 1;

			if (info->weaponnum == WEAPON_SHOTGUN && funcnum == FUNC_SECONDARY) {
				minqty = 2;
			}

			if (info->weaponnum == WEAPON_TRANQUILIZER && funcnum == FUNC_SECONDARY) {
				minqty = bgunGetMinClipQty(WEAPON_TRANQUILIZER, FUNC_SECONDARY);
			}

			result = 1;

			if (hand->loadedammo[ammoindex] < minqty) {
				result = 0;

				if (g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]] == 0) {
					result = -1;
				}
			} else {
				if (g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]] == 0) {
					result = 2;
				}
			}
		}
	}

	return result;
}

void bgunRefillMagazine(int weaponfunc, struct handweaponinfo *info, struct hand *hand, uint8_t onebullet, uint8_t checkunequipped)
{
	struct weaponfunc *func = weaponGetFunction(&hand->gset, weaponfunc);

	if (func && func->ammoindex != -1) {
		int ammoindex = func->ammoindex;

		if (info->gunctrl->ammotypes[ammoindex] >= 0) {
			int amount = hand->clipsizes[ammoindex] - hand->loadedammo[ammoindex];

			int reloadindex = bgunGetUnequippedReloadIndex(info->weaponnum);

			if (g_FrIsValidWeapon) {
				reloadindex = -1;
			}

			if (checkunequipped && reloadindex >= 0) {
				amount -= hand->gunroundsspent[reloadindex] >> 8;
			}

			if (onebullet) {
				amount = 1;
			}

			if (amount > g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]]) {
				amount = g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]];
			}

			// In most versions of the game, reloading the shotgun while going
			// through a teleport in Deep Sea will cause the shotgun to load
			// more ammo than its capacity. JPN Final fixes this here.
			// Ben's comment: integrating the JPN change
			if (amount > hand->clipsizes[ammoindex] - hand->loadedammo[ammoindex]) {
				amount = hand->clipsizes[ammoindex] - hand->loadedammo[ammoindex];
			}

			hand->loadedammo[ammoindex] += amount;
			g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]] -= amount;

			if (info->definition->ammos[ammoindex]->flags & AMMOFLAG_NORESERVE) {
				g_Vars.currentplayer->ammoheldarr[info->gunctrl->ammotypes[ammoindex]] = 0;
			}
		}
	}
}

// Fill magazine when the weapon is drawn
void bgunFillMagazine(struct handweaponinfo *info, struct hand *hand)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (weaponGetFunction(&hand->gset, i)) {
			bgunRefillMagazine(i, info, hand, 0, 1);
		}
	}
}

bool bgunDetermineReloadTypeByHand(int handnum)
{
	struct handweaponinfo info;

	bgunGetWeaponInfo(&info, handnum);

	if (bgunDetermineReloadType(0, &info, &g_Vars.currentplayer->hands[handnum]) > 0) {
		return true;
	}

	if (bgunDetermineReloadType(1, &info, &g_Vars.currentplayer->hands[handnum]) > 0) {
		return true;
	}

	return false;
}

bool bgun0f0990b0(struct weaponfunc *basefunc, struct weapon *weapon)
{
	if (!basefunc) {
		return true;
	}

	if (basefunc->type == INVENTORYFUNCTYPE_NONE) {
		return true;
	}

	if ((basefunc->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
		return true;
	}

	if ((basefunc->type & 0xff) == INVENTORYFUNCTYPE_SPECIAL) {
		struct weaponfunc_special *func = (struct weaponfunc_special *)basefunc;

		if (func->specialfunc != HANDATTACKTYPE_DETONATE
				&& func->specialfunc != HANDATTACKTYPE_BOOST
				&& func->specialfunc != HANDATTACKTYPE_REVERTBOOST) {
			return true;
		}
	}

	if ((basefunc->type & 0xff) == INVENTORYFUNCTYPE_THROW) {
		if (basefunc->ammoindex <= -1) {
			return true;
		}
	}

	if (basefunc->ammoindex >= 0
			&& weapon->ammos[basefunc->ammoindex]
			&& bgunGetAmmoCount(weapon->ammos[basefunc->ammoindex]->type) <= 0) {
		return true;
	}

	return false;
}

bool bgun0f099188(struct hand *hand, int gunfunc)
{
	struct weaponfunc *func = weaponGetFunction(&hand->gset, gunfunc);
	struct weapon *weapon = weaponFindById(hand->gset.weaponnum);

	if (bgunIsUsingSecondaryFunction() == gunfunc) {
		return false;
	}

	return bgun0f0990b0(func, weapon);
}

int bgunTickIncIdle(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	bool usesec;
	int gunfunc = bgunIsUsingSecondaryFunction();
	int reloadtype;
	int sp30;
	bool changefunc;
	int next;
	struct hand *lhand;
	struct weaponfunc *func;

	hand->lastdirvalid = false;
	hand->burstbullets = 0;
	hand->animframeincfreal = hand->animframeinc;
	hand->shotremainder = 0;

	// If ready to change gun due to manual switch, just do that
	if (bgunIsReadyToSwitch(handnum) && bgunSetState(handnum, HANDSTATE_CHANGEGUN)) {
		return lvupdate;
	}

	if (gunfunc == hand->gset.weaponfunc) {
		hand->unk0cc8_07 = false;
	}

	hand->unk0cc8_08 = false;

	if (hand->inuse) {
		reloadtype = bgunDetermineReloadType(hand->gset.weaponfunc, info, hand);

		// Handle changing gun function
		if (gunfunc != hand->gset.weaponfunc && hand->modenext != HANDMODE_RELOAD) {
			changefunc = true;

			if (hand->unk0cc8_07 && bgunDetermineReloadType(1 - hand->gset.weaponfunc, info, hand) < 0) {
				changefunc = false;
			}

			if (changefunc && info->weaponnum == WEAPON_COMBATKNIFE) {
				if (reloadtype == 0) { // Trigger Reload
					hand->count60 = 0;
					hand->count = 0;
					hand->gset.weaponfunc = gunfunc;

					if (bgunSetState(handnum, HANDSTATE_RELOAD)) {
						return lvupdate;
					}
				} else {
					if (reloadtype < 0) { // No ammo or gun doesn't exist
						changefunc = false;
					}
				}
			}

			if (changefunc) {
				hand->unk0cc8_07 = false;

				if (bgunSetState(handnum, HANDSTATE_CHANGEFUNC)) {
					return lvupdate;
				}
			}
		}

		if (reloadtype < 0) { // No ammo or gun doesn't exist
			// Attempted to shoot with no ammo

			// Consider switching to another weapon
			if (weaponHasFlag(info->weaponnum, WEAPONFLAG_THROWABLE)
					&& (info->weaponnum != WEAPON_REMOTEMINE || handnum != HAND_LEFT)
					&& bgunSetState(handnum, HANDSTATE_AUTOSWITCH)) {
				return lvupdate;
			}

			// Consider switching to other gun function
			usesec = FUNCISSEC();

			if (usesec == gunfunc) {
				sp30 = bgunDetermineReloadType(1 - hand->gset.weaponfunc, info, hand);

				if (bgun0f099188(hand, 1 - hand->gset.weaponfunc)
						&& info->weaponnum != WEAPON_REAPER) {
					if (info->gunctrl->wantammo) {
						func = weaponGetFunction(&hand->gset, 1 - hand->gset.weaponfunc);

						if ((func->type & 0xff) != INVENTORYFUNCTYPE_MELEE) {
							sp30 = -1;
						}
					} else {
						sp30 = -1;
					}
				}

				if (sp30 < 0) {
					hand->unk0cc8_08 = true;
				} else {
					if (!weaponHasFlag(info->weaponnum, WEAPONFLAG_04000000)
							|| hand->gset.weaponfunc == FUNC_SECONDARY) {
						hand->unk0cc8_07 = true;

						if (bgunSetState(handnum, HANDSTATE_CHANGEFUNC)) {
							return lvupdate;
						}
					}
				}
			}
		} else if (reloadtype == 0) {
			// Clip is empty
			if (hand->triggeron && info->weaponnum != WEAPON_NONE) {
				hand->unk0cc8_01 = false;

				if (bgunSetState(handnum, HANDSTATE_ATTACKEMPTY)) {
					return lvupdate;
				}
			} else {
				hand->count60 = 0;
				hand->count = 0;

				if (bgunSetState(handnum, HANDSTATE_RELOAD)) {
					hand->modenext = HANDMODE_NONE;
					return lvupdate;
				}
			}
		} else {
			// Clip has ammo
			if (hand->triggeron || (hand->activatesecondary && hand->gset.weaponfunc == FUNC_SECONDARY)) {
				if (info->weaponnum != WEAPON_NONE) {
					g_Vars.currentplayer->doautoselect = false;

					hand->mode = HANDMODE_ATTACK;
					hand->count = 0;
					hand->count60 = 0;
					hand->triggerreleased = false;
					hand->activatesecondary = false;

					if (bgunSetState(handnum, HANDSTATE_ATTACK)) {
						return lvupdate;
					}
				}
			}

			// Not attacking, but the player may have attempted
			// to change guns or reload while firing
			if (hand->modenext != HANDMODE_NONE) {
				next = hand->modenext;

				hand->mode = hand->modenext;
				hand->count60 = 0;
				hand->count = 0;
				hand->modenext = HANDMODE_NONE;

				if (next == HANDMODE_RELOAD && reloadtype < 2 && reloadtype >= 0) {
					if (bgunSetState(handnum, HANDSTATE_RELOAD)) {
						if (handnum && handnum && handnum);
						return lvupdate;
					}
				}
			}
		}
	}

	if (handnum == HAND_RIGHT) {
		if (info->gunctrl->wantammo) {
			bgunAutoSwitchWeapon();
		} else {
			lhand = &g_Vars.currentplayer->hands[1] - handnum;

			if ((hand->unk0cc8_08 || !hand->inuse)
					&& (lhand->unk0cc8_08 || !lhand->inuse)
					&& (hand->triggeron || lhand->triggeron)) {
				bgunAutoSwitchWeapon();
			}

			hand->unk0cc8_08 = lhand->unk0cc8_08 = false;
		}
	}

	return 0;
}

void bgunSetArmPitch(struct hand *hand, float angle)
{
	hand->useposrot = true;

	mtx4LoadXRotation(angle, &hand->posrotmtx);

	hand->posrotmtx.m[3][0] = 0;
	hand->posrotmtx.m[3][1] = (1.0f - cosf(angle)) * -80.0f;
	hand->posrotmtx.m[3][2] = sinf(angle) * 15.0f;
}

int bgunTickIncAutoSwitch(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	int reloadtype;
	int gunfunc = bgunIsUsingSecondaryFunction();

	if (!hand->inuse && bgunSetState(handnum, HANDSTATE_IDLE)) {
		return lvupdate;
	}

	if (hand->stateminor == HANDSTATEMINOR_AUTOSWITCH_UNEQUIP) {
		int delay = TICKS(16);

		if (g_Vars.normmplayerisrunning) {
			delay = TICKS(12);
		}

		if (hand->stateframes >= delay) {
			hand->stateminor++; // to HANDSTATEMINOR_AUTOSWITCH_DELETE
		} else {
			bgunSetArmPitch(hand, hand->stateframes * MAX_PITCH / delay);
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_AUTOSWITCH_DELETE) {
		hand->lastdirvalid = false;
		hand->animframeincfreal = hand->animframeinc;
		hand->shotremainder = 0;

		if (bgunIsReadyToSwitch(handnum) && bgunSetState(handnum, HANDSTATE_CHANGEGUN)) {
			if (g_Vars.mplayerisrunning) {
				playermgrDeleteWeapon(handnum);
			}

			bgunFreeHeldRocket(handnum);

			hand->mode = HANDMODE_6;
			hand->stateminor = HANDSTATEMINOR_AUTOSWITCH_2;
			hand->count = 0;
			return 0;
		}

		if (hand->inuse) {
			reloadtype = bgunDetermineReloadType(gunfunc, info, hand);

			if (info->weaponnum == WEAPON_TIMEDMINE || info->weaponnum == WEAPON_PROXIMITYMINE) {
				hand->gset.weaponfunc = gunfunc;
			}

			if (info->weaponnum == WEAPON_REMOTEMINE
					&& gunfunc != hand->gset.weaponfunc
					&& bgunSetState(handnum, HANDSTATE_CHANGEFUNC)) {
				return lvupdate;
			}

			if (g_Vars.currentplayer->doautoselect) {
				struct hand *otherhand = &g_Vars.currentplayer->hands[1 - handnum];
				struct handweaponinfo otherinfo;
				bool ready = true;

				bgunGetWeaponInfo(&otherinfo, 1 - handnum);

				if (otherhand->inuse) {
					if (bgunDetermineReloadType(0, &otherinfo, otherhand) >= 0) {
						ready = false;
					}

					if (bgunDetermineReloadType(1, &otherinfo, otherhand) >= 0) {
						ready = false;
					}

					if (bgun0f099188(otherhand, otherhand->gset.weaponfunc)) {
						ready = true;
					}
				}

				if (otherhand->state != HANDSTATE_IDLE && otherhand->state != HANDSTATE_AUTOSWITCH) {
					ready = false;
				}

				if (ready) {
					bgunAutoSwitchWeapon();
				}
			}

			// reloadtype:
			// * 0 = trigger reload
			// * 1 = has ammo in clip and ammo in reserve
			if (reloadtype <= 1 && reloadtype >= 0) { 
				if (g_Vars.currentplayer->hands[1 - handnum].state != HANDSTATE_RELOAD) {
					hand->count60 = 0;
					hand->count = 0;

					if (bgunSetState(handnum, HANDSTATE_RELOAD)) {
						if (info->weaponnum == WEAPON_COMBATKNIFE) {
							hand->mode = HANDMODE_11;
							hand->pausetime60 = TICKS(17);
							hand->count60 = 0;
							hand->count = -1;
							hand->stateminor = HANDSTATEMINOR_AUTOSWITCH_2;
						}

						return lvupdate;
					}
				}
			}

			if (hand->modenext) {
				hand->mode = hand->modenext;
				hand->count60 = 0;
				hand->count = 0;
				hand->modenext = HANDMODE_NONE;
			}
		}

		bgunSetArmPitch(hand, MAX_PITCH);
	}

	return 0;
}

bool bgunIsReloading(struct hand *hand)
{
	if (hand->state == HANDSTATE_RELOAD) {
		return true;
	}

	return false;
}

int bgunTickIncReload(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	struct weaponfunc *func = gsetGetWeaponFunction(&hand->gset);

	if (g_Vars.currentplayer->isdead) {
		hand->animmode = HANDANIMMODE_IDLE;
		hand->animload = -1;

		if (bgunSetState(handnum, HANDSTATE_IDLE)) {
			return lvupdate;
		}
	}

	if (hand->statecycles == 0) {
		struct hand *otherhand = &g_Vars.currentplayer->hands[1 - handnum];

		hand->gs_int1 = -1;
		hand->gs_int2 = 0;

		if (otherhand->state == HANDSTATE_RELOAD && otherhand->stateframes < TICKS(20)) {
			hand->stateminor = HANDSTATEMINOR_RELOAD_WAIT;
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_RELOAD_WAIT) {
		struct hand *otherhand = &g_Vars.currentplayer->hands[1 - handnum];

		if (otherhand->state == HANDSTATE_RELOAD && otherhand->stateframes < TICKS(20)) {
			return 0;
		}

		hand->stateframes = 0;
		hand->statecycles = 0;
		hand->stateminor = HANDSTATEMINOR_RELOAD_MAIN;
		hand->statelastframe = 0;
	}

	if (hand->stateminor == HANDSTATEMINOR_RELOAD_MAIN) {
		if (hand->statecycles == 0) {
			if (func && (func->ammoindex == 0 || func->ammoindex == 1)) {
				if (info->definition->ammos[func->ammoindex]->reload_animation
						&& info->weaponnum != WEAPON_COMBATKNIFE) {
					bgunStartAnimation(info->definition->ammos[func->ammoindex]->reload_animation, handnum, hand);

					hand->unk0d0e_07 = true;

					if (info->definition->ammos[func->ammoindex]->flags & AMMOFLAG_INCREMENTALRELOAD) {
						hand->incrementalreloading = true;
					}

					if (info->weaponnum == WEAPON_GRENADE || info->weaponnum == WEAPON_NBOMB) {
						hand->ejectstate = EJECTSTATE_INACTIVE;
					}
				} else {
					hand->stateminor++; // to HANDSTATEMINOR_RELOAD_LOWER
				}
			} else {
				if (bgunSetState(handnum, HANDSTATE_IDLE)) {
					return lvupdate;
				}
			}
		} else {
			if (info->definition->ammos[func->ammoindex]->flags & AMMOFLAG_INCREMENTALRELOAD) {
				if (bgun0f098a44(hand, 1)) {
					if ((hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
						int value;

						bgunRefillMagazine(hand->gset.weaponfunc, info, hand, 1, 0);
						hand->stateflags |= HANDSTATEFLAG_00000010;
						value = bgunDetermineReloadType(hand->gset.weaponfunc, info, hand);

						if (value >= 2) {
							hand->incrementalreloading = false;
						}

						if (value == -1) {
							hand->incrementalreloading = false;
						}
					}
				} else {
					hand->stateflags = 0;
				}

				if (hand->triggeron) {
					hand->incrementalreloading = false;
				}

				if (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY) {
					hand->incrementalreloading = false;
				}
			} else {
				if ((hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
					if (bgun0f098a44(hand, 1)) {
						bgunRefillMagazine(hand->gset.weaponfunc, info, hand, 0, 0);
						hand->stateflags |= HANDSTATEFLAG_00000010;
					}
				}
			}

			if (hand->animmode != HANDANIMMODE_BUSY) {
				if (bgunSetState(handnum, HANDSTATE_IDLE)) {
					return lvupdate;
				}
			}
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_RELOAD_LOWER) {
		if (hand->count60 > TICKS(15) || !hand->visible) {
			hand->mode = HANDMODE_11;
			hand->stateminor++; // to HANDSTATEMINOR_RELOAD_SOUND
			hand->pausetime60 = TICKS(17);
			hand->count60 = 0;
			hand->count = 0;
		} else {
			bgunSetArmPitch(hand, hand->count60 * MAX_PITCH / TICKS(16));
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_RELOAD_SOUND) {
		if (hand->count == 0) {
			if (info->weaponnum == WEAPON_COMBATKNIFE
					&& func->ammoindex >= 0
					&& info->definition->ammos[func->ammoindex]->reload_animation) {
				bgunStartAnimation(info->definition->ammos[func->ammoindex]->reload_animation, handnum, hand);
				hand->unk0cc8_02 = true;
			}

			if ((hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
				bgunRefillMagazine(hand->gset.weaponfunc, info, hand, 0, 0);
			}

			if (g_Vars.lvupdate240 > 0
					&& g_Vars.currentplayer->cameramode != CAMERAMODE_THIRDPERSON
					&& bgunIsLoaded()
					&& !g_PlayerInvincible
					&& !g_Vars.currentplayer->isdead) {
				switch (info->weaponnum) {
				case WEAPON_NONE:
				case WEAPON_UNARMED:
				case WEAPON_COMBATKNIFE:
				case WEAPON_LASER:
				case WEAPON_GRENADE:
				case WEAPON_TIMEDMINE:
				case WEAPON_PROXIMITYMINE:
				case WEAPON_REMOTEMINE:
				case WEAPON_ECMMINE:
				case WEAPON_COMMSRIDER:
				case WEAPON_TRACERBUG:
				case WEAPON_TARGETAMPLIFIER:
				case WEAPON_BRIEFCASE2:
					// No reload sound
					break;
				default:
					sndStart(var80095200, SFX_RELOAD_DEFAULT, 0, -1, -1, -1, -1, -1);
					break;
				}
			}
		}

		if (hand->count60 >= hand->pausetime60 && hand->count >= 2) {
			hand->mode = HANDMODE_12;
			hand->stateminor++; // to HANDSTATEMINOR_RELOAD_RAISE
			hand->count60 = 0;
			hand->count = 0;
		} else {
			bgunSetArmPitch(hand, MAX_PITCH);
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_RELOAD_RAISE) {
		if (info->weaponnum == WEAPON_COMBATKNIFE) {
			hand->animmode = HANDANIMMODE_IDLE;
		}

		if (hand->count == 0) {
			g_Vars.currentplayer->doautoselect = false;
		}

		if (hand->count60 >= TICKS(23)
				|| !weaponGetFileNum2(info->weaponnum)
				|| !weaponHasFlag(info->weaponnum, WEAPONFLAG_00000040)
				|| weaponHasFlag(info->weaponnum, WEAPONFLAG_00000080)) {
			hand->mode = HANDMODE_NONE;
			hand->count60 = 0;
			hand->count = 0;

			if (bgunSetState(handnum, HANDSTATE_IDLE)) {
				return lvupdate;
			}
		} else {
			bgunSetArmPitch(hand, (TICKS(23) - hand->count60) * MAX_PITCH / TICKS(23));
		}
	}

	return 0;
}

int bgunTickIncChangeFunc(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	struct guncmd *cmd;
	bool more = false;

	if (hand->statecycles == 0) {
		if (hand->gset.weaponfunc == FUNC_PRIMARY) {
			cmd = gsetGetPriToSecAnim(&hand->gset);
			hand->gset.weaponfunc = FUNC_SECONDARY;
		} else {
			cmd = gsetGetSecToPriAnim(&hand->gset);
			hand->gset.weaponfunc = FUNC_PRIMARY;
		}

		more = false;

		if (cmd != NULL) {
			bgunStartAnimation(cmd, handnum, hand);
			more = true;
			g_Vars.currentplayer->hands[HAND_RIGHT].unk0dd4 = -1;
		}
	} else {
		if (hand->animmode == HANDANIMMODE_BUSY) {
			more = true;
		}
	}

	if (!more && bgunSetState(handnum, HANDSTATE_IDLE)) {
		return lvupdate;
	}

	return 0;
}

// return values:
// -1 = clip is empty
// 0  = can't fire yet
// 1  = fire a single shot
// 2  = fire a burst
int bgunAttemptFire(struct hand *hand, struct weaponfunc *func)
{
	bool burst = false;
	bool smallburst = false;
	struct gunctrl *ctrl = &g_Vars.currentplayer->gunctrl;

	if ((func->flags & FUNCFLAG_BURST3) && hand->burstbullets < 3) {
		// Make automatics do single shot when holding aim
		if (!g_Vars.currentplayer->insightaimmode || (func->type & 0xff00) != 0x100) {
			// Not aiming and not an automatic weapon
			smallburst = true;
		}
	}

	if ((func->flags & FUNCFLAG_BURST2) && hand->burstbullets < 2) {
		smallburst = true;
	}

	if ((func->flags & FUNCFLAG_BURST5) && hand->burstbullets < 5) {
		smallburst = true;
	}

	if ((func->flags & FUNCFLAG_BURST50) && hand->burstbullets < 50) {
		burst = true;
	}

	if (smallburst) {
		burst = true;
	}

	if (hand->triggeron || (hand->stateflags & HANDSTATEFLAG_00000010) == 0 || burst) {
		if (func->ammoindex >= 0
				&& hand->loadedammo[func->ammoindex] == 0
				&& ctrl->ammotypes[func->ammoindex] >= 0) {
			// Clip is empty
			return -1;
		}

		if ((func->type & 0xff00) == 0x100) {
			struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *) func;

			if (autofunc->turretaccel > 0) {
				if (hand->gs_float1 < 1) {
					hand->gs_float1 += LVUPDATE60FREAL() / autofunc->turretaccel;

					if (hand->gs_float1 > 1) {
						hand->gs_float1 = 1;
						return 1;
					}
				}
			} else {
				hand->gs_float1 = 1;
			}

			return 1;
		}

		hand->gs_float1 = 1;

		if (smallburst) {
			if (hand->burstbullets > 0) {
				int delay = 3;

				if (hand->gset.weaponnum == WEAPON_SHOTGUN) {
					delay = TICKS(13);
				}

				if (hand->stateframes < delay) {
					return 0;
				}
			}

			hand->stateframes = 0;
		}

		if ((func->flags & FUNCFLAG_BURST3) && hand->burstbullets == 2) {
			smallburst = false;
		}

		if ((func->flags & FUNCFLAG_BURST2) && hand->burstbullets == 1) {
			smallburst = false;
		}

		if ((func->flags & FUNCFLAG_BURST5) && hand->burstbullets == 4) {
			smallburst = false;
		}

		if (smallburst) {
			return 1;
		}

		return 2;
	}

	if ((func->type & 0xff00) == (INVENTORYFUNCTYPE_SHOOT_AUTOMATIC & 0xff00)) {
		struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *) func;

		if (autofunc->turretdecel > 0) {
			if (hand->gs_float1 > 0) {
				hand->gs_float1 -= LVUPDATE60FREAL() / autofunc->turretdecel;

				if (hand->gs_float1 < 0) {
					hand->gs_float1 = 0;
					return -1;
				}

				return 1;
			}
		} else {
			hand->gs_float1 = 0;
		}

		return -1;
	}

	return -1;
}

void bgun0f09a6f8(struct handweaponinfo *info, int handnum, struct hand *hand, struct weaponfunc *func)
{
	bool usesammo = true;

	static uint32_t rontime = 2;
	static uint32_t rofftime = 4;

	hand->firing = true;

	if ((func->type & 0xff00) == 0x100) {
		struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *) func;
		float tmp;
		float tmp2;

		tmp = autofunc->initialrpm + (autofunc->maxrpm - autofunc->initialrpm) * hand->gs_float1;
		tmp2 = tmp / 60.0f * (LVUPDATE60FREAL() / 60.0f) + hand->shotremainder;

		hand->shotstotake = tmp2;
		hand->shotremainder = tmp2 - hand->shotstotake;

		if (hand->shotstotake <= 0) {
			if ((hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
				hand->shotstotake++;
			} else {
				hand->firing = false;
			}
		}
	} else {
		hand->shotstotake = 1;

		if (hand->gset.weaponnum == WEAPON_LASER) {
			usesammo = false;
		}
	}

	hand->burstbullets += hand->shotstotake;

	if (func->flags & FUNCFLAG_NOMUZZLEFLASH) {
		hand->flashon = false;
	} else {
		if (g_BgunGeMuzzleFlashes) {
			if (func->type == INVENTORYFUNCTYPE_SHOOT_SINGLE || (hand->shotstotake & 1)) {
				hand->flashon = true;
			}
		} else {
			hand->flashon = true;
		}
	}

	bgunStartSlide(handnum);

	hand->loadslide = 0;

	if (hand->firing) {
		hand->statevar1 = hand->stateframes;
		hand->stateflags |= HANDSTATEFLAG_00000020;
		hand->stateflags |= HANDSTATEFLAG_00000010;

		bgunRumble(handnum, info->weaponnum);

		if (usesammo && func->ammoindex >= 0) {
			hand->loadedammo[func->ammoindex] -= hand->shotstotake;

			if (hand->loadedammo[func->ammoindex] < 0) {
				// Note: loadedammo is negative
				hand->shotstotake += hand->loadedammo[func->ammoindex];
				hand->loadedammo[func->ammoindex] = 0;
			}
		}

		switch (func->type & 0xff00) {
		case 0:
		case 0x100:
			hand->attacktype = HANDATTACKTYPE_SHOOT;
			break;
		case 0x200:
			hand->attacktype = HANDATTACKTYPE_SHOOTPROJECTILE;
			break;
		}
	}

	if (hand->firing) {
		bool playsound = false;

		if (gsetGetFireslotDuration(&hand->gset) > 0) {
			if (g_Vars.lvframe60 != g_Vars.currentplayer->hands[1 - handnum].lastshootframe60
					&& g_Vars.lvframe60 > hand->allowshootframe) {
				hand->allowshootframe = g_Vars.lvframe60 + gsetGetFireslotDuration(&hand->gset);
				playsound = true;
			}
		} else {
			if (hand->firing) {
				playsound = true;
			}
		}

		if (playsound) {
			if (hand->audiohandle2 && sndGetState(hand->audiohandle2) != AL_STOPPED) {
				audioStop(hand->audiohandle2);
			}

			if (hand->audiohandle3 && sndGetState(hand->audiohandle3) != AL_STOPPED) {
				audioStop(hand->audiohandle3);
			}

			if (gsetGetSingleShootSound(&hand->gset)) {
				struct sndstate *handle = NULL;

				if (hand->audiohandle2 == NULL) {
					handle = sndStart(var80095200, gsetGetSingleShootSound(&hand->gset), &hand->audiohandle2, -1, -1, -1, -1, -1);
				} else if (hand->audiohandle3 == NULL) {
					handle = sndStart(var80095200, gsetGetSingleShootSound(&hand->gset), &hand->audiohandle3, -1, -1, -1, -1, -1);
				}

				hand->lastshootframe60 = g_Vars.lvframe60;

				if (hand->gset.weaponnum == WEAPON_MAULER && handle) {
					int matmot = hand->matmot1;
					float tmp;
					float frac = matmot / 3.0f;

					if (frac > 1.0f) {
						frac = 1.0f;
					}

					tmp = 1.0f - frac * 0.4f;

					audioPostEvent(handle, AL_SNDP_PITCH_EVT, *(int *) &tmp);
				}

			}
		}
	}
}

bool bgun0f09aba4(struct hand *hand, struct handweaponinfo *info, int handnum, struct weaponfunc_shoot *func)
{
	int unk24;
	int unk25;
	int sum;
	int unk26;
	int unk27;
	int recoverytime60;
	int frames;
	struct weapon *weapondef;
	float mult1;
	float recoildist;
	float recoilangle;
	float mult2;

	unk24 = func->unk24;
	unk25 = func->unk25;
	sum = unk24 + unk25;
	unk26 = func->unk26;
	unk27 = func->unk27;
	recoverytime60 = func->recoverytime60;
	weapondef = info->definition;

	frames = hand->stateframes - hand->statevar1;

	if (sum < 1) {
		sum = 0;
	} else {
		if (hand->triggerreleased
				&& hand->triggeron
				&& frames >= unk26
				&& unk26 > 0
				&& unk27 >= 0
				&& (hand->stateflags & HANDSTATEFLAG_00000040) == 0
				&& frames + unk27 < sum) {
			hand->stateflags |= HANDSTATEFLAG_00000040;
			hand->statevar1 = frames;

			hand->rotxstart = hand->rotxoffset;
			hand->rotxend = 0;

			hand->posend.x = 0;
			hand->posend.y = 0;
			hand->posend.z = 0;

			hand->posstart.x = hand->posoffset.x;
			hand->posstart.y = hand->posoffset.y;
			hand->posstart.z = hand->posoffset.z;
		}

		if (hand->stateflags & HANDSTATEFLAG_00000040) {
			if (unk27 > frames - hand->statevar1) {
				mult1 = cosf((float)(unk27 - frames + hand->statevar1) * 1.5707963705063f / (float)unk27) * 0.5f + 0.5f;

				hand->rotxoffset = modelTweenRotAxis(hand->rotxstart, hand->rotxend, mult1);
				hand->useposrot = true;

				hand->posoffset.x = (hand->posend.x - hand->posstart.x) * mult1 + hand->posstart.x;
				hand->posoffset.y = (hand->posend.y - hand->posstart.y) * mult1 + hand->posstart.y;
				hand->posoffset.z = (hand->posend.z - hand->posstart.z) * mult1 + hand->posstart.z;

				mtx4LoadXRotation(hand->rotxoffset, &hand->posrotmtx);
				mtx4SetTranslation(&hand->posoffset, &hand->posrotmtx);
			} else {
				mtx4LoadIdentity(&hand->posrotmtx);
				hand->useposrot = false;
				return true;
			}
		}

		if (frames < sum && (hand->stateflags & HANDSTATEFLAG_00000040) == 0) {
			recoildist = func->recoildist;
			recoilangle = func->recoilangle;

			if ((hand->stateflags & HANDSTATEFLAG_00000080) == 0) {
				hand->stateflags |= HANDSTATEFLAG_00000080;
				hand->rotxstart = hand->rotxoffset;
				hand->posstart.x = hand->posoffset.x;
				hand->posstart.y = hand->posoffset.y;
				hand->posstart.z = hand->posoffset.z;
			}

			hand->rotxend = M_TAU - (recoilangle * M_TAU) / 360.0f;

			hand->posend.x = (func0f0b131c(handnum) - hand->aimpos.x) * recoildist / 1000.0f;
			hand->posend.y = 0;
			hand->posend.z = (weapondef->posz - hand->aimpos.z) * recoildist / 1000.0f;

			if (frames < unk24) {
				mult2 = sinf(frames * 1.5707963705063f / (float)unk24);
			} else {
				mult2 = cosf((float)(frames - unk24) * M_PI / (float)unk25) * 0.5f + 0.5f;
			}

			hand->rotxoffset = modelTweenRotAxis(hand->rotxstart, hand->rotxend, mult2);
			hand->useposrot = true;

			hand->posoffset.x = (hand->posend.x - hand->posstart.x) * mult2 + hand->posstart.x;
			hand->posoffset.y = (hand->posend.y - hand->posstart.y) * mult2 + hand->posstart.y;
			hand->posoffset.z = (hand->posend.z - hand->posstart.z) * mult2 + hand->posstart.z;

			mtx4LoadXRotation(hand->rotxoffset, &hand->posrotmtx);
			mtx4SetTranslation(&hand->posoffset, &hand->posrotmtx);
		}
	}

	if (sum <= frames) {
		if (unk27 >= 0 && hand->triggerreleased && hand->triggeron) {
			return true;
		} else if (sum + recoverytime60 <= frames) {
			return true;
		}
	}

	return false;
}

bool bgunTickIncAttackingShoot(struct handweaponinfo *info, int handnum, struct hand *hand)
{
	static uint32_t var80070128 = 99;

	struct weaponfunc *func = gsetGetWeaponFunction(&hand->gset);
	bool sp68;
	int sp64;
	int attemptfireresult;

	if (func == NULL) {
		return true;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SHOOT_0) {
		sp64 = 1;

		if (hand->statecycles == 0) {
			hand->gs_float1 = 0;

			if (func->fire_animation) {
				bgunStartAnimation(func->fire_animation, handnum, hand);
				hand->unk0cc8_01 = true;
			}

			hand->burstbullets = 0;
		}

		if (!bgun0f098a44(hand, 2)) {
			sp64 = 0;
		}

		if (sp64) {
			hand->stateminor = HANDSTATEMINOR_ATTACK_SHOOT_1;
		}

		hand->matmot2 = hand->gs_float1;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SHOOT_1) {
		attemptfireresult = bgunAttemptFire(hand, func);

		if ((func->type & 0xff00) == 0x100) {
			struct weaponfunc_shootauto *autofunc = (struct weaponfunc_shootauto *) func;
			float floats[12];

			if (autofunc->vibrationstart != NULL && autofunc->vibrationmax != NULL) {
				Lerp2D(autofunc->vibrationstart, autofunc->vibrationmax, hand->gs_float1, floats);
				ScaleVector2D(hand->upgrademult, floats, hand->finalmult);
			}
		}

		// Firing was successful
		if (attemptfireresult > 0) {
			bgun0f09a6f8(info, handnum, hand, func);
		}

		// Clip is empty or burst fire
		if (attemptfireresult < 0 || attemptfireresult == 2) {
			hand->stateminor = HANDSTATEMINOR_ATTACK_SHOOT_2;
		}

		hand->matmot2 = hand->gs_float1;

		if (hand->triggeron && hand->matmot2 < 0.4f) {
			hand->matmot2 = 0.4f;
		}

		if (hand->triggerreleased) {
			hand->unk0cc8_01 = false;
		}

		return false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SHOOT_2) {
		if (hand->stateflags & HANDSTATEFLAG_00000020) {
			sp68 = bgun0f09aba4(hand, info, handnum, (struct weaponfunc_shoot *) func);
		} else {
			sp68 = true;
		}

		if (hand->gset.weaponnum == WEAPON_SHOTGUN && hand->animmode == HANDANIMMODE_BUSY) {
			sp68 = false;
		}

		hand->matmot2 = hand->gs_float1;

		if (sp68 && !hand->triggeron) {
			hand->matmot2 = 0;
		}

		if (hand->gset.weaponnum == WEAPON_MAULER) {
			hand->matmot1 = 0;
		}

		return sp68;
	}

	return false;
}

bool bgunTickIncAttackingThrow(int handnum, struct hand *hand)
{
	struct weaponfunc_throw *func = (struct weaponfunc_throw *) gsetGetWeaponFunction(&hand->gset);

	if (func == NULL) {
		return true;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_THROW_0) {
		if (hand->statecycles == 0) {
			if (func->base.flags & FUNCFLAG_DISCARDWEAPON) {
				invRemoveItemByNum(hand->gset.weaponnum);
				g_Vars.currentplayer->gunctrl.throwing = true;
				bgunSwitchToPrevious();
				hand->primetimer60 = 0;
				return true;
			}

			if (func->base.fire_animation) {
				bgunStartAnimation(func->base.fire_animation, handnum, hand);
				hand->unk0cc8_01 = true;
			}
		}

		if (func->base.fire_animation) {
			if (hand->triggerreleased) {
				hand->unk0cc8_01 = false;
			}

			if (bgun0f098a44(hand, 2)) {
				hand->stateminor = HANDSTATEMINOR_ATTACK_THROW_1;
				hand->unk0cc8_01 = false;
			}
		} else {
			hand->stateminor = HANDSTATEMINOR_ATTACK_THROW_1;
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_THROW_1) {
		hand->firing = true;
		hand->attacktype = HANDATTACKTYPE_THROWPROJECTILE;
		hand->loadedammo[func->base.ammoindex]--;
		hand->stateminor = HANDSTATEMINOR_ATTACK_THROW_2;
		return false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_THROW_2) {
		if (hand->stateframes > TICKS(func->recoverytime60)) {
			return true;
		}

		if (hand->gset.weaponnum == WEAPON_REMOTEMINE
				&& bgunIsUsingSecondaryFunction() == true
				&& hand->triggerreleased
				&& hand->triggeron) {
			return true;
		}

		return false;
	}

	// This state is only used after having a grenade explode in the player's
	// hand. It waits 4 seconds before finishing, which means the player won't
	// pull out another grenade until the flames have cleared.
	if (hand->stateminor == HANDSTATEMINOR_ATTACK_THROW_GRENADEWAIT) {
		bgunResetAnim(hand);

		if (hand->stateframes > TICKS(func->activatetime60 + 240)) {
			return true;
		}

		return false;
	}

	hand->primetimer60 = hand->stateframes;

	// If held a grenade too long, force throw it and enter the wait state
	if (hand->gset.weaponnum == WEAPON_GRENADE
			&& hand->gset.weaponfunc == FUNC_PRIMARY
			&& hand->primetimer60 > TICKS(func->activatetime60)) {
		hand->firing = true;
		hand->attacktype = HANDATTACKTYPE_THROWPROJECTILE;
		hand->loadedammo[func->base.ammoindex]--;
		hand->stateminor = HANDSTATEMINOR_ATTACK_THROW_GRENADEWAIT;

		return false;
	}

	return false;
}

int bgunGetMinClipQty(int weaponnum, int funcnum)
{
	if (weaponnum == WEAPON_TRANQUILIZER && funcnum == FUNC_SECONDARY) {
		return 4;
	}

	return 1;
}

bool bgunTickIncAttackingMelee(int handnum, struct hand *hand)
{
	struct weaponfunc *func = gsetGetWeaponFunction(&hand->gset);

	if (func == NULL) {
		return true;
	}

	if (hand->gset.weaponnum == WEAPON_REAPER) {
		if (hand->statecycles == 0) {
			hand->matmot2 = 0.1f;
			hand->burstbullets = 0;
		}

		hand->firing = true;
		hand->attacktype = HANDATTACKTYPE_MELEE;
		hand->burstbullets++;

		if (hand->triggeron) {
			hand->matmot2 += 0.01f * LVUPDATE60FREAL();

			if (hand->matmot2 > 1) {
				hand->matmot2 = 1;
			}
		} else {
			hand->matmot2 = 0;
			return true;
		}

		return false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_MELEE_0) {
		if (hand->statecycles == 0) {
			hand->firing = true;
			hand->attacktype = HANDATTACKTYPE_MELEENOUNCLOAK;

			if (func->fire_animation) {
				bgunStartAnimation(func->fire_animation, handnum, hand);
				hand->unk0cc8_01 = true;
			}
		}

		if (func->fire_animation) {
			if (hand->triggerreleased) {
				hand->unk0cc8_01 = false;
			}

			if (bgun0f098a44(hand, 2)) {
				hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_1;
				hand->unk0cc8_01 = false;
			}
		} else {
			hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_1;
		}
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_MELEE_3 && bgun0f098a44(hand, 3)) {
		hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_1;
		hand->unk0cc8_01 = false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_MELEE_1) {
		hand->firing = true;
		hand->attacktype = HANDATTACKTYPE_MELEE;

		if (hand->gset.weaponnum == WEAPON_TRANQUILIZER && func->ammoindex >= 0) {
			if (hand->loadedammo[func->ammoindex] > bgunGetMinClipQty(WEAPON_TRANQUILIZER, FUNC_SECONDARY)) {
				hand->loadedammo[func->ammoindex] -= bgunGetMinClipQty(WEAPON_TRANQUILIZER, FUNC_SECONDARY);
			} else {
				hand->loadedammo[func->ammoindex] = 0;
			}
		}

		if (func->fire_animation) {
			if (func->fire_animation && !bgun0f098a44(hand, 3)) {
				hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_3;
			} else {
				hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_2;
			}
		}

		if (cheatIsActive(CHEAT_HURRICANEFISTS)) {
			hand->stateminor = HANDSTATEMINOR_ATTACK_MELEE_2;
		}

		return false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_MELEE_2) {
		if (!bgunIsAnimBusy(hand)) {
			return true;
		}

		if (cheatIsActive(CHEAT_HURRICANEFISTS) && hand->gset.weaponnum == WEAPON_UNARMED) {
			return true;
		}

		if (hand->stateframes > TICKS(60)) {
			return true;
		}

		return false;
	}

	return false;
}

bool bgunTickIncAttackingSpecial(struct hand *hand)
{
	struct weaponfunc_special *func = (struct weaponfunc_special *) gsetGetWeaponFunction(&hand->gset);

	if (!func) {
		return true;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SPECIAL_START) {
		hand->stateminor = HANDSTATEMINOR_ATTACK_SPECIAL_EXECUTE;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SPECIAL_EXECUTE) {
		hand->firing = true;
		hand->attacktype = func->specialfunc;

		if (func->base.ammoindex >= 0) {
			hand->loadedammo[func->base.ammoindex]--;
		}

		hand->stateminor = HANDSTATEMINOR_ATTACK_SPECIAL_RECOVER;
		return false;
	}

	if (hand->stateminor == HANDSTATEMINOR_ATTACK_SPECIAL_RECOVER) {
		if (hand->stateframes > TICKS(func->recoverytime60)) {
			return true;
		}

		return false;
	}

	return false;
}

int bgunTickIncAttackEmpty(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	bool playsound = false;

	switch (info->weaponnum) {
	case WEAPON_FALCON2:
	case WEAPON_FALCON2_SILENCER:
	case WEAPON_FALCON2_SCOPE:
	//case WEAPON_FALCON2_SANDS:
	case WEAPON_MAGSEC4:
	case WEAPON_MAULER:
	case WEAPON_PHOENIX:
	case WEAPON_DY357MAGNUM:
	case WEAPON_DY357LX:
	case WEAPON_CMP150:
	case WEAPON_CYCLONE:
	case WEAPON_CALLISTO:
	case WEAPON_RCP120:
	case WEAPON_LAPTOPGUN:
	case WEAPON_REAPER:
	case WEAPON_TRANQUILIZER:
	case WEAPON_PP9I:
	case WEAPON_CC13:
		// These weapons are weapons with visible finger trigger animations
		if (hand->stateframes > TICKS(25)) {
			hand->stateframes -= TICKS(25);
			hand->stateflags = 0;

			bgunResetAnim(hand);
		}

		if (hand->animmode != HANDANIMMODE_BUSY) {
			bool restartedanim = false;

			if ((hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
				struct weaponfunc *func = NULL;

				if (info->definition) {
					func = gsetGetWeaponFunction(&hand->gset);
				}

				if (func && func->fire_animation) {
					bgunStartAnimation(func->fire_animation, handnum, hand);
					restartedanim = true;
				}
			}

			if (!restartedanim && hand->stateframes > TICKS(25)) {
				playsound = true;
			}
		} else if (bgun0f098a44(hand, 5)) {
			playsound = true;
		}
		break;
	default:
		// Weapons without visible trigger animations must
		// still play the click sound every 25 frames
		if (hand->stateframes > TICKS(25)) {
			playsound = true;

			hand->stateframes -= TICKS(25);
			hand->stateflags = 0;

			bgunResetAnim(hand);
		}
	}

	hand->mode = HANDMODE_13;
	hand->count60 = 0;
	hand->count = 0;

	if (playsound && (hand->stateflags & HANDSTATEFLAG_00000010) == 0) {
		hand->stateflags |= HANDSTATEFLAG_00000010;

		switch (info->weaponnum) {
		case WEAPON_PHOENIX:
		case WEAPON_CALLISTO:
		case WEAPON_FARSIGHT:
			{
				// Maian weapons have a wet sounding click effect
				float speed = 2.07f;
				struct sndstate *handle;

				handle = sndStart(var80095200, SFX_HIT_WATER, NULL, -1, -1, -1, -1, -1);

				if (handle) {
					audioPostEvent(handle, AL_SNDP_PITCH_EVT, *(int *)&speed);
				}
			}
			// fall-through - unsure if intentional
		case WEAPON_TRANQUILIZER:
		case WEAPON_PSYCHOSISGUN:
			{
				// The tranquliser and psychosis gun use the standard click
				// effect but slightly faster.
				float speed = 1.5f;
				struct sndstate *handle;
				handle = sndStart(var80095200, SFX_FIREEMPTY, NULL, -1, -1, -1, -1, -1);

				if (handle) {
					audioPostEvent(handle, AL_SNDP_PITCH_EVT, *(int *)&speed);
				}
			}
			break;
		case WEAPON_UNARMED:
		case WEAPON_COMBATKNIFE:
		case WEAPON_GRENADE:
		case WEAPON_NBOMB:
		case WEAPON_TIMEDMINE:
		case WEAPON_PROXIMITYMINE:
		case WEAPON_REMOTEMINE:
		case WEAPON_COMBATBOOST:
			// No sound effect
			break;
		default:
			// Default click sound effect
			sndStart(var80095200, SFX_FIREEMPTY, NULL, -1, -1, -1, -1, -1);
			break;
		}
	}

	// Handle releasing trigger
	if (!hand->triggeron) {
		hand->mode = HANDMODE_NONE;
		hand->count60 = 0;
		hand->count = 0;

		if (bgunSetState(handnum, HANDSTATE_IDLE)) {
			return lvupdate;
		}

		bgunResetAnim(hand);
	}

	return 0;
}

int bgunTickIncAttack(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	struct weaponfunc *func = NULL;
	bool finished = true;

	if (info->definition) {
		func = gsetGetWeaponFunction(&hand->gset);
	}

	if (func != NULL) {
		switch (func->type & 0xff) {
		case INVENTORYFUNCTYPE_SHOOT:
			finished = bgunTickIncAttackingShoot(info, handnum, hand);
			break;
		case INVENTORYFUNCTYPE_THROW:
			finished = bgunTickIncAttackingThrow(handnum, hand);
			break;
		case INVENTORYFUNCTYPE_MELEE:
			finished = bgunTickIncAttackingMelee(handnum, hand);
			break;
		case INVENTORYFUNCTYPE_SPECIAL:
			finished = bgunTickIncAttackingSpecial(hand);
			break;
		}
	}

	if (finished) {
		if (hand->gset.weaponnum == WEAPON_REAPER && hand->triggeron) {
			hand->gset.weaponfunc = FUNC_SECONDARY;
			finished = false;
		}

		if (finished && bgunSetState(handnum, HANDSTATE_IDLE)) {
			return lvupdate;
		}
	}

	return 0;
}

bool bgunIsReadyToSwitch(int handnum)
{
	struct player *player = g_Vars.currentplayer;

	// Dont switch if... something firing range related
	if (g_FrIsValidWeapon
			&& frGetWeaponBySlot(frGetSlot()) == player->hands[HAND_RIGHT].gset.weaponnum
			&& g_Vars.currentplayer->gunctrl.throwing == false) {
		return false;
	}

	// Don't switch right hand if left hand is about to auto switch
	if (handnum == HAND_RIGHT
			&& player->hands[HAND_LEFT].inuse
			&& player->hands[HAND_LEFT].state == HANDSTATE_AUTOSWITCH
			&& player->hands[HAND_LEFT].stateminor == HANDSTATEMINOR_AUTOSWITCH_UNEQUIP) {
		return false;
	}

	if (player->gunctrl.switchtoweaponnum >= 0) {
		return true;
	}

	if (handnum == HAND_LEFT) {
		if (handnum == HAND_LEFT) {
			if (player->hands[HAND_RIGHT].state == HANDSTATE_RELOAD) {
				return false;
			}

			if (player->hands[HAND_RIGHT].state == HANDSTATE_CHANGEFUNC) {
				return false;
			}

			if (player->hands[HAND_RIGHT].state == HANDSTATE_ATTACK) {
				return false;
			}
		}

		if (player->hands[handnum].inuse && !player->gunctrl.dualwielding) {
			return true;
		}

		if (!player->hands[handnum].inuse && player->gunctrl.dualwielding) {
			return true;
		}
	}

	return false;
}

bool bgunCanFreeWeapon(int handnum)
{
	struct player *player = g_Vars.currentplayer;

	if (player->hands[handnum].state == HANDSTATE_CHANGEGUN
			&& player->hands[handnum].stateminor == HANDSTATEMINOR_CHANGEGUN_LOAD
			&& player->hands[handnum].count >= 3
			&& player->gunctrl.throwing == false) {
		return true;
	}

	return false;
}

bool bgun0f09bf44(int handnum)
{
	bool result = true;
	struct player *player = g_Vars.currentplayer;

	if (!bgunIsLoaded()) {
		result = false;
	}

	if (player->gunctrl.switchtoweaponnum != -1) {
		result = false;
	}

	if (handnum == HAND_LEFT && player->gunctrl.dualwielding != player->hands[handnum].inuse) {
		result = false;
	}

	if (player->gunctrl.gunmemnew >= 0) {
		result = false;
	}

	if (player->hands[1 - handnum].state == HANDSTATE_RELOAD) {
		result = false;
	}

	return result;
}

int bgunTickIncChangeGun(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	struct weapon *weapon = info->definition;

	if (hand->statecycles == 0) {
		if (g_Vars.normmplayerisrunning == false) {
			hand->pausetime60 = 0;
		} else {
			hand->pausetime60 = 0;
		}
	}

	// Handle unequip animation. Wait in this state until the animation is
	// finished, or skip this state if there is no animation to play.
	if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_UNEQUIP) {
		bool skipanim = false;

		if (weaponHasFlag(info->weaponnum, WEAPONFLAG_THROWABLE)
				&& !(info->weaponnum == WEAPON_REMOTEMINE && handnum == HAND_LEFT)
				&& bgunDetermineReloadType(0, info, hand) < 0) {
			skipanim = true;
		}

		hand->count = 0;

		if (!skipanim) {
			if (weapon->unequip_animation
					&& hand->inuse == true
					&& !(hand->ejectstate != EJECTSTATE_INACTIVE && hand->ejecttype == EJECTTYPE_GUN)) {
				if (hand->statecycles == 0) {
					bgunStartAnimation(weapon->unequip_animation, handnum, hand);
				} else if (hand->animmode == HANDANIMMODE_IDLE) {
					hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_LOWER
				}
			} else {
				hand->stateflags |= HANDSTATEFLAG_00000001;

				if (hand->ejectstate == EJECTSTATE_INIT) {
					return 0;
				}

				hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_LOWER
			}
		} else {
			hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_LOWER
		}

		if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_LOWER) {
			hand->stateframes = 0;
		}
	}

	// For classic guns, handle lowering it to offscreen.
	// Throw the gun if that's what the player is doing.
	if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_LOWER) {
		int delay = TICKS(16);
		bool throwing = false;

		hand->count = 0;

		if (g_Vars.normmplayerisrunning) {
			delay = TICKS(12);
		}

		if (weapon->unequip_animation && (hand->stateflags & HANDSTATEFLAG_00000001) == 0) {
			delay = 1;
		}

		if (!hand->inuse) {
			delay = 1;
		}

		if (hand->ejecttype == EJECTTYPE_GUN
				&& (hand->ejectstate == EJECTSTATE_INIT || hand->ejectstate == EJECTSTATE_AIRBORNE)) {
			throwing = true;
		}

		if (g_Vars.currentplayer->gunctrl.throwing == true) {
			throwing = true;
		}

		if (hand->stateframes >= delay) {
			if (!throwing) {
				if (g_Vars.mplayerisrunning) {
					playermgrDeleteWeapon(handnum);
				}

				bgunFreeHeldRocket(handnum);
				hand->mode = HANDMODE_6;
				hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_LOAD
			} else {
				bgunSetArmPitch(hand, MAX_PITCH);

				if (g_Vars.currentplayer->gunctrl.throwing == true && hand->inuse) {
					hand->firing = true;
					hand->attacktype = HANDATTACKTYPE_THROWPROJECTILE;
					hand->gset.weaponfunc = FUNC_SECONDARY;
				}
			}
		} else {
			bgunSetArmPitch(hand, hand->stateframes * MAX_PITCH / delay);
		}
	}

	// Wait for the new gun to be loaded, then start its equip animation
	// (if any) and move on to the next state.
	if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_LOAD) {
		hand->animmode = HANDANIMMODE_IDLE;

		if (hand->pausechange == 0 || hand->pausetime60 <= hand->count60) {
			if (hand->mode == HANDMODE_6) {
				if (bgun0f09bf44(handnum)) {
					hand->mode = HANDMODE_7;

					if (!hand->inuse && bgunSetState(handnum, HANDSTATE_IDLE)) {
						return lvupdate;
					}
				}
			} else {
				if (bgunIsLoaded()) {
					if (info->definition->equip_animation) {
						bgunStartAnimation(info->definition->equip_animation, handnum, hand);
						hand->unk0cc8_02 = true;
					}

					hand->mode = HANDMODE_EQUIP;
					hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_RAISE
					hand->count60 = 0;
					hand->count = 0;
				}
			}
		}

		if (hand->mode == HANDMODE_6 || hand->mode == HANDMODE_7) {
			bgunSetArmPitch(hand, MAX_PITCH);
		}
	}

	// Handle raising the new gun and playing the equipped sound effect.
	if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_RAISE) {
		int delay = TICKS(23);

		if (g_Vars.normmplayerisrunning) {
			delay = TICKS(12);
		}

		if (weaponHasFlag(hand->gset.weaponnum, WEAPONFLAG_00004000)) {
			hand->animmode = HANDANIMMODE_IDLE;
		} else if (weapon->equip_animation) {
			delay = 1;
		}

		if (hand->count == 0) {
			if (g_Vars.mplayerisrunning) {
				playermgrCreateWeapon(handnum);
			}

			bgunFillMagazine(info, hand);

			if (weaponHasFlag(info->weaponnum, WEAPONFLAG_THROWABLE)
					&& (info->weaponnum != WEAPON_REMOTEMINE || handnum != HAND_LEFT)
					&& bgunDetermineReloadType(0, info, hand) < 0
					&& bgunSetState(handnum, HANDSTATE_AUTOSWITCH)) {
				hand->stateminor = HANDSTATEMINOR_AUTOSWITCH_DELETE;
				return lvupdate;
			}

			g_Vars.currentplayer->doautoselect = false;

			if (g_Vars.lvupdate240 > 0
					&& g_Vars.currentplayer->cameramode != CAMERAMODE_THIRDPERSON
					&& bgunIsLoaded()
					&& !g_PlayerInvincible
					&& !g_Vars.currentplayer->isdead) {
				struct sndstate *handle1;
				float speed1;
				struct sndstate *handle2;
				float speed2;
				struct sndstate *handle3;
				float speed3;

				switch (info->weaponnum) {
				case WEAPON_HORIZONSCANNER:
					speed1 = 3.5f;
					handle1 = sndStart(var80095200, SFX_EQUIP_HORIZONSCANNER, 0, -1, -1, -1, -1, -1);

					if (handle1) {
						audioPostEvent(handle1, AL_SNDP_PITCH_EVT, *(int *)&speed1);
					}

					break;
				case WEAPON_LASER:
					sndStart(var80095200, SFX_PICKUP_LASER, 0, -1, -1, -1, -1, -1);
					break;
				case WEAPON_COMBATKNIFE:
					sndStart(var80095200, SFX_PICKUP_KNIFE, 0, -1, -1, -1, -1, -1);
					break;
				case WEAPON_REMOTEMINE:
					if (handnum == HAND_RIGHT) {
						sndStart(var80095200, SFX_PICKUP_MINE, 0, -1, -1, -1, -1, -1);
					}
					break;
				case WEAPON_TIMEDMINE:
				case WEAPON_PROXIMITYMINE:
				case WEAPON_ECMMINE:
				case WEAPON_DATAUPLINK:
				case WEAPON_RTRACKER:
				case WEAPON_PRESIDENTSCANNER:
				case WEAPON_DOORDECODER:
				case WEAPON_AUTOSURGEON:
				case WEAPON_COMMSRIDER:
				case WEAPON_TRACERBUG:
				case WEAPON_TARGETAMPLIFIER:
					sndStart(var80095200, SFX_PICKUP_MINE, 0, -1, -1, -1, -1, -1);
					break;
				case WEAPON_TRANQUILIZER:
				case WEAPON_PSYCHOSISGUN:
					speed2 = 1.5f;
					handle2 = sndStart(var80095200, SFX_PICKUP_GUN, 0, -1, -1, -1, -1, -1);

					if (handle2) {
						audioPostEvent(handle2, AL_SNDP_PITCH_EVT, *(int *)&speed2);
					}

					break;
				case WEAPON_REAPER:
					speed3 = 0.85f;
					handle3 = sndStart(var80095200, SFX_PICKUP_GUN, 0, -1, -1, -1, -1, -1);

					if (handle3) {
						audioPostEvent(handle3, AL_SNDP_PITCH_EVT, *(int *)&speed3);
					}

					break;
				case WEAPON_NONE:
				case WEAPON_UNARMED:
				case WEAPON_LAPTOPGUN:
				case WEAPON_CROSSBOW:
				case WEAPON_GRENADE:
				case WEAPON_NBOMB:
				case WEAPON_COMBATBOOST:
				case WEAPON_CLOAKINGDEVICE:
				case WEAPON_EXPLOSIVES:
				case WEAPON_SKEDARBOMB:
				case WEAPON_DISGUISE40:
				case WEAPON_DISGUISE41:
				case WEAPON_FLIGHTPLANS:
				case WEAPON_RESEARCHTAPE:
				case WEAPON_BACKUPDISK:
				case WEAPON_KEYCARD45:
				case WEAPON_KEYCARD46:
				case WEAPON_KEYCARD47:
				case WEAPON_KEYCARD48:
				case WEAPON_KEYCARD49:
				case WEAPON_KEYCARD4A:
				case WEAPON_KEYCARD4B:
				case WEAPON_KEYCARD4C:
				case WEAPON_SUITCASE:
				case WEAPON_BRIEFCASE:
				case WEAPON_NECKLACE:
				case WEAPON_BRIEFCASE2:
					// No equip sound
					break;
				default:
					sndStart(var80095200, SFX_PICKUP_GUN, 0, -1, -1, -1, -1, -1);
					break;
				}
			}
		}

		if (hand->count60 >= delay
				|| !weaponGetFileNum2(info->weaponnum)
				|| !weaponHasFlag(info->weaponnum, WEAPONFLAG_00000040)
				|| weaponHasFlag(info->weaponnum, WEAPONFLAG_00000080)) {
			hand->mode = HANDMODE_NONE;
			hand->stateminor++; // to HANDSTATEMINOR_CHANGEGUN_EQUIP

			if (weaponHasFlag(hand->gset.weaponnum, WEAPONFLAG_00004000) == 0) {
				hand->unk0cc8_02 = false;
			}

			hand->count60 = 0;
			hand->count = 0;
		} else {
			bgunSetArmPitch(hand, (delay - hand->count60) * MAX_PITCH / delay);
		}
	}

	// Wait for equip animation to finish then go to idle state
	if (hand->stateminor == HANDSTATEMINOR_CHANGEGUN_EQUIP) {
		if (info->definition->equip_animation && !weaponHasFlag(hand->gset.weaponnum, WEAPONFLAG_00004000)) {
			if (hand->animmode == HANDANIMMODE_IDLE) {
				if (bgunSetState(handnum, HANDSTATE_IDLE)) {
					return lvupdate;
				}
			}
		} else {
			if (bgunSetState(handnum, HANDSTATE_IDLE)) {
				return lvupdate;
			}
		}
	}

	return 0;
}

void bgunUpdateHandState2(struct hand *hand, int handnum, struct coord *viewmodelpos, struct weaponfunc *funcdef, Mtxf *arg4, Mtxf *arg5)
{
	float tmp;
	struct coord sp38 = {0, 0, 0};
	
	// Go into turn up mode if the player has been looking at a wall for more than half a second OR the player is moving quickly on a hoverbike
	if (((g_CloseToWallTimer > TICKS(30) && g_Vars.currentplayer->speedforwards < 0.9f && g_Vars.currentplayer->speedforwards > -0.9f ) || (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE && g_Vars.currentplayer->hovspeed >= 0.8f)) && funcdef && (funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT && (hand->state == HANDSTATE_IDLE || hand->state == HANDSTATE_2)) {
		if (hand->turnuprot < 1.0f) {
			// Rotate into turned up position
				hand->turnuprot += LVUPDATE60FREAL() / 30.0f;

				if (hand->turnuprot > 1.0f) {
					hand->turnuprot = 1.0f;
				}
		}
	} else {
		// At this point we don't want the gun to be in the turned up position.
		// However we don't want it to revert immediately, so a timer is used.
		//float inversespeed = 30.0f;
		float inversespeed = 20.0f;

		if (hand->animmode == HANDANIMMODE_BUSY) {
			// Revert faster
			//inversespeed = 15.0f;
			inversespeed = 5.0f;
		}

		if (hand->turnuprot > 0.0f) {
			bool revert = false;

			revert = true;

			if (hand->animmode == HANDANIMMODE_BUSY && funcdef && (funcdef->type & 0xff) != INVENTORYFUNCTYPE_SHOOT) {
				revert = true;
			}

			if (hand->state != HANDSTATE_IDLE
					&& hand->state != HANDSTATE_2
					&& hand->state != HANDSTATE_ATTACKEMPTY
					&& hand->state != HANDSTATE_ATTACK) {
				revert = true;
			}

			if (revert) {
				hand->turnuprot -= LVUPDATE60FREAL() / inversespeed;
			}

			if (hand->turnuprot < 0.0f) {
				hand->turnuprot = 0.0f;
			}
		} else {
			// Not rotated
		}
	}
	

	tmp = -cosf(hand->turnuprot * M_PI) * 0.5f + 0.50f;
	sp38.x = (tmp * 90.0f * 0.017453292f * -1.0f); // 0.017453292f for degree to radian conversion
	sp38.y = (tmp * 10.0f * 0.017453292f) * (handnum != HAND_RIGHT ? -1.0f : 1.0f);

	mtx4LoadRotation(&sp38, arg4);
	mtx00015be0(arg4, arg5);

	viewmodelpos->y -= 1.0f * hand->turnuprot;
	viewmodelpos->x -= 9.0f * hand->turnuprot * (handnum != HAND_RIGHT ? 1.0f : -1.0f);
}

/**
 * This function may have implemented an early beta feature where the gun could
 * be held at the side of the screen, pointed upwards. The feature was shown in
 * a demo video but doesn't exist in any public version of the game.
 */
int bgunTickIncState2(struct handweaponinfo *info, int handnum, struct hand *hand, int lvupdate)
{
	return 0;
}

int bgunTickInc(struct handweaponinfo *info, int handnum, int lvupdate)
{
	int result = 0;
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];
	int prevstate = hand->state;

	hand->firing = false;
	hand->flashon = false;
	hand->stateframes += lvupdate;

	if (g_Vars.lvupdate240 > 0) {
		hand->count60 += g_Vars.lvupdate60;
		hand->count++;
	}

	hand->useposrot = false;

	switch (hand->state) {
	case HANDSTATE_IDLE:
		result = bgunTickIncIdle(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_RELOAD:
		result = bgunTickIncReload(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_ATTACK:
		result = bgunTickIncAttack(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_2:
		result = bgunTickIncState2(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_CHANGEGUN:
		result = bgunTickIncChangeGun(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_ATTACKEMPTY:
		result = bgunTickIncAttackEmpty(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_AUTOSWITCH:
		result = bgunTickIncAutoSwitch(info, handnum, hand, lvupdate);
		break;
	case HANDSTATE_CHANGEFUNC:
		result = bgunTickIncChangeFunc(info, handnum, hand, lvupdate);
		break;
	}

	hand->statelastframe = hand->stateframes;

	if (hand->state != prevstate) {
		hand->statelastframe = -result;
	} else {
		hand->stateframes -= result;
		hand->statecycles++;
	}

	return result;
}

bool bgunSetState(int handnum, int state)
{
	bool valid = true;
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	// Sanity check - don't allow changing function if there is no other
	if (state == HANDSTATE_CHANGEFUNC && weaponGetFunction(&hand->gset, 1 - hand->gset.weaponfunc) == NULL) {
		valid = false;
	}

	if (valid) {
		hand->state = state;
		hand->stateframes = 0;
		hand->stateflags = 0;
		hand->statecycles = 0;
		hand->stateminor = 0;
		hand->statelastframe = 0;
	}

	return valid;
}

void bgunTickHand(int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];
	struct handweaponinfo info;
	int lvupdate;
	int i = 20;
	bgunGetWeaponInfo(&info, handnum);

	lvupdate = g_Vars.lvupdate60;

	hand->animframeinc = g_Vars.lvupdate60;
	hand->animframeincfreal += PALUPF(g_Vars.lvupdate60);

	while (i >= 0) {
		lvupdate = bgunTickInc(&info, handnum, lvupdate);
		i--;

		if (lvupdate <= 0) {
			break;
		}
	}
}

void bgunTickSwitch(void)
{
	bgunTickSwitch2();
}

void bgunInitHandAnims(void)
{
	struct hand *hand;
	int i;

	for (i = 0; i < 2; i++) {
		if (i == 0) {
			hand = &g_Vars.currentplayer->hands[1];
		} else {
			hand = &g_Vars.currentplayer->hands[0];
		}

		hand->gangstarot = 0;
		hand->state = HANDSTATE_IDLE;
		hand->animload = -1;
		hand->animmode = HANDANIMMODE_IDLE;

		animInit(&hand->anim);

		hand->gunmodel.anim = &hand->anim;
		hand->handmodel.anim = &hand->anim;
	}
}

float bgunGetNoiseRadius(int handnum)
{
	return g_Vars.currentplayer->hands[handnum].noiseradius;
}

void bgunDecreaseNoiseRadius(void)
{
	struct player *player = g_Vars.currentplayer;
	float consideramount;
	struct gset gsetleft;
	struct gset gsetright;
	struct noisesettings noisesettingsleft;
	struct noisesettings noisesettingsright;
	float subamount;

	gsetPopulateFromCurrentPlayer(HAND_LEFT, &gsetleft);
	gsetPopulateFromCurrentPlayer(HAND_RIGHT, &gsetright);

	gsetGetNoiseSettings(&gsetleft, &noisesettingsleft);
	gsetGetNoiseSettings(&gsetright, &noisesettingsright);

	// Right hand
	if (bgunIsFiring(HAND_RIGHT)) {
		player->hands[HAND_RIGHT].noiseradius += noisesettingsright.incradius;

		if (player->hands[HAND_RIGHT].noiseradius > noisesettingsright.maxradius) {
			player->hands[HAND_RIGHT].noiseradius = noisesettingsright.maxradius;
		}
	}

	subamount = g_Vars.lvupdate60freal * noisesettingsright.incradius / (noisesettingsright.decbasespeed * 60.0f);
	consideramount = (player->hands[HAND_RIGHT].noiseradius - noisesettingsright.minradius) * g_Vars.lvupdate60freal / (noisesettingsright.decremspeed * 60.0f);

	if (consideramount > subamount) {
		subamount = consideramount;
	}

	player->hands[HAND_RIGHT].noiseradius -= subamount;

	if (player->hands[HAND_RIGHT].noiseradius < noisesettingsright.minradius) {
		player->hands[HAND_RIGHT].noiseradius = noisesettingsright.minradius;
	}

	// Left hand
	if (bgunIsFiring(HAND_LEFT)) {
		player->hands[HAND_LEFT].noiseradius += noisesettingsleft.incradius;

		if (player->hands[HAND_LEFT].noiseradius > noisesettingsleft.maxradius) {
			player->hands[HAND_LEFT].noiseradius = noisesettingsleft.maxradius;
		}
	}

	subamount = g_Vars.lvupdate60freal * noisesettingsleft.incradius / (noisesettingsleft.decbasespeed * 60.0f);
	consideramount = (player->hands[HAND_LEFT].noiseradius - noisesettingsleft.minradius) * g_Vars.lvupdate60freal / (noisesettingsleft.decremspeed * 60.0f);

	if (consideramount > subamount) {
		subamount = consideramount;
	}

	player->hands[HAND_LEFT].noiseradius -= subamount;

	if (player->hands[HAND_LEFT].noiseradius < noisesettingsleft.minradius) {
		player->hands[HAND_LEFT].noiseradius = noisesettingsleft.minradius;
	}
}

void bgunCalculateBlend(int handnum)
{
	int sp60[2];
	int sp58[2];
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(handnum));
	float sway = weapon->sway;
	struct player *player = g_Vars.currentplayer;

	sp60[handnum] = (player->hands[handnum].curblendpos + 2) % 4;
	sp58[handnum] = (player->hands[handnum].curblendpos + 1) % 4;
	player->hands[handnum].curblendpos = sp58[handnum];

	player->hands[handnum].blendlook[sp60[handnum]].x = (RANDOMFRAC() - 0.5f) * 0.08f * sway;
	player->hands[handnum].blendlook[sp60[handnum]].y = (RANDOMFRAC() - 0.5f) * 0.1f * sway;
	player->hands[handnum].blendlook[sp60[handnum]].z = -1;

	player->hands[handnum].blendup[sp60[handnum]].x = (RANDOMFRAC() - 0.5f) * 0.1f * sway;
	player->hands[handnum].blendup[sp60[handnum]].y = 1;
	player->hands[handnum].blendup[sp60[handnum]].z = (RANDOMFRAC() - 0.5f) * 0.1f * sway;

	player->hands[handnum].blendpos[sp60[handnum]].x = (RANDOMFRAC() * 0.75f) + 1.5f;
	player->hands[handnum].blendpos[sp60[handnum]].y = (2 + RANDOMFRAC()) * player->hands[handnum].blendscale1;
	player->hands[handnum].blendpos[sp60[handnum]].z = (RANDOMFRAC() - 0.5f) * 2.5f;

	if (player->hands[handnum].sideflag < 0) {
		player->hands[handnum].blendpos[sp60[handnum]].x *= -1;

		if (player->hands[handnum].sideflag == -2) {
			player->hands[handnum].sideflag = 1;
		} else {
			player->hands[handnum].sideflag = -2;
		}
	} else {
		if (player->hands[handnum].sideflag == 2) {
			player->hands[handnum].sideflag = -1;
		} else {
			player->hands[handnum].sideflag = 2;
		}
	}

	player->hands[handnum].blendscale1 = -player->hands[handnum].blendscale1;
}

void bgunUpdateBlend(struct hand *hand, int handnum)
{
	int i;
	struct coord sp5c = {0, 0, 0};
	struct coord sp50 = {0, 0, -1};
	struct coord sp44 = {0, 1, 0};
	int pos = hand->curblendpos;
	struct player *player = g_Vars.currentplayer;

	CatmullRomSplineInterp(&hand->blendpos[(pos + 3) % 4], &hand->blendpos[pos], &hand->blendpos[(pos + 1) % 4], &hand->blendpos[(pos + 2) % 4], hand->dampt, &sp5c);
	CatmullRomSplineInterp(&hand->blendlook[(pos + 3) % 4], &hand->blendlook[pos], &hand->blendlook[(pos + 1) % 4], &hand->blendlook[(pos + 2) % 4], hand->dampt, &sp50);
	CatmullRomSplineInterp(&hand->blendup[(pos + 3) % 4], &hand->blendup[pos], &hand->blendup[(pos + 1) % 4], &hand->blendup[(pos + 2) % 4], hand->dampt, &sp44);

	sp5c.x *= player->gunposamplitude;
	sp5c.y *= player->gunposamplitude;
	sp5c.z *= player->gunposamplitude;

	sp5c.x += hand->adjustdamp.x;
	sp5c.y += hand->adjustdamp.y;

	sp5c.x += handGetXShift(handnum);

	for (i = 0; i < g_Vars.lvupdate240; i++) {
		hand->damppossum.x = (PAL ? 0.9847f : 0.9872f) * hand->damppossum.x + sp5c.f[0];
		hand->damppossum.y = (PAL ? 0.9847f : 0.9872f) * hand->damppossum.y + sp5c.f[1];
		hand->damppossum.z = (PAL ? 0.9847f : 0.9872f) * hand->damppossum.z + sp5c.f[2];

		hand->damplooksum.x = (PAL ? 0.9847f : 0.9872f) * hand->damplooksum.x + sp50.f[0];
		hand->damplooksum.y = (PAL ? 0.9847f : 0.9872f) * hand->damplooksum.y + sp50.f[1];
		hand->damplooksum.z = (PAL ? 0.9847f : 0.9872f) * hand->damplooksum.z + sp50.f[2];

		hand->dampupsum.x = (PAL ? 0.9847f : 0.9872f) * hand->dampupsum.x + sp44.f[0];
		hand->dampupsum.y = (PAL ? 0.9847f : 0.9872f) * hand->dampupsum.y + sp44.f[1];
		hand->dampupsum.z = (PAL ? 0.9847f : 0.9872f) * hand->dampupsum.z + sp44.f[2];
	}

	hand->damppos.x = hand->damppossum.x * (PAL ? 0.01529997587204f : 0.012799978f) * 2;
	hand->damppos.y = hand->damppossum.y * (PAL ? 0.01529997587204f : 0.012799978f) * 2;
	hand->damppos.z = hand->damppossum.z * (PAL ? 0.01529997587204f : 0.012799978f) * 2;

	hand->damplook.x = hand->damplooksum.x * (PAL ? 0.01529997587204f : 0.012799978f);
	hand->damplook.y = hand->damplooksum.y * (PAL ? 0.01529997587204f : 0.012799978f);
	hand->damplook.z = hand->damplooksum.z * (PAL ? 0.01529997587204f : 0.012799978f);

	hand->dampup.x = hand->dampupsum.x * (PAL ? 0.01529997587204f : 0.012799978f);
	hand->dampup.y = hand->dampupsum.y * (PAL ? 0.01529997587204f : 0.012799978f);
	hand->dampup.z = hand->dampupsum.z * (PAL ? 0.01529997587204f : 0.012799978f);
}

void bgun0f09d8dc(float breathing, float arg1, float arg2, float arg3, float arg4)
{
	float dampt[2];
	struct player *player = g_Vars.currentplayer;
	int i;
	float sp50 = arg2;
	float sp4c;

	if (sp50 < 0.0f) {
		sp50 = -sp50;
	}

	if (arg1 > 0.8f) {
		player->gunposamplitude = 1.0f;
	} else {
		if (arg1 > 0.1f) {
			float tmp = 1.0f - cosf((arg1 - 0.1f) * M_TAU / 2.8f);
			player->gunposamplitude = 0.8f * tmp + 0.2f;
		} else {
			player->gunposamplitude = 0.1f;
		}
	}

	if (bmoveGetCrouchPos() != CROUCHPOS_SQUAT) {
		if (player->gunposamplitude < 0.3f * g_Vars.currentplayer->bondbreathing) {
			player->gunposamplitude = 0.3f * g_Vars.currentplayer->bondbreathing;
		}
	}

	if (player->gunposamplitude < 0.5f * sp50) {
		player->gunposamplitude = 0.5f * sp50;
	}

	for (i = 0; i < g_Vars.lvupdate240; i++) {
		player->gunampsum = 0.9872f * player->gunampsum + player->gunposamplitude;
	}

	player->gunposamplitude = 0.012799978256226f * player->gunampsum;

	if (breathing < 0.016666667535901f * sp50) {
		breathing = 0.016666667535901f * sp50;
	}

	for (i = 0; i < g_Vars.lvupdate240; i++) {
		player->cyclesum = 0.9872f * player->cyclesum + breathing;
	}

	breathing = player->cyclesum * 0.012799978256226f;
	sp4c = breathing * g_Vars.lvupdate60freal;
	dampt[0] = player->hands[0].dampt + sp4c;

	while (dampt[0] >= 1.0f) {
		bgunCalculateBlend(HAND_RIGHT);
		dampt[0] -= 1.0f;
		player->syncoffset++;
	}

	player->synccount += g_Vars.lvupdate60freal;

	if (player->synccount > 60.0f) {
		player->synccount = 0.0f;
		player->syncchange = (RANDOMFRAC() - 0.5f) * 0.2f / 60.0f;
	}

	if (player->syncchange + sp4c > 0.0f) {
		player->gunsync += player->syncchange;
	}

	if (player->gunsync > 0.5f) {
		player->gunsync = 0.5f;
	} else if (player->gunsync < -0.5f) {
		player->gunsync = -0.5f;
	} else if (player->gunsync < 0.1f && player->gunsync > -0.1f) {
		if (player->gunsync > 0.0f) {
			player->gunsync = -0.1f;
		} else {
			player->gunsync = 0.1f;
		}
	}

	dampt[1] = dampt[0] + player->syncoffset + player->gunsync;

	while (dampt[1] >= 1.0f) {
		bgunCalculateBlend(HAND_LEFT);
		dampt[1] -= 1.0f;
		player->syncoffset--;
	}

	for (i = 0; i < 2; i++) {
		player->hands[i].dampt = dampt[i];
		player->hands[i].adjustdamp.x = -1.75f * arg3 + -0.8f * arg4;
		player->hands[i].adjustdamp.y = -2.0f * arg2;
	}
}

bool bgunIsLoaded(void)
{
	if (g_Vars.currentplayer->gunctrl.gunmemowner != GUNMEMOWNER_BONDGUN) {
		return false;
	}

	return g_Vars.currentplayer->gunctrl.gunmemtype == WEAPON_NONE
		|| (g_Vars.currentplayer->gunctrl.gunmemnew < 0
				&& g_Vars.currentplayer->gunctrl.masterloadstate == MASTERLOADSTATE_LOADED);
}

uint32_t bgunGetGunMemType(void)
{
	return g_Vars.currentplayer->gunctrl.gunmemtype;
}

struct modeldef *bgunGetGunModeldef(void)
{
	return g_Vars.currentplayer->gunctrl.gunmodeldef;
}

uint8_t *bgunGetGunMem(void)
{
	return g_Vars.currentplayer->gunctrl.gunmem;
}

uint32_t bgunCalculateGunMemCapacity(void)
{
	if (PLAYERCOUNT() == 1) {
		return g_BgunGunMemBaseSizeDefault + stageGetCurrent()->extragunmem;
	}

	return g_BgunGunMemBaseSizeDefault;
}

void bgunFreeGunMem(void)
{
	g_Vars.currentplayer->gunctrl.gunmemowner = GUNMEMOWNER_FREE;
	
	// gunmem is stale and so are the textures in it
	// TODO: figure out how to purge only those textures
	videoResetTextureCache();
}

void bgunSetGunMemWeapon(int weaponnum)
{
	struct player *player = g_Vars.currentplayer;

	if (player->gunctrl.gunmemowner == GUNMEMOWNER_BONDGUN) {
		player->gunctrl.masterloadstate = MASTERLOADSTATE_FLUX;
		player->gunctrl.gunloadstate = GUNLOADSTATE_FLUX;
		player->gunctrl.gunmemnew = weaponnum;
		player->gunctrl.gunlocktimer = -1;
	} else {
		player->gunctrl.gunmemnew = weaponnum;
	}
}

void bgunEnterFlux(void)
{
	int i;
	struct casing *end;
	struct casing *casing;

	g_Vars.currentplayer->gunctrl.handfilenum = 0xffff;
	g_Vars.currentplayer->gunctrl.handmodeldef = NULL;
	g_Vars.currentplayer->gunctrl.handmemloadptr = 0;
	g_Vars.currentplayer->gunctrl.handmemloadremaining = 0;
	g_Vars.currentplayer->gunctrl.masterloadstate = MASTERLOADSTATE_FLUX;
	g_Vars.currentplayer->gunctrl.gunloadstate = GUNLOADSTATE_FLUX;

	end = g_Casings + ARRAYCOUNT(g_Casings);
	casing = g_Casings;

	while (casing < end) {
		casing->modeldef = NULL;
		casing++;
	}

	g_CasingsActive = false;
}

bool bgunChangeGunMem(int newowner)
{
	struct player *player = g_Vars.currentplayer;

	if (player->gunctrl.gunmemowner == newowner) {
		return true;
	}

	if (player->gunctrl.gunlocktimer < 0) {
		player->gunctrl.gunlocktimer--;

		if (player->gunctrl.gunlocktimer < -2) {
			player->gunctrl.gunlocktimer = 0;
			player->gunctrl.gunmemowner = newowner;
			return true;
		}
	} else {
		bool unlock = false;

		switch (player->gunctrl.gunmemowner) {
		case GUNMEMOWNER_BONDGUN:
			if (player->gunctrl.gunmemtype != -1) {
				player->gunctrl.gunmemnew = player->gunctrl.gunmemtype;
			}

			player->gunctrl.gunmemtype = -1;
			bgunEnterFlux();
			player->gunctrl.loadall = true;
			unlock = true;
			break;
		case GUNMEMOWNER_CHRBODY:
			if (g_Vars.mplayerisrunning) {
				unlock = true;
			}

			if (!player->haschrbody) {
				unlock = true;
			}

			if (newowner == GUNMEMOWNER_INVMENU && g_GamePaused != 0) {
				unlock = true;
				playerRemoveChrBody();
			}
			break;
		case GUNMEMOWNER_3:
			unlock = true;
			break;
		case GUNMEMOWNER_CHANGING:
		case GUNMEMOWNER_FREE:
			unlock = true;
			break;
		}

		if (unlock) {
			player->gunctrl.gunlocktimer = -1;
			player->gunctrl.gunmemowner = GUNMEMOWNER_CHANGING;
		}
	}

	return false;
}

/**
 * This function loads resources for a gun change.
 *
 * The caller sets properties in the player's gunctrl struct which tell this
 * function what to load, where the gunmem is that it can use, and how much
 * gunmem there is. This function is then called a couple of times on subsequent
 * ticks, loading data incrementally to avoid a significant lag spike.
 *
 * The function keeps track of its progress in the gunloadstate property, and
 * updates the gunmem properties to reflect its usage.
 *
 * The first call loads the model definition from the ROM and decompresses it.
 * The second call loads and decompress to 3 textures.
 * This continues on further calls until all textures are loaded.
 * The final call does some one-off processing on the model's display lists.
 *
 * Although the name contains "Gun", it's used for more than just the gun model.
 * It's used for the hand model, the gun model and the cartridge model.
 */
void bgunTickGunLoad(void)
{
	int i;
	int numthistick;
	uint64_t remaining;
	int padding;
	uint64_t allocsize;
	uint64_t loadsize;
	uintptr_t ptr;
	struct player *player = g_Vars.currentplayer;
	struct modeldef *modeldef;
	struct fileinfo *fileinfo;
	struct fileinfo *gunfileinfo;
	uintptr_t newvalue;
	uintptr_t end;

	if (player->gunctrl.gunloadstate == GUNLOADSTATE_MODEL) {
		ptr = *player->gunctrl.loadmemptr;
		remaining = *player->gunctrl.loadmemremaining;

		// Align ptr to the next 16 byte boundary
		if (ptr % 16) {
			padding = 16 - (ptr % 16);
			ptr += padding;
			remaining -= padding;
		}

		*player->gunctrl.loadmemptr = ptr;
		*player->gunctrl.loadmemremaining = remaining;

		loadsize = ALIGN64(fileGetInflatedSize(player->gunctrl.loadfilenum, LOADTYPE_MODEL)) + 0x8000;

		if (loadsize > remaining) {
			loadsize = remaining;
		}

		// Load the model file to ptr
		g_LoadType = LOADTYPE_GUN;

		modeldef = fileLoadToAddr(player->gunctrl.loadfilenum, FILELOADMETHOD_EXTRAMEM, (uint8_t *)ptr, loadsize);

		// Reserve some space for textures
		allocsize = fileGetLoadedSize(player->gunctrl.loadfilenum) + 0xe00;
#ifdef PLATFORM_64BIT
		allocsize += 0xe00;
#endif

		fileGetLoadedSize(player->gunctrl.loadfilenum);

		fileinfo = &g_FileInfo[player->gunctrl.loadfilenum];
		fileinfo->allocsize = allocsize;
		end = ALIGN16((uintptr_t)ptr + allocsize);
		allocsize = end - ptr;
		remaining -= allocsize;

		texInitPool(&player->gunctrl.texpool, (uint8_t *)end, remaining);

		// Tidy up the model
		modelPromoteTypeToPointer(modeldef);
		modelPromoteOffsetsToPointers(modeldef, 0x05000000, (uintptr_t)modeldef);

		*player->gunctrl.loadtomodeldef = modeldef;

		player->gunctrl.nexttexturetoload = 0;
		player->gunctrl.fileinfo = *fileinfo;

		player->gunctrl.gunloadstate = GUNLOADSTATE_TEXTURES;
		return;
	}

	if (player->gunctrl.gunloadstate == GUNLOADSTATE_TEXTURES) {
		gunfileinfo = &player->gunctrl.fileinfo;
		fileinfo = &g_FileInfo[player->gunctrl.loadfilenum];
		*fileinfo = *gunfileinfo;
		modeldef = *player->gunctrl.loadtomodeldef;

		// Load textures - up to 3 per call
		numthistick = 0;

		for (i = player->gunctrl.nexttexturetoload; i < modeldef->numtexconfigs; i++) {
			if (modeldef->texconfigs[i].texturenum < NUM_TEXTURES) {
				texLoad(&modeldef->texconfigs[i].texturenum, &player->gunctrl.texpool);
				modeldef->texconfigs[i].unk0b = 1;
			}

			numthistick++;

			if (numthistick == 3) {
				return;
			}

			// @bug: This should be incremented prior to the return, otherwise
			// subsequent ticks will waste time loading a texture that's already
			// been loaded.
			player->gunctrl.nexttexturetoload++;
		}

		*gunfileinfo = *fileinfo;

		player->gunctrl.gunloadstate = GUNLOADSTATE_DLS;
		return;
	}

	if (player->gunctrl.gunloadstate == GUNLOADSTATE_DLS) {
		fileinfo = &g_FileInfo[player->gunctrl.loadfilenum];
		*fileinfo = player->gunctrl.fileinfo;
		modeldef = *player->gunctrl.loadtomodeldef;

		modeldef0f1a7560(modeldef, player->gunctrl.loadfilenum, 0x05000000, modeldef, &player->gunctrl.texpool, false);

		fileGetInflatedSize(player->gunctrl.loadfilenum, LOADTYPE_MODEL);
		fileGetLoadedSize(player->gunctrl.loadfilenum);
		fileGetLoadedSize(player->gunctrl.loadfilenum);
		fileGetLoadedSize(player->gunctrl.loadfilenum);

		modelAllocateRwData(modeldef);

		newvalue = ALIGN64(texGetPoolLeftPos(&player->gunctrl.texpool));
		remaining = *player->gunctrl.loadmemremaining;
		remaining -= (intptr_t)(newvalue - *player->gunctrl.loadmemptr);

		*player->gunctrl.loadmemptr = newvalue;
		*player->gunctrl.loadmemremaining = remaining;
		 player->gunctrl.gunloadstate = GUNLOADSTATE_LOADED;
	}
}

void bgunTickMasterLoad(void)
{
	int newweaponnum;
	struct player *player = g_Vars.currentplayer;
	bool hashands;
	uint16_t handfilenum;
	int sum;
	uint16_t filenum;
	int i;
	struct casing *casing;
	struct hand *hand;
	struct weaponfunc *func;
	struct weaponfunc *shootfunc;
	struct weapon *weapondef;
	int casingindex;
	struct inventory_ammo *ammodef;
	int value;
	int bodynum;
	int headnum;

	if ((player->gunctrl.gunmemowner == GUNMEMOWNER_BONDGUN || bgunChangeGunMem(GUNMEMOWNER_BONDGUN)) && player->gunctrl.gunmemnew >= 0) {
		if (player->gunctrl.gunlocktimer == 0) {
			newweaponnum = player->gunctrl.gunmemnew;

			playerChooseBodyAndHead(&bodynum, &headnum, NULL);

			handfilenum = g_HeadsAndBodies[bodynum].handfilenum;
			filenum = weaponGetFileNum(newweaponnum);

			if (player->gunctrl.masterloadstate != MASTERLOADSTATE_LOADED || newweaponnum != player->gunctrl.gunmemtype) {
				if (filenum) {
					hashands = false;

					if (weaponHasFlag(newweaponnum, WEAPONFLAG_HASHANDS)) {
						hashands = true;
					}

					if (newweaponnum == WEAPON_UNARMED) {
						// For unarmed, the fists are implemented
						// as weapon models rather than hand models
						filenum = handfilenum;
						handfilenum = 0 * (player->gunctrl.gunloadstate == 4);
						hashands = false;
					}

					if (player->gunctrl.masterloadstate == MASTERLOADSTATE_FLUX) {
						casing = g_Casings;

						while (casing < &g_Casings[ARRAYCOUNT(g_Casings)]) {
							if (casing->modeldef == player->gunctrl.cartmodeldef) {
								casing->modeldef = NULL;
							}

							casing++;
						}

						g_CasingsActive = false;

						casing = g_Casings;

						while (casing < &g_Casings[ARRAYCOUNT(g_Casings)]) {
							if (casing->modeldef != NULL) {
								g_CasingsActive = true;
							}

							casing++;
						}

						player->gunctrl.cartmodeldef = NULL;
						player->gunctrl.masterloadstate = MASTERLOADSTATE_HANDS;
					} else if (player->gunctrl.masterloadstate == MASTERLOADSTATE_HANDS) {
						if (hashands) {
							if (handfilenum != player->gunctrl.handfilenum) {
								if (player->gunctrl.gunloadstate == GUNLOADSTATE_FLUX) {
									player->gunctrl.handmemloadptr = bgunGetGunMem();
									player->gunctrl.handmemloadremaining = bgunCalculateGunMemCapacity();
									player->gunctrl.gunloadstate = GUNLOADSTATE_MODEL;
									player->gunctrl.loadfilenum = handfilenum;
									player->gunctrl.loadtomodeldef = &player->gunctrl.handmodeldef;
									player->gunctrl.loadmemptr = (uintptr_t *) &player->gunctrl.handmemloadptr;
									player->gunctrl.loadmemremaining = (uintptr_t*) &player->gunctrl.handmemloadremaining;
								}

								bgunTickGunLoad();

								if (player->gunctrl.gunloadstate == GUNLOADSTATE_LOADED) {
									player->gunctrl.handfilenum = handfilenum;
								} else {
									return;
								}
							}
						} else {
							player->gunctrl.handfilenum = 0;
							player->gunctrl.handmodeldef = NULL;
							player->gunctrl.handmemloadptr = bgunGetGunMem();
							player->gunctrl.handmemloadremaining = bgunCalculateGunMemCapacity();
						}

						player->gunctrl.masterloadstate = MASTERLOADSTATE_GUN;
						player->gunctrl.gunloadstate = GUNLOADSTATE_FLUX;
					} else if (player->gunctrl.masterloadstate == MASTERLOADSTATE_GUN) {
						if (player->gunctrl.gunloadstate == GUNLOADSTATE_FLUX) {
							player->gunctrl.memloadptr = (uint8_t *) player->gunctrl.handmemloadptr;
							player->gunctrl.memloadremaining = player->gunctrl.handmemloadremaining;
							player->gunctrl.gunloadstate = GUNLOADSTATE_MODEL;
							player->gunctrl.loadfilenum = filenum;
							player->gunctrl.loadtomodeldef = &player->gunctrl.gunmodeldef;
							player->gunctrl.loadmemptr = (uintptr_t*) &player->gunctrl.memloadptr;
							player->gunctrl.loadmemremaining = (uintptr_t*) &player->gunctrl.memloadremaining;
						}

						bgunTickGunLoad();

						if (player->gunctrl.gunloadstate == GUNLOADSTATE_LOADED) {
							player->gunctrl.masterloadstate = MASTERLOADSTATE_CARTS;
							player->gunctrl.gunloadstate = GUNLOADSTATE_FLUX;
						}
					} else if (player->gunctrl.masterloadstate == MASTERLOADSTATE_CARTS) {
						if (player->gunctrl.gunloadstate == GUNLOADSTATE_LOADED) {
							player->gunctrl.gunloadstate = GUNLOADSTATE_FLUX;
						}

						if (player->gunctrl.gunloadstate == GUNLOADSTATE_FLUX && player->gunctrl.cartmodeldef == NULL && PLAYERCOUNT() == 1) {
							for (i = 0; i < 2; i++) {
								hand = player->hands + i;
								func = gsetGetWeaponFunction2(&hand->gset);
								shootfunc = NULL;
								weapondef = weaponFindById(player->gunctrl.weaponnum);
								casingindex = -1;

								if (func != NULL) {
									if ((func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
										shootfunc = func;
									}

									if (weapondef && shootfunc) {
										ammodef = func->ammoindex >= 0 ? weapondef->ammos[func->ammoindex] : NULL;

										if (ammodef) {
											casingindex = ammodef->casingeject;
										}
									}

									if (casingindex >= 0) {
										if (player->gunctrl.cartmodeldef == NULL) {
											filenum = g_CartFileNums[casingindex];
											player->gunctrl.loadfilenum = filenum;
											player->gunctrl.gunloadstate = GUNLOADSTATE_MODEL;
											player->gunctrl.loadtomodeldef = &player->gunctrl.cartmodeldef;
											player->gunctrl.loadmemptr = (uintptr_t *) &player->gunctrl.memloadptr;
											player->gunctrl.loadmemremaining = (uintptr_t*) &player->gunctrl.memloadremaining;
											break;
										}

										break;
									}
								}
							}
						}

						if (player->gunctrl.gunloadstate != GUNLOADSTATE_FLUX) {
							bgunTickGunLoad();
							return;
						}

						sum = 0;

						for (i = 0; i < 2; i++) {
							hand = &player->hands[i];

							modelInit(&hand->gunmodel, player->gunctrl.gunmodeldef, hand->unk0a6c, 0);

							if (player->gunctrl.handmodeldef != 0) {
								modelInit(&hand->handmodel, player->gunctrl.handmodeldef, hand->handsavedata, false);
							}

							hand->unk0dcc = (uintptr_t *) player->gunctrl.memloadptr;

							value = bgunCreateModelCmdList(&hand->gunmodel, player->gunctrl.gunmodeldef->rootnode, (uintptr_t *) player->gunctrl.memloadptr);

							sum += value;
							player->gunctrl.memloadptr += value;
							player->gunctrl.memloadremaining -= value;

							if (player->gunctrl.handmodeldef != 0) {
								hand->unk0dd0 = (uintptr_t*) player->gunctrl.memloadptr;

								value = bgunCreateModelCmdList(&hand->handmodel, player->gunctrl.handmodeldef->rootnode, (uintptr_t*) player->gunctrl.memloadptr);

								sum += value;
								player->gunctrl.memloadptr += value;
								player->gunctrl.memloadremaining -= value;
							}
						}

						hand = &player->hands[0];
						hand->unk0dd4 = -1;

						if (player->gunctrl.memloadremaining > 50 * sizeof(Mtxf)) {
							hand->unk0dd8 = (Mtxf *) player->gunctrl.memloadptr;
							player->gunctrl.memloadptr += 50 * sizeof(Mtxf);
							player->gunctrl.memloadremaining -= 50 * sizeof(Mtxf);
						} else {
							hand->unk0dd8 = NULL;
						}

						bgunCalculateGunMemCapacity();

						player->gunctrl.masterloadstate = MASTERLOADSTATE_LOADED;
						player->gunctrl.gunmemtype = newweaponnum;
						player->gunctrl.gunmemnew = -1;
					}
				}
				else {
					player->gunctrl.masterloadstate = MASTERLOADSTATE_LOADED;
					player->gunctrl.gunmemtype = newweaponnum;
					player->gunctrl.gunmemnew = -1;
				}
			}
		} else {
			player->gunctrl.gunlocktimer--;

			if (player->gunctrl.gunlocktimer < -2) {
				player->gunctrl.gunlocktimer = 0;
			}
		}
	}
}

void bgunTickLoad(void)
{
	int i;

	for (i = 0; i < g_Vars.lvupdate240; i += 8) {
		bgunTickMasterLoad();
	}
}

/**
 * Load all gun data (models, hands etc) again after returning from a different
 * gunmem type. For example, after returning from a cutscene, pause menu or from
 * controlling the eyespy.
 *
 * Gun data is typically loaded over several frames when switching, but that
 * cannot happen here as the gun must be available for the next frame.
 *
 * If the previous gun mem owner cannot release its memory right now,
 * lockscreen will be set. This causes the game to repaint the previous frame.
 *
 * Return false on success, or true if the load all should be retried on the
 * next frame.
 */
bool bgunLoadAll(void)
{
	// PAL adds a check for the eyespy being used
//#if VERSION >= VERSION_PAL_BETA
	if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)) {
		g_Vars.currentplayer->gunctrl.loadall = false;
		return false;
	}
//#endif

	bgunEnterFlux();

	if (g_Vars.currentplayer->gunctrl.weaponnum != WEAPON_NONE) {
		g_Vars.currentplayer->gunctrl.gunmemnew = g_Vars.currentplayer->gunctrl.weaponnum;
	} else {
		return false;
	}

	if (g_Vars.currentplayer->gunctrl.gunmemtype != -1) {
		return false;
	}

	if (g_Vars.currentplayer->gunctrl.gunmemowner != GUNMEMOWNER_BONDGUN) {
		bgunChangeGunMem(GUNMEMOWNER_BONDGUN);

		if (g_Vars.currentplayer->gunctrl.gunmemowner != GUNMEMOWNER_BONDGUN) {
			g_Vars.lockscreen = true;
			return true;
		}
	}

	bgunEnterFlux();

	do {
		bgunTickMasterLoad();
	} while (!bgunIsLoaded());

	g_Vars.currentplayer->gunctrl.loadall = false;

	return false;
}

struct modeldef *bgunGetCartModeldef(void)
{
	return g_Vars.currentplayer->gunctrl.cartmodeldef;
}

void bgunCreateXBowBolt(struct defaultobj *obj, struct coord *coord, RoomNum *rooms, Mtxf *matrix1, struct coord *velocity, Mtxf *matrix2, struct prop *prop, struct coord *pos)
{
	struct prop *objprop = obj->prop;

	if (objprop) {
		propActivate(objprop);
		propEnable(objprop);
		mtx00015f04(obj->model->scale, matrix1);
		func0f06a580(obj, coord, matrix1, rooms);

		if (obj->type == OBJTYPE_WEAPON && ((struct weaponobj *) obj)->weaponnum == WEAPON_BOLT) {
			int beamnum = boltbeamFindByProp(objprop);

			if (beamnum == -1) {
				beamnum = boltbeamCreate(objprop);
			}

			if (beamnum != -1) {
				boltbeamSetHeadPos(beamnum, pos);
				boltbeamSetTailPos(beamnum, pos);
			}
		}

		ensurePropHasProjectile(objprop);

		if (obj->hidden & OBJHFLAG_PROJECTILE) {
			obj->projectile->flags |= PROJECTILEFLAG_AIRBORNE;
			obj->projectile->ownerprop = prop;

			projectileSetSticky(objprop);
			mtx4Copy(matrix2, (Mtxf *)&obj->projectile->mtx);

			obj->projectile->speed.x = velocity->x;
			obj->projectile->speed.y = velocity->y;
			obj->projectile->speed.z = velocity->z;
			obj->projectile->obj = obj;
			obj->projectile->unk0d8 = g_Vars.lvframenum;
		}
	}
}

void bgun0f09ed2c(struct defaultobj *obj, struct coord *newpos, Mtxf *arg2, struct coord *velocity, Mtxf *arg4)
{
	struct prop *objprop = obj->prop;
	struct coord pos;
	RoomNum rooms[8];

	if (objprop) {
		struct prop *playerprop = g_Vars.currentplayer->prop;

		pos.x = playerprop->pos.x;
		pos.y = playerprop->pos.y;
		pos.z = playerprop->pos.z;

		roomsCopy(playerprop->rooms, rooms);

		bgunCreateXBowBolt(obj, &pos, rooms, arg2, velocity, arg4, playerprop, newpos);

		if (obj->hidden & OBJHFLAG_PROJECTILE) {
			obj->projectile->flags |= PROJECTILEFLAG_LAUNCHING;

			obj->projectile->nextsteppos.x = newpos->x;
			obj->projectile->nextsteppos.y = newpos->y;
			obj->projectile->nextsteppos.z = newpos->z;
		}
	}
}

struct defaultobj *bgunCreateThrownProjectile2(struct chrdata *chr, struct gset *gset, struct coord *pos, RoomNum *rooms, Mtxf *arg4, struct coord *velocity)
{
	struct defaultobj *obj = NULL;
	struct weaponfunc *basefunc;
	struct weaponfunc_throw *func;
	struct weapon *weapon = weaponFindById(gset->weaponnum);
	struct weaponobj *weaponobj;
	struct autogunobj *autogun;
	Mtxf mtx;
	int playernum;

	if (weapon == NULL) {
		return false;
	}

	basefunc = weapon->functions[gset->weaponfunc];
	func = (struct weaponfunc_throw *) basefunc;

	if (func == NULL) {
		return false;
	}

	if (gset->weaponnum == WEAPON_COMBATKNIFE) {
		mtxRotateF(mtx.m, 90.0f / (RANDOMFRAC() + 12.1f),
				arg4->m[1][0], arg4->m[1][1], arg4->m[1][2]);
	} else {
		mtxLoadRandomRotation(&mtx);
	}

	if (gset->weaponnum == WEAPON_LAPTOPGUN) {
		autogun = laptopDeploy(func->projectilemodelnum, gset, chr);

		if (autogun != NULL) {
			obj = &autogun->base;
		}
	} else {
		weaponobj = weaponCreateProjectileFromGset(func->projectilemodelnum, gset, chr);

		if (weaponobj != NULL) {
			obj = &weaponobj->base;

			// Note this timer is converted to 240 time immediately below
			weaponobj->timer240 = func->activatetime60;

			if (weaponobj->timer240 >= 2) {
				weaponobj->timer240 = TICKS(weaponobj->timer240 * 4);
			}

			if (weaponobj->weaponnum == WEAPON_GRENADE || weaponobj->weaponnum == WEAPON_NBOMB) {
				propSetDangerous(weaponobj->base.prop);
			}

			if (func->projectilemodelnum == MODEL_CHRREMOTEMINE
					|| func->projectilemodelnum == MODEL_CHRTIMEDMINE
					|| func->projectilemodelnum == MODEL_CHRPROXIMITYMINE
					|| func->projectilemodelnum == MODEL_CHRECMMINE) {
				weaponobj->base.flags3 |= OBJFLAG3_00000008;
			}
		}
	}

	if (obj != NULL) {
		bgunCreateXBowBolt(obj, pos, rooms, arg4, velocity, &mtx, chr->prop, pos);

		obj->hidden &= 0x0fffffff;

		if (g_Vars.normmplayerisrunning) {
			playernum = mpPlayerGetIndex(chr);
		} else {
			playernum = playermgrGetPlayerNumByProp(chr->prop);
		}

		obj->hidden |= playernum << 28;

		if (obj->hidden & OBJHFLAG_PROJECTILE) {
			obj->projectile->flags |= PROJECTILEFLAG_00000002;
			obj->projectile->bounciness = 0.1f;
			obj->projectile->pickuptimer240 = TICKS(240);

			psCreate(NULL, obj->prop, SFX_THROW, -1,
					-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
		}
	}

	return obj;
}

/**
 * handnum supports some unusual values:
 *
 * 0 = right hand
 * 1 = left hand
 * 2 = fumbling grenade from right hand (due to nbomb)
 * 3 = fumbling grenade from left hand (actually not possible)
 */
void bgunCreateThrownProjectile(int handnum, struct gset *gset)
{
	struct coord velocity = {0, 0, 0};
	Mtxf sp1f4;
	struct coord gunpos;
	struct coord gundir;
	struct prop *playerprop = g_Vars.currentplayer->prop;
	struct coord *prevpos = &g_Vars.currentplayer->bondprevpos;
	struct coord *extrapos = &g_Vars.currentplayer->bondextrapos;
	Mtxf sp190;
	struct defaultobj *obj;
	struct weaponobj *weapon;
	struct coord muzzlepos;
	struct coord spawnpos;
	RoomNum spawnrooms[8];
	bool droppinggrenade = false;
	struct hand *hand;
	struct coord aimpos;
	struct coord sp140;
	float frac;
	float radians;
	Mtxf spf8;
	Mtxf spb8;
	Mtxf sp78;
	float sp68[4];
	float sp58[4];
	float sp48[4];
	struct trainingdata *data;

	if (handnum >= 2) {
		droppinggrenade = true;
		handnum -= 2;
	}

	hand = g_Vars.currentplayer->hands + handnum;

	muzzlepos.x = g_Vars.currentplayer->hands[handnum].muzzlepos.x;
	muzzlepos.y = g_Vars.currentplayer->hands[handnum].muzzlepos.y;
	muzzlepos.z = g_Vars.currentplayer->hands[handnum].muzzlepos.z;

	mtx4LoadIdentity(&sp1f4);

	if (gset->weaponnum == WEAPON_COMBATKNIFE) {
		mtx4LoadZRotation(4.711639f, &sp1f4);
		mtx4LoadXRotation(3.1410925f, &sp190);
		mtx4MultMtx4InPlace(&sp190, &sp1f4);
	}

	mtx4Copy(&g_Vars.currentplayer->hands[handnum].muzzlemat, &sp190);

	utilsNormalizeF(&sp190.m[0][0], &sp190.m[0][1], &sp190.m[0][2]);
	utilsNormalizeF(&sp190.m[1][0], &sp190.m[1][1], &sp190.m[1][2]);
	utilsNormalizeF(&sp190.m[2][0], &sp190.m[2][1], &sp190.m[2][2]);

	sp190.m[3][0] = 0.0f;
	sp190.m[3][1] = 0.0f;
	sp190.m[3][2] = 0.0f;

	mtx4MultMtx4InPlace(&sp190, &sp1f4);

	playerSetPerimEnabled(playerprop, false);

	if (cdTestLos11(&playerprop->pos, playerprop->rooms, &muzzlepos, spawnrooms, CDTYPE_ALL) != CDRESULT_COLLISION) {
		spawnpos.x = muzzlepos.x;
		spawnpos.y = muzzlepos.y;
		spawnpos.z = muzzlepos.z;
	} else {
		spawnpos.x = playerprop->pos.x;
		spawnpos.y = playerprop->pos.y;
		spawnpos.z = playerprop->pos.z;

		roomsCopy(playerprop->rooms, spawnrooms);
	}

	playerSetPerimEnabled(playerprop, true);

	bgunCalculatePlayerShotSpread(&gunpos, &gundir, handnum, true);
	mtx4RotateVecInPlace(camGetProjectionMtxF(), &gundir);

	if (droppinggrenade) {
		// Dropping a grenade because player is in an nbomb storm
		velocity.x = gundir.x * 1.6666666f;
		velocity.y = gundir.y * 1.6666666f;
		velocity.z = gundir.z * 1.6666666f;
	} else if (gsetHasFunctionFlags(&hand->gset, FUNCFLAG_CALCULATETRAJECTORY)) {
		// Calculate the velocity based on the trajectory to the aimpos
		propFindAimingAt(HAND_RIGHT, false, FINDPROPCONTEXT_QUERY);

		if (hand->hasdotinfo) {
			aimpos.x = hand->dotpos.x;
			aimpos.y = hand->dotpos.y;
			aimpos.z = hand->dotpos.z;

			chrCalculateTrajectory(&spawnpos, 21.666666f, &aimpos, &sp140);

			radians = acosf(gundir.f[0] * sp140.f[0] + gundir.f[1] * sp140.f[1] + gundir.f[2] * sp140.f[2]);

			// Check within 20 degrees
			if (radians > 0.34901026f || radians < -0.34901026f) {
				mtx00016b58(&spf8, 0, 0, 0, gundir.x, gundir.y, gundir.z, 0, 1, 0);
				mtx00016b58(&spb8, 0, 0, 0, sp140.x, sp140.y, sp140.z, 0, 1, 0);

				quaternion0f097044(&spf8, sp68);
				quaternion0f097044(&spb8, sp58);
				quaternionAvoidFlips(sp68, sp58);

				frac = 0.34901025891304f / radians;

				if (frac < 0.0f) {
					frac = -frac;
				}

				quaternionSlerp(sp68, sp58, frac, sp48);
				quaternionToMtx(sp48, &sp78);

				gundir.x = -sp78.m[2][0];
				gundir.y = -sp78.m[2][1];
				gundir.z = -sp78.m[2][2];
			} else {
				gundir.x = sp140.x;
				gundir.y = sp140.y;
				gundir.z = sp140.z;
			}
		}

		velocity.x = gundir.x * 21.666666f;
		velocity.y = gundir.y * 21.666666f;
		velocity.z = gundir.z * 21.666666f;
	} else {
		// Simple velocity
		velocity.x = gundir.x * 16.666666f;
		velocity.y = gundir.y * 16.666666f;
		velocity.z = gundir.z * 16.666666f;

		if (gset->weaponnum == WEAPON_GRENADE || gset->weaponnum == WEAPON_NBOMB) {
			velocity.y += 1.6666666f;
		} else {
			velocity.y += 5.0f;
		}
	}

	if (gset->weaponnum == WEAPON_LAPTOPGUN) {
		bgunFreeWeapon(handnum);
	}

	// Add player movement to velocity
	if (g_Vars.lvupdate240 > 0) {
		velocity.x += (playerprop->pos.x - prevpos->x + extrapos->x) / g_Vars.lvupdate60freal;
		velocity.y += (playerprop->pos.y - prevpos->y + extrapos->y) / g_Vars.lvupdate60freal;
		velocity.z += (playerprop->pos.z - prevpos->z + extrapos->z) / g_Vars.lvupdate60freal;
	}

	obj = bgunCreateThrownProjectile2(g_Vars.currentplayer->prop->chr, gset, &spawnpos, spawnrooms, &sp1f4, &velocity);

	if (obj) {
		if (obj->type == OBJTYPE_WEAPON) {
			weapon = (struct weaponobj *)obj;
			if(weapon)
			{
				if (gset->weaponnum == WEAPON_GRENADE && gset->weaponfunc == FUNC_PRIMARY) {
					if (weapon->timer240 < hand->primetimer60 * 4) {
						weapon->timer240 = 0;
					} else {
						weapon->timer240 -= hand->primetimer60 * 4;
					}

					weapon->gunfunc = gset->weaponfunc;
				} else if (gset->weaponnum == WEAPON_ECMMINE && g_Vars.stagenum == STAGE_CITRAINING) {
					data = dtGetData();

					if (data->intraining) {
						data->obj = obj;
					}
				}
			

				if (obj->hidden & OBJHFLAG_PROJECTILE) {
					obj->projectile->flags |= PROJECTILEFLAG_LAUNCHING;
					obj->projectile->nextsteppos.x = muzzlepos.x;
					obj->projectile->nextsteppos.y = muzzlepos.y;
					obj->projectile->nextsteppos.z = muzzlepos.z;

					if (gset->weaponnum == WEAPON_GRENADE && gset->weaponfunc == FUNC_SECONDARY) {
						obj->projectile->bounciness = 1.0f;
					}

					if (gset->weaponnum == WEAPON_COMBATKNIFE) {
						// In theory, weapon can be uninitialised here,
						// but in practice it's always set.
						weapon->base.projectile->flags |= PROJECTILEFLAG_00000002;
						weapon->base.projectile->bounciness = 0.1f;
						weapon->base.projectile->pickuptimer240 = TICKS(240);
						weapon->base.hidden |= OBJHFLAG_THROWNKNIFE;
					}
				}
			}
		}
	}
}

void bgunUpdateHeldRocket(int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];
	struct defaultobj *obj = &hand->rocket->base;

	if (obj) {
		struct prop *objprop = obj->prop;
		Mtxf mtx;

		if (objprop) {
			struct prop *playerprop = g_Vars.currentplayer->prop;
			struct model *model = obj->model;

			if (!hand->firedrocket) {
				mtx4Copy(&hand->posmtx, &mtx);

				mtx.m[3][0] = 0;
				mtx.m[3][1] = 0;
				mtx.m[3][2] = 0;

				mtx00015f04(obj->model->scale, &mtx);
				func0f06a580(obj, &hand->muzzlepos, &mtx, playerprop->rooms);
				propDeregisterRooms(objprop);
			}

			model->matrices = gfxAllocate(model->definition->nummatrices * sizeof(Mtxf));

			mtx4Copy(&hand->muzzlemat, &model->matrices[0]);
			modelUpdateRelationsQuick(model, model->definition->rootnode);

			objprop->flags |= PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONTHISSCREENTHISTICK;
			objprop->z = -model->matrices[0].m[3][2];
		}
	}
}

void bgunCreateHeldRocket(int handnum, struct weaponfunc_shootprojectile *func)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];
	struct weaponobj *obj;

	if (hand->rocket == NULL) {
		hand->firedrocket = false;

		obj = weaponCreateProjectileFromWeaponNum(func->projectilemodelnum, WEAPON_ROCKET, g_Vars.currentplayer->prop->chr);

		if (obj != NULL) {
			hand->rocket = obj;
			hand->firedrocket = false;

			obj->timer240 = 1;
			obj->base.flags |= OBJFLAG_HELDROCKET;
			obj->base.flags2 |= OBJFLAG2_THROWTHROUGH;
		}
	}
}

void bgunFreeHeldRocket(int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	if (hand->rocket) {
		objFreePermanently(&hand->rocket->base, true);
		hand->rocket = NULL;
	}
}

void bgunCreateFiredProjectile(int handnum)
{
	struct weapon *weapondef;
	struct hand *hand;
	Mtxf sp270;
	struct coord sp264;
	float sp260;
	float sp25c;
	struct coord sp250;
	Mtxf sp210;
	struct coord gunpos;
	struct coord gundir;
	struct prop *playerprop;
	struct coord *prevpos;
	struct coord *extrapos;
	struct coord spawnpos;
	struct weaponobj *weapon;
	struct weaponfunc *tmp;
	struct weaponfunc_shootprojectile *funcdef;
	struct coord aimpos;
	struct coord sp1bc;
	float frac;
	float radians;
	Mtxf sp174;
	Mtxf sp134;
	Mtxf spf4;
	float spe4[4];
	float spd4[4];
	float spc4[4];

	hand = g_Vars.currentplayer->hands + handnum;

	playerprop = g_Vars.currentplayer->prop;
	prevpos = &g_Vars.currentplayer->bondprevpos;
	extrapos = &g_Vars.currentplayer->bondextrapos;

	weapondef = weaponFindById(hand->gset.weaponnum);

	if (weapondef) {
		tmp = weapondef->functions[hand->gset.weaponfunc];

		if (tmp && tmp->type == INVENTORYFUNCTYPE_SHOOT_PROJECTILE) {
			funcdef = (struct weaponfunc_shootprojectile *)tmp;

			mtx4LoadIdentity(&sp270);
			bgunCalculatePlayerShotSpread(&gunpos, &gundir, handnum, true);
			mtx4RotateVecInPlace(camGetProjectionMtxF(), &gundir);

			spawnpos.x = hand->muzzlepos.x;
			spawnpos.y = hand->muzzlepos.y;
			spawnpos.z = hand->muzzlepos.z;

			if (hand->gset.weaponnum == WEAPON_SLAYER && hand->gset.weaponfunc == FUNC_SECONDARY) {
				spawnpos.x += 50.0f * gundir.x;
				spawnpos.y += 50.0f * gundir.y;
				spawnpos.z += 50.0f * gundir.z;
			}

			sp260 = funcdef->speed * 1.6666666f / 60.0f;
			sp25c = funcdef->traveldist * 1.6666666f;

			if (gsetHasFunctionFlags(&hand->gset, FUNCFLAG_CALCULATETRAJECTORY)) {
				propFindAimingAt(HAND_RIGHT, false, FINDPROPCONTEXT_QUERY);

				if (hand->hasdotinfo) {
					aimpos.x = hand->dotpos.x;
					aimpos.y = hand->dotpos.y;
					aimpos.z = hand->dotpos.z;

					chrCalculateTrajectory(&spawnpos, sp25c, &aimpos, &sp1bc);

					radians = acosf(gundir.f[0] * sp1bc.f[0] + gundir.f[1] * sp1bc.f[1] + gundir.f[2] * sp1bc.f[2]);

					if (radians > 0.17450513f || radians < -0.17450513f) {
						mtx00016b58(&sp174, 0.0f, 0.0f, 0.0f, gundir.x, gundir.y, gundir.z, 0.0f, 1.0f, 0.0f);
						mtx00016b58(&sp134, 0.0f, 0.0f, 0.0f, sp1bc.x, sp1bc.y, sp1bc.z, 0.0f, 1.0f, 0.0f);

						quaternion0f097044(&sp174, spe4);
						quaternion0f097044(&sp134, spd4);
						quaternionAvoidFlips(spe4, spd4);

						frac = 0.17450513f / radians;

						if (frac < 0.0f) {
							frac = -frac;
						}

						quaternionSlerp(spe4, spd4, frac, spc4);
						quaternionToMtx(spc4, &spf4);

						gundir.x = -spf4.m[2][0];
						gundir.y = -spf4.m[2][1];
						gundir.z = -spf4.m[2][2];
					} else {
						gundir.x = sp1bc.x;
						gundir.y = sp1bc.y;
						gundir.z = sp1bc.z;
					}
				}
			}

			sp250.x = gundir.x * sp260;
			sp250.y = gundir.y * sp260;
			sp250.z = gundir.z * sp260;

			sp264.x = sp250.f[0] * g_Vars.lvupdate60freal + gundir.f[0] * sp25c;
			sp264.y = sp250.f[1] * g_Vars.lvupdate60freal + gundir.f[1] * sp25c;
			sp264.z = sp250.f[2] * g_Vars.lvupdate60freal + gundir.f[2] * sp25c;

			if ((funcdef->base.base.flags & FUNCFLAG_FLYBYWIRE) == 0 && g_Vars.lvupdate240 > 0) {
				sp264.x += (playerprop->pos.x - prevpos->x + extrapos->x) / g_Vars.lvupdate60freal;
				sp264.y += (playerprop->pos.y - prevpos->y + extrapos->y) / g_Vars.lvupdate60freal;
				sp264.z += (playerprop->pos.z - prevpos->z + extrapos->z) / g_Vars.lvupdate60freal;
			}

			mtx4Copy(&g_Vars.currentplayer->hands[handnum].posmtx, &sp210);

			sp210.m[3][0] = 0.0f;
			sp210.m[3][1] = 0.0f;
			sp210.m[3][2] = 0.0f;

			if (hand->rocket) {
				hand->firedrocket = true;

				weapon = hand->rocket;
				weapon->base.flags2 &= ~OBJFLAG2_THROWTHROUGH;
				weapon->base.flags &= ~OBJFLAG_HELDROCKET;

				if (funcdef->base.base.flags & FUNCFLAG_HOMINGROCKET) {
					weapon->weaponnum = WEAPON_HOMINGROCKET;
				}
			} else if (hand->gset.weaponnum == WEAPON_ROCKETLAUNCHER || hand->gset.weaponnum == WEAPON_SLAYER) {
				int weaponnum = WEAPON_ROCKET;

				if (funcdef->base.base.flags & FUNCFLAG_HOMINGROCKET) {
					weaponnum = WEAPON_HOMINGROCKET;
				}

				weapon = weaponCreateProjectileFromWeaponNum(funcdef->projectilemodelnum, weaponnum, g_Vars.currentplayer->prop->chr);
			} else if (hand->gset.weaponnum == WEAPON_CROSSBOW) {
				weapon = weaponCreateProjectileFromWeaponNum(funcdef->projectilemodelnum, WEAPON_BOLT, g_Vars.currentplayer->prop->chr);

				if (weapon) {
					weapon->gunfunc = hand->gset.weaponfunc;
				}
			} else if (hand->gset.weaponnum == WEAPON_DEVASTATOR) {
				weapon = weaponCreateProjectileFromWeaponNum(funcdef->projectilemodelnum, WEAPON_GRENADEROUND, g_Vars.currentplayer->prop->chr);

				if (weapon) {
					weapon->gunfunc = hand->gset.weaponfunc;
				}
			} else if (hand->gset.weaponnum == WEAPON_SUPERDRAGON) {
				weapon = weaponCreateProjectileFromWeaponNum(funcdef->projectilemodelnum, WEAPON_GRENADEROUND, g_Vars.currentplayer->prop->chr);

				if (weapon) {
					weapon->gunfunc = FUNC_2;
				}
			} else {
				weapon = weaponCreateProjectileFromGset(funcdef->projectilemodelnum, &hand->gset, g_Vars.currentplayer->prop->chr);
			}

			if (weapon) {
				bool failed = false;
				Mtxf sp78;
				struct coord sp6c;
				struct coord sp60;

				if (weapon->base.model && weapon->base.model->definition) {
					weapon->timer240 = funcdef->timer60;

					if (weapon->timer240 != -1) {
						weapon->timer240 = TICKS(weapon->timer240 * 4);
					}

					weapon->base.hidden &= 0x0fffffff;
					weapon->base.hidden |= g_Vars.currentplayernum << 28;

					bgun0f09ed2c(&weapon->base, &spawnpos, &sp210, &sp264, &sp270);

					if (weapon->base.hidden & OBJHFLAG_PROJECTILE) {
						if (funcdef->base.base.flags & FUNCFLAG_PROJECTILE_LIGHTWEIGHT) {
							weapon->base.projectile->flags |= PROJECTILEFLAG_LIGHTWEIGHT;
						} else if (funcdef->base.base.flags & FUNCFLAG_PROJECTILE_POWERED) {
							weapon->base.projectile->flags |= PROJECTILEFLAG_POWERED;
						}

						weapon->base.projectile->targetprop = g_Vars.currentplayer->trackedprops[0].prop;

						if (funcdef->scale != 1.0f) {
							weapon->base.model->scale *= funcdef->scale;

							mtx3ToMtx4(weapon->base.realrot, &sp78);
							mtx00015f04(funcdef->scale, &sp78);
							mtx4ToMtx3(&sp78, weapon->base.realrot);
						}

						weapon->base.projectile->powerlimit240 = TICKS(1200);
						weapon->base.projectile->unk0a8 = weapon->base.prop->pos.y;
						weapon->base.projectile->unk0ac = weapon->base.projectile->speed.y;
						weapon->base.projectile->unk010 = sp250.x;
						weapon->base.projectile->unk014 = sp250.y;
						weapon->base.projectile->unk018 = sp250.z;
						weapon->base.projectile->pickuptimer240 = TICKS(240);
						weapon->base.projectile->bounciness = funcdef->reflectangle;
						weapon->base.projectile->unk098 = funcdef->unk50 * 1.6666666f;

						if (funcdef->soundnum > 0) {
							psCreate(NULL, weapon->base.prop, funcdef->soundnum, -1, -1, 0, 0, PSTYPE_NONE, 0, -1.0f, 0, -1, -1.0f, -1.0f, -1.0f);
						}

						if (funcdef->base.base.flags & FUNCFLAG_FLYBYWIRE) {
							playerLaunchSlayerRocket(weapon);
						}

						if (weapon->base.projectile->flags & PROJECTILEFLAG_LAUNCHING) {
							projectileLaunch(&weapon->base, weapon->base.projectile, &sp6c, &sp60);
						}
					} else {
						failed = true;
					}
				} else {
					failed = true;
				}

				if (failed) {
					weapon->timer240 = -1;

					if (weapon->base.prop) {
						propFree(weapon->base.prop);
					}

					if (weapon->base.model) {
						modelmgrFreeModel(weapon->base.model);
					}

					weapon->base.prop = NULL;
					weapon->base.model = NULL;
				}
			}
		}
	}
}

void bgunSwivel(float screenx, float screeny, float crossdamp, float aimdamp)
{
	float screenwidth = camGetScreenWidth();
	float screenheight = camGetScreenHeight();
	struct player *player = g_Vars.currentplayer;
	struct coord aimpos;
	int l;
	int h;
	float x[2];
	float y[2];
	bool ignore[2] = {false, false};
	int numframes;
	struct hand *hand;
	struct coord sp94;
	float sp8c[2];

	x[HAND_RIGHT] = x[HAND_LEFT] = screenx;
	y[HAND_RIGHT] = y[HAND_LEFT] = screeny;

	ignore[HAND_LEFT] = !player->hands[HAND_LEFT].inuse;
	ignore[HAND_RIGHT] = !player->hands[HAND_RIGHT].inuse;

	// If using right hand only and reloading,
	// recentre until the reload animation is almost complete
	if (!player->hands[HAND_LEFT].inuse
			&& player->hands[HAND_RIGHT].state == HANDSTATE_RELOAD
			&& player->hands[HAND_RIGHT].unk0ce8) {
		numframes = 25;

		if (player->hands[HAND_RIGHT].gset.weaponnum == WEAPON_CROSSBOW) {
			numframes = 5;
		}

		if ((int)bgun0f09815c(&player->hands[HAND_RIGHT]) < modelGetNumAnimFrames(&player->hands[HAND_RIGHT].gunmodel) - numframes) {
			x[HAND_RIGHT] = 0.0f;
			y[HAND_RIGHT] = 0.0f;
			ignore[HAND_RIGHT] = true;
		}
	}

	if (player->hands[HAND_LEFT].gset.weaponnum == WEAPON_REMOTEMINE) {
		x[HAND_LEFT] = g_Vars.currentplayer->speedtheta * 0.3f + g_Vars.currentplayer->gunextraaimx;
		y[HAND_LEFT] = -g_Vars.currentplayer->speedverta * 0.1f + g_Vars.currentplayer->gunextraaimy;
		ignore[HAND_LEFT] = true;
	}

	if (player->hands[HAND_RIGHT].gset.weaponnum == WEAPON_UNARMED) {
		x[HAND_RIGHT] = g_Vars.currentplayer->speedtheta * 0.3f + g_Vars.currentplayer->gunextraaimx;
		y[HAND_RIGHT] = -g_Vars.currentplayer->speedverta * 0.1f + g_Vars.currentplayer->gunextraaimy;
		ignore[HAND_RIGHT] = true;
	}

	if (g_Vars.currentplayer->teleportstate != TELEPORTSTATE_INACTIVE) {
		ignore[HAND_LEFT] = ignore[HAND_RIGHT] = true;
	}

	// This loop only iterates once
	for (h = 0; h < 1; h++) {
		if (!ignore[h]) {
			hand = &player->hands[h];

			if (hand->hasdotinfo && !g_Vars.mplayerisrunning) {
				sp94.x = hand->dotpos.x;
				sp94.y = hand->dotpos.y;
				sp94.z = hand->dotpos.z;

				mtx4TransformVecInPlace(camGetWorldToScreenMtxf(), &sp94);

				if (!(sp94.z < 0.0000001f) || !(sp94.z > -0.0000001f)) {
					if (sp94.z > -6000.0f) {
						cam0f0b4d04(&sp94, sp8c);

						x[h] = sp8c[0];
						y[h] = sp8c[1];

						x[h] = 2.0f * (x[h] / viGetViewWidth()) - 1.0f;
						y[h] = 2.0f * (y[h] / viGetViewHeight()) - 1.0f;
					}
				}
			}
		}
	}

	player->oldcrosspos[0] = player->crosspos[0];
	player->oldcrosspos[1] = player->crosspos[1];

	if (crossdamp != player->guncrossdamp) {
		player->crosspossum[0] = player->crosspossum[0] * (1.0f - player->guncrossdamp) / (1.0f - crossdamp);
		player->crosspossum[1] = player->crosspossum[1] * (1.0f - player->guncrossdamp) / (1.0f - crossdamp);
		player->guncrossdamp = crossdamp;
	}

	if (aimdamp != player->gunaimdamp) {
		player->crosssum2[0] = player->crosssum2[0] * (1.0f - player->gunaimdamp) / (1.0f - aimdamp);
		player->crosssum2[1] = player->crosssum2[1] * (1.0f - player->gunaimdamp) / (1.0f - aimdamp);
		player->gunaimdamp = aimdamp;
	}

	for (l = 0; l < g_Vars.lvupdate240; l++) {
		player->crosspossum[0] = player->crosspossum[0] * crossdamp + screenx;
		player->crosspossum[1] = player->crosspossum[1] * crossdamp + screeny;

		for (h = 0; h < 2; h++) {
			hand = &player->hands[h];
			hand->guncrosspossum[0] = (PAL ? 0.913f : 0.9269697f) * hand->guncrosspossum[0] + x[h];
			hand->guncrosspossum[1] = (PAL ? 0.913f : 0.9269697f) * hand->guncrosspossum[1] + y[h];
		}
	}

	player->crosspos[0] = player->crosspossum[0] * (1.0f - crossdamp) * screenwidth * 0.5f + screenwidth * 0.5f;
	player->crosspos[1] = player->crosspossum[1] * (1.0f - crossdamp) * screenheight * 0.5f + screenheight * 0.5f;

	if (player->crosspos[0] < 3.0f) {
		player->crosspos[0] = 3.0f;
	} else if (player->crosspos[0] > screenwidth - 4.0f) {
		player->crosspos[0] = screenwidth - 4.0f;
	}

	if (player->crosspos[1] < 3.0f) {
		player->crosspos[1] = 3.0f;
	} else if (player->crosspos[1] > screenheight - 4.0f) {
		player->crosspos[1] = screenheight - 4.0f;
	}

	player->crosspos[0] += camGetScreenLeft();
	player->crosspos[1] += camGetScreenTop();

	for (h = 0; h < 2; h++) {
		player->hands[h].crosspos[0] = player->hands[h].guncrosspossum[0] * (PAL ? 0.08700001f : 0.07303029f) * screenwidth * 0.5f + screenwidth * 0.5f;
		player->hands[h].crosspos[1] = player->hands[h].guncrosspossum[1] * (PAL ? 0.08700001f : 0.07303029f) * screenheight * 0.5f + screenheight * 0.5f;

		if (player->hands[h].crosspos[0] < 3.0f) {
			player->hands[h].crosspos[0] = 3.0f;
		} else if (player->hands[h].crosspos[0] > screenwidth - 4.0f) {
			player->hands[h].crosspos[0] = screenwidth - 4.0f;
		}

		if (player->hands[h].crosspos[1] < 3.0f) {
			player->hands[h].crosspos[1] = 3.0f;
		} else if (player->hands[h].crosspos[1] > screenheight - 4.0f) {
			player->hands[h].crosspos[1] = screenheight - 4.0f;
		}

		player->hands[h].crosspos[0] += camGetScreenLeft();
		player->hands[h].crosspos[1] += camGetScreenTop();
	}

	for (l = 0; l < g_Vars.lvupdate240; l++) {
		player->crosssum2[0] = player->crosssum2[0] * aimdamp + screenx;
		player->crosssum2[1] = player->crosssum2[1] * aimdamp + screeny;
	}

	player->crosspos2[0] = player->crosssum2[0] * (1.0f - aimdamp) * screenwidth * 0.5f + screenwidth * 0.5f;
	player->crosspos2[1] = player->crosssum2[1] * (1.0f - aimdamp) * screenheight * 0.5f + screenheight * 0.5f;
	player->crosspos2[0] += camGetScreenLeft();
	player->crosspos2[1] += camGetScreenTop();

	cam0f0b4c3c(player->crosspos2, &aimpos, 1000);

	bgunSetAimPos(&aimpos);
}

/**
 * Swivel the gun towards the given screen coordinates, dampening the movement
 * speed as it reaches the target.
 *
 * This is used for auto aim, the CMP's follow lock-on, and general turning.
 */
void bgunSwivelWithDamp(float screenx, float screeny, float crossdamp)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));
	float aimdamp = PAL ? weapon->aimsettings->aimdamppal : weapon->aimsettings->aimdamp;

	if (aimdamp < crossdamp) {
		aimdamp = crossdamp;
	}

	bgunSwivel(screenx, screeny, crossdamp, aimdamp);
}

/**
 * Swivel the gun towards the given screen coordinates without slowing the speed
 * speed as it reaches the target.
 *
 * This is used when manual aiming.
 */
void bgunSwivelWithoutDamp(float screenx, float screeny)
{
	struct weapon *weapon = weaponFindById(bgunGetWeaponNum(HAND_RIGHT));
	float aimdamp = PAL ? weapon->aimsettings->aimdamppal : weapon->aimsettings->aimdamp;

	bgunSwivel(screenx, screeny, PAL ? 0.935f : 0.945f, aimdamp);
}

void bgunGetCrossPos(float *x, float *y)
{
	struct player *player = g_Vars.currentplayer;

	*x = player->crosspos[0];
	*y = player->crosspos[1];
}

void bgun0f0a0c08(struct coord *arg0, struct coord *arg1)
{
	arg0->x = 0;
	arg0->y = 0;
	arg0->z = 0;

	cam0f0b4c3c(g_Vars.currentplayer->crosspos, arg1, 1);
}

void bgun0f0a0c44(int handnum, struct coord *arg1, struct coord *arg2)
{
	arg1->x = 0;
	arg1->y = 0;
	arg1->z = 0;

	cam0f0b4c3c(g_Vars.currentplayer->hands[handnum].crosspos, arg2, 1);
}

void bgunCalculatePlayerShotSpread(struct coord *gunpos2d, struct coord *gundir2d, int handnum, bool dorandom)
{
	float crosspos[2];
	float spread = 0;
	float scaledspread;
	float randfactor;
	struct weaponfunc *func = currentPlayerGetWeaponFunction(handnum);
	struct player *player = g_Vars.currentplayer;

	if (func != NULL && (func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		struct weaponfunc_shoot *shootfunc = (struct weaponfunc_shoot *) func;
		spread = shootfunc->spread;
	}

	if (weaponHasAimFlag(bgunGetWeaponNum2(handnum), INVAIMFLAG_ACCURATESINGLESHOT)
			&& player->hands[handnum].burstbullets == 1) {
		spread *= 0.25f;
	}

	// Decrease spread if double crouched
	if (bmoveGetCrouchPos() == CROUCHPOS_SQUAT) {
		spread *= 0.5f;
	}

	// Increase spread if dual wielding
	if (player->hands[HAND_LEFT].inuse) {
		spread *= 1.5f;
	}

	scaledspread = 120.0f * spread / viGetFovY();

	if (dorandom) {
		randfactor = (RANDOMFRAC() - 0.5f) * RANDOMFRAC();
	} else {
		randfactor = 0;
	}

	crosspos[0] = player->crosspos[0] + randfactor * scaledspread * camGetScreenWidth()
		/ (viGetHeight() * camGetPerspAspect());

	if (dorandom) {
		randfactor = (RANDOMFRAC() - 0.5f) * RANDOMFRAC();
	} else {
		randfactor = 0;
	}

	crosspos[1] = player->crosspos[1] + (randfactor * scaledspread * camGetScreenHeight())
		/ viGetHeight();

	gunpos2d->x = 0;
	gunpos2d->y = 0;
	gunpos2d->z = 0;

	cam0f0b4c3c(crosspos, gundir2d, 1);
}

void bgunCalculateBotShotSpread(struct coord *arg0, int weaponnum, int funcnum, bool arg3, int crouchpos, bool dual)
{
	float spread = 0.0f;
	float radius;
	struct weapon *weapondef = weaponFindById(weaponnum);
	float x;
	float y;
	Mtxf mtx;
	struct coord sp48;

	if (weapondef) {
		struct weaponfunc *funcdef = weapondef->functions[funcnum];

		if (funcdef && (funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
			struct weaponfunc_shoot *shootfunc = (struct weaponfunc_shoot *)funcdef;
			spread = shootfunc->spread;
		}
	}

	if (arg3 && weaponHasAimFlag(weaponnum, INVAIMFLAG_ACCURATESINGLESHOT)) {
		spread *= 0.25f;
	}

	if (crouchpos == CROUCHPOS_SQUAT) {
		spread *= 0.5f;
	}

	if (dual) {
		spread *= 1.5f;
	}

	radius = 120.0f * spread / viGetFovY();
	x = (RANDOMFRAC() - 0.5f) * RANDOMFRAC() * radius;
	y = (RANDOMFRAC() - 0.5f) * RANDOMFRAC() * radius;

	sp48.x = g_Vars.currentplayer->c_scalex * x;
	sp48.y = g_Vars.currentplayer->c_scaley * y;
	sp48.z = -1.0f;

	utilsNormalizeF(&sp48.x, &sp48.y, &sp48.z);
	mtx00016b58(&mtx, 0.0f, 0.0f, 0.0f, arg0->x, arg0->y, arg0->z, 0.0f, -1.0f, 0.0f);
	mtx4RotateVec(&mtx, &sp48, arg0);
}

bool bgunGetLastShootInfo(struct coord *pos, struct coord *dir, int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	if (!hand->lastdirvalid) {
		return false;
	}

	pos->x = hand->lastshootpos.x;
	pos->y = hand->lastshootpos.y;
	pos->z = hand->lastshootpos.z;

	dir->x = hand->lastshootdir.x;
	dir->y = hand->lastshootdir.y;
	dir->z = hand->lastshootdir.z;

	return true;
}

void bgunSetLastShootInfo(struct coord *pos, struct coord *dir, int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	hand->lastdirvalid = true;

	hand->lastshootpos.x = pos->x;
	hand->lastshootpos.y = pos->y;
	hand->lastshootpos.z = pos->z;

	hand->lastshootdir.x = dir->x;
	hand->lastshootdir.y = dir->y;
	hand->lastshootdir.z = dir->z;
}

int bgunGetShotsToTake(int handnum)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	return hand->shotstotake;
}

void bgunFreeWeapon(int handnum)
{
	struct player *player = g_Vars.currentplayer;
	int i;

	if (player->hands[handnum].inuse) {
		for (i = 0; i < 2; i++) {
			if (player->gunctrl.ammotypes[i] >= 0) {
				int spaceinclip = player->hands[handnum].clipsizes[i] - player->hands[handnum].loadedammo[i];
				int index = bgunGetUnequippedReloadIndex(player->gunctrl.weaponnum);

				if (index != -1) {
					player->hands[handnum].gunroundsspent[index] = (spaceinclip << 8) | 0xff;
				}

				if (player->hands[handnum].loadedammo[i] > 0) {
					player->ammoheldarr[player->gunctrl.ammotypes[i]] += player->hands[handnum].loadedammo[i];
				}

				player->hands[handnum].loadedammo[i] = 0;
			}
		}
	}

	if (g_Vars.mplayerisrunning) {
		playermgrDeleteWeapon(handnum);
	}

	bgunFreeHeldRocket(handnum);
}

void bgunTickSwitch2(void)
{
	struct player *player = g_Vars.currentplayer;
	struct gunctrl *ctrl = &g_Vars.currentplayer->gunctrl;
	int i;

	if (ctrl->switchtoweaponnum >= 0) {
		if (bgunCanFreeWeapon(HAND_RIGHT) && bgunCanFreeWeapon(HAND_LEFT)) {
			int weaponnum = player->gunctrl.weaponnum;
			int previnuse = player->hands[HAND_LEFT].inuse;
			struct hand *lefthand;
			struct hand *righthand;

			if (currentPlayerGetDeviceState(ctrl->switchtoweaponnum) != DEVICESTATE_UNEQUIPPED) {
				ctrl->switchtoweaponnum = WEAPON_UNARMED;
			}

			if (ctrl->dualwielding && !invHasDoubleWeaponIncAllGuns(ctrl->switchtoweaponnum, ctrl->switchtoweaponnum)) {
				ctrl->dualwielding = false;
			}

			//func0f0d7364(); Ben's comment: I'm not sure if this does anything in practice?
			bgunFreeWeapon(HAND_LEFT);
			bgunFreeWeapon(HAND_RIGHT);

			if (weaponnum == WEAPON_HORIZONSCANNER) {
				g_Vars.currentplayer->insightaimmode = false;
			}

			if (weaponnum == WEAPON_RCP120) {
				int amount = player->hands[HAND_RIGHT].matmot1;

				if (amount > player->ammoheldarr[ctrl->ammotypes[0]]) {
					amount = player->ammoheldarr[ctrl->ammotypes[0]];
				}

				player->ammoheldarr[ctrl->ammotypes[0]] -= amount;
			}

			if (weaponnum == WEAPON_HORIZONSCANNER) {
				g_Vars.currentplayer->zoomintimemax = 0;
				g_Vars.currentplayer->zoomintime = g_Vars.currentplayer->zoomintimemax;
				g_Vars.currentplayer->zoominfovynew = 60;
				g_Vars.currentplayer->zoominfovy = g_Vars.currentplayer->zoominfovynew;
			}

			lefthand = &player->hands[HAND_LEFT];
			righthand = &player->hands[HAND_RIGHT];

			if (ctrl->switchtoweaponnum == WEAPON_NONE) {
				lefthand->inuse = false;
				righthand->inuse = false;
				ctrl->weaponnum = WEAPON_NONE;
			} else {
				bgunSetGunMemWeapon(ctrl->switchtoweaponnum);
				ctrl->weaponnum = ctrl->switchtoweaponnum;
				lefthand->inuse = true;
				righthand->inuse = true;
			}

			if (ctrl->weaponnum == WEAPON_REMOTEMINE) {
				ctrl->dualwielding = true;
			}

			if (!ctrl->dualwielding) {
				lefthand->inuse = false;
			}

			if (weaponnum <= WEAPON_PSYCHOSISGUN && weaponnum >= WEAPON_UNARMED) {
				player->gunctrl.prevweaponnum = weaponnum;
			}

			if (previnuse) {
				player->gunctrl.prevwasdualwielding = true;
			} else {
				player->gunctrl.prevwasdualwielding = false;
			}

			g_Vars.currentplayer->gunctrl.invertgunfunc = false;
			g_Vars.currentplayer->usedowntime = -1;

			for (i = 0; i < 2; i++) {
				player->hands[i].ejectstate = EJECTSTATE_INACTIVE;
				player->hands[i].ejecttype = EJECTTYPE_GUN;
				player->hands[i].unk0d0f_02 = false;
				player->hands[i].activatesecondary = false;

				player->hands[i].matmot1 = 0.0f;
				player->hands[i].matmot2 = 0.0f;
				player->hands[i].matmot3 = 0.0f;
				player->hands[i].angledamper = 0.0f;
				player->hands[i].gunsmokepoint = 0.0f;
				player->hands[i].burstbullets = 0;
				player->hands[i].loadslide = 0.0f;
				player->hands[i].allowshootframe = 0;
				player->hands[i].lastshootframe60 = 0;
				player->hands[i].gset.weaponfunc = FUNC_PRIMARY;
				player->hands[i].gset.weaponnum = ctrl->weaponnum;
				player->hands[i].gset.unk0639 = (ctrl->upgradewant >> (i * 4)) & 0xf;
				player->hands[i].gangstarot = 0.0f;

				bgun0f0abd30(i);

				animInit(&player->hands[i].anim);

				if (player->hands[i].audiohandle && sndGetState(player->hands[i].audiohandle) != AL_STOPPED) {
					audioStop(player->hands[i].audiohandle);
				}
			}

			invCalculateCurrentIndex();

			ctrl->switchtoweaponnum = -1;
			ctrl->fnfader = 0;

			if (ctrl->weaponnum == WEAPON_DISGUISE40 || ctrl->weaponnum == WEAPON_DISGUISE41) {
				struct chrdata *chr = player->prop->chr;

				sndStart(var80095200, SFX_DISGUISE_ON, 0, -1, -1, -1, -1, -1);

				g_Vars.currentplayer->disguised = true;

				chr->hidden |= CHRHFLAG_DISGUISED;

				if (g_Vars.stagenum == STAGE_RESCUE) {
					chr->hidden |= CHRHFLAG_UNTARGETABLE;
				}

				invRemoveItemByNum(ctrl->weaponnum);
				bgunCycleBack();
			}

			ctrl->curfnstr = 0;
			ctrl->fnstrtimer = 0;
			ctrl->throwing = false;
		}
	} else {
		if (((player->hands[HAND_LEFT].inuse && !player->gunctrl.dualwielding)
					|| (!player->hands[HAND_LEFT].inuse && player->gunctrl.dualwielding))
				&& bgunCanFreeWeapon(HAND_LEFT)) {
			bgunFreeWeapon(HAND_LEFT);
			player->hands[HAND_LEFT].inuse = player->gunctrl.dualwielding;
		}
	}
}

void bgunEquipWeapon(int weaponnum)
{
	struct player *player = g_Vars.currentplayer;

	if (player->gunctrl.weaponnum == weaponnum && player->gunctrl.switchtoweaponnum == -1) {
		return;
	}

	player->gunctrl.switchtoweaponnum = weaponnum;
	player->gunctrl.wantammo = false;
}

int bgunGetWeaponNum(int handnum)
{
	if (!g_Vars.currentplayer->hands[handnum].inuse) {
		return WEAPON_NONE;
	}

	return g_Vars.currentplayer->gunctrl.weaponnum;
}

int bgunGetWeaponNum2(int handnum)
{
	return bgunGetWeaponNum(handnum);
}

bool bgun0f0a1a10(int weaponnum)
{
	if (weaponHasFlag(weaponnum, WEAPONFLAG_00000400)
			&& (bgunGetAmmoTypeForWeapon(weaponnum, FUNC_PRIMARY) == 0 || bgunGetAmmoQtyForWeapon(weaponnum, FUNC_PRIMARY) > 0)) {
		return true;
	}

	return false;
}

int bgunGetSwitchToWeapon(int handnum)
{
	int weaponnum;

	if (g_Vars.currentplayer->gunctrl.switchtoweaponnum >= 0) {
		weaponnum = g_Vars.currentplayer->gunctrl.switchtoweaponnum;
	} else {
		weaponnum = g_Vars.currentplayer->gunctrl.weaponnum;
	}

	if (!g_Vars.currentplayer->gunctrl.dualwielding && handnum == HAND_LEFT) {
		weaponnum = WEAPON_NONE;
	}

	return weaponnum;
}

void bgunSwitchToPrevious(void)
{
	if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
		struct player *player = g_Vars.currentplayer;
		int dualweaponnum;

		if (invHasSingleWeaponIncAllGuns(player->gunctrl.prevweaponnum)) {
			bgunEquipWeapon2(HAND_RIGHT, player->gunctrl.prevweaponnum);

			dualweaponnum = invHasDoubleWeaponIncAllGuns(player->gunctrl.prevweaponnum, player->gunctrl.prevweaponnum)
				* player->gunctrl.prevweaponnum * player->gunctrl.prevwasdualwielding;
			bgunEquipWeapon2(HAND_LEFT, dualweaponnum);
		} else {
			bgunAutoSwitchWeapon();
		}
	}
}

void bgunCycleForward(void)
{
	int weaponnum1;
	int weaponnum2;
	struct player *player = g_Vars.currentplayer;

	if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
		weaponnum1 = bgunGetSwitchToWeapon(HAND_RIGHT);
		weaponnum2 = bgunGetSwitchToWeapon(HAND_LEFT);

		if (weaponnum1 > WEAPON_PSYCHOSISGUN || weaponnum2 > WEAPON_PSYCHOSISGUN) {
			weaponnum1 = player->gunctrl.prevweaponnum;
			weaponnum2 = player->gunctrl.prevweaponnum * player->gunctrl.prevwasdualwielding;
		} else {
			invChooseCycleForwardWeapon(&weaponnum1, &weaponnum2, false);
		}

		if (weaponnum2 != weaponnum1) {
			player->gunctrl.dualwielding = false;
		} else {
			player->gunctrl.dualwielding = true;
		}

		bgunEquipWeapon(weaponnum1);
	}
}

void bgunCycleBack(void)
{
	int weaponnum1;
	int weaponnum2;
	struct player *player = g_Vars.currentplayer;

	if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
		weaponnum1 = bgunGetSwitchToWeapon(HAND_RIGHT);
		weaponnum2 = bgunGetSwitchToWeapon(HAND_LEFT);

		if (weaponnum2 == WEAPON_REMOTEMINE) {
			weaponnum2 = WEAPON_NONE;
		}

		if (weaponnum1 > WEAPON_PSYCHOSISGUN || weaponnum2 > WEAPON_PSYCHOSISGUN) {
			weaponnum1 = player->gunctrl.prevweaponnum;
			weaponnum2 = player->gunctrl.prevweaponnum * player->gunctrl.prevwasdualwielding;
		} else {
			invChooseCycleBackWeapon(&weaponnum1, &weaponnum2, false);
		}

		if (weaponnum2 == WEAPON_NONE) {
			player->gunctrl.dualwielding = false;
		} else {
			player->gunctrl.dualwielding = true;
		}

		bgunEquipWeapon(weaponnum1);
	}
}

/**
 * Return true if the player has ammo for the given weapon (for either function)
 * or if the weapon doesn't support ammo.
 *
 * Used by the active menu to colour the slots.
 */
bool bgunHasAmmoForWeapon(int weaponnum)
{
	bool ammodefexists = false;
	bool hasammo = false;
	struct weapon *weapon = weaponFindById(weaponnum);
	int i;

	if (weapon == NULL) {
		return true;
	}

	for (i = 0; i < 2; i++) {
		struct weaponfunc *func = weaponGetFunctionById(weaponnum, i);

		if (func && func->ammoindex >= 0) {
			struct inventory_ammo *ammo = weapon->ammos[func->ammoindex];

			if (ammo) {
				ammodefexists = true;

				if (bgunGetAmmoCount(ammo->type) > 0) {
					hasammo = true;
				}
			}
		}
	}

	if (!ammodefexists) {
		return true;
	}

	if (hasammo == true) {
		return true;
	}

	return false;
}

uint8_t g_AutoSwitchWeaponsPrimary[] = {
	WEAPON_RCP120,
	WEAPON_RCP45,
	WEAPON_SUPERDRAGON, // primary function
	WEAPON_K7AVENGER,
	WEAPON_AR34,
	WEAPON_AR53,
	WEAPON_KF7SPECIAL,
	WEAPON_CALLISTO,
	WEAPON_LAPTOPGUN,
	WEAPON_DRAGON,
	WEAPON_CMP150,
	WEAPON_CYCLONE,
	WEAPON_ZZT,
	WEAPON_DMC,
	WEAPON_KL01313,
	WEAPON_FARSIGHT,
	WEAPON_SHOTGUN,
	WEAPON_REAPER,
	WEAPON_DY357LX,
	WEAPON_MAULER,
	WEAPON_DY357MAGNUM,
	WEAPON_MAGSEC4,
	WEAPON_PHOENIX,
	WEAPON_FALCON2_SCOPE,
	WEAPON_FALCON2,
	WEAPON_FALCON2_SILENCER,
	//WEAPON_FALCON2_SANDS,
	WEAPON_PP9I,
	WEAPON_CC13,
	WEAPON_SNIPERRIFLE,
	WEAPON_CROSSBOW,
	WEAPON_TRANQUILIZER,
	WEAPON_LASER,
	WEAPON_SUPERDRAGON, // secondary function
	WEAPON_DEVASTATOR,
	WEAPON_ROCKETLAUNCHER,
	WEAPON_SLAYER,
	WEAPON_GRENADE,
	WEAPON_NBOMB,
	WEAPON_PROXIMITYMINE,
	WEAPON_TIMEDMINE,
	WEAPON_REMOTEMINE,
	WEAPON_COMBATKNIFE,
	WEAPON_UNARMED,
};

uint8_t g_AutoSwitchWeaponsSecondary[] = {
	WEAPON_REAPER,
	WEAPON_DY357LX,
	WEAPON_DY357MAGNUM,
	WEAPON_FALCON2_SCOPE,
	WEAPON_FALCON2,
	WEAPON_FALCON2_SILENCER,
	//WEAPON_FALCON2_SANDS,
	WEAPON_UNARMED,
};

/**
 * Automatically choose and equip a new weapon after trying to fire a weapon
 * which is out of ammo.
 *
 * The weapon preference order is stored in two arrays; one for weapons which
 * should have their primary functions considered and which require ammo, and
 * another for weapons which should have their secondary functions considered
 * and don't require ammo for those functions. The second is only used if no
 * weapons are usable from the primary array.
 *
 * For the primary array, the weapon must not be out of ammo. If the player's
 * current weapon is not in the primary array then the first available primary
 * will be selected. If the player's current weapon is in the primary array then
 * the last available weapon earlier than their current weapon will be selected.
 * If there are no weapons earlier than their current weapon then the first
 * weapon after their current weapon is selected.
 *
 * In the primary array, the SuperDragon is a special case and appears twice.
 * The first use is for the primary function while the second use is for the
 * secondary function.
 *
 * For the secondary array, the player's current weapon must not be in the
 * array. The first available weapon is selected. The player's "wantammo" flag
 * will be set which will force the weapon onto the second function.
 */
void bgunAutoSwitchWeapon(void)
{
	int i;
	struct weapon *weapon;
	struct weaponfunc *func;
	int weaponnum;
	int newweaponnum = -1;
	int firstweaponnum = -1;
	int foundsuperdragon = 0;
	bool foundcurrent = false;
	int curweaponnum = g_Vars.currentplayer->gunctrl.weaponnum;
	bool wantammo = false;

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		return;
	}

	// Loop through g_AutoSwitchWeaponsPrimary, checking which weapons the
	// player has which are usable. Stop when both a usable weapon is found
	// and when the player's current weapon is found. Note the first and last
	// usable weapons.
	i = 0;

	do {
		bool usable = false;

		if (invHasSingleWeaponIncAllGuns(g_AutoSwitchWeaponsPrimary[i])) {
			weaponnum = g_AutoSwitchWeaponsPrimary[i];
			weapon = weaponFindById(weaponnum);
			func = weaponGetFunctionById(weaponnum, FUNC_PRIMARY);

			if (!bgun0f0990b0(func, weapon) && (func->flags & FUNCFLAG_AUTOSWITCHUNSELECTABLE) == 0) {
				usable = true;
			}

			if (weaponnum == WEAPON_SUPERDRAGON && !foundsuperdragon) {
				foundsuperdragon++;
			} else {
				func = weaponGetFunctionById(weaponnum, FUNC_SECONDARY);

				if (!bgun0f0990b0(func, weapon) && (func->flags & FUNCFLAG_AUTOSWITCHUNSELECTABLE) == 0) {
					usable = true;
				}
			}

			if (weaponnum == curweaponnum) {
				foundcurrent = true;
			} else if (usable) {
				newweaponnum = weaponnum;

				if (firstweaponnum == -1) {
					firstweaponnum = weaponnum;
				}
			}
		}

		if (++i >= ARRAYCOUNT(g_AutoSwitchWeaponsPrimary)) {
			break;
		}
	} while (newweaponnum == -1 || !foundcurrent);

	if (!foundcurrent) {
		newweaponnum = firstweaponnum;
	}

	if (newweaponnum == -1) {
		newweaponnum = WEAPON_UNARMED;
	}

	if (newweaponnum == WEAPON_UNARMED) {
		bool foundcurrent = false;
		int firstweaponnum = -1;
		int weaponnum;

		// No usable weapon was found in the primary array,
		// so search the secondary array.
		for (i = 0; i < ARRAYCOUNT(g_AutoSwitchWeaponsSecondary); i++) {
			weaponnum = g_AutoSwitchWeaponsSecondary[i];

			if (invHasSingleWeaponIncAllGuns(weaponnum)) {
				if (weaponnum == curweaponnum) {
					foundcurrent = true;
				}

				if (firstweaponnum == -1) {
					firstweaponnum = weaponnum;
				}
			}
		}

		newweaponnum = firstweaponnum;

		if (newweaponnum == -1) {
			newweaponnum = WEAPON_UNARMED;
		}

		if (foundcurrent) {
			newweaponnum = -1;
		}

		wantammo = true;
	}

	// Switch to newweaponnum
	if (newweaponnum >= 0 && newweaponnum != curweaponnum) {
		if (invHasDoubleWeaponIncAllGuns(newweaponnum, newweaponnum)) {
			g_Vars.currentplayer->gunctrl.dualwielding = true;
		} else {
			g_Vars.currentplayer->gunctrl.dualwielding = false;
		}

		bgunEquipWeapon(newweaponnum);

		if (wantammo) {
			g_Vars.currentplayer->gunctrl.wantammo = true;
		}
	}
}

void bgunEquipWeapon2(int handnum, int weaponnum)
{
	if (handnum == HAND_LEFT) {
		if (weaponnum == WEAPON_NONE) {
			g_Vars.currentplayer->gunctrl.dualwielding = false;
		} else {
			g_Vars.currentplayer->gunctrl.dualwielding = true;
		}
	} else {
		if (weaponnum > WEAPON_SUICIDEPILL) {
			weaponnum = WEAPON_UNARMED;
		}

		bgunEquipWeapon(weaponnum);
	}
}

int bgunIsFiring(int handnum)
{
	return g_Vars.currentplayer->hands[handnum].firing;
}

int bgunGetAttackType(int handnum)
{
	return g_Vars.currentplayer->hands[handnum].attacktype;
}

char *bgunGetName(int weaponnum)
{
	struct weapon *weapon = g_Weapons[weaponnum];

	if (weapon) {
		return langGet(weapon->name);
	}

	return "** error\n";
}

uint16_t bgunGetNameId(int weaponnum)
{
	struct weapon *weapon = g_Weapons[weaponnum];

	if (weapon) {
		return weapon->name;
	}

	return 0;
}

char *bgunGetShortName(int weaponnum)
{
	struct weapon *weapon = g_Weapons[weaponnum];

	if (weapon) {
		return langGet(weapon->shortname);
	}

	return "** error\n";
}

void bgunReloadIfPossible(int handnum)
{
	struct player *player = g_Vars.currentplayer;

	if (bgunGetAmmoTypeForWeapon(bgunGetWeaponNum(handnum), FUNC_PRIMARY)
			&& player->hands[handnum].modenext == HANDMODE_NONE) {
		player->hands[handnum].modenext = HANDMODE_RELOAD;
	}
}

void bgunSetAdjustPos(float angle)
{
	struct player *player = g_Vars.currentplayer;

	player->hands[0].adjustpos.z = (1 - cosf(angle)) * 5;
	player->hands[1].adjustpos.z = (1 - cosf(angle)) * 5;
}

void bgunStartSlide(int handnum)
{
	g_Vars.currentplayer->hands[handnum].slideinc = true;
}

/**
 * Update the slide on weapons which have them (eg. Falcon 2).
 *
 * The slide moves back and then forward when firing. If the gun no longer has
 * any ammo loaded in it, the slide moves back and remains in the back position.
 */
void bgunUpdateSlide(int handnum)
{
	float slidemax = 0.0f;
	struct weaponfunc *funcdef = currentPlayerGetWeaponFunction(handnum);
	struct player *player = g_Vars.currentplayer;

	if (funcdef && ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT)) {
		struct weaponfunc_shoot *shootfunc = (struct weaponfunc_shoot *)funcdef;
		slidemax = shootfunc->slidemax;
	}

	if (player->hands[handnum].slideinc) {
		// Slide is moving backwards
		if (player->hands[handnum].slidetrans < slidemax) {
			player->hands[handnum].slidetrans += slidemax * 0.25f * g_Vars.lvupdate60freal;
		}

		if (player->hands[handnum].slidetrans >= slidemax) {
			player->hands[handnum].slidetrans = slidemax;
			player->hands[handnum].slideinc = false;
		}
	} else if (player->hands[handnum].loadedammo[FUNC_PRIMARY] > 0) {
		if (bgun0f098a44(&player->hands[handnum], 3)) {
			// Slide is moving forwards
			if (player->hands[handnum].slidetrans > 0.0f) {
				player->hands[handnum].slidetrans -= slidemax * 0.16666667f * g_Vars.lvupdate60freal;
			}

			if (player->hands[handnum].slidetrans < 0.0f) {
				player->hands[handnum].slidetrans = 0.0f;
			}
		}
	}
}

float bgun0f0a2498(float arg0, float arg1, float arg2, float arg3)
{
	float a = arg0 - arg2;

	return asinf(a / sqrtf(a * a + (arg1 - arg3) * (arg1 - arg3)));
}

void bgun0f0a24f0(struct coord *arg0, int handnum)
{
	struct coord b;
	struct coord a;

	bgun0f0a0c44(handnum, &a, &b);

	b.x *= 1000;
	b.y *= 1000;
	b.z *= 1000;

	arg0->x = b.x;
	arg0->y = b.y;
	arg0->z = b.z;
}

/**
 * This function is a callback that is passed to model code.
 */
void bgun0f0a256c(int mtxindex, Mtxf *mtx)
{
	Mtxf sp78;
	Mtxf sp38;
	struct coord rot;

	if (mtxindex == var8009d148) {
		if (var8009d144->ejectstate == EJECTSTATE_INIT) {
			var8009d144->unk0d14 = mtx->m[3][0];
			var8009d144->unk0d18 = mtx->m[3][1];
			var8009d144->unk0d1c = mtx->m[3][2];

			var8009d144->unk0d2c[0][0] = mtx->m[0][0];
			var8009d144->unk0d2c[0][1] = mtx->m[0][1];
			var8009d144->unk0d2c[0][2] = mtx->m[0][2];
			var8009d144->unk0d2c[1][0] = mtx->m[1][0];
			var8009d144->unk0d2c[1][1] = mtx->m[1][1];
			var8009d144->unk0d2c[1][2] = mtx->m[1][2];
			var8009d144->unk0d2c[2][0] = mtx->m[2][0];
			var8009d144->unk0d2c[2][1] = mtx->m[2][1];
			var8009d144->unk0d2c[2][2] = mtx->m[2][2];
		} else if (var8009d144->ejectstate >= EJECTSTATE_AIRBORNE) {
			mtx->m[3][0] = var8009d144->unk0d14;
			mtx->m[3][1] = var8009d144->unk0d18;
			mtx->m[3][2] = var8009d144->unk0d1c;

			mtx->m[0][0] = var8009d144->unk0d2c[0][0];
			mtx->m[0][1] = var8009d144->unk0d2c[0][1];
			mtx->m[0][2] = var8009d144->unk0d2c[0][2];
			mtx->m[1][0] = var8009d144->unk0d2c[1][0];
			mtx->m[1][1] = var8009d144->unk0d2c[1][1];
			mtx->m[1][2] = var8009d144->unk0d2c[1][2];
			mtx->m[2][0] = var8009d144->unk0d2c[2][0];
			mtx->m[2][1] = var8009d144->unk0d2c[2][1];
			mtx->m[2][2] = var8009d144->unk0d2c[2][2];
		}
	}

	if (mtxindex == var8009d0dc) {
		rot.x = 0.0f;
		rot.y = 0.0f;
		rot.z = var8009d140;

		mtx4LoadIdentity(&sp78);
		mtx4LoadRotation(&rot, &sp78);
		mtx4MultMtx4(mtx, &sp78, &sp38);
		mtx4Copy(&sp38, mtx);
	}

	if (mtxindex == var8009d0f0[0] || mtxindex == var8009d0f0[1] || mtxindex == var8009d0f0[2]) {
		rot.x = 0.0f;
		rot.y = 0.0f;
		rot.z = 2.0f * -var8009d140;

		mtx4LoadIdentity(&sp78);
		mtx4LoadRotation(&rot, &sp78);
		mtx4MultMtx4(mtx, &sp78, &sp38);
		mtx4Copy(&sp38, mtx);
	}
}

bool bgun0f0a27c8(void)
{
	struct hand *hand;
	struct weaponfunc *func;

	hand = &g_Vars.currentplayer->hands[HAND_RIGHT];
	func = gsetGetWeaponFunction2(&hand->gset);

	if (func
			&& (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE
			&& hand->state == HANDSTATE_ATTACK
			&& hand->unk0ce8 != NULL
			&& hand->animmode == HANDANIMMODE_BUSY
			&& !bgun0f098a44(hand, 2)) {
		return true;
	}

	hand = &g_Vars.currentplayer->hands[HAND_LEFT];

	if (hand->inuse) {
		func = gsetGetWeaponFunction2(&hand->gset);

		if (func
				&& (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE
				&& hand->state == HANDSTATE_ATTACK
				&& hand->unk0ce8 != NULL
				&& hand->animmode == HANDANIMMODE_BUSY
				&& !bgun0f098a44(hand, 2)) {
			return true;
		}
	}

	return false;
}

void bgunHandlePlayerDead(void)
{
	struct player *player = g_Vars.currentplayer;
	int i;

	if (player->gunctrl.weaponnum != WEAPON_NONE && player->gunctrl.switchtoweaponnum != WEAPON_NONE) {
		// Eject held weapons
		if (player->hands[HAND_LEFT].inuse) {
			player->hands[HAND_LEFT].ejectstate = EJECTSTATE_INIT;
			player->hands[HAND_LEFT].ejecttype = EJECTTYPE_GUN;
		}

		if (player->hands[HAND_RIGHT].inuse) {
			player->hands[HAND_RIGHT].ejectstate = EJECTSTATE_INIT;
			player->hands[HAND_RIGHT].ejecttype = EJECTTYPE_GUN;
		}

		for (i = 0; i < 2; i++) {
			player->hands[i].matmot1 = 0;
			player->hands[i].matmot2 = 0;
			player->hands[i].matmot3 = 0;

			bgunSetState(i, HANDSTATE_IDLE);
		}

		bgunEquipWeapon2(HAND_LEFT, WEAPON_NONE);
		bgunEquipWeapon2(HAND_RIGHT, WEAPON_NONE);
	}
}

bool bgunIsMissionCritical(int weaponnum)
{
	if (weaponnum == WEAPON_TIMEDMINE
			|| weaponnum == WEAPON_REMOTEMINE
			|| weaponnum == WEAPON_ECMMINE
			|| weaponnum == WEAPON_TRACERBUG) {
		return true;
	}

	return false;
}

// This is for the player being disarmed
void bgunDisarm(struct prop *attackerprop)
{
	struct player *player = g_Vars.currentplayer;
	int weaponnum = player->hands[0].gset.weaponnum;
	struct chrdata *chr;
	int modelnum;
	int i;
	bool drop;

	if (!weaponHasFlag(weaponnum, WEAPONFLAG_UNDROPPABLE) && weaponnum <= WEAPON_RCP45) {
		// Coop must not allow player to drop a mission critical weapon
		// because AI lists can fail the mission if the player has zero
		// quantity.
		if (g_Vars.coopplayernum >= 0
				&& (attackerprop == g_Vars.bond->prop || attackerprop == g_Vars.coop->prop)
				&& bgunIsMissionCritical(weaponnum)) {
			return;
		}

		if (weaponnum <= WEAPON_UNARMED || player->gunctrl.switchtoweaponnum != -1) {
			return;
		}

		chr = player->prop->chr;
		drop = true;

		// RC-P120 and cloaking device: turn off cloak if active
		if (weaponnum == WEAPON_RCP120) {
			g_Vars.currentplayer->devicesactive &= ~DEVICE_CLOAKRCP120;
		}

		if (weaponnum == WEAPON_CLOAKINGDEVICE) {
			g_Vars.currentplayer->devicesactive &= ~DEVICE_CLOAKDEVICE;
		}

		// Grenade and nbomb: if pin is pulled, throw it?
		// Or drop it at player's feet with the pin pulled maybe...
		if (weaponnum == WEAPON_GRENADE || weaponnum == WEAPON_NBOMB) {
			for (i = 0; i < 2; i++) {
				struct weaponfunc *func = gsetGetWeaponFunction(&player->hands[i].gset);

#ifdef AVOID_UB
				if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_THROW
						&& player->hands[i].state == HANDSTATE_ATTACK
						&& player->hands[i].stateminor == HANDSTATEMINOR_ATTACK_THROW_0) {
#else
				if ((func->type & 0xff) == INVENTORYFUNCTYPE_THROW
						&& player->hands[i].state == HANDSTATE_ATTACK
						&& player->hands[i].stateminor == HANDSTATEMINOR_ATTACK_THROW_0) {
#endif
					drop = false;
					bgunCreateThrownProjectile(i + 2, &player->hands[i].gset);
				}
			}
		}

		weaponDeleteFromChr(chr, HAND_RIGHT);
		weaponDeleteFromChr(chr, HAND_LEFT);

		// Actually drop the weapon
		modelnum = playermgrGetModelOfWeapon(weaponnum);

		if (modelnum >= 0 && drop) {
			struct prop *prop2 = weaponCreateForChr(chr, modelnum, weaponnum, OBJFLAG_WEAPON_AICANNOTUSE, NULL, NULL);

			if (prop2 && prop2->obj) {
				struct defaultobj *obj = prop2->obj;
				objSetDropped(prop2, DROPTYPE_DEFAULT);

				if (obj->hidden & OBJHFLAG_PROJECTILE) {
					obj->projectile->pickuptimer240 = TICKS(240);
					obj->projectile->pickupby = attackerprop;
				}

				objDrop(prop2, true);
			}
		}

		invRemoveItemByNum(weaponnum);

		player->hands[1].state = HANDSTATE_IDLE;
		player->hands[1].ejectstate = EJECTSTATE_INIT;
		player->hands[1].ejecttype = EJECTTYPE_GUN;
		player->hands[0].ejectstate = EJECTSTATE_INIT;
		player->hands[0].ejecttype = EJECTTYPE_GUN;
		player->hands[0].state = HANDSTATE_IDLE;

		// Exit slayer rocket mode if player was using it
		if (player->visionmode == VISIONMODE_SLAYERROCKET) {
			struct weaponobj *rocket = g_Vars.currentplayer->slayerrocket;

			if (rocket && rocket->base.prop) {
				rocket->timer240 = 0;
			}

			player->visionmode = VISIONMODE_NORMAL;
		}

		bgunEquipWeapon2(HAND_RIGHT, WEAPON_UNARMED);
		bgunEquipWeapon2(HAND_LEFT, WEAPON_NONE);
	}
}

/**
 * Execute some sort of command list that was generated by the function below.
 *
 * With this function stubbed, part of the CMP150 model does not render.
 */
void bgunExecuteModelCmdList(uintptr_t *ptr)
{
	union modelrwdata *rwdata;
	struct modelnode *node;

	if (ptr != NULL) {
		while (*ptr != 6) {
			switch (*ptr) {
			case 0:
				rwdata = (union modelrwdata *)ptr[1];
				node = (struct modelnode *)ptr[2];
				rwdata->distance.visible = false;
				node->child = (struct modelnode *)ptr[3];
				ptr += 4;
				break;
			case 1:
				rwdata = (union modelrwdata *)ptr[1];
				node = (struct modelnode *)ptr[2];
				rwdata->toggle.visible = true;
				node->child = (struct modelnode *)ptr[3];
				ptr += 4;
				break;
			case 2:
				rwdata = (union modelrwdata *)ptr[1];
				rwdata->headspot.headmodeldef = NULL;
				rwdata->headspot.rwdatas = NULL;
				ptr += 2;
				break;
			case 3:
				rwdata = (union modelrwdata *)ptr[1];
				rwdata->type0b.unk00 = 0;
				ptr += 2;
				break;
			case 4:
				rwdata = (union modelrwdata *)ptr[1];
				rwdata->chrgunfire.visible = false;
				ptr += 2;
				break;
			case 5:
				rwdata = (union modelrwdata *)ptr[1];
				rwdata->dl.vertices = (Vtx *)ptr[2];
				rwdata->dl.gdl = (Gfx *)ptr[3];
				rwdata->dl.colours = (Col *)ptr[4];
				ptr += 5;
				break;
			}
		}
	}
}

/**
 * Generate some sort of command list to be executed by the function above.
 *
 * This appears to be a performance optimisation, so the tick code can quickly
 * iterate the command list to update part visibility rather than iterate the
 * full model tree.
 */
int bgunCreateModelCmdList(struct model *model, struct modelnode *nodearg, uintptr_t *ptr)
{
	int len = 0;
	struct modelnode *node = nodearg;
	union modelrodata *rodata;
	union modelrwdata *rwdata;

	while (node) {
		uint32_t type = node->type;

		switch ((uint8_t)type) {
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->distance.visible = false;
			node->child = rodata->distance.target;
			ptr[0] = 0;
			ptr[1] = (uintptr_t)rwdata;
			ptr[2] = (uintptr_t)node;
			ptr[3] = (uintptr_t)rodata->distance.target;
			ptr += 4;
			len += 4 * sizeof(uintptr_t);
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->toggle.visible = true;
			node->child = rodata->toggle.target;
			ptr[0] = 1;
			ptr[1] = (uintptr_t)rwdata;
			ptr[2] = (uintptr_t)node;
			ptr[3] = (uintptr_t)rodata->toggle.target;
			ptr += 4;
			len += 4 * sizeof(uintptr_t);
			break;
		case MODELNODETYPE_HEADSPOT:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->headspot.headmodeldef = NULL;
			rwdata->headspot.rwdatas = NULL;
			ptr[0] = 2;
			ptr[1] = (uintptr_t)rwdata;
			ptr += 2;
			len += 2 * sizeof(uintptr_t);
			break;
		case MODELNODETYPE_0B:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->type0b.unk00 = 0;
			ptr[0] = 3;
			ptr[1] = (uintptr_t)rwdata;
			ptr += 2;
			len += 2 * sizeof(uintptr_t);
			break;
		case MODELNODETYPE_CHRGUNFIRE:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->chrgunfire.visible = false;
			ptr[0] = 4;
			ptr[1] = (uintptr_t)rwdata;
			ptr += 2;
			len += 2 * sizeof(uintptr_t);
			break;
		case MODELNODETYPE_DL:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->dl.vertices = rodata->dl.vertices;
			rwdata->dl.gdl = rodata->dl.opagdl;
			rwdata->dl.colours = (void *)ALIGN8((uintptr_t)&rodata->dl.vertices[rodata->dl.numvertices]);
			ptr[0] = 5;
			ptr[1] = (uintptr_t)rwdata;
			ptr[2] = (uintptr_t)rwdata->dl.vertices;
			ptr[3] = (uintptr_t)rwdata->dl.gdl;
			ptr[4] = (uintptr_t)rwdata->dl.colours;
			ptr += 5;
			len += 5 * sizeof(uintptr_t);
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == nodearg->parent) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	*ptr = 6;
	len += sizeof(uintptr_t);

	return len;
}

struct guncmd var80070200[2] = {
	{ GUNCMD_PLAYANIMATION, 0, ANIM_0434, 10000 },
	{ GUNCMD_END },
};

void bgunStartDetonateAnimation(int playernum)
{
	int prevplayernum = g_Vars.currentplayernum;
	setCurrentPlayerNum(playernum);

	if (g_Vars.currentplayer->hands[HAND_LEFT].gset.weaponnum == WEAPON_REMOTEMINE) {
		bgunStartAnimation(var80070200, 1, &g_Vars.currentplayer->hands[HAND_LEFT]);
	}

	setCurrentPlayerNum(prevplayernum);
}

/**
 * Update the gangsta-style rotation of the player's gun.
 *
 * When close to an enemy and aiming at them with a pistol, the gun is rotated
 * sideways. The enemy and aiming check is done elsewhere (autoaim code) and
 * sets the gunctrl's gangsta property to true or false based on whether this
 * criteria is met on the current (or previous?) frame.
 *
 * bgunUpdateGangsta uses this property and increments the rotation of the gun
 * accordingly. It also checks that the gun is in a state that allows gangsta
 * rotation (reloading and equip/unequip do not). It also implements a delay on
 * reverting to the normal rotation.
 */

// Ben's comment: in this version, it also checks if the player wants their gun to go into gangsta mode.
void bgunUpdateGangsta(struct hand *hand, int handnum, struct coord *viewmodelpos, struct weaponfunc *funcdef, Mtxf *arg4, Mtxf *arg5)
{
	float tmp;
	struct coord sp38 = {0, 0, 0};

	if ((g_Vars.currentplayer->gunctrl.gangsta || g_Vars.currentplayer->wantsgangsta) // Change it so gangsta happens either within min range OR when the player wants it
			&& funcdef
			&& (funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT
			&& (hand->state == HANDSTATE_IDLE
				|| hand->state == HANDSTATE_2
				|| hand->state == HANDSTATE_ATTACKEMPTY
				|| hand->state == HANDSTATE_ATTACK)) {
		if (hand->gangstarot < 1.0f) {
			// Rotate into gangsta position
			hand->ispare1 += g_Vars.lvupdate240;

			//if (hand->ispare1 > TICKS(60)) {
				hand->gangstarot += LVUPDATE60FREAL() / 30.0f;

				if (hand->gangstarot > 1.0f) {
					hand->gangstarot = 1.0f;
				}
			//}
		} else {
			// Already in gangsta position
			hand->ispare1 = 0;
		}
	} else {
		// At this point we don't want the gun to be in the gangsta position.
		// However we don't want it to revert immediately, so a timer is used.
		//float inversespeed = 30.0f;
		float inversespeed = 20.0f;

		if (hand->animmode == HANDANIMMODE_BUSY) {
			// Revert faster
			//inversespeed = 15.0f;
			inversespeed = 10.0f;
		}

		if (hand->gangstarot > 0.0f) {
			bool revert = false;

			hand->ispare1 += g_Vars.lvupdate240;

			if (hand->gangstarot < 1.0f) {
				hand->ispare1 = TICKS(244);
			}

			// Remove the delay on going out of gangsta
			//if (hand->ispare1 > TICKS(120)) {
				revert = true;
			//}

			if (hand->animmode == HANDANIMMODE_BUSY && funcdef && (funcdef->type & 0xff) != INVENTORYFUNCTYPE_SHOOT) {
				revert = true;
			}

			if (hand->state != HANDSTATE_IDLE
					&& hand->state != HANDSTATE_2
					&& hand->state != HANDSTATE_ATTACKEMPTY
					&& hand->state != HANDSTATE_ATTACK) {
				revert = true;
			}

			if (revert) {
				hand->gangstarot -= LVUPDATE60FREAL() / inversespeed;
			}

			if (hand->gangstarot < 0.0f) {
				hand->gangstarot = 0.0f;
			}
		} else {
			// Not rotated
			hand->ispare1 = 0;
		}
	}

	tmp = -cosf(hand->gangstarot * M_PI) * 0.5f + 0.50f;
	sp38.z = (tmp * 66.6f * 0.017453292f) * (handnum != HAND_RIGHT ? 1.0f : -1.0f);

	mtx4LoadRotation(&sp38, arg4);
	mtx00015be0(arg4, arg5);

	viewmodelpos->y += 4.0f * hand->gangstarot;
	viewmodelpos->x += 2.0f * hand->gangstarot * (handnum != HAND_RIGHT ? 1.0f : -1.0f);
}

/**
 * Check if smoke needs to be created at the muzzle of the current weapon.
 *
 * gunsmokepoint is basically the temperature of the gun. It increases when
 * firing and cools down when idle. It's only used for pistols; automatics will
 * create smoke based on the number of shots in the current burst.
 *
 * createsmoke must be set to create any smoke at all.
 *
 * forcecreatesmoke controls whether smoke should be created while the gun is
 * still firing.
 */
void bgunUpdateSmoke(struct hand *hand, int handnum, int weaponnum, struct weaponfunc *funcdef)
{
	if (hand->firing) {
		if (weaponnum == WEAPON_DY357MAGNUM || weaponnum == WEAPON_DY357LX) {
			if ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
				hand->gunsmokepoint += 0.6f;
			}
		} else {
			hand->gunsmokepoint += 0.2f;
		}
	}

	hand->gunsmokepoint -= LVUPDATE60FREAL() / 120.0f;

	if (hand->gunsmokepoint < 0.0f) {
		hand->gunsmokepoint = 0.0f;
	}

	if (funcdef && (funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		float mult = 1.0f;

		if (g_Vars.currentplayer->hands[HAND_LEFT].inuse) {
			mult = 1.5f;
		}

		hand->forcecreatesmoke = false;

		switch (weaponnum) {
		case WEAPON_FALCON2:
		case WEAPON_FALCON2_SCOPE:
			if (hand->gunsmokepoint * mult > 0.66f) {
				hand->createsmoke = true;
			}
			break;
		case WEAPON_MAGSEC4:
		case WEAPON_MAULER:
			if (hand->gunsmokepoint * mult > 0.75f) {
				hand->createsmoke = true;
			}
			break;
		case WEAPON_DY357MAGNUM:
		case WEAPON_DY357LX:
			if (hand->gunsmokepoint * mult > 0.9f) {
				hand->createsmoke = true;
			}
			break;
		case WEAPON_CMP150:
		case WEAPON_DRAGON:
		case WEAPON_K7AVENGER:
		case WEAPON_AR34:
		case WEAPON_SUPERDRAGON:
			hand->forcecreatesmoke = true;

			if (hand->burstbullets > 14) {
				hand->createsmoke = true;
			}
			break;
		case WEAPON_CYCLONE:
		case WEAPON_LAPTOPGUN:
			if (hand->burstbullets > 20) {
				hand->createsmoke = true;
			}

			hand->forcecreatesmoke = true;
			break;
		case WEAPON_RCP120:
			hand->forcecreatesmoke = true;

			if (hand->burstbullets > 25) {
				hand->createsmoke = true;
			}
			break;
		case WEAPON_REAPER:
			hand->forcecreatesmoke = true;
			// fall-through
		case WEAPON_SHOTGUN:
			if (hand->firing) {
				hand->createsmoke = true;
			}
			break;
		}
	}

	if (hand->createsmoke && (hand->state != HANDSTATE_ATTACK || hand->forcecreatesmoke)) {
		struct coord smokepos;
		RoomNum smokerooms[2];
		int smoketype = SMOKETYPE_MUZZLE_AUTOMATIC;

		switch (weaponnum) {
		case WEAPON_FALCON2:
		case WEAPON_FALCON2_SCOPE:
		case WEAPON_MAGSEC4:
		case WEAPON_MAULER:
		case WEAPON_DY357MAGNUM:
		case WEAPON_DY357LX:
			smoketype = SMOKETYPE_MUZZLE_PISTOL;
			break;
		case WEAPON_REAPER:
			smoketype = SMOKETYPE_MUZZLE_REAPER;
			break;
		case WEAPON_SHOTGUN:
			smoketype = SMOKETYPE_MUZZLE_SHOTGUN;
			break;
		}

		smokerooms[0] = g_Vars.currentplayer->cam_room;
		smokerooms[1] = -1;

		smokepos.x = hand->muzzlepos.x;
		smokepos.y = hand->muzzlepos.y;
		smokepos.z = hand->muzzlepos.z;

		hand->gunsmokepoint = 0.0f;

		if (smokeCreateForHand(&smokepos, smokerooms, smoketype, handnum)) {
			hand->createsmoke = false;
		}
	}
}

/**
 * Update the red beam and dot (used by the Falcon 2 and its variants).
 */
void bgunUpdateLasersight(struct hand *hand, struct modeldef *modeldef, int handnum, uint8_t *allocation)
{
	struct modelnode *node;
	struct coord beamfar;
	struct coord dotpos;
	struct coord dotrot;
	struct coord beamnear;
	int mtxindex;
	struct coord sp54;
	struct coord sp48;
	struct coord sp3c;
	struct coord sp30;
	bool busy;

	node = modelGetPart(modeldef, MODELPART_GUN_LASERSIGHT);

	if (node) {
		mtxindex = modelFindNodeMtxIndex(node, 0);

		beamnear.x = ((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][0];
		beamnear.y = ((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][1];
		beamnear.z = ((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][2];

		mtx4TransformVecInPlace(camGetProjectionMtxF(), &beamnear);

		if (hand->useposrot
				|| (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_XRAYSCANNER)) {
			beamfar.x = 0.0f;
			beamfar.y = 0.0f;
			beamfar.z = 1.0f;

			mtx4RotateVecInPlace(&hand->cammtx, &beamfar);

			sp48.x = beamfar.x;
			sp48.y = beamfar.y;
			sp48.z = beamfar.z;

			sp3c.x = beamnear.x;
			sp3c.y = beamnear.y;
			sp3c.z = beamnear.z;

			mtx4TransformVec(camGetWorldToScreenMtxf(), &sp3c, &sp54);
			mtx4RotateVec(camGetProjectionMtxF(), &sp48, &sp30);

			beamfar.x *= 500.0f;
			beamfar.y *= 500.0f;
			beamfar.z *= 500.0f;

			mtx4RotateVecInPlace(camGetProjectionMtxF(), &beamfar);

			beamfar.x += beamnear.x;
			beamfar.y += beamnear.y;
			beamfar.z += beamnear.z;

			lasersightSetBeam(handnum, 1, &beamnear, &beamfar);
			return;
		}

		busy = false;

		if (hand->animmode == HANDANIMMODE_BUSY || hand->turnuprot > 0.0f) {
			busy = true;
		}

		if (busy) {
			mtxindex = modelFindNodeMtxIndex(node, 0);

			beamfar.x = 0.0f;
			beamfar.y = 0.0f;
			beamfar.z = 500.0f;

			mtx4TransformVecInPlace((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)), &beamfar);
		} else {
			cam0f0b4c3c(g_Vars.currentplayer->crosspos, &beamfar, 1);

			beamfar.x *= 500.0f;
			beamfar.y *= 500.0f;
			beamfar.z *= 500.0f;
		}

		mtx4TransformVecInPlace(camGetProjectionMtxF(), &beamfar);
		lasersightSetBeam(handnum, 1, &beamnear, &beamfar);

		if (handnum == HAND_RIGHT && hand->hasdotinfo && !busy) {
			dotpos.x = hand->dotpos.x;
			dotpos.y = hand->dotpos.y;
			dotpos.z = hand->dotpos.z;

			dotrot.x = hand->dotrot.x;
			dotrot.y = hand->dotrot.y;
			dotrot.z = hand->dotrot.z;

			lasersightSetDot(handnum, &dotpos, &dotrot);
		}
	}
}

/**
 * Increment the main barrel spinning, play sounds and (probably) fire shots.
 */
void bgunUpdateReaper(struct hand *hand, struct modeldef *modeldef)
{
	struct modelnode *node;
	float f2;
	float f12;
	int tmp;

	node = modelGetPart(modeldef, MODELPART_REAPER_002C);

	if (hand->matmot3 <= hand->matmot2) {
		if (hand->matmot2 < 0.0f) {
			hand->matmot2 += 0.01f * LVUPDATE60FREAL();

			if (hand->matmot2 > 0.0f) {
				hand->matmot2 = 0.0f;
			}
		}

		hand->matmot3 = hand->matmot2;
	} else {
		f12 = LVUPDATE60FREAL() * 0.005;

		if (hand->matmot2 < 0.0000001f) {
			hand->matmot2 = -0.14f;

			if (hand->matmot3 < 0.15f) {
				f12 *= 4.0f;
			}
		}

		f2 = hand->matmot3 - hand->matmot2;

		if (f12 < f2) {
			f2 = f12;
		}

		hand->matmot3 -= f2;
	}

	if (hand->matmot3 < 0.0f) {
		hand->matmot1 = hand->matmot1 - (1.0f - cosf(hand->matmot3 * M_PI)) * 0.5f * LVUPDATE60FREAL() * 0.2f;
	} else {
		hand->matmot1 = hand->matmot1 + (1.0f - cosf(hand->matmot3 * M_PI)) * 0.5f * LVUPDATE60FREAL() * 0.2f;
	}

	tmp = hand->matmot1 / 6.2831802368164f;
	hand->matmot1 -= tmp * 6.2831802368164f;
	var8009d140 = hand->matmot1;

	if (hand->audiohandle == NULL && hand->matmot3 > 0.1f && g_Vars.lvupdate240 != 0) {
		sndStart(var80095200, SFX_805E, &hand->audiohandle, -1, -1, -1.0f, -1, -1);
	}

	if (hand->audiohandle != NULL) {
		float sp34 = hand->matmot3 / 0.50f + 0.4f;
		int volume = AL_VOL_FULL;

		if (hand->matmot3 < 0.1f) {
			audioStop(hand->audiohandle);
		} else {
			if (hand->matmot3 < 0.6f) {
				volume = (hand->matmot3 - 0.1f) * AL_VOL_FULL / 0.5f;
			}

			audioPostEvent(hand->audiohandle, AL_SNDP_VOL_EVT, volume);
			audioPostEvent(hand->audiohandle, AL_SNDP_PITCH_EVT, *(int *)&sp34);
		}
	}

	if (node) {
		var8009d0dc = modelFindNodeMtxIndex(node, 0);
		g_ModelJointPositionedFunc = bgun0f0a256c;
		var8009d0f0[0] = var8009d0f0[1] = var8009d0f0[2] = -1;
	}

	node = modelGetPart(modeldef, MODELPART_REAPER_002D);

	if (node) {
		var8009d0f0[0] = modelFindNodeMtxIndex(node, 0);
	}

	node = modelGetPart(modeldef, MODELPART_REAPER_002E);

	if (node) {
		var8009d0f0[1] = modelFindNodeMtxIndex(node, 0);
	}

	node = modelGetPart(modeldef, MODELPART_REAPER_002F);

	if (node) {
		var8009d0f0[2] = modelFindNodeMtxIndex(node, 0);
	}
}

/**
 * Move/extend the scope on the gun model when the zoom function is used.
 */
void bgunUpdateSniperRifle(struct modeldef *modeldef, uint8_t *allocation)
{
	struct modelnode *nodes[4];
	float sp88[4] = {0, 0, 0, 0};
	int i;
	float f26;
	int mtxindex;
	struct coord sp70;

	f26 = 1.0f - (currentPlayerGetGunZoomFov() - 2.0f) / 58.0f;

	nodes[0] = modelGetPart(modeldef, MODELPART_SNIPERRIFLE_SCOPE1);
	nodes[1] = modelGetPart(modeldef, MODELPART_SNIPERRIFLE_SCOPE2);
	nodes[2] = modelGetPart(modeldef, MODELPART_SNIPERRIFLE_SCOPE3);
	nodes[3] = modelGetPart(modeldef, MODELPART_SNIPERRIFLE_SCOPE4);

	for (i = 0; i < ARRAYCOUNT(nodes); i++) {
		if (nodes[i]) {
			float f20 = f26 * 4.0f;
			mtxindex = modelFindNodeMtxIndex(nodes[i], 0);
			sp88[i] = f20 - i;

			if (f20 < i) {
				sp88[i] = 0.0f;
			}

			sp88[i] *= 100.0f;

			sp70.x = 0.0f;
			sp70.y = 0.0f;
			sp70.z = sp88[i];

			mtx4RotateVecInPlace((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)), &sp70);

			((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][0] += sp70.x;
			((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][1] += sp70.y;
			((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][2] += sp70.z;
		}
	}
}

/**
 * Animate the cartridge slider thing in the Devastator model.
 */
void bgunUpdateDevastator(struct hand *hand, uint8_t *allocation, struct modeldef *modeldef)
{
	struct modelnode *node = modelGetPart(modeldef, MODELPART_DEVASTATOR_0028);

	if (node) {
		int mtxindex = modelFindNodeMtxIndex(node, 0);
		struct coord sp24;

		hand->loadslide += 0.01f * LVUPDATE60FREAL();

		if (hand->loadslide > 1.0f) {
			hand->loadslide = 1.0f;
		}

		sp24.x = hand->loadslide * -10.0f * 1.636f;
		sp24.y = 0.0f;
		sp24.z = 0.0f;

		mtx4RotateVecInPlace((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)), &sp24);

		((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][0] += sp24.x;
		((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][1] += sp24.y;
		((Mtxf *)((uintptr_t)allocation + mtxindex * sizeof(Mtxf)))->m[3][2] += sp24.z;
	}
}

/**
 * Display the shotgun's starburst when appropriate.
 *
 * This logic is different to most guns, likely because most guns display the
 * starburst when the trigger is pressed while the shotgun has the double blast
 * function.
 */
void bgunUpdateShotgun(struct hand *hand, uint8_t *allocation, bool *arg2, struct modeldef *modeldef)
{
	if (hand->flashon) {
		hand->matmot1 = 1.0f;
	}

	if (hand->matmot1 > 0.0f) {
		hand->matmot1 -= LVUPDATE60FREAL() / 6.0f;

		if (hand->matmot1 < 0.01f) {
			hand->matmot1 = 0.0f;
		}
	}

	if (hand->matmot1 > 0.0f) {
		int sp34;
		int sp28[3] = {0, 0, 0};
		struct modelnode *node = modelGetPart(modeldef, MODELPART_SHOTGUN_0050);

		*arg2 = true;

		if (node) {
			sp34 = modelFindNodeMtxIndex(node, 0);

			mtx00015ea8((1.0f - hand->matmot1) * 8.0f + 0.5f, (Mtxf *)((uintptr_t)allocation + sp34 * sizeof(Mtxf)));
			mtx00015df0((1.0f - hand->matmot1) * 3.0f + 1.0f, (Mtxf *)((uintptr_t)allocation + sp34 * sizeof(Mtxf)));
			mtx00015e4c((1.0f - hand->matmot1) * 3.0f + 1.0f, (Mtxf *)((uintptr_t)allocation + sp34 * sizeof(Mtxf)));
		}
	}
}

void bgunUpdateLaser(struct hand *hand)
{
	if (hand->firing && hand->gset.weaponfunc == FUNC_SECONDARY) {
		if (hand->audiohandle == NULL && g_Vars.lvupdate240 != 0) {
			sndStart(var80095200, SFX_LASER_STREAM, &hand->audiohandle, -1, -1, -1, -1, -1);
		}

		hand->matmot1 = 1;
		return;
	}

	if (hand->matmot1 > 0) {
		hand->matmot1 -= LVUPDATE60FREAL() / 10.0f;
	} else if (hand->audiohandle != NULL && sndGetState(hand->audiohandle) != AL_STOPPED) {
		audioStop(hand->audiohandle);
	}
}

/**
 * Create ammo casing so they can be ejected during reload.
 */
void bgunUpdateMagnum(struct hand *hand, int handnum, struct modeldef *modeldef, Mtxf *mtx)
{
	float ground = g_Vars.currentplayer->vv_ground;
	int i;

	if (modeldef != NULL) {
		for (i = 0; i < hand->unk0cc8_04; i++) {
			struct modelnode *node = modelGetPart(modeldef, 0x0a + rngRandom() % 6);

			if (node) {
				int index = modelFindNodeMtxIndex(node, 0);
				Mtxf *tmp = mtx;
				Mtxf sp4c;

				tmp += index;

				mtx4Copy(tmp, &sp4c);
				mtx00015f04(9.999999f, &sp4c);
				mtx4MultMtx4InPlace(camGetProjectionMtxF(), &sp4c);

				casingCreateForHand(handnum, ground, &sp4c);
			}
		}
	}
}

/**
 * Create and/or update the rocket prop that sits inside the rocket launcher.
 */
void bgunUpdateRocketLauncher(struct hand *hand, int handnum, struct weaponfunc_shootprojectile *func)
{
	if (hand->rocket == NULL && hand->loadedammo[0] > 0) {
		bgunCreateHeldRocket(handnum, func);
	}

	if (hand->rocket) {
		bgunUpdateHeldRocket(handnum);
	}
}

void bgun0f0a45d0(struct hand *hand, struct modeldef *modeldef, bool isdetonator)
{
	struct modelnode *node = NULL;

	switch (hand->ejecttype) {
	case EJECTTYPE_GUN:
		if (isdetonator) {
			node = modelGetPart(modeldef, 0x2a);
		} else {
			node = modelGetPart(modeldef, 0x37);
		}
		break;
	case EJECTTYPE_GRENADEPIN:
		node = modelGetPart(modeldef, 0x2b);
		break;
	case EJECTTYPE_TRANQCASE:
		node = modelGetPart(modeldef, 0x2b);
		break;
	}

	if (node) {
		var8009d148 = modelFindNodeMtxIndex(node, 0);
		g_ModelJointPositionedFunc = bgun0f0a256c;
	} else {
		var8009d148 = -1;
	}
}

/**
 * With this function stubbed, the tranquilizer's spent ammo does not detach
 * when reloading, and the pulled pin on grenades and nbombs appears to move
 * with the model rather than detaching properly.
 */
void bgunTickEject(struct hand *hand, struct modeldef *modeldef, bool isdetonator)
{
	float lvupdate;
	struct coord spd0;
	Mtxf sp90;
	struct coord sp84;
	Mtxf sp44;
	int i;
	float newval;
	float mult = 3;

	switch (hand->ejectstate) {
	case EJECTSTATE_INIT:
		switch (hand->ejecttype) {
		case EJECTTYPE_GUN:
			hand->unk0d20.f[0] = (RANDOMFRAC() - 0.5f) * 0.5333333f * 0.0625f + 0.5333333f;
			hand->unk0d20.f[1] = RANDOMFRAC() * 2.5f * 0.0625f + 2.5f;
			hand->unk0d20.f[2] = 0.0f;
			spd0.f[0] = RANDOMFRAC() * 2.0f * M_TAU / 184.0f - 0.03414231f;
			spd0.f[1] = RANDOMFRAC() * 2.0f * M_TAU / 184.0f - 0.03414231f;
			spd0.f[2] = RANDOMFRAC() * 2.0f * M_TAU / 184.0f - 0.03414231f;
			break;
		case EJECTTYPE_GRENADEPIN:
			hand->unk0d20.f[0] = -((RANDOMFRAC() - 0.5f) * 0.5333333f * 0.0625f + mult * 0.5333333f);
			hand->unk0d20.f[1] = RANDOMFRAC() * 2.5f * 0.125f + 2.5f;
			hand->unk0d20.f[2] = -(RANDOMFRAC() + 1.0f);
			spd0.f[0] = (RANDOMFRAC() + 3.0f) * PALUPF(M_TAU) / 208.0f;
			spd0.f[1] = RANDOMFRAC() * 2.0f * M_TAU / 544.0f - 0.0115481345f;
			spd0.f[2] = RANDOMFRAC() * 2.0f * M_TAU / 544.0f - 0.0115481345f;
			break;
		case EJECTTYPE_TRANQCASE:
			hand->unk0d20.f[0] = 0.0f;
			hand->unk0d20.f[1] = RANDOMFRAC() * 2.5f * 0.125f + 2.5f;
			hand->unk0d20.f[2] = (RANDOMFRAC() + 1.0f) * 0.25f;
			spd0.f[0] = (RANDOMFRAC() + 3.0f) * PALUPF(M_TAU) / 368.0f;
			spd0.f[1] = RANDOMFRAC() * 2.0f * M_TAU / 944.0f - 0.006654857f;
			spd0.f[2] = RANDOMFRAC() * 2.0f * M_TAU / 944.0f - 0.006654857f;
			break;
		}

		hand->unk0d10 = hand->unk0d14 - 200.0f;

		mtx4LoadRotation(&spd0, &sp90);
		mtx4ToMtx3(&sp90, hand->unk0d50);

		if (g_Vars.lvupdate240 > 0 && hand->ejecttype != EJECTTYPE_GUN) {
			sp84.f[0] = (hand->posmtx.m[3][0] - hand->prevmtx.m[3][0]) / g_Vars.lvupdate60freal;
			sp84.f[1] = (hand->posmtx.m[3][1] - hand->prevmtx.m[3][1]) / g_Vars.lvupdate60freal;
			sp84.f[2] = (hand->posmtx.m[3][2] - hand->prevmtx.m[3][2]) / g_Vars.lvupdate60freal;

			mtxFullInverse4x4(hand->posmtx.m, sp44.m);
			mtx4RotateVecInPlace(&sp44, &sp84);

			hand->unk0d20.f[0] += sp84.f[0] * 0.3f;
			hand->unk0d20.f[1] += sp84.f[1] * 0.3f;
			hand->unk0d20.f[2] += sp84.f[2] * 0.3f;
		}

		hand->ejectstate = EJECTSTATE_AIRBORNE;
		break;
	case EJECTSTATE_AIRBORNE:
		lvupdate = g_Vars.lvupdate60freal;

		if (g_Vars.currentplayer->isdead && lvupdate > 1.5f) {
			lvupdate = 1.5f;
		}

		newval = hand->unk0d20.f[1] - lvupdate * 0.2777778f;

		if (hand->unk0d18 < hand->unk0d10) {
			hand->ejectstate = EJECTSTATE_FINISHED;
			break;
		}

		hand->unk0d18 += lvupdate * 0.5f * (hand->unk0d20.f[1] + newval);
		hand->unk0d14 += lvupdate * hand->unk0d20.f[0];
		hand->unk0d1c += lvupdate * hand->unk0d20.f[2];

		hand->unk0d20.f[1] = newval;

		for (i = 0; i < g_Vars.lvupdate240; i++) {
			mtx00016110(hand->unk0d50, hand->unk0d2c);
		}

		break;
	}
}

void bgun0f0a4e44(struct hand *hand, struct weapon *weapondef, struct modeldef *modeldef,
		struct weaponfunc *funcdef, int maxburst, uint8_t *allocation, int weaponnum,
		bool **arg7, int mtxindex, Mtxf *arg9, Mtxf *arg10)
{
	Mtxf spd8;
	int index;
	int shotstotake;
	bool spc4[3] = {false, false, false};
	Mtxf *mtx;
	int i;
	int partnum;
	float spb4;
	float muzzlez;
	Mtxf sp70;

	index = hand->burstbullets % maxburst;
	shotstotake = hand->shotstotake;

	spb4 = RANDOMFRAC() * 0.25f + 1.0f;
	muzzlez = weapondef->muzzlez;

	mtx4LoadIdentity(&spd8);

	if (funcdef && (funcdef->flags & FUNCFLAG_00000001)) {
		mtx4LoadZRotation(RANDOMFRAC() * M_TAU, &spd8);
	}

	mtx4LoadZRotation((RANDOMFRAC() * 0.3 - 0.15), &spd8);

	mtx = (Mtxf *)allocation;
	mtx += mtxindex;

	mtx4MultMtx4InPlace(mtx, &spd8);
	mtx00015f04(spb4, &spd8);
	mtx00015ea8(muzzlez, &spd8);
	mtx4Copy(&spd8, mtx);

	if (shotstotake == 0 && weaponnum != WEAPON_REAPER) {
		shotstotake++;
	}

	for (i = 0; i < shotstotake; i++) {
		spc4[index] = true;
		index++;

		if (index >= maxburst) {
			index = 0;
		}
	}

	for (i = 0; i < maxburst; i++) {
		if (spc4[i] && arg7[i] != NULL) {
			*arg7[i] = true;
		}
	}

	for (partnum = 0x50; partnum <= 0x52; partnum++) {
		struct modelnode *node = modelGetPart(modeldef, partnum);
		struct coord sp60;

		if (node && weaponnum != WEAPON_REAPER && weaponnum != WEAPON_SHOTGUN) {
			struct modelrodata_position *rodata = &node->rodata->position;
			int mtxindex = modelFindNodeMtxIndex(node, 0);

			sp60.x = rodata->pos.x * spd8.m[0][0] + rodata->pos.y * spd8.m[1][0] + rodata->pos.z * spd8.m[2][0] + spd8.m[3][0];
			sp60.y = rodata->pos.x * spd8.m[0][1] + rodata->pos.y * spd8.m[1][1] + rodata->pos.z * spd8.m[2][1] + spd8.m[3][1];
			sp60.z = rodata->pos.x * spd8.m[0][2] + rodata->pos.y * spd8.m[1][2] + rodata->pos.z * spd8.m[2][2] + spd8.m[3][2];

			mtx4LoadIdentity(&sp70);
			mtx4Align(sp70.m, RANDOMFRAC() * M_TAU, -sp60.x, -sp60.y, -sp60.z);
			mtx00015f04(0.10000001f * spb4, &sp70);

			mtx = (Mtxf *)allocation;

			mtx00016e98(arg10->m, 0, mtx->m[3][0] - hand->aimpos.x, mtx->m[3][1] - hand->aimpos.y, mtx->m[3][2] - hand->aimpos.z);
			mtx4MultMtx4InPlace(arg10, &sp70);
			mtx00016710(muzzlez, sp70.m);
			mtx4MultMtx4InPlace(arg9, &sp70);
			mtx4SetTranslation(&sp60, &sp70);

			mtx = (Mtxf *)allocation;
			mtx += mtxindex;

			mtx4Copy(&sp70, mtx);
		}
	}
}

/**
 * Create casing and beam for a fired weapon,
 * and uncloak if the weapon is a throwable or fired projectile.
 */
void bgunCreateFx(struct hand *hand, int handnum, struct weaponfunc *funcdef, int weaponnum, struct modeldef *modeldef, uint8_t *allocation)
{
	float ground;
	bool createbeam = true;

	g_Vars.currentplayer->gunctrl.throwing = false;

	if (funcdef) {
		ground = g_Vars.currentplayer->vv_ground;

		if (modeldef && weaponnum != WEAPON_DY357MAGNUM && weaponnum != WEAPON_DY357LX) {
			int partnum = MODELPART_GUN_CARTEJECTPOS;
			struct modelnode *node;

			if (weaponnum == WEAPON_REAPER) {
				partnum = (hand->burstbullets & 1) == 1 ? MODELPART_REAPER_CARTEJECTPOS1 : MODELPART_REAPER_CARTEJECTPOS2;
			}

			node = modelGetPart(modeldef, partnum);

			if (node) {
				Mtxf *mtx = (Mtxf *)allocation;
				Mtxf sp24;

				mtx += modelFindNodeMtxIndex(node, 0);

				mtx4Copy(mtx, &sp24);
				mtx00015f04(9.999999f, &sp24);
				mtx4MultMtx4InPlace(camGetProjectionMtxF(), &sp24);

				casingCreateForHand(handnum, ground, &sp24);
			} else {
				casingCreateForHand(handnum, ground, &hand->posmtx);
			}

			bgunSetPartVisible(MODELPART_GUN_CARTFLAPCLOSED, false, hand, modeldef);
			bgunSetPartVisible(MODELPART_GUN_CARTFLAPOPEN, true, hand, modeldef);
		}

		if (funcdef->type == INVENTORYFUNCTYPE_SHOOT_PROJECTILE) {
			chrUncloakTemporarily(g_Vars.currentplayer->prop->chr);
		} else if ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_THROW) {
			chrUncloakTemporarily(g_Vars.currentplayer->prop->chr);
		}
	}

	if (funcdef) {
		if ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_MELEE || (funcdef->type & INVENTORYFUNCTYPE_0200)) {
			createbeam = false;
		}

		if ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_SPECIAL) {
			createbeam = false;
		}

		if ((funcdef->type & 0xff) == INVENTORYFUNCTYPE_THROW) {
			createbeam = false;
		}
	}

	if (createbeam) {
		switch (weaponnum) {
		case WEAPON_FALCON2:
		case WEAPON_FALCON2_SILENCER:
		case WEAPON_FALCON2_SCOPE:
		//case WEAPON_FALCON2_SANDS:
		case WEAPON_MAGSEC4:
		case WEAPON_MAULER:
		case WEAPON_PHOENIX:
		case WEAPON_DY357MAGNUM:
		case WEAPON_DY357LX:
		case WEAPON_CMP150:
		case WEAPON_CYCLONE:
		case WEAPON_CALLISTO:
		case WEAPON_RCP120:
		case WEAPON_LAPTOPGUN:
		case WEAPON_DRAGON:
		case WEAPON_K7AVENGER:
		case WEAPON_AR34:
		case WEAPON_SUPERDRAGON:
		case WEAPON_REAPER:
		case WEAPON_SNIPERRIFLE:
		case WEAPON_FARSIGHT:
		case WEAPON_TRANQUILIZER:
		case WEAPON_PP9I:
		case WEAPON_CC13:
		case WEAPON_KL01313:
		case WEAPON_KF7SPECIAL:
		case WEAPON_ZZT:
		case WEAPON_DMC:
		case WEAPON_AR53:
		case WEAPON_RCP45:
			beamCreateForHand(handnum);
			hand->numfires++;
			return;
		case WEAPON_LASER:
			hand->numfires++;
			beamCreateForHand(handnum);
			break;
		}
	}
}

// offset calculation from NeonNyan/perfect-dark

static inline float bgunGetFovOffsetZ(void)
{
	return (PLAYER_DEFAULT_FOV - 60.f) / 3.f;
}

static inline float bgunGetFovOffsetY(void)
{
	return (PLAYER_DEFAULT_FOV - 60.f) / (2.75f * 4.f);
}

bool bgunCheckForCloseWall()
{
	bool nearbywall = false;
	struct hitthing hit;
	struct coord hitpos;
	RoomNum spc8[8];
	RoomNum spb8[8];
	RoomNum rooms[131];
	RoomNum *roomsptr;
	float checkdistance = 100;
	struct prop *playerprop = g_Vars.currentplayer->prop;

	if (!g_Vars.currentplayer) {
		return false;
	}

	struct coord checkpos;
	checkpos.x = g_Vars.currentplayer->cam_pos.x + g_Vars.currentplayer->cam_look.x * checkdistance;
	checkpos.y = g_Vars.currentplayer->cam_pos.y + g_Vars.currentplayer->cam_look.y * checkdistance;
	checkpos.z = g_Vars.currentplayer->cam_pos.z + g_Vars.currentplayer->cam_look.z * checkdistance;

	spc8[0] = g_Vars.currentplayer->cam_room;
	spc8[1] = -1;
	portal00018148(&g_Vars.currentplayer->cam_pos, &checkpos, spc8, spb8, rooms, 30);

	roomsptr = rooms;

	while (*roomsptr != -1) {
		roomsptr++;
	}

	int i = 0;

	for (i = 0; rooms[i] != -1; i++) {
		nearbywall = bgTestHitInRoom(&g_Vars.currentplayer->cam_pos, &checkpos, rooms[i], &hit);
		if(nearbywall) {
			return true;
		}
	}
	
	return false;
}

void bgun0f0a5550(int handnum)
{
	uint8_t *mtxallocation;
	Mtxf sp2c4;
	Mtxf sp284;
	struct modeldef *modeldef = NULL;
	struct coord viewmodelpos = {0, 0, 0};
	Mtxf sp234;
	Mtxf sp1f4;
	union modelrodata *rodata;
	bool *sp1e4[3] = {NULL, NULL, NULL};
	int sp1e0 = 0;
	struct modelnode *node;
	struct player *player = g_Vars.currentplayer;
	struct hand *hand = player->hands + handnum;
	struct weaponfunc *funcdef;
	struct weaponfunc_shoot *shootfunc = NULL;
	int i;
	int weaponnum = bgunGetWeaponNum2(handnum);
	struct weapon *weapondef;
	Mtxf *mtx;
	bool isdetonator = false;
	float fspare1;
	float fspare2;
	struct coord sp1a4;
	Mtxf sp164;
	Mtxf sp124;
	struct coord sp118;
	int j;

	weapondef = weaponFindById(weaponnum);

	if (handnum == HAND_LEFT && weaponnum == WEAPON_REMOTEMINE) {
		isdetonator = true;
	}

	funcdef = gsetGetWeaponFunction2(&hand->gset);

	if (funcdef && (funcdef->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		shootfunc = (struct weaponfunc_shoot *)funcdef;
	}

	bgunUpdateBlend(hand, handnum);

	if (handnum == HAND_RIGHT) {
		if (weaponHasFlag(bgunGetWeaponNum2(HAND_LEFT), WEAPONFLAG_00000040)) {
			hand->xshift += 2.0f * g_Vars.lvupdate60freal / 240.0f;

			if (hand->xshift > 2.0f) {
				hand->xshift = 2.0f;
			}
		} else {
			hand->xshift -= 2.0f * g_Vars.lvupdate60freal / 240.0f;

			if (hand->xshift < 0.0f) {
				hand->xshift = 0.0f;
			}
		}
	} else {
		if (weaponHasFlag(bgunGetWeaponNum2(HAND_RIGHT), WEAPONFLAG_00000040)) {
			hand->xshift -= 2.0f * g_Vars.lvupdate60freal / 240.0f;

			if (hand->xshift < -2.0f) {
				hand->xshift = -2.0f;
			}
		} else {
			hand->xshift += 2.0f * g_Vars.lvupdate60freal / 240.0f;

			if (hand->xshift > 0.0f) {
				hand->xshift = 0.0f;
			}
		}
	}

	if (handnum == HAND_RIGHT) {
		viewmodelpos.x = func0f0b131c(handnum) + hand->damppos.f[0] + hand->adjustpos.f[0];
		viewmodelpos.y = weapondef->posy + hand->damppos.f[1] + hand->adjustpos.f[1];
		viewmodelpos.z = weapondef->posz + hand->damppos.f[2] + hand->adjustpos.f[2];
	} else if (isdetonator) {
		viewmodelpos.x = 6.5f + hand->damppos.f[0] - hand->adjustpos.f[0];
		viewmodelpos.y = -16.5f + hand->damppos.f[1] + hand->adjustpos.f[1];
		viewmodelpos.z = -16.0f + hand->damppos.f[2] + hand->adjustpos.f[2];
	} else {
		viewmodelpos.x = func0f0b131c(handnum) + hand->damppos.f[0] - hand->adjustpos.f[0];
		viewmodelpos.y = weapondef->posy + hand->damppos.f[1] + hand->adjustpos.f[1];
		viewmodelpos.z = weapondef->posz + hand->damppos.f[2] + hand->adjustpos.f[2];
	}

	viewmodelpos.y += player->guncloseroffset * 5.0f / -90.0f * 50.0f;
	viewmodelpos.z -= player->guncloseroffset * 15.0f / -90.0f * 50.0f;

	// adjust viewmodel position for different FOVs
	viewmodelpos.y -= bgunGetFovOffsetY();
	viewmodelpos.z += bgunGetFovOffsetZ();

	if (hand->firing && shootfunc && g_Vars.lvupdate240 != 0 && shootfunc->recoilsettings != NULL) {
		viewmodelpos.x += (RANDOMFRAC() - 0.5f) * shootfunc->recoilsettings->xrange * hand->finalmult[0];
		viewmodelpos.y += (RANDOMFRAC() - 0.5f) * shootfunc->recoilsettings->yrange * hand->finalmult[0];
		viewmodelpos.z += (RANDOMFRAC() - 0.5f) * shootfunc->recoilsettings->zrange * hand->finalmult[0];
	}

	hand->fspare1 = (player->crosspos2[0] - camGetScreenLeft() - camGetScreenWidth() * 0.5f) * weapondef->aimsettings->guntransside / (camGetScreenWidth() * 0.5f);

	if (player->crosspos2[1] - camGetScreenTop() > camGetScreenHeight() * 0.5f) {
		hand->fspare2 = (player->crosspos2[1] - camGetScreenTop() - camGetScreenHeight() * 0.5f) * weapondef->aimsettings->guntransdown / (camGetScreenHeight() * 0.5f);
	} else {
		hand->fspare2 = (player->crosspos2[1] - camGetScreenTop() - camGetScreenHeight() * 0.5f) * weapondef->aimsettings->guntransup / (camGetScreenHeight() * 0.5f);
	}

	fspare1 = hand->fspare1;
	fspare2 = hand->fspare2;

	viewmodelpos.f[0] += fspare1;
	viewmodelpos.f[1] -= fspare2;

	hand->visible = true;

	if (!weaponHasFlag(weaponnum, WEAPONFLAG_00000040)
			|| weaponHasFlag(weaponnum, WEAPONFLAG_00000080)
			|| hand->mode == HANDMODE_6
			|| hand->mode == HANDMODE_7
			|| !bgunIsLoaded()
			|| hand->inuse == false
			|| bgunGetGunMemType() == 0) {
		hand->visible = false;
	}

	if (hand->visible) {
		modeldef = player->gunctrl.gunmodeldef;
		mtxallocation = gfxAllocate(modeldef->nummatrices * sizeof(Mtxf));

		if (weaponHasFlag(weaponnum, WEAPONFLAG_02000000)) {
			for (i = 0; i < modeldef->nummatrices; i++) {
				mtx = (Mtxf *)(mtxallocation + i * sizeof(Mtxf));
				mtx4LoadIdentity(mtx);
			}
		}

		bgunExecuteModelCmdList(hand->unk0dcc);

		if (player->gunctrl.handmodeldef != NULL) {
			bgunExecuteModelCmdList(hand->unk0dd0);
		}

		bgun0f098030(hand, modeldef);

		if (weaponHasFlag(weaponnum, WEAPONFLAG_00002000)) {
			bgun0f0981e8(hand, modeldef);
		}
	}

	mtx4LoadIdentity(&sp234);

	if (weaponHasFlag(weaponnum, WEAPONFLAG_GANGSTA)) {
		bgunUpdateGangsta(hand, handnum, &viewmodelpos, funcdef, &sp284, &sp234);
	}

	// Ben's comment: weapons with the gangsta flag check for a nearby wall to see if they can be turned up
	if (weaponHasFlag(weaponnum, WEAPONFLAG_TURNUP)) {
		g_CloseToWall = bgunCheckForCloseWall();
		if(g_CloseToWall) {
			g_CloseToWallTimer++;
		}
		else {
			g_CloseToWallTimer = 0;
			//hand->turnuprot -= LVUPDATE60FREAL() * 5;
			if(hand->turnuprot < 0) {
				hand->turnuprot = 0;
			}
		}
		bgunUpdateHandState2(hand, handnum, &viewmodelpos, funcdef, &sp284, &sp234);
	}

	if (hand->useposrot) {
		viewmodelpos.f[0] += hand->posrotmtx.m[3][0];
		viewmodelpos.f[1] += hand->posrotmtx.m[3][1];
		viewmodelpos.f[2] += hand->posrotmtx.m[3][2];

		mtx00015be0(&hand->posrotmtx, &sp234);

		sp234.m[3][0] = 0.0f;
		sp234.m[3][1] = 0.0f;
		sp234.m[3][2] = 0.0f;
	} else {
		hand->rotxoffset = 0.0f;
		hand->posoffset.x = 0.0f;
		hand->posoffset.y = 0.0f;
		hand->posoffset.z = 0.0f;
	}

	mtx00016d58(&sp284, 0.0f, 0.0f, 0.0f,
			hand->damplook.x, hand->damplook.y, hand->damplook.z,
			hand->dampup.x, hand->dampup.y, hand->dampup.z);

	mtx00015be0(&sp284, &sp234);

	sp1a4.x = 0.0f;
	sp1a4.y = M_PI;
	sp1a4.z = 0.0f;

	mtx4LoadRotation(&sp1a4, &sp164);

	sp1a4.y = 0.0f;

	bgun0f0a24f0(&sp118, handnum);

	sp1a4.y = -bgun0f0a2498(sp118.x, sp118.z, viewmodelpos.f[0], viewmodelpos.f[2]);
	sp1a4.x = bgun0f0a2498(sp118.y, sp118.z, viewmodelpos.f[1], viewmodelpos.f[2]);

	hand->lastrotangx = sp1a4.f[0];
	hand->lastrotangy = sp1a4.f[1];

	mtx4LoadRotation(&sp1a4, &sp124);
	mtx4MultMtx4(&sp124, &sp164, &sp284);
	mtx4MultMtx4InPlace(&sp284, &sp234);
	mtx4Copy(&sp234, &sp2c4);
	mtx4SetTranslation(&viewmodelpos, &sp2c4);

	mtx4Copy(&sp2c4, &hand->cammtx);
	mtx4Copy(&hand->posmtx, &hand->prevmtx);

	mtxApplyAffineTransform(camGetProjectionMtxF(), &hand->cammtx, &hand->posmtx);

	if (hand->visible) {
		for (j = 0x5a; j < 0x5d; j++) {
			node = modelGetPart(modeldef, j);

			if (node) {
				rodata = node->rodata;
				sp1e4[sp1e0] = (bool *)&hand->unk0a6c[rodata->toggle.rwdataindex];
				sp1e0++;
			}
		}

		hand->gunmodel.matrices = (Mtxf *)mtxallocation;
		hand->handmodel.matrices = (Mtxf *)mtxallocation;

		if (weaponHasFlag(weaponnum, WEAPONFLAG_DUALFLIP) && handnum == HAND_LEFT) {
			mtx00015e24(-1, &sp2c4);
		}

		mtx00015f04(0.10000001f, &sp2c4);

		mtx4Copy(&sp2c4, (Mtxf *)mtxallocation);

		if (hand->unk0cc8_04 > 0) {
			switch (weaponnum) {
			case WEAPON_GRENADE:
			case WEAPON_NBOMB:
				hand->ejectstate = EJECTSTATE_INIT;
				hand->ejecttype = EJECTTYPE_GRENADEPIN;
				break;
			case WEAPON_TRANQUILIZER:
				hand->ejectstate = EJECTSTATE_INIT;
				hand->ejecttype = EJECTTYPE_TRANQCASE;
				break;
			}
		}

		var8009d144 = hand;

		if (hand->ejectstate > EJECTSTATE_INACTIVE) {
			bgun0f0a45d0(hand, modeldef, isdetonator);
		}

		var8009d0dc = -1;
		var8009d0f0[0] = var8009d0f0[1] = var8009d0f0[2] = -1;

		switch (weaponnum) {
		case WEAPON_LASER:
			bgunUpdateLaser(hand);
			break;
		case WEAPON_REAPER:
			bgunUpdateReaper(hand, modeldef);
			break;
		}

		{
			bool a0 = true;
			struct modelrenderdata renderdata = {NULL, true, 3};
//#if VERSION >= VERSION_PAL_BETA
			bool a3 = false;
//#endif
			int spcc;
			Mtxf *spc8;
			Mtxf *spc4;
			Mtxf sp84;
			uint32_t sp80;
			struct coord sp74;
			int sp6c;

			renderdata.unk00 = &sp2c4;
			renderdata.unk10 = hand->gunmodel.matrices;

			if (hand->animmode != HANDANIMMODE_IDLE) {
				a0 = false;
			}

			switch (weaponnum) {
			case WEAPON_REAPER:
				a0 = false;
				break;
			case WEAPON_COMBATKNIFE:
				if (player->hands[HAND_LEFT].loadedammo[0] == 0) {
					a0 = false;
				}
				// fall through
			case WEAPON_GRENADE:
			case WEAPON_NBOMB:
			case WEAPON_TIMEDMINE:
			case WEAPON_PROXIMITYMINE:
			case WEAPON_REMOTEMINE:
			case WEAPON_ECMMINE:
				if (player->hands[HAND_RIGHT].loadedammo[0] == 0) {
					a0 = false;
				}

				if (player->hands[handnum].state == HANDSTATE_AUTOSWITCH) {
					a0 = false;
				}

				if (player->hands[handnum].state == HANDSTATE_ATTACK) {
					a0 = false;
				}
				break;
			}

			if (hand->ejectstate != EJECTSTATE_INACTIVE) {
				a0 = false;
			}

			if (player->hands[handnum].state == HANDSTATE_CHANGEGUN
					&& player->hands[handnum].stateminor <= HANDSTATEMINOR_CHANGEGUN_LOWER
					&& weapondef->unequip_animation != NULL) {
				a0 = false;
			}

//#if VERSION >= VERSION_PAL_BETA
			switch (modelGetAnimNum(&hand->gunmodel)) {
			case ANIM_GUN_CROSSBOW_EQUIP:
			case ANIM_GUN_LAPTOP_EQUIP:
			case ANIM_GUN_LAPTOP_UNEQUIP:
			case ANIM_GUN_LAPTOP_RELOAD:
			case ANIM_GUN_FALCON2_RELOAD:
			case ANIM_GUN_CMP150_RELOAD:
			case ANIM_GUN_FARSIGHT_SHOOT:
			case ANIM_GUN_SHOTGUN_SHOOT_SINGLE:
			case ANIM_GUN_REAPER_SHOOT:
			case ANIM_GUN_MAGSEC4_RELOAD:
			case ANIM_GUN_CYCLONE_RELOAD:
			case ANIM_GUN_SNIPER_RELOAD:
			case ANIM_GUN_PHOENIX_RELOAD:
			case ANIM_GUN_FALCON2_RELOAD_SCOPE:
			case ANIM_GUN_REMOTEMINE_EQUIP:
				a3 = 1;
				break;
			}
//#endif

			if (a0) {
				if (player->hands[HAND_RIGHT].unk0dd4 == -1) {
					mtx4LoadIdentity(&sp84);

					spc4 = hand->gunmodel.matrices;

					renderdata.unk00 = &sp84;
					renderdata.unk10 = player->hands[HAND_RIGHT].unk0dd8;

					modelSetMatricesWithAnim(&renderdata, &hand->gunmodel);

					player->hands[HAND_RIGHT].unk0dd4 = 1;

					hand->gunmodel.matrices = spc4;
				}

				spc8 = player->hands[HAND_RIGHT].unk0dd8;
				spc4 = hand->gunmodel.matrices;

				for (spcc = 0; spcc < hand->gunmodel.definition->nummatrices; spcc++) {
					mtxApplyAffineTransform(&sp2c4, spc8, spc4);
					spc8++;
					spc4++;
				}
			} else {
				modelSetMatricesWithAnim(&renderdata, &hand->gunmodel);
			}

			g_ModelJointPositionedFunc = 0;

			node = modelGetPart(modeldef, MODELPART_GUN_SLIDE);

			if (node) {
				sp80 = modelFindNodeMtxIndex(node, 0);

				bgunUpdateSlide(handnum);

				sp74.f[0] = 0.0f;
				sp74.f[1] = 0.0f;
				sp74.f[2] = -hand->slidetrans;

				mtx = (Mtxf *)mtxallocation;
				mtx += sp80;

				mtx4RotateVecInPlace(mtx, &sp74);

				mtx->m[3][0] += sp74.f[0];
				mtx->m[3][1] += sp74.f[1];
				mtx->m[3][2] += sp74.f[2];
			}

			if (sp1e4[0] != NULL) {
				*sp1e4[0] = false;
			}

			if (sp1e4[1] != NULL) {
				*sp1e4[1] = false;
			}

			if (sp1e4[2] != NULL) {
				*sp1e4[2] = false;
			}

			switch (weaponnum) {
			case WEAPON_SNIPERRIFLE:
				bgunUpdateSniperRifle(modeldef, mtxallocation);
				break;
			case WEAPON_DEVASTATOR:
				bgunUpdateDevastator(hand, mtxallocation, modeldef);
				break;
			case WEAPON_SHOTGUN:
				bgunUpdateShotgun(hand, mtxallocation, sp1e4[0], modeldef);
				break;
			}

			node = modelGetPart(modeldef, MODELPART_GUN_MUZZLEPOS);

			if (weaponnum == WEAPON_REAPER) {
				if (hand->flashon || hand->firing) {
					node = modelGetPart(modeldef, MODELPART_REAPER_001E + (hand->burstbullets % 3));
				} else {
					node = modelGetPart(modeldef, MODELPART_REAPER_001E + (g_Vars.lvframenum % 3));
				}
			}

			if (node) {
				sp6c = modelFindNodeMtxIndex(node, 0);

				mtx = (Mtxf *)mtxallocation;
				mtx += sp6c;

				hand->muzzlepos.f[0] = mtx->m[3][0];
				hand->muzzlepos.f[1] = mtx->m[3][1];
				hand->muzzlepos.f[2] = mtx->m[3][2];

				mtx4Copy(mtx, &hand->muzzlemat);
				mtx4TransformVecInPlace(camGetProjectionMtxF(), &hand->muzzlepos);

				hand->muzzlez = -((Mtxf *)((uintptr_t)mtxallocation + sp6c * sizeof(Mtxf)))->m[3][2];

				if (hand->flashon && sp1e0 > 0 && weaponnum != WEAPON_SHOTGUN && g_Vars.lvupdate240 != 0) {
					bgun0f0a4e44(hand, weapondef, modeldef, funcdef, sp1e0, mtxallocation, weaponnum, sp1e4, sp6c, &sp234, &sp1f4);
				}
			} else if (weaponnum == WEAPON_GRENADE
					|| weaponnum == WEAPON_TIMEDMINE
					|| weaponnum == WEAPON_REMOTEMINE
					|| weaponnum == WEAPON_PROXIMITYMINE
					|| weaponnum == WEAPON_NBOMB) {
				sp6c = modelFindNodeMtxIndex(modelGetPart(modeldef, MODELPART_GUN_HOLDPOS), 0);

				mtx = (Mtxf *)mtxallocation;
				mtx += sp6c;

				hand->muzzlepos.x = mtx->m[3][0];
				hand->muzzlepos.y = mtx->m[3][1];
				hand->muzzlepos.z = mtx->m[3][2];

				mtx4Copy(mtx, &hand->muzzlemat);
				mtx4TransformVecInPlace(camGetProjectionMtxF(), &hand->muzzlepos);

				hand->muzzlez = -((Mtxf *)((uintptr_t)mtxallocation + sp6c * sizeof(Mtxf)))->m[3][2];
			} else {
				hand->muzzlepos.x = hand->posmtx.m[3][0];
				hand->muzzlepos.y = hand->posmtx.m[3][1];
				hand->muzzlepos.z = hand->posmtx.m[3][2];

				mtx4Copy(&hand->posmtx, &hand->muzzlemat);

				hand->muzzlez = -hand->cammtx.m[3][2];
			}
		}
	} else {
		hand->muzzlepos.x = hand->posmtx.m[3][0];
		hand->muzzlepos.y = hand->posmtx.m[3][1];
		hand->muzzlepos.z = hand->posmtx.m[3][2];

		mtx4Copy(&hand->posmtx, &hand->muzzlemat);

		hand->muzzlez = -hand->cammtx.m[3][2];
	}

	switch (weaponnum) {
	case WEAPON_ROCKETLAUNCHER:
		bgunUpdateRocketLauncher(hand, handnum, (struct weaponfunc_shootprojectile *)funcdef);
		break;
	case WEAPON_DY357MAGNUM:
	case WEAPON_DY357LX:
		if (hand->unk0cc8_04 > 0) {
			bgunUpdateMagnum(hand, handnum, modeldef, (Mtxf *)mtxallocation);
		}
		break;
	}

	if (hand->firing && g_Vars.lvupdate240 != 0) {
		bgunCreateFx(hand, handnum, funcdef, weaponnum, modeldef, mtxallocation);
	}

	if (PLAYERCOUNT() == 1 && g_Vars.lvupdate240 != 0) {
		bgunUpdateSmoke(hand, handnum, weaponnum, funcdef);
	}

	if (hand->ejectstate > EJECTSTATE_INACTIVE) {
		bgunTickEject(hand, modeldef, isdetonator);
	}

	if (PLAYERCOUNT() == 1 && hand->visible
			&& weaponnum >= WEAPON_FALCON2 && weaponnum <= WEAPON_FALCON2_SCOPE) {
		bgunUpdateLasersight(hand, modeldef, handnum, mtxallocation);
	} else {
		lasersightFree(handnum);
	}

	hand->animframeinc = 0;

//#if VERSION >= VERSION_PAL_BETA
	hand->animframeincfreal = 0;
//#endif
}

void bgunTickMaulerCharge(void)
{
	struct player *player = g_Vars.currentplayer;
	int i;

	for (i = 0; i < 2; i++) {
		struct hand *hand = &player->hands[i];
		uint32_t charging = false;

		if (hand->inuse) {
			if (bgunIsReloading(hand)) {
				// Reloading - reset charge amount
				hand->matmot1 = 0;
			} else if (hand->gset.weaponfunc == FUNC_SECONDARY) {
				// Charging or fully charged
				int oldvalue = hand->matmot1;
				int newvalue;

				if (hand->loadedammo[0] >= 2 && hand->matmot1 < 5) {
					charging = true;
					hand->matmot1 += g_Vars.lvupdate60freal * 0.05f;
				}

				if (hand->matmot1 > 5) {
					hand->matmot1 = 5;
				}

				newvalue = hand->matmot1;

				if (oldvalue != newvalue && hand->loadedammo[0] >= 2) {
					hand->loadedammo[0]--;
				}
			} else {
				// Using primary function - make the charge wear off slowly
				hand->matmot1 -= g_Vars.lvupdate60freal * 0.005f;

				if (hand->matmot1 < 0) {
					hand->matmot1 = 0;
				}
			}

			/**
			 * Probable @bug: In other places where audio is started and then
			 * its speed is adjusted, the game raises the priority of the main
			 * thread (this thread) to above the audio thread's priority so that
			 * the audio thread cannot execute and start playing the audio
			 * between the calls to sndStart and audioPostEvent. But this pattern
			 * is not done here.
			 *
			 * There is a known issue where the Mauler charge sound is played
			 * correctly then the game proceeds to play other unrelated sound
			 * effects before eventually crashing. It's suspected that this lack
			 * of thread priority adjusting is the root cause. Perhaps a race
			 * condition exists where the audio thread does something with the
			 * sound while this thread is in the middle of reconfiguring it.
			 * This is not yet confirmed.
			 */
			if (hand->audiohandle == NULL
					&& hand->matmot1 > 0.1f
					&& charging
					&& g_Vars.lvupdate240 != 0) {
				sndStart(var80095200, SFX_MAULER_CHARGE, &hand->audiohandle, -1, -1, -1, -1, -1);
			}

			if (hand->audiohandle) {
				float speed = 0.5f + hand->matmot1 / 3.0f + sinf(g_20SecIntervalFrac * M_PI * 32.0f) * 0.03f;

				if (hand->matmot1 < 0.1f || !charging) {
					audioStop(hand->audiohandle);
				} else {
					audioPostEvent(hand->audiohandle, AL_SNDP_PITCH_EVT, *(int *)&speed);
				}
			}
		}
	}
}

void bgunTickGameplay2(void)
{
	struct player *player = g_Vars.currentplayer;
	struct hand *hand;
	int i;

	if (player->gunctrl.loadall) {
		// empty
	} else {
		bgunTickLoad();
	}

	// Return control to Jo if eyespy has been deselected
	if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY) == 0
			&& player->eyespy) {
		player->eyespy->active = false;
	}

	if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_XRAYSCANNER)
			&& (bgunGetWeaponNum(HAND_RIGHT) != WEAPON_FARSIGHT || player->gunsightoff)) {
		// Using normal xray scanner (not Farsight zoom)
		if (player->visionmode != VISIONMODE_XRAY) {
			player->erasertime = 0;
		} else {
			player->erasertime += g_Vars.lvupdate240;
		}

		player->visionmode = VISIONMODE_XRAY;
		player->ecol_1 = 24;
		player->ecol_2 = 8;
		player->ecol_3 = 24;
		player->epcol_0 = 2;
		player->epcol_1 = 0;
		player->epcol_2 = 1;
	} else {
		if (player->gunsightoff == 0) {
			if (player->hands[HAND_RIGHT].gset.weaponnum == WEAPON_FARSIGHT) {
				// Aiming with the Farsight
				if (player->visionmode != VISIONMODE_XRAY) {
					player->erasertime = 0;
				} else {
					player->erasertime += g_Vars.lvupdate240;
				}

				player->visionmode = VISIONMODE_XRAY;
				player->ecol_1 = 16;
				player->ecol_2 = 24;
				player->ecol_3 = 8;
				player->epcol_0 = 0;
				player->epcol_1 = 1;
				player->epcol_2 = 2;
			} else {
				// Aiming with non-Farsight
				if (player->visionmode != VISIONMODE_SLAYERROCKET) {
					player->visionmode = VISIONMODE_NORMAL;
				}
			}
		} else {
			// Not aiming
			if (player->visionmode != VISIONMODE_SLAYERROCKET) {
				player->visionmode = VISIONMODE_NORMAL;
			}
		}
	}

	if (player->gunctrl.weaponnum == WEAPON_MAULER) {
		bgunTickMaulerCharge();
	}

	if (g_Vars.lvupdate240 == 0) {
		for (i = 0; i < 2; i++) {
			hand = &player->hands[i];

			if (hand->audiohandle) {
				audioStop(hand->audiohandle);
			}
		}
	}

	if (g_Vars.currentplayer->devicesactive &
			~g_Vars.currentplayer->devicesinhibit & DEVICE_CLOAKRCP120) {
		if (player->gunctrl.weaponnum == WEAPON_RCP120) {
			struct chrdata *chr = player->prop->chr;

			// Handle RCP120 cloak ammo usage
			if ((chr->hidden & CHRHFLAG_CLOAKED) && chr->cloakfadefinished == true) {
				hand = &player->hands[HAND_RIGHT];
				hand->matmot1 += LVUPDATE60FREAL() * 0.4f;

				if (hand->matmot1 > 1.0f) {
					int usedqty = hand->matmot1;

					if (usedqty > hand->loadedammo[0]) {
						usedqty = hand->loadedammo[0];
					}

					hand->matmot1 -= usedqty;
					hand->loadedammo[0] -= usedqty;

					// If out of ammo, turn off cloak
					if (hand->loadedammo[0] == 0 && hand->state != HANDSTATE_RELOAD) {
						int stilltogo = hand->matmot1;

						if (stilltogo > player->ammoheldarr[player->gunctrl.ammotypes[0]]) {
							g_Vars.currentplayer->devicesactive &= ~DEVICE_CLOAKRCP120;
						}
					}
				}
			}
		} else {
			// No longer using RCP120, so turn off cloak
			player->devicesactive &= ~DEVICE_CLOAKRCP120;
		}
	} else if (player->gunctrl.weaponnum == WEAPON_RCP120) {
		hand = &player->hands[HAND_RIGHT];

		// I think this is handling situations where the player has turned off
		// RCP120 cloak but there's still a bit of ammo to be subtracted on
		// this tick.
		if (hand->matmot1 > 1.0f) {
			int usedqty = hand->matmot1;

			if (usedqty > hand->loadedammo[0]) {
				usedqty = hand->loadedammo[0];
			}

			hand->matmot1 -= usedqty;
			hand->loadedammo[0] -= usedqty;

			if (hand->matmot1 > 1.0f) {
				int usedqty = hand->matmot1;

				if (usedqty > player->ammoheldarr[player->gunctrl.ammotypes[0]]) {
					usedqty = player->ammoheldarr[player->gunctrl.ammotypes[0]];
				}

				player->ammoheldarr[player->gunctrl.ammotypes[0]] -= usedqty;
				hand->matmot1 = 0;
			}
		}
	}

	bgunTickUnequippedReload();
	bgun0f0a5550(HAND_RIGHT);

	if (player->hands[HAND_LEFT].inuse) {
		bgun0f0a5550(HAND_LEFT);
	} else {
		player->hands[HAND_LEFT].ejectstate = EJECTSTATE_INACTIVE;
	}

	bgunIsUsingSecondaryFunction();
}

int8_t bgunFreeFireslotWrapper(int slotnum)
{
	return bgunFreeFireslot(slotnum);
}

int8_t bgunFreeFireslot(int fireslot_id)
{
	if (fireslot_id >= 0 && fireslot_id < ARRAYCOUNT(g_Fireslots)) {
		g_Fireslots[fireslot_id].endlvframe = -1;
	}

	return -1;
}

int bgunAllocateFireslot(void)
{
	int index = -1;
	int i;

	for (i = 0; i < ARRAYCOUNT(g_Fireslots); i++) {
		if (g_Fireslots[i].endlvframe < 0) {
			g_Fireslots[i].endlvframe = 0;

#if VERSION < VERSION_NTSC_1_0
			g_Fireslots[i].unk04nb = 0;
			g_Fireslots[i].unk08nb = 0;
#endif

			g_Fireslots[i].beam.age = -1;
			index = i;
			break;
		}
	}

	return index;
}

// Mismatch: Goal uses different codegen for accessing vertices
void bgunRender(Gfx **gdlptr)
{
	Gfx *gdl = *gdlptr;
	struct modelrenderdata renderdata = {NULL, true, 3}; // 10c
	struct player *player;
	int i;

	static bool renderhand = true;

	player = g_Vars.currentplayer;

	if (player->visionmode == VISIONMODE_XRAY) {
		for (i = 0; i < 2; i++) {
			if (g_Vars.currentplayer->hands[i].firedrocket) {
				g_Vars.currentplayer->hands[i].rocket = NULL;
			}
		}
		return;
	}

	gdl = viPrepareZbuf(gdl);
	gdl = viPrepareHudDraw(gdl);

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, viGetViewLeft(), viGetViewTop(),
			viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());

	gdl = viSetupProjectionWithZRange(gdl, 1.5, 1000);

	if (g_Vars.currentplayer->teleportstate != TELEPORTSTATE_INACTIVE) {
		float f2;

		f2 = player0f0bd358();

		gdl = viSetupWeaponProjection(gdl, 60, f2);
	}

	gdl = lasersightRenderBeam(gdl);

	for (i = 0; i < 2; i++) {
		struct hand *hand;
		int j;
		int alpha;
		int weaponnum; // ec
		struct modelnode *node; // e8
		uint32_t colour; // e4

		hand = player->hands + i;

		weaponnum = bgunGetWeaponNum2(i);

		if (hand->visible) {
			gdl = beamRender(gdl, &hand->beam, 0, 0);

			if (weaponHasFlag(hand->gset.weaponnum, WEAPONFLAG_00008000)) {
				gSPSetLights1(gdl++, g_GunLight);
				gSPLookAt(gdl++, camGetLookAt());
			}

			//gSPPerspNormalize(gdl++, mtx00016dcc(0, 300));

			// There is support for guns having a TV screen on them
			// but no guns have this model part so it's not used.
			node = modelGetPart(hand->gunmodel.definition, MODELPART_0010);

			if (node) {
				union modelrwdata *rwdata = modelGetNodeRwData(&hand->gunmodel, modelGetPart(hand->gunmodel.definition, MODELPART_0011));

				if (rwdata) {
					rwdata->toggle.visible = true;
				}

				gdl = tvscreenRender(&hand->gunmodel, node, &var8009cf88, gdl, 0, 1);
			}

			renderdata.gdl = gdl;
			renderdata.unk30 = 4;

			if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
				uint8_t *col = player->gunshadecol;
				uint32_t shade;
				int nvcol[4];
				int ircol[4];

				if (col[0] > col[1] && col[0] > col[2]) {
					shade = col[0];
				} else if (col[1] > col[2]) {
					shade = col[1];
				} else {
					shade = col[2];
				}

				renderdata.envcolour = (shade << 24 | shade << 16 | shade << 8) + col[3];

				if (USINGDEVICE(DEVICE_NIGHTVISION)) {
					nvcol[0] = g_NVChrHighlight;
					nvcol[1] = g_NVChrHighlight;
					nvcol[2] = g_NVChrHighlight;
					nvcol[3] = g_NVChrBrightness;

					colour = (nvcol[0] << 24 | nvcol[1] << 16 | nvcol[2] << 8) + nvcol[3];
				} else if (USINGDEVICE(DEVICE_IRSCANNER)) {
					ircol[0] = 0xff;
					ircol[1] = 0;
					ircol[2] = 0;
					ircol[3] = 0x80;

					colour = (ircol[0] << 24 | ircol[1] << 16 | ircol[2] << 8) + ircol[3];
				}

				if (weaponnum == WEAPON_UNARMED) {
					renderdata.envcolour = colour;
				}
			} else {
				renderdata.envcolour = player->gunshadecol[0] << 24 | player->gunshadecol[1] << 16 | player->gunshadecol[2] << 8 | player->gunshadecol[3];
				colour = renderdata.envcolour;

				// 838
				if (hand->gset.weaponnum == WEAPON_MAULER) {
					uint32_t weight = hand->matmot1 * 50.0f;
					renderdata.envcolour = colourBlend(0xff00007f, renderdata.envcolour, weight);
				}
			}

			// Apply transparency based on player's cloak
			alpha = chrGetCloakAlpha(player->prop->chr);

			if (alpha < 255) {
				colour = (int) (alpha * 0.74509805f) + 0x41;
				renderdata.unk30 = 5;
				renderdata.fogcolour = renderdata.envcolour;
				renderdata.envcolour = colour;
			}

			renderdata.zbufferenabled = true;

			mtx00016760();

			// Render rocket launcher's rocket if it's in Jo's hand or in the launcher
			if (hand->rocket) {
				struct model *rocketmodel = hand->rocket->base.model; // 98

				if (rocketmodel && rocketmodel->definition) {
					modelRender(&renderdata, rocketmodel);
					mtxF2LBulk(rocketmodel->matrices, rocketmodel->definition->nummatrices);

					if (hand->firedrocket) {
						hand->rocket = NULL;
					}
				}
			}

			if (weaponHasFlag(weaponnum, WEAPONFLAG_DUALFLIP)) {
				gSPClearGeometryMode(renderdata.gdl++, G_CULL_BOTH);

				if (i == HAND_RIGHT) {
					renderdata.cullmode = CULLMODE_BACK;
				} else {
					renderdata.cullmode = CULLMODE_FRONT;
				}
			}

			// Slide the laser's liquid texture
			if (PLAYERCOUNT() == 1) {
				node = modelGetPart(hand->gunmodel.definition, MODELPART_GUN_LASERLIQUID);

				if (node) {
					struct modelrodata_gundl *rodata;
					rodata = &node->rodata->gundl;

					for (j = 0; j < rodata->numvertices; j++) {
						int k;

						(rodata->vertices + j)->t -= g_Vars.lvupdate240 * PALUP(25);

						if ((rodata->vertices + j)->t < -0x6000) {
							for (k = 0; k < rodata->numvertices; k++) {
								(rodata->vertices + k)->t += 0x2000;
							}
						}
					}
				}
			}

			// Render the gun
			modelRender(&renderdata, &hand->gunmodel);

			// Render the hand
			if (player->gunctrl.handmodeldef && renderhand) {
				int prevcolour = renderdata.envcolour; // 7c

				hand->handmodel.matrices = hand->gunmodel.matrices;

				modelUpdateRelations(&hand->handmodel);

				renderdata.envcolour = colour;
				modelRender(&renderdata, &hand->handmodel);
				renderdata.envcolour = prevcolour;
			}

			// Clean up
			gdl = renderdata.gdl;

			if (weaponHasFlag(weaponnum, WEAPONFLAG_DUALFLIP)) {
				gSPClearGeometryMode(gdl++, G_CULL_BOTH);
			}

			mtxF2LBulk(hand->gunmodel.matrices, hand->gunmodel.definition->nummatrices);
			mtx00016784();
		}
	}

	casingsRender(&gdl);

	gdl = zbufConfigureRdp(gdl);
	gdl = viPrepareHudDraw(gdl);

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, viGetViewLeft(), viGetViewTop(),
			viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());

	*gdlptr = gdl;
}

/**
 * Find and return an available audio handle out of a pool of four.
 */
struct sndstate **bgunAllocateAudioHandle(void)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_BgunAudioHandles); i++) {
		if (g_BgunAudioHandles[i] == NULL) {
			return &g_BgunAudioHandles[i];
		}
	}

	return NULL;
}

void bgunPlayPropHitSound(struct gset *gset, struct prop *prop, int texturenum)
{
	uint32_t rand1 = rngRandom();
	uint32_t rand2 = rngRandom();
	struct sndstate **handle;

	if (g_Vars.lvupdate240 <= 0) {
		return;
	}

	if (texturenum >= 0 && texturenum < NUM_TEXTURES
			&& g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]->numsounds == 0) {
		return;
	}

	if (gset->weaponnum == WEAPON_REMOTEMINE
			|| gset->weaponnum == WEAPON_PROXIMITYMINE
			|| gset->weaponnum == WEAPON_TIMEDMINE
			|| gset->weaponnum == WEAPON_COMMSRIDER
			|| gset->weaponnum == WEAPON_TRACERBUG
			|| gset->weaponnum == WEAPON_TARGETAMPLIFIER
			|| gset->weaponnum == WEAPON_ECMMINE) {
		psCreate(NULL, prop, SFX_80AA, -1, -1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
		return;
	}

	handle = bgunAllocateAudioHandle();

	if (handle) {
		if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
			struct chrdata *chr = prop->chr;
			int16_t soundnum = -1;
			bool overridden = false;
			int vol;
			int pan;

			if (chrGetShield(chr) > 0) {
				soundnum = SFX_SHIELD_DAMAGE;
			} else if (gset->weaponnum == WEAPON_COMBATKNIFE
					|| gset->weaponnum == WEAPON_COMBATKNIFE // duplicate
					|| gset->weaponnum == WEAPON_BOLT) {
				soundnum = SFX_05F6;
				overridden = true;
			} else if (gset->weaponnum == WEAPON_UNARMED
					|| (gset->weaponfunc == FUNC_SECONDARY
						&& (gset->weaponnum == WEAPON_FALCON2
							|| gset->weaponnum == WEAPON_FALCON2_SILENCER
							|| gset->weaponnum == WEAPON_FALCON2_SCOPE
							//|| gset->weaponnum == WEAPON_FALCON2_SANDS
							|| gset->weaponnum == WEAPON_DY357MAGNUM
							|| gset->weaponnum == WEAPON_DY357LX))) {
				int16_t sounds[] = { SFX_002F, SFX_0030, SFX_0031 };
				soundnum = sounds[rand1 % ARRAYCOUNT(sounds)];
			} else {
				int16_t sounds[] = { SFX_HIT_CHR, SFX_HIT_CHR };
				soundnum = sounds[rand1 % ARRAYCOUNT(sounds)];
			}

			if (soundnum != -1) {
				psGetTheoreticalVolPan(&prop->pos, prop->rooms, soundnum, &vol, &pan);

				if (vol) {
					sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);

					if (*handle) {
						sndAdjust(handle, 0, vol, pan, soundnum, 1, 1, -1, 1);
					}
				}
			}

			if (overridden) {
				return;
			}
		} else {
			int16_t soundnum = -1;
			bool overridden = false;
			int vol;
			int pan;

			if (texturenum == 10000) {
				soundnum = SFX_SHIELD_DAMAGE;
			} else if (gset->weaponnum == WEAPON_LASER) {
				if (gset->weaponfunc == FUNC_PRIMARY || ((gset->unk063a % 4) == 0 && (rngRandom() % 2))) {
					if ((rngRandom() % 2) == 0) {
						soundnum = SFX_CLOAK_ON;
					} else {
						soundnum = SFX_CLOAK_OFF;
					}
				}

				overridden = true;
			} else {
				if (gset->weaponnum == WEAPON_COMBATKNIFE || gset->weaponnum == WEAPON_BOLT) {
					soundnum = SFX_HIT_METAL_8079;
					overridden = true;
				} else {
					int16_t sounds[] = {
						SFX_001B, SFX_001C, SFX_001D, SFX_001E,
						SFX_001B, SFX_001C, SFX_001D, SFX_001E,
						SFX_001B, SFX_001C, SFX_001D, SFX_001E,
						SFX_0023, SFX_0024, SFX_0025, SFX_0026,
						SFX_0027, SFX_0028, SFX_0029, SFX_002A,
					};

					soundnum = sounds[rand1 % ARRAYCOUNT(sounds)];
				}
			}

			if (soundnum != -1) {
				psGetTheoreticalVolPan(&prop->pos, prop->rooms, soundnum, &vol, &pan);

				if (vol) {
					sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);

					if (*handle) {
						sndAdjust(handle, 0, vol, pan, soundnum, 1, 1, -1, 1);
					}
				}
			}

			if (overridden) {
				return;
			}
		}
	}

	if (texturenum >= 0 && texturenum < NUM_TEXTURES && g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]) {
		int16_t soundnum = -1;

		handle = bgunAllocateAudioHandle();

		if (handle) {
			if (g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]->numsounds > 0) {
				int index = rand2 % g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]->numsounds;
				soundnum = g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]->sounds[index];

				if (soundnum != -1) {
					sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
				}
			}

			if (*handle && soundnum != -1) {
				psApplyVolPan(*handle, &prop->pos, 400, 2500, 3000, prop->rooms, soundnum, AL_VOL_FULL, 0);
			}
		}
	}
}

void bgunPlayGlassHitSound(struct coord *pos, RoomNum *rooms, int texturenum)
{
	if (g_Vars.lvupdate240 > 0) {
		struct sndstate **handle = bgunAllocateAudioHandle();

		if (handle) {
			sndStart(var80095200, SFX_HIT_GLASS, handle, -1, -1, -1, -1, -1);

			if (*handle) {
				psApplyVolPan(*handle, pos, 400, 2500, 3000, rooms, SFX_HIT_GLASS, AL_VOL_FULL, 0);
			}
		}
	}
}

void bgunPlayBgHitSound(struct gset *gset, struct coord *hitpos, int texturenum, RoomNum *rooms)
{
	struct sndstate **handle;
	uint32_t rand1 = rngRandom();
	uint32_t rand2 = rngRandom();
	bool playdefault;
	int16_t soundnum;
	bool overridden;

	if (g_Vars.lvupdate240 <= 0) {
		return;
	}

	if (texturenum >= 0 && texturenum < NUM_TEXTURES && g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype]->numsounds == 0) {
		return;
	}

	playdefault = true;
	handle = bgunAllocateAudioHandle();

	if (handle) {
		soundnum = -1;
		overridden = false;

		if (gset->weaponnum == WEAPON_LASER) {
			playdefault = false;

			if (gset->weaponfunc == FUNC_PRIMARY || ((gset->unk063a % 4) == 0 && (rngRandom() % 2))) {
				// Laser sounds
				int16_t sounds[] = {SFX_CLOAK_ON, SFX_CLOAK_OFF};
				soundnum = sounds[rand1 % ARRAYCOUNT(sounds)];
				sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
				overridden = true;
			}
		} else if (gset->weaponnum == WEAPON_COMBATKNIFE || gset->weaponnum == WEAPON_BOLT) {
			// Knives and bolts make a metal sound
			soundnum = SFX_HIT_METAL_8079;
			sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
			overridden = true;
		} else if (gset->weaponnum == WEAPON_REMOTEMINE
				|| gset->weaponnum == WEAPON_PROXIMITYMINE
				|| gset->weaponnum == WEAPON_TIMEDMINE
				|| gset->weaponnum == WEAPON_COMMSRIDER
				|| gset->weaponnum == WEAPON_TRACERBUG
				|| gset->weaponnum == WEAPON_TARGETAMPLIFIER
				|| gset->weaponnum == WEAPON_ECMMINE) {
			// Mine landing/activation sound
			soundnum = SFX_80AA;
			sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
			overridden = true;
		} else {
			// Ricochet sounds
			int16_t sounds[] = {
				SFX_0013, SFX_0014, SFX_0015, SFX_0016,
				SFX_0017, SFX_0018, SFX_0019, SFX_001A,
				SFX_0017, SFX_0018, SFX_0019, SFX_001A,
				SFX_0017, SFX_0018, SFX_0019, SFX_001A,
				SFX_001F, SFX_0020, SFX_0020, SFX_0021,
				SFX_001F, SFX_0020, SFX_0020, SFX_0021,
				SFX_001F, SFX_0020, SFX_0020, SFX_0021,
				SFX_0023, SFX_0024, SFX_0025, SFX_0026,
				SFX_0027, SFX_0028, SFX_0029, SFX_002A,
			};

			soundnum = sounds[rand1 % ARRAYCOUNT(sounds)];
			sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
			overridden = false;
		}

		if (*handle != NULL) {
			psApplyVolPan(*handle, hitpos, 400, 2500, 3000, rooms, soundnum, AL_VOL_FULL, 0);
		}

		if (overridden) {
			return;
		}
	}

	if (playdefault) {
		handle = bgunAllocateAudioHandle();

		if (handle != NULL && texturenum >= 0 && texturenum < NUM_TEXTURES) {
			int16_t soundnum;
			struct surfacetype *type = g_SurfaceTypes[g_Textures[texturenum].soundsurfacetype];

			if (type->numsounds > 0) {
				soundnum = -1;

				if (type != NULL) {
					int index = rand2 % type->numsounds;
					soundnum = type->sounds[index];
					sndStart(var80095200, soundnum, handle, -1, -1, -1, -1, -1);
				}

				if (*handle != NULL) {
					psApplyVolPan(*handle, hitpos, 400, 2500, 3000, rooms, soundnum, AL_VOL_FULL, 0);
				}
			}
		}
	}
}

void bgunSetTriggerOn(int handnum, bool on)
{
	struct hand *hand = &g_Vars.currentplayer->hands[handnum];

	hand->triggerprev = hand->triggeron;
	hand->triggeron = on;

	if (!on) {
		hand->triggerreleased = true;
	}
}

#define SETFUNCPRI() g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].gunfuncs[(g_Vars.currentplayer->gunctrl.weaponnum - 1) >> 3] &= ~(1 << ((g_Vars.currentplayer->gunctrl.weaponnum - 1) & 7))
#define SETFUNCSEC() g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].gunfuncs[(g_Vars.currentplayer->gunctrl.weaponnum - 1) >> 3] |= 1 << ((g_Vars.currentplayer->gunctrl.weaponnum - 1) & 7)

/**
 * This is called once B has been held for 25 ticks, or earlier if pressing B+Z.
 *
 * The function may choose whether to change the gun function,
 * or activate the secondary function without switching to it.
 *
 * Return the following;
 * - USETIMER_CONTINUE if the B button timer should continue incrementing.
 * - USETIMER_STOP if the B button timer should stop (ie. the B press is consumed)
 * - USETIMER_REPEAT if this function should be called again on each frame until B is released.
 */
int bgunConsiderToggleGunFunction(int usedowntime, bool trigpressed, bool fromactivemenu, bool fromdedicatedbutton)
{
	const bool extcontrols = PLAYER_EXTCFG().extcontrols;
	bool docontinue;
	switch (bgunGetWeaponNum(HAND_RIGHT)) {
	case WEAPON_RCP120:
		// very special alt-button handling for RCP-120's cloaking
		if (!trigpressed && extcontrols && fromdedicatedbutton) {
			if (g_Vars.currentplayer->devicesactive & DEVICE_CLOAKRCP120) {
				g_Vars.currentplayer->devicesactive &= ~DEVICE_CLOAKRCP120;
			} else {
				g_Vars.currentplayer->devicesactive = (g_Vars.currentplayer->devicesactive & ~DEVICE_CLOAKRCP120) | DEVICE_CLOAKRCP120;
			}
			return USETIMER_STOP;
		}
	case WEAPON_LAPTOPGUN:
	case WEAPON_DRAGON:
	case WEAPON_REMOTEMINE:
		// These weapons use temporary alt functions
		if (extcontrols) {
			g_Vars.currentplayer->gunctrl.invertgunfunc = !g_Vars.currentplayer->gunctrl.invertgunfunc;
		} else {
			g_Vars.currentplayer->gunctrl.invertgunfunc = true;
		}

		if (fromactivemenu && bgunIsUsingSecondaryFunction() == true) {
			g_Vars.currentplayer->hands[HAND_RIGHT].activatesecondary = true;
		}

		return USETIMER_STOP;
	case WEAPON_MAULER:
	case WEAPON_CMP150:
	case WEAPON_K7AVENGER:
	case WEAPON_AR34:
	case WEAPON_FARSIGHT:
	case WEAPON_TIMEDMINE:
		// These weapons disallow B+Z
		if (!trigpressed) {
			if (VALIDWEAPON()) {
				if (1 - FUNCISSEC()) {
					SETFUNCSEC();
				} else {
					SETFUNCPRI();
				}
			}

			return USETIMER_STOP;
		}

		return USETIMER_CONTINUE;
	default:
		if (trigpressed) {
			g_Vars.currentplayer->gunctrl.invertgunfunc = true;
		} else {
			if (VALIDWEAPON()) {
				if (!FUNCISSEC()) {
					SETFUNCSEC();
				} else {
					SETFUNCPRI();
				}
			}
		}

		return USETIMER_STOP;
	}
}

void bgunDisallowInvertFunc(void)
{
	switch (bgunGetWeaponNum(HAND_RIGHT)) {
	case WEAPON_RCP120:
	case WEAPON_LAPTOPGUN:
	case WEAPON_DRAGON:
	case WEAPON_REMOTEMINE:
		if (PLAYER_EXTCFG().extcontrols) {
			return;
		}
		break;
	}
	if (g_Vars.currentplayer->hands[HAND_RIGHT].activatesecondary == false) {
		g_Vars.currentplayer->gunctrl.invertgunfunc = false;
	}
}

bool bgunIsUsingSecondaryFunction(void)
{
	struct player *player = g_Vars.currentplayer;
	int weaponnum = player->gunctrl.weaponnum;

	if(weaponnum == WEAPON_RCP120)
	{
		if(g_Vars.currentplayer->devicesactive & DEVICE_CLOAKRCP120) {
			return true;
		}
		else {
			return false;
		}
	}

	if (weaponnum >= WEAPON_UNARMED && weaponnum <= WEAPON_COMBATBOOST) {
		int index = (weaponnum - 1) >> 3;
		int value = 1 << ((weaponnum - 1) & 7);

		if (g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].gunfuncs[index] & value) {
			if (player->gunctrl.invertgunfunc == true) {
				return false;
			}

			return true;
		}
	}

	if (player->gunctrl.invertgunfunc == true) {
		return true;
	}

	return false;
}

/**
 * Tick gun-related things during first-person gameplay.
 *
 * This function is not called during cutscenes.
 */
void bgunTickGameplay(bool triggeron)
{
	int gunsfiring[2] = {false, false};
	struct player *player = g_Vars.currentplayer;
	int i;

	// Remove weapons if in passive mode
	if (g_Vars.currentplayer->gunctrl.passivemode) {
		struct chrdata *chr = g_Vars.currentplayer->prop->chr;
		triggeron = false;

		if (invGetCount() > 1) {
			invClear();
			invGiveSingleWeapon(WEAPON_UNARMED);
		}

		if (g_Vars.currentplayer->gunctrl.weaponnum != WEAPON_UNARMED
				&& g_Vars.currentplayer->gunctrl.switchtoweaponnum != WEAPON_UNARMED) {
			bgunEquipWeapon(WEAPON_UNARMED);
		}

		g_Vars.currentplayer->gunctrl.dualwielding = false;
		g_Vars.currentplayer->devicesactive = 0;

		chr->cloakpause = 0;
		chr->cloakfadefrac = 0;
		chr->cloakfadefinished = false;
		chr->hidden &= ~CHRHFLAG_CLOAKED;
	}

	// Remove throwable items from inventory if there's no more left
	for (i = 0; i < invGetCount(); i++) {
		struct weapon *weapon;
		int weaponnum = invGetWeaponNumByIndex(i);
		int equippedweaponnum;

		switch (weaponnum) {
		case WEAPON_COMBATKNIFE:
		case WEAPON_GRENADE:
		case WEAPON_NBOMB:
		case WEAPON_COMBATBOOST:
		case WEAPON_CLOAKINGDEVICE:
		case WEAPON_ECMMINE:
		case WEAPON_COMMSRIDER:
		case WEAPON_TRACERBUG:
		case WEAPON_TARGETAMPLIFIER:
			weapon = weaponFindById(weaponnum);

			if (weapon && weapon->ammos[0]
					&& bgunGetAmmoCount(weapon->ammos[0]->type) == 0) {
				equippedweaponnum = bgunGetWeaponNum(HAND_RIGHT);
				invRemoveItemByNum(weaponnum);

				if (weaponnum == equippedweaponnum && !invHasSingleWeaponIncAllGuns(weaponnum)) {
					invCalculateCurrentIndex();
					bgunEquipWeapon(invGetWeaponNumByIndex(g_Vars.currentplayer->equipcuritem));
				}
			}
		}
	}

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		triggeron = false;
		g_Vars.currentplayer->hands[HAND_LEFT].firing = false;
		g_Vars.currentplayer->hands[HAND_RIGHT].firing = false;
	}

	player->playertriggerprev = player->playertriggeron;
	player->playertriggeron = triggeron;

	if (triggeron == false && player->playertriggerprev) {
		// Releasing trigger
		player->doautoselect = true;
	}

	// Handle gun firing - particularly alternating
	// between left and right if dual wielding
	if (player->playertriggeron) {
		player->playertrigtime240 += g_Vars.lvupdate240;

		if (player->hands[HAND_LEFT].inuse
				&& player->hands[HAND_RIGHT].inuse
				&& player->gunctrl.weaponnum != WEAPON_REMOTEMINE) {
			if (player->playertrigtime240 > TICKS(80)) {
				gunsfiring[player->curguntofire] = 1;

				if (bgunDetermineReloadTypeByHand(1 - player->curguntofire)
						|| player->hands[1 - player->curguntofire].triggeron) {
					gunsfiring[1 - player->curguntofire] = 1;
				}
			} else {
				if (player->playertriggerprev == false &&
						(bgunDetermineReloadTypeByHand(1 - player->curguntofire) || !bgunDetermineReloadTypeByHand(player->curguntofire))) {
					player->curguntofire = 1 - player->curguntofire;
				}

				gunsfiring[player->curguntofire] = 1;
				gunsfiring[1 - player->curguntofire] = 0;
			}
		} else {
			if (!player->hands[player->curguntofire].inuse
					&& player->hands[1 - player->curguntofire].inuse) {
				player->curguntofire = 1 - player->curguntofire;
			}

			if (player->gunctrl.weaponnum == WEAPON_REMOTEMINE) {
				player->curguntofire = 0;
			}

			gunsfiring[player->curguntofire] = 1;
			gunsfiring[1 - player->curguntofire] = 0;
		}
	} else {
		player->playertrigtime240 = 0;
	}

	bgunSetTriggerOn(HAND_RIGHT, gunsfiring[0]);
	bgunSetTriggerOn(HAND_LEFT, gunsfiring[1]);

	if (g_Vars.tickmode == TICKMODE_NORMAL && g_Vars.lvupdate240 > 0) {
		bgunTickHand(HAND_RIGHT);
		bgunTickHand(HAND_LEFT);
		bgunTickSwitch();

		if (cheatIsActive(CHEAT_UNLIMITEDAMMONORELOADS)) {
			int i;
			struct weapon *weapon;
			struct hand *lhand = &g_Vars.currentplayer->hands[HAND_LEFT];
			struct hand *rhand = &g_Vars.currentplayer->hands[HAND_RIGHT];

			weapon = weaponFindById(rhand->gset.weaponnum);

			for (i = 0; i != 2; i++) {
				if (weapon && weapon->ammos[i] &&
						bgunAmmotypeAllowsUnlimitedAmmo(weapon->ammos[i]->type)) {
					rhand->loadedammo[i] = rhand->clipsizes[i];
					lhand->loadedammo[i] = lhand->clipsizes[i];
				}
			}

			bgunGiveMaxAmmo(false);
		} else if (cheatIsActive(CHEAT_UNLIMITEDAMMO)) {
			bgunGiveMaxAmmo(false);
		}
	}

	bgunDecreaseNoiseRadius();

	if (player->resetshadecol) {
		propCalculateShadeColour(g_Vars.currentplayer->prop, player->gunshadecol, player->floorcol);
		player->resetshadecol = 0;
	} else {
		uint8_t shadecol[4];
		propCalculateShadeColour(g_Vars.currentplayer->prop, shadecol, player->floorcol);
		colourTween(player->gunshadecol, shadecol);
	}

	invIncrementHeldTime(bgunGetWeaponNum(HAND_RIGHT), bgunGetWeaponNum(HAND_LEFT));
}

void bgunSetPassiveMode(bool enable)
{
	int i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		g_Vars.players[i]->gunctrl.passivemode = enable;
	}
}

void bgunSetAimType(uint32_t aimtype)
{
	g_Vars.currentplayer->aimtype = aimtype;
}

void bgunSetAimPos(struct coord *coord)
{
	struct player *player = g_Vars.currentplayer;

	player->hands[HAND_RIGHT].aimpos.x = handGetXShift(HAND_RIGHT) + coord->x;
	player->hands[HAND_RIGHT].aimpos.y = coord->y;
	player->hands[HAND_RIGHT].aimpos.z = coord->z;

	player->hands[HAND_LEFT].aimpos.x = handGetXShift(HAND_LEFT) + coord->x;
	player->hands[HAND_LEFT].aimpos.y = coord->y;
	player->hands[HAND_LEFT].aimpos.z = coord->z;
}

void bgunSetHitPos(struct coord *coord)
{
	struct player *player = g_Vars.currentplayer;

	player->hands[HAND_LEFT].hitpos.x = player->hands[HAND_RIGHT].hitpos.x = coord->x;
	player->hands[HAND_LEFT].hitpos.y = player->hands[HAND_RIGHT].hitpos.y = coord->y;
	player->hands[HAND_LEFT].hitpos.z = player->hands[HAND_RIGHT].hitpos.z = coord->z;
}

void bgun0f0a9494(uint32_t operation)
{
	switch (operation) {
	case 0:
		g_Vars.currentplayer->hands[HAND_LEFT].hasdotinfo = g_Vars.currentplayer->hands[HAND_RIGHT].hasdotinfo = false;
		break;
	case 1:
		break;
	}
}

void bgun0f0a94d0(uint32_t operation, struct coord *pos, struct coord *rot)
{
	struct player *player = g_Vars.currentplayer;

	switch (operation) {
	case 0:
		if (pos->x > -100000.0f && pos->x < 100000.0f
				&& pos->y > -100000.0f && pos->y < 100000.0f
				&& pos->z > -100000.0f && pos->z < 100000.0f) {
			player->hands[HAND_RIGHT].hasdotinfo = true;
			player->hands[HAND_LEFT].hasdotinfo = true;

			player->hands[HAND_LEFT].dotpos.x = player->hands[HAND_RIGHT].dotpos.x = pos->x;
			player->hands[HAND_LEFT].dotpos.y = player->hands[HAND_RIGHT].dotpos.y = pos->y;
			player->hands[HAND_LEFT].dotpos.z = player->hands[HAND_RIGHT].dotpos.z = pos->z;

			player->hands[HAND_LEFT].dotrot.x = player->hands[HAND_RIGHT].dotrot.x = rot->x;
			player->hands[HAND_LEFT].dotrot.y = player->hands[HAND_RIGHT].dotrot.y = rot->y;
			player->hands[HAND_LEFT].dotrot.z = player->hands[HAND_RIGHT].dotrot.z = rot->z;
		}
		break;
	case 1:
	case 2:
		lasersightSetDot(operation - 1, pos, rot);
		break;
	}
}

void bgunSetGunAmmoVisible(uint32_t reason, bool enable)
{
	if (enable) {
		g_Vars.currentplayer->gunammooff &= ~reason;
	} else {
		g_Vars.currentplayer->gunammooff |= reason;
	}
}

struct ammotype g_AmmoTypes[] = {
	{ 0,            0, 0  },
	{ 800,          0, 0  }, // AMMOTYPE_PISTOL
	{ 800,          0, 0  }, // AMMOTYPE_SMG
	{ 69,           0, 0  }, // AMMOTYPE_CROSSBOW
	{ 400,          0, -2 }, // AMMOTYPE_RIFLE
	{ 100,          0, 0  }, // AMMOTYPE_SHOTGUN
	{ 100,          0, 0  }, // AMMOTYPE_FARSIGHT
	{ 12,           0, 0  }, // AMMOTYPE_GRENADE
	{ 3,            0, -2 }, // AMMOTYPE_ROCKET
	{ 10,           0, 0  }, // AMMOTYPE_KNIFE
	{ 200,          0, 0  }, // AMMOTYPE_MAGNUM
	{ 40,           0, 0  }, // AMMOTYPE_DEVASTATOR
	{ 10,           0, 1  }, // AMMOTYPE_REMOTE_MINE
	{ 10,           0, 1  }, // AMMOTYPE_PROXY_MINE
	{ 10,           0, 1  }, // AMMOTYPE_TIMED_MINE
	{ 800,          0, 0  }, // AMMOTYPE_REAPER
	{ 15,           0, -2 }, // AMMOTYPE_HOMINGROCKET
	{ 50,           0, 0  }, // AMMOTYPE_DART
	{ 10,           0, 0  }, // AMMOTYPE_NBOMB
	{ 200,          0, 0  }, // AMMOTYPE_SEDATIVE
	{ TICKS(18000), 0, 0  }, // AMMOTYPE_CLOAK
	{ 4,            0, 0  }, // AMMOTYPE_BOOST
	{ 200,          0, 0  }, // AMMOTYPE_PSYCHOSIS
	{ 2,            0, 0  }, // AMMOTYPE_17
	{ 10,           0, 0  }, // AMMOTYPE_BUG
	{ 10,           0, 0  }, // AMMOTYPE_MICROCAMERA
	{ 10,           0, 0  }, // AMMOTYPE_PLASTIQUE
	{ 1000,         0, 0  }, // AMMOTYPE_1B
	{ 10,           0, 0  }, // AMMOTYPE_1C
	{ 50,           0, -1 }, // AMMOTYPE_1D
	{ 1,            0, 0  }, // AMMOTYPE_TOKEN
	{ 200,          0, 0  }, // AMMOTYPE_1F
	{ 10,           0, 0  }, // AMMOTYPE_ECM_MINE
};

void bgunSetAmmoQuantity(int ammotype, int quantity)
{
	struct player *player = g_Vars.currentplayer;
	int weaponnum = bgunGetWeaponNum(HAND_RIGHT);
	int funcnum = -1;
	int magamount;

	// Check if this ammo type applies to the player's equipped weapon
	if (bgunGetAmmoTypeForWeapon(weaponnum, FUNC_PRIMARY) == ammotype) {
		funcnum = FUNC_PRIMARY;
	}

	if (bgunGetAmmoTypeForWeapon(weaponnum, FUNC_SECONDARY) == ammotype) {
		funcnum = FUNC_SECONDARY;
	}

	if (funcnum != -1 && weaponHasAmmoFlag(weaponnum, funcnum, AMMOFLAG_NORESERVE)) {
		// For cloak and combat boost, ammo cannot be held outside of the weapon.
		// So just add it to the loaded clip.
		player->hands[0].loadedammo[funcnum] += quantity;

		if (player->hands[0].loadedammo[funcnum] > player->hands[0].clipsizes[funcnum]) {
			player->hands[0].loadedammo[funcnum] = player->hands[0].clipsizes[funcnum];
		}

		player->ammoheldarr[ammotype] = 0;
		return;
	}

	magamount = 0;

	// For throwable items, the capacity applies to reserve + loaded
	if (funcnum != -1 && weaponHasAmmoFlag(weaponnum, funcnum, AMMOFLAG_EQUIPPEDISRESERVE)) {
		magamount = player->hands[0].loadedammo[funcnum] + player->hands[1].loadedammo[funcnum];
	}

	if (quantity > g_AmmoTypes[ammotype].capacity - magamount) {
		player->ammoheldarr[ammotype] = g_AmmoTypes[ammotype].capacity - magamount;
	} else {
		player->ammoheldarr[ammotype] = quantity;
	}
}

int bgunGetReservedAmmoCount(int ammotype)
{
	int i;
	int j;
	int total = g_Vars.currentplayer->ammoheldarr[ammotype];
	struct player *player = g_Vars.currentplayer;

	for (i = 0; i < 2; i++) {
		if (player->hands[i].inuse) {
			for (j = 0; j < 2; j++) {
				if (player->gunctrl.ammotypes[j] == ammotype && weaponHasAmmoFlag(player->hands[i].gset.weaponnum, j, AMMOFLAG_NORESERVE)) {
					total = total + player->hands[i].loadedammo[j];
				}
			}
		}
	}

	return total;
}

int bgunGetAmmoCount(int ammotype)
{
	int i;
	int j;
	int total = g_Vars.currentplayer->ammoheldarr[ammotype];
	struct player *player = g_Vars.currentplayer;

	for (i = 0; i < 2; i++) {
		if (player->hands[i].inuse) {
			for (j = 0; j < 2; j++) {
				if (player->gunctrl.ammotypes[j] == ammotype) {
					total = total + player->hands[i].loadedammo[j];
				}
			}
		}
	}

	return total;
}

int bgunGetCapacityByAmmotype(int ammotype)
{
	return g_AmmoTypes[ammotype].capacity;
}

bool bgunAmmotypeAllowsUnlimitedAmmo(uint32_t ammotype)
{
	switch (ammotype) {
	case AMMOTYPE_REMOTE_MINE:
		if (g_Vars.stagenum == STAGE_CHICAGO) {
			return false;
		}
		break;
	case AMMOTYPE_TIMED_MINE:
		if (g_Vars.stagenum == STAGE_AIRFORCEONE) {
			return false;
		}
		break;
	case AMMOTYPE_PSYCHOSIS:
	case AMMOTYPE_17:
	case AMMOTYPE_BUG:
	case AMMOTYPE_MICROCAMERA:
	case AMMOTYPE_PLASTIQUE:
	case AMMOTYPE_1B:
	case AMMOTYPE_1C:
	case AMMOTYPE_1D:
	case AMMOTYPE_TOKEN:
	case AMMOTYPE_1F:
	case AMMOTYPE_ECM_MINE:
		return false;
	}

	return true;
}

void bgunGiveMaxAmmo(bool force)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_AmmoTypes); i++) {
		bool give = true;

		if (!force) {
			give = bgunAmmotypeAllowsUnlimitedAmmo(i);
		}

		if (give) {
			bgunSetAmmoQuantity(i, g_AmmoTypes[i].capacity);
		}
	}
}

uint32_t bgunGetAmmoTypeForWeapon(uint32_t weaponnum, uint32_t func)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (!weapon) {
		return 0;
	}

	if (!weapon->ammos[func]) {
		return 0;
	}

	return weapon->ammos[func]->type;
}

int bgunGetAmmoQtyForWeapon(uint32_t weaponnum, uint32_t func)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (weapon) {
		struct inventory_ammo *ammo = weapon->ammos[func];

		if (ammo) {
			return bgunGetReservedAmmoCount(ammo->type);
		}
	}

	return 0;
}

void bgunSetAmmoQtyForWeapon(uint32_t weaponnum, uint32_t func, uint32_t quantity)
{
	struct weapon *weapon = weaponFindById(weaponnum);

	if (weapon) {
		struct inventory_ammo *ammo = weapon->ammos[func];

		if (ammo) {
			bgunSetAmmoQuantity(ammo->type, quantity);
		}
	}
}

int bgunGetAmmoCapacityForWeapon(int weaponnum, int func)
{
	struct weapon *weapon = weaponFindById(weaponnum);
	struct inventory_ammo *ammo = weapon->ammos[func];

	if (ammo) {
		return g_AmmoTypes[ammo->type].capacity;
	}

	return 0;
}

Gfx *bgunDrawHudString(Gfx *gdl, char *text, int x, bool halign, int y, int valign, uint32_t colour)
{
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
	int textheight;
	int textwidth;

	textwidth = 0;
	textheight = 0;

	textMeasure(&textheight, &textwidth, text, g_CharsNumeric, g_FontNumeric, 0);

	if (halign == HUDHALIGN_LEFT) { // left
		x2 = x + textwidth;
		x1 = x;
	} else if (halign == HUDHALIGN_RIGHT) { // right
		x1 = x - textwidth;
		x2 = x;
	} else if (halign == HUDHALIGN_MIDDLE) { // middle
		x2 = x + textwidth / 2;
		x1 = x2 - textwidth;
	}

	if (valign == HUDVALIGN_TOP) { // top
		y2 = y + textheight;
		y1 = y;
	} else if (valign == HUDVALIGN_BOTTOM) { // bottom
		y1 = y - textheight;
		y2 = y;
	} else if (valign == HUDVALIGN_MIDDLE) { // middle
		y2 = y + textheight / 2;
		y1 = y2 - textheight;
	}

	gdl = text0f153858(gdl, &x1, &y1, &x2, &y2);
	gdl = textRender(gdl, &x1, &y1, text, g_CharsNumeric, g_FontNumeric, colour, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);

	return gdl;
}

Gfx *bgunDrawHudInteger(Gfx *gdl, int value, int x, bool halign, int y, int valign, uint32_t colour)
{
	char buffer[12];

	sprintf(buffer, "%d\n", value);
	gdl = bgunDrawHudString(gdl, buffer, x, halign, y, valign, colour);

	return gdl;
}

void bgunResetAbmag(struct abmag *abmag)
{
	abmag->loadedammo = 0;
	abmag->change = 0;
	abmag->ref = 0;
	abmag->timer60 = 0;
}

void bgun0f0a9da8(struct abmag *mag, int remaining, int capacity, int height)
{
	int newchange;

	if (capacity > 20) {
		int newremaining = height * remaining / capacity;

		if (remaining > 0 && newremaining < 1) {
			newremaining = 1;
		}

		capacity = height;

		if (newremaining == mag->ref && mag->loadedammo > remaining) {
			mag->ref++;
		}

		mag->loadedammo = remaining;
		remaining = newremaining;
	}

	newchange = remaining - mag->ref;

	if ((mag->change < 0 && newchange > 0) || (mag->change > 0 && newchange < 0)) {
		mag->ref += mag->change;
		mag->change = 0;
		mag->timer60 = 0;

		newchange = remaining - mag->ref;
	}

	if (mag->change < 0 && mag->change > newchange) {
		if (mag->timer60 > -mag->change * TICKS(64)) {
			mag->timer60 = -mag->change * TICKS(64);
		}
	}

	mag->change = newchange;

	if (mag->change > 0) {
		height = capacity;

		if (capacity < 6) {
			height = 6;
		}
	} else {
		height = 8;

		if (mag->change < -3) {
			height += -mag->change * 2;
		}
	}

	if (mag->change != 0) {
		mag->timer60 += (int16_t)g_Vars.lvupdate60 * height;

		if (mag->timer60 > TICKS(255)) {
			if (mag->change > 0) {
				while (mag->timer60 > TICKS(255) && mag->change > 0) {
					mag->change--;
					mag->ref++;
					mag->timer60 -= TICKS(64);
				}
			} else {
				while (mag->timer60 > TICKS(255) && mag->change < 0) {
					mag->change++;
					mag->ref--;
					mag->timer60 -= TICKS(64);
				}
			}
		}
	} else {
		mag->timer60 = 0;
	}
}

/**
 * Render an ammo gauge on the HUD.
 *
 * Ammo gauges can be displayed in two ways. If the capacity is less than 20,
 * each bullet is displayed as a separate block with a gap between each. If the
 * capacity is 20 or more, a single block covers the whole gauge and the block
 * is partitioned into two (filled and empty).
 *
 * For the separated mode, a unit refers to a single bullet/block.
 * For the merged mode, a unit refers to a single 1px high line in the gauge.
 */
Gfx *bgunDrawHudGauge(Gfx *gdl, int x1, int y1, int x2, int y2, struct abmag *abmag, int remaining, int capacity, uint32_t emptycolour, uint32_t filledcolour, bool flip)
{
	int gaugeheight = y2 - y1;
	int unitheight;
	int remainder1;
	int remainder2;
	int gaugetop;
	float ref;
	int numunits = capacity;
	int i;

	bgun0f0a9da8(abmag, remaining, numunits, gaugeheight);

	if (numunits > 20) {
		// Use a single merged bar
		ref = abmag->ref;
		unitheight = 1;
		numunits = gaugeheight;
		gaugetop = y2 - gaugeheight;
	} else {
		// Use a separate block for each bullet
		ref = abmag->ref;
		unitheight = gaugeheight / numunits;
		remainder1 = unitheight * numunits - gaugeheight;
		remainder2 = (unitheight + 1) * numunits - gaugeheight;

		if (remainder1 < 0) {
			remainder1 = -remainder1;
		}

		if (remainder2 < 0) {
			remainder2 = -remainder2;
		}

		if (remainder2 < remainder1) {
			unitheight++;
		}

		gaugetop = y2 - unitheight * capacity + 1;

		if (unitheight <= 2) {
			gaugetop--;
		}
	}

	if (unitheight == 0) {
		/**
		 * Using separate blocks, but the clip capacity is more than the gauge
		 * height meaning each block is less than 1px. This is impossible
		 * because the gauge switches modes away from separate blocks at 20,
		 * therefore this code is unreachable.
		 *
		 * This code renders the gauge in the merged style, but uses 1px per
		 * bullet and truncates the gauge at the gaugetop if needed. This is
		 * clearly an early revision of the code, as it is visually misleading
		 * and also lacks the transition effect.
		 */
		int partitiony;
		int tmp;

		gaugeheight = y2 - gaugetop;
		partitiony = y2 - gaugeheight * ref / numunits;
		tmp = y2;

		if (partitiony > gaugetop) {
			// Render empty partition
			gdl = textSetPrimColour(gdl, emptycolour);

			if (flip) {
				gDPFillRectangleScaled(gdl++, x1, y2 - partitiony + y1, x2, gaugeheight + y1);
			} else {
				gDPFillRectangleScaled(gdl++, x1, gaugetop, x2, partitiony);
			}

			gdl = textSetCCCustom02(gdl);
		}

		// Render filled partition
		gdl = textSetPrimColour(gdl, filledcolour);

		if (flip) {
			gDPFillRectangleScaled(gdl++, x1, y2 - tmp + y1, x2, y2 - partitiony + y1);
		} else {
			gDPFillRectangleScaled(gdl++, x1, partitiony, x2, y2);
		}
	} else {
		uint32_t colour;
		int unittop;
		int unitbottom;

		gdl = textSetPrimColour(gdl, emptycolour);

		unittop = gaugetop;
		unitbottom = -1;

		for (i = 0; i < numunits; i++) {
			bool newstate = false;
			uint32_t weight;

			if (abmag->change > 0) {
				// Loading or reloading
				if (i >= numunits - (int)ref - abmag->change && i < numunits - (int)ref) {
					// Unit is potentially unsettled
					int fadeamount = abmag->timer60 - (numunits - (int)ref - i - 1) * TICKS(64);

					if (fadeamount >= 0) {
						if (fadeamount >= TICKS(64)) {
							// Unit is transitioning to filled
							weight = (fadeamount * 4 - TICKS(252)) / 3;
							weight = PALUP(weight);

							if (weight > 255) {
								weight = 255;
							}

							colour = colourBlend(filledcolour, 0xffffffbf, weight);
						} else {
							// Unit is bright and has not started transitioning to filled yet
							weight = fadeamount * 4;
							weight = PALUP(weight);
							colour = colourBlend(0xffffffbf, emptycolour, weight);
						}

						newstate = true;
					}
				}
			} else if (abmag->change < 0) {
				// Firing
				if (i < numunits - (int)ref - abmag->change && i >= numunits - (int) ref) {
					int fadeamount = abmag->timer60 - (i - numunits + (int) ref) * TICKS(64);

					if (fadeamount >= 0) {
						weight = PALUP(fadeamount);

						if (weight > 255) {
							colour = emptycolour;
						} else {
							// Unit was recently emptied
							colour = colourBlend(emptycolour, filledcolour | 0xff, weight);
						}

						newstate = true;
					}
				}
			}

			// Special case for units which are one after the last one being
			// faded. I think their colour is calculated incorrectly by the code
			// above and this is resetting them to the normal filled colour.
			if (abmag->change < 0) {
				// Firing
				if (i == numunits - (int) ref - abmag->change) {
					colour = filledcolour;
					newstate = true;
				}
			} else {
				if (i == numunits - (int) ref) {
					colour = filledcolour;
					newstate = true;
				}
			}

			// Calculate unittop and unitbottom. For merged gauges keep unittop
			// as it is if possible, so the empty and filled partitions can be
			// drawn whenever the state is changed in order to save gfx calls.
			if (unitheight <= 2) {
				if (newstate) {
					if (unitbottom >= 0) {
						// Render empty or transitioning unit of merged gauge
						if (flip) {
							gDPFillRectangleScaled(gdl++, x1, y2 - unitbottom + y1, x2, y2 - unittop + y1);
						} else {
							gDPFillRectangleScaled(gdl++, x1, unittop, x2, unitbottom);
						}
					}

					unittop = gaugetop + i * unitheight;
				}

				unitbottom = gaugetop + i * unitheight + unitheight;
			} else {
				// Separate blocks - reduce unitbottom by 1 to make a gap
				unittop = gaugetop + i * unitheight;
				unitbottom = gaugetop + i * unitheight + unitheight - 1;
			}

			if (newstate) {
				gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
			}

			// For separate blocks, clip the unit bottom to the bottom of the gauge
			if (unitbottom >= y2 - 1 && unitheight >= 2) {
				unitbottom = y2;
			}

			// Render separated blocks
			if (unitheight >= 3) {
				if (flip) {
					gDPFillRectangleScaled(gdl++, x1, y2 - unitbottom + y1, x2, y2 - unittop + y1);
				} else {
					gDPFillRectangleScaled(gdl++, x1, unittop, x2, unitbottom);
				}
			}
		} // end loop

		// For merged gauges, render the final partition
		if (unitheight <= 2) {
			if (flip) {
				gDPFillRectangleScaled(gdl++, x1, y2 - unitbottom + y1, x2, y2 - unittop + y1);
			} else {
				gDPFillRectangleScaled(gdl++, x1, unittop, x2, unitbottom);
			}
		}
	}

	gdl = textSetCCCustom02(gdl);

	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	return gdl;
}

Gfx *bgunDrawHud(Gfx *gdl)
{
	struct player *player = g_Vars.currentplayer;
	int bottom = viGetViewTop() + viGetViewHeight() - 13;
	int playercount = PLAYERCOUNT();
	int playernum = g_Vars.currentplayernum;
	struct gunctrl *ctrl;
	int secs60;
	int speedpilltime;
	int ammoindex = 0;
	int barwidth = 9;
	int reserveheight = 36;
	int clipheight = 57;
	int xpos;
	struct weapon *weapon = weaponFindById(player->gunctrl.weaponnum);
	uint32_t alpha;
	uint32_t fncolour;
	int funcnum;
	int fnfaderinc;
	int tmpfuncnum;
	struct handweaponinfo info;
	struct hand *hand = &player->hands[HAND_RIGHT];
	char *str;
	uint32_t colour;
	int x;
	int y;
	int textheight;
	int textwidth;
	struct weaponfunc *func;
	uint16_t nameid;
	struct hand *lefthand = &player->hands[HAND_LEFT];

	ctrl = &player->gunctrl;

	if (player->isdead) {
		return gdl;
	}

	if (g_Vars.currentplayer->gunctrl.passivemode) {
		return gdl;
	}

	if (g_Vars.lvframenum < 5) {
		return gdl;
	}
	
	gdl = textConfigureGfxPipeline(gdl);

	if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
		gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeR);
	}

	if (playercount >= 2) {
		barwidth = 5;
		reserveheight = 26;
		clipheight = 47;

		if (playercount == 2) {
			if ((optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && playernum == 0)) {
				bottom += 10;
			} else {
				bottom += 2;
			}
		} else if (playercount >= 3) {
			if (playernum < 2) {
				bottom += 10;
			} else {
				bottom += 2;
			}
		}
	} else if (optionsGetEffectiveScreenSize() != SCREENSIZE_FULL) {
		bottom += 8;
	}

	fncolour = 0xff000040;
	funcnum = hand->gset.weaponfunc;
	fnfaderinc = PALUP(g_Vars.lvupdate240 * 2);

	bgunGetWeaponInfo(&info, HAND_RIGHT);
	tmpfuncnum = bgunIsUsingSecondaryFunction();

	if (bgunDetermineReloadType(tmpfuncnum, &info, hand) >= 0) {
		funcnum = tmpfuncnum;
	}

	xpos = (viGetViewLeft() + viGetViewWidth()) / 1 - barwidth - 24;

	if (playercount == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) && playernum == 0) {
		xpos += 15;
	} else if (playercount >= 3 && (playernum % 2) == 0) {
		xpos += 15;
	}


	// Draw function square (red square or yellow sqaure)
	if ((funcnum == FUNC_SECONDARY) && ctrl->fnfader < 255) {
		if (ctrl->fnfader < 128) {
			ctrl->fnfader = 128;
		}

		if (ctrl->fnfader + fnfaderinc > 255) {
			ctrl->fnfader = 255;
		} else {
			ctrl->fnfader += fnfaderinc;
		}
	}

	if ((funcnum == FUNC_PRIMARY) && ctrl->fnfader > 0) {
		if (ctrl->fnfader - fnfaderinc < 0) {
			ctrl->fnfader = 0;
		} else {
			ctrl->fnfader -= fnfaderinc;
		}
	}

	if (ctrl->fnfader > 128) {
		fncolour = ((ctrl->fnfader * 2) - 256) << 16 | 0xff000040;
	}

	gdl = textSetPrimColour(gdl, fncolour);

	gDPFillRectangleScaled(gdl++, xpos - 13, bottom - 11, xpos - 2, bottom);

	gdl = textSetCCCustom02(gdl);

	// Draw weapon name and function name
	if (optionsGetShowGunFunction(g_Vars.currentplayerstats->mpindex)) {
		func = weaponGetFunctionById(hand->gset.weaponnum, hand->gset.weaponfunc);
		nameid = invGetNameIdByIndex(invGetCurrentIndex());
		str = langRemoveNewline(langGet(nameid));

		if (ctrl->curgunstr != nameid) {
			ctrl->guntypetimer = 0;
			ctrl->curgunstr = nameid;
		}

		if (ctrl->guntypetimer < 255) {
			colour = 0x55ffffff;

			if (ctrl->guntypetimer + g_Vars.lvupdate60 > 255) {
				ctrl->guntypetimer = 255;
			} else {
				ctrl->guntypetimer += (uint16_t) g_Vars.lvupdate60;
			}

			textMeasure(&textheight, &textwidth, str, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
			textwidth += 2;

			if (textwidth > ctrl->guntypetimer * 3) {
				textwidth = ctrl->guntypetimer * 3;
			}

			if (playercount >= 2) {
				x = xpos - textwidth - 13;
			} else {
				x = xpos - textwidth - 2;
			}

			y = bottom - textheight - 20;

			if (ctrl->guntypetimer > 192) {
				alpha = 255 - (ctrl->guntypetimer - 192) * 255 / 63U;
				colour = (colour & 0xffffff00) | alpha;
				if (0xffffff00);
			}

			gdl = textSetPrimColour(gdl, 0);

			gDPFillRectangleScaled(gdl++, x - 1, y - 1, xpos - 11, bottom);

			gdl = textSetCCCustom02(gdl);
			textSetWaveBlend(g_20SecIntervalFrac * 50.0f, 0, 50);
			textSetWaveColours(0xffffffff, 0xffffffff);
			gdl = textRenderProjected(gdl, &x, &y, str, g_CharsHandelGothicXs, g_FontHandelGothicXs, colour, textwidth, 1000, 0, 0);
			textResetBlends();
		}

		if (func) {
			langRemoveNewline(langGet(func->name));

			colour = 0xff5555ff;

			if ((ctrl->curfnstr != func->name && ctrl->fnfader > 128) || ctrl->curfnstr == 0) {
				ctrl->fnstrtimer = 0;
				ctrl->curfnstr = func->name;
			}

			str = langRemoveNewline(langGet(ctrl->curfnstr));

			struct player *player = g_Vars.currentplayer;
			int weaponnum = player->gunctrl.weaponnum;

			if(weaponnum == WEAPON_RCP120) {
				if(g_Vars.currentplayer->devicesactive & DEVICE_CLOAKRCP120) {
					str = "Cloak\n";
				}
				else {
					str = "Rapid Fire\n";
				}
			}

			if (ctrl->fnstrtimer < 255) {
				if (ctrl->fnstrtimer + g_Vars.lvupdate60 > 255) {
					ctrl->fnstrtimer = 255;
				} else {
					ctrl->fnstrtimer += (uint16_t) g_Vars.lvupdate60;
				}

				if ((funcnum == FUNC_SECONDARY || g_Vars.currentplayer->devicesactive & DEVICE_CLOAKRCP120) && func->name == ctrl->curfnstr) {
					colour = 0xffff55ff;
				}

				if ((funcnum == FUNC_PRIMARY || g_Vars.currentplayer->devicesactive & DEVICE_CLOAKRCP120) && func->name != ctrl->curfnstr) {
					colour = 0xff5555ff;
				}

				textMeasure(&textheight, &textwidth, str, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
				textwidth += 2;

				if (textwidth > ctrl->fnstrtimer * 3) {
					textwidth = ctrl->fnstrtimer * 3;
				}

				x = xpos - textwidth - 13;
				//y = bottom - textheight + 3;
				y = bottom - textheight - 7;

				if (ctrl->fnstrtimer > 192) {
					alpha = 255 - (ctrl->fnstrtimer - 192) * 255 / 63U;
					colour = (colour & 0xffffff00) | alpha;
				}

				gdl = textSetPrimColour(gdl, 0);

				gDPFillRectangleScaled(gdl++, x - 1, y - 1, xpos - 11, bottom + 3);

				gdl = textSetCCCustom02(gdl);

				textSetWaveBlend(g_20SecIntervalFrac * 50.0f, 0, 50);
				textSetWaveColours(0xffffffff, 0xffffffff);

				// Render the function name text
				gdl = textRenderProjected(gdl, &x, &y, str,
						g_CharsHandelGothicXs, g_FontHandelGothicXs, colour, textwidth,
						1000, 0, 0);

				textResetBlends();
			}
		}
	}

	if (weapon && weapon->functions[hand->gset.weaponfunc] != NULL) {
		ammoindex = ((struct weaponfunc *)(weapon->functions[hand->gset.weaponfunc]))->ammoindex;
	}

	if (ammoindex == -1) {
		if (weapon->functions[1 - hand->gset.weaponfunc] != NULL) {
			ammoindex = ((struct weaponfunc *)(weapon->functions[1 - hand->gset.weaponfunc]))->ammoindex;
		}

		if (ammoindex == -1) {
			gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);
			gdl = text0f153780(gdl);
			return gdl;
		}
	}

	if (ammoindex != ctrl->lastmag) {
		bgunResetAbmag(&player->hands[HAND_LEFT].abmag);
		bgunResetAbmag(&hand->abmag);
		bgunResetAbmag(&ctrl->abmag);
		ctrl->lastmag = ammoindex;
	}

	// Left hand - mag
	if (lefthand->inuse
			&& weapon->ammos[ammoindex] != NULL
			&& lefthand->gset.weaponnum != WEAPON_REMOTEMINE) {
		
		/*xpos = viGetViewLeft() / 25;

		if (playercount == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) && playernum == 1) {
			xpos += 14;
		} else if (playercount >= 3 && (playernum & 1) == 1) {
			xpos += 14;
		}*/

		xpos = 28;

		if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
			gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeL);
		}

		if (lefthand->clipsizes[ammoindex] > 0 && (weapon->ammos[ammoindex]->flags & AMMOFLAG_EQUIPPEDISRESERVE) == 0) {
			gdl = bgunDrawHudGauge(gdl,
					xpos, bottom - reserveheight - clipheight - 3, xpos + barwidth, bottom - reserveheight - 3,
					&lefthand->abmag, lefthand->loadedammo[ammoindex], lefthand->clipsizes[ammoindex],
					0x00300080, 0x00ff0040, false);
			gdl = bgunDrawHudInteger(gdl, lefthand->loadedammo[ammoindex], xpos + barwidth + 2, true,
					bottom - reserveheight - 8, 0, 0x00ff00a0);
		}
	}

	// Right hand - mag, reserve and combat boost timer
	if (hand->inuse && ctrl->ammotypes[ammoindex] >= 0) {
		int ammotype;
		int ammoheld;
		int ammototal;

		ammotype = player->gunctrl.ammotypes[ammoindex];

		xpos = (viGetViewLeft() + viGetViewWidth()) / 1 - barwidth - 24;

		if (playercount == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) && playernum == 0) {
			xpos += 15;
		} else if (playercount >= 3 && (playernum % 2) == 0) {
			xpos += 15;
		}

		if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
			gSPExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeR);
		}

		// Mag
		ammoheld = player->ammoheldarr[ammotype];

		if (hand->clipsizes[ammoindex] > 0
				&& weapon->ammos[ammoindex] != NULL
				&& (weapon->ammos[ammoindex]->flags & AMMOFLAG_EQUIPPEDISRESERVE) == 0) {
			gdl = bgunDrawHudGauge(gdl, xpos, bottom - reserveheight - clipheight - 3, xpos + barwidth,
					bottom - reserveheight - 3, &hand->abmag, hand->loadedammo[ammoindex], hand->clipsizes[ammoindex],
					0x00300080, 0x00ff0040, false);
			gdl = bgunDrawHudInteger(gdl, hand->loadedammo[ammoindex], xpos - 2, false,
					bottom - reserveheight - 8, 0, 0x00ff00a0);
		}

		// Reserve
		if (g_AmmoTypes[ammotype].capacity > 0
				&& (weapon->ammos[ammoindex]->flags & AMMOFLAG_NORESERVE) == 0) {
			ammototal = ammoheld;

			if (weapon->ammos[ammoindex]->flags & AMMOFLAG_EQUIPPEDISRESERVE) {
				if (hand->clipsizes[ammoindex] > 0) {
					ammototal += hand->loadedammo[ammoindex];
				}

				if (lefthand->clipsizes[ammoindex] > 0) {
					ammototal += lefthand->loadedammo[ammoindex];
				}
			}

			gdl = bgunDrawHudGauge(gdl, xpos, bottom - reserveheight, xpos + barwidth,
					bottom, &ctrl->abmag, ammototal, g_AmmoTypes[ammotype].capacity,
					0x00403080, 0x00ffc040, true);
			gdl = bgunDrawHudInteger(gdl, ammototal, xpos - 2, false, bottom - reserveheight + 1, 0, 0x00ffc0a0);
		}

		// Combat boost timer
		if (hand->gset.weaponnum == WEAPON_COMBATBOOST) {
			int mins;
			char text[32];

			speedpilltime = g_Vars.speedpilltime;
			mins = speedpilltime / TICKS(3600);
			secs60 = speedpilltime - mins * TICKS(3600);

			if (mins >= 1) {
				sprintf(text, "%02d:%02d:%02d\n", mins, secs60 / TICKS(60), (secs60 - (secs60 / TICKS(60)) * TICKS(60)) * 100 / TICKS(60));
			} else {
				sprintf(text, "%02d:%02d\n", secs60 / TICKS(60), (secs60 - (secs60 / TICKS(60)) * TICKS(60)) * 100 / TICKS(60));
			}

			gdl = bgunDrawHudString(gdl, text, xpos + barwidth - 2, false, bottom - reserveheight + 1, 0, 0x00ffc0a0);
		}
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

	gdl = text0f153780(gdl);

	return gdl;
}

void bgunAddBoost(int amount)
{
	g_Vars.speedpilltime += amount;

	if (g_Vars.speedpilltime > 5 * 60 * TICKS(60)) { // 5 minutes
		g_Vars.speedpilltime = 5 * 60 * TICKS(60);
	}

	if (!g_Vars.speedpillwant) {
		uint32_t sound = lvGetSlowMotionType() ? SFX_ARGH_JO_02AD : SFX_JO_BOOST_ACTIVATE;

		sndStart(var80095200, sound, 0, -1, -1, -1, -1, -1);
	}

	g_Vars.speedpillwant = true;
}

void bgunSubtractBoost(int amount)
{
	g_Vars.speedpilltime -= amount;

	if (g_Vars.speedpilltime <= 0) {
		g_Vars.speedpilltime = 0;
		g_Vars.speedpillwant = false;
	}
}

void bgunApplyBoost(void)
{
	if (lvGetSlowMotionType() != SLOWMOTION_OFF) {
		bgunSubtractBoost(TICKS(1200));
	} else {
		bgunAddBoost(TICKS(600));
	}
}

void bgunRevertBoost(void)
{
	if (lvGetSlowMotionType() != SLOWMOTION_OFF) {
		bgunAddBoost(TICKS(1200));
	} else {
		bgunSubtractBoost(TICKS(600));
	}
}

/**
 * The main tick function as called from lvTick.
 *
 * This function doesn't do much because it's called during both cutscenes and
 * gameplay, while most gun tick operations happen during gameplay only.
 * See bgunTickGameplay for that.
 */
void bgunTickBoost(void)
{
	if (g_Vars.speedpillon && g_Vars.speedpilltime > 0 && !g_Vars.in_cutscene) {
		g_Vars.speedpilltime -= g_Vars.lvupdate60;

		if (g_Vars.speedpilltime <= 0) {
			g_Vars.speedpilltime = 0;
			g_Vars.speedpillwant = false;
		}
	}
}

/**
 * gunsightoff is 0 if the full sight is visible, ie. player is holding R.
 *
 * Otherwise, gunsightoff holds bit values for reasons why the sight is off.
 * This is typically 2, which is GUNSIGHTREASON_NOTAIMING.
 *
 * If the visible argument is true, it removes the reason from the field, thus
 * making the sight visible if there are no other reasons.
 */
void bgunSetSightVisible(uint32_t reason, bool visible)
{
	if (visible) {
		g_Vars.currentplayer->gunsightoff &= ~reason;
		return;
	}

	g_Vars.currentplayer->gunsightoff |= reason;
}

Gfx *bgunDrawSight(Gfx *gdl)
{
	if (g_Vars.currentplayer->gunsightoff == 0 && !g_Vars.currentplayer->mpmenuon) {
		// Player is aiming with R
		gdl = sightDraw(gdl, true, currentPlayerGetSight());
	} else {
		gdl = sightDraw(gdl, false, currentPlayerGetSight());
	}

	return gdl;
}

void bgun0f0abd30(int handnum)
{
	struct player *player = g_Vars.currentplayer;
	struct hand *hand = &player->hands[handnum];
	struct gunctrl *gunctrl = &player->gunctrl;
	struct weapon *weapon = weaponFindById(hand->gset.weaponnum);
	int i;

	for (i = 0; i < 2; i++) {
		if (handnum == HAND_RIGHT) {
			gunctrl->ammotypes[i] = -1;
		}

		if (weapon && weapon->ammos[i]) {
			if (handnum == HAND_RIGHT) {
				gunctrl->ammotypes[i] = weapon->ammos[i]->type;
			}

			hand->clipsizes[i] = weapon->ammos[i]->clipsize;

			if (handnum == HAND_LEFT && hand->gset.weaponnum == WEAPON_REMOTEMINE) {
				hand->clipsizes[i] = 0;
			}

			hand->loadedammo[i] = 0;
		}
	}

	hand->upgrademult[0] = 1;
	hand->upgrademult[1] = 1;
	hand->finalmult[0] = 1;
	hand->finalmult[1] = 1;

	if (gunctrl->ammotypes[0] >= 0) {
		bgunResetAbmag(&hand->abmag);

		if (handnum == HAND_RIGHT) {
			bgunResetAbmag(&gunctrl->abmag);
		}

		gunctrl->lastmag = false;
	}
}
