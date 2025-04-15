#include <ultra64.h>
#include <math.h>
#include <stdio.h>
#include "constants.h"
#include "game/bg.h"
#include "game/body.h"
#include "game/bondgun.h"
#include "game/bot.h"
#include "game/botact.h"
#include "game/botcmd.h"
#include "game/botinv.h"
#include "game/camera.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/chrai.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/env.h"
#include "game/explosions.h"
#include "game/footstep.h"
#include "game/menuutils.h"
#include "game/mtxutils.h"
#include "game/weaponutils.h"
#include "game/textutils.h"
#include "game/gunfx.h"
#include "game/inv.h"
#include "game/lv.h"
#include "game/modelmgr.h"
#include "game/mplayer/mplayer.h"
#include "game/mpstats.h"
#include "game/objectives.h"
#include "game/options.h"
#include "game/pad.h"
#include "game/padhalllv.h"
#include "game/pak.h"
#include "game/pdmode.h"
#include "game/player.h"
#include "game/playermgr.h"
#include "game/prop.h"
#include "game/propobj.h"
#include "game/propsnd.h"
#include "game/room.h"
#include "game/sparks.h"
#include "game/stagetable.h"
#include "game/tex.h"
#include "game/wallhit.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/joy.h"
#include "lib/lib_17ce0.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/ailist.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"

int g_RecentQuipsPlayed[5];
uint8_t g_RecentQuipsIndex;

float g_EnemyAccuracyScale = 1;
float g_PlayerDamageRxScale = 1;
float g_PlayerDamageTxScale = 1;
float g_AttackWalkDurationScale = 1;
int g_NumChrsWithPlayerTarget = 0;
int g_NumChrsSeenPlayer = 0;
int g_NumChrsSeenPlayerRecently = 0;
int g_NumChrsSeenPlayerRecently2 = 0;

struct animtablerow g_DeathAnimsHumanLfoot[] = {
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 0, 26, -1 },
	{ 0,      0,  0, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanLshin[] = {
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 0, 26, -1 },
	{ 0,      0,  0, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanLthigh[] = {
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 1, 26, -1  },
	{ ANIM_DEATH_STOMACH_LONG,  1, -1, 0.5, 0, -1, -1  },
	{ ANIM_DEATH_GRAB_THIGH_RT, 1, -1, 0.4, 0, 42, 103 },
	{ ANIM_0258, 1, -1, 0.5, 0, 43, 100 },
	{ 0,      0, 0,  0.5, 0, -1, -1  },
};

struct animtablerow g_DeathAnimsHumanRfoot[] = {
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 0, 26, -1 },
	{ 0,      0, 0,  0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanRshin[] = {
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 0, 26, -1 },
	{ 0,      0, 0,  0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanRthigh[] = {
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 1, 26, -1  },
	{ ANIM_DEATH_STOMACH_LONG,   0, -1, 0.5, 0, -1, -1  },
	{ ANIM_DEATH_GRAB_THIGH_RT,  0, -1, 0.4, 0, 42, 103 },
	{ ANIM_0258, 0, -1, 0.5, 0, 43, 100 },
	{ 0,      0, 0,  0.5, 0, -1, -1  },
};

struct animtablerow g_DeathAnimsHumanPelvis[] = {
	{ ANIM_DEATH_001A, 0, -1, 0.5, 0, 55,  39  },
	{ ANIM_DEATH_001A, 1, -1, 0.5, 0, 55,  39  },
	{ ANIM_DEATH_001C, 0, -1, 0.5, 1, 29,  -1  },
	{ ANIM_DEATH_001C, 1, -1, 0.5, 1, 29,  -1  },
	{ ANIM_DEATH_0021, 0, -1, 0.5, 0, 97,  64  },
	{ ANIM_DEATH_0021, 1, -1, 0.5, 0, 97,  64  },
	{ ANIM_DEATH_0023, 0, -1, 0.5, 0, 31,  -1  },
	{ ANIM_DEATH_0023, 1, -1, 0.5, 0, 31,  -1  },
	{ ANIM_DEATH_0024, 0, -1, 0.5, 0, 36,  -1  },
	{ ANIM_DEATH_0024, 1, -1, 0.5, 0, 36,  -1  },
	{ ANIM_DEATH_0025, 0, -1, 0.5, 0, 28,  -1  },
	{ ANIM_DEATH_0025, 1, -1, 0.5, 0, 28,  -1  },
	{ ANIM_DEATH_HOLD_GUT,   0, -1, 0.6, 0, 157, 234 },
	{ ANIM_DEATH_HOLD_GUT,   1, -1, 0.6, 0, 157, 234 },
	{ ANIM_DEATH_HOLD_GUT2,   0, -1, 0.6, 0, 75,  265 },
	{ ANIM_DEATH_HOLD_GUT2,   1, -1, 0.6, 0, 75,  265 },
	{ ANIM_0250, 0, -1, 0.5, 0, 65,  105 },
	{ ANIM_0250, 1, -1, 0.5, 0, 65,  105 },
	{ 0,      0, -1, 0.5, 0, -1,  -1  },
};

struct animtablerow g_DeathAnimsHumanHead[] = {
	{ ANIM_DEATH_001A, 0, -1, 0.5, 0, 55,  39  },
	{ ANIM_DEATH_001A, 1, -1, 0.5, 0, 55,  39  },
	{ ANIM_DEATH_001C, 0, -1, 0.5, 1, 29,  -1  },
	{ ANIM_DEATH_001C, 1, -1, 0.5, 1, 29,  -1  },
	{ ANIM_DEATH_TWIST,       0, -1, 0.5, 1, 26,  -1  },
	{ ANIM_DEATH_TWIST,       1, -1, 0.5, 1, 26,  -1  },
	{ ANIM_DEATH_0021, 0, -1, 0.5, 0, 97,  64  },
	{ ANIM_DEATH_0021, 1, -1, 0.5, 0, 97,  64  },
	{ ANIM_DEATH_0022, 0, -1, 0.5, 0, 94,  66  },
	{ ANIM_DEATH_0022, 1, -1, 0.5, 0, 94,  66  },
	{ ANIM_DEATH_0023, 0, -1, 0.5, 0, 31,  -1  },
	{ ANIM_DEATH_0023, 1, -1, 0.5, 0, 31,  -1  },
	{ ANIM_DEATH_0024, 0, -1, 0.5, 0, 36,  -1  },
	{ ANIM_DEATH_0024, 1, -1, 0.5, 0, 36,  -1  },
	{ ANIM_DEATH_0025, 0, -1, 0.5, 0, 28,  -1  },
	{ ANIM_DEATH_0025,        1, -1, 0.5, 0, 28,  -1  },
	{ ANIM_DEATH_NECK_LONG,   0, -1, 0.5, 0, -1,  -1  },
	{ ANIM_DEATH_NECK_LONG,   1, -1, 0.5, 0, -1,  -1  },
	{ ANIM_0251, 0, -1, 0.5, 0, 132, 201 },
	{ ANIM_0251, 1, -1, 0.5, 0, 132, 201 },
	{ ANIM_0252, 0, -1, 0.5, 0, 83,  150 },
	{ ANIM_0252, 1, -1, 0.5, 0, 83,  150 },
	{ ANIM_0256, 0, -1, 0.5, 0, 63,  -1  },
	{ ANIM_0256, 1, -1, 0.5, 0, 63,  -1  },
	{ 0,      0, -1, 0.5, 0, -1,  -1  },
};

struct animtablerow g_DeathAnimsHumanLhand[] = {
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 0, 26, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanForearm[] = {
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 0, 26, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanLbicep[] = {
	{ ANIM_DEATH_TWIST,    0, -1, 0.5,  1, 26, -1 },
	{ ANIM_BLAST_TWIST_BK, 1, -1, 0.45, 1, 52, -1 },
	{ 0,                   0, -1, 0.5,  0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanRhand[] = {
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 0, 26, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanRforearm[] = {
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 0, 26, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanRbicep[] = {
	{ ANIM_DEATH_TWIST,    1, -1, 0.5,  1, 26, -1 },
	{ ANIM_BLAST_TWIST_BK, 0, -1, 0.45, 1, 52, -1 },
	{ 0,                   0, -1, 0.5,  0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanTorso[] = {
	{ ANIM_DEATH_001A, 0, -1, 0.5, 0, 55, 39 },
	{ ANIM_DEATH_001A, 1, -1, 0.5, 0, 55, 39 },
	{ ANIM_DEATH_001C, 0, -1, 0.5, 1, 29, -1 },
	{ ANIM_DEATH_001C, 1, -1, 0.5, 1, 29, -1 },
	{ ANIM_DEATH_TWIST, 0, -1, 0.5, 1, 26, -1 },
	{ ANIM_DEATH_TWIST, 1, -1, 0.5, 1, 26, -1 },
	{ ANIM_DEATH_0021, 0, -1, 0.5, 0, 97, 64 },
	{ ANIM_DEATH_0021, 1, -1, 0.5, 0, 97, 64 },
	{ ANIM_DEATH_0022, 0, -1, 0.5, 0, 94, 66 },
	{ ANIM_DEATH_0022, 1, -1, 0.5, 0, 94, 66 },
	{ ANIM_DEATH_0023, 0, -1, 0.5, 0, 31, -1 },
	{ ANIM_DEATH_0023, 1, -1, 0.5, 0, 31, -1 },
	{ ANIM_DEATH_0024, 0, -1, 0.5, 0, 36, -1 },
	{ ANIM_DEATH_0024, 1, -1, 0.5, 0, 36, -1 },
	{ ANIM_DEATH_0025, 0, -1, 0.5, 0, 28, -1 },
	{ ANIM_DEATH_0025, 1, -1, 0.5, 0, 28, -1 },
	{ ANIM_024E, 0, -1, 0.4, 0, 60, -1 },
	{ ANIM_024E, 1, -1, 0.4, 0, 60, -1 },
	{ ANIM_024F, 0, -1, 0.5, 0, 49, 80 },
	{ ANIM_024F, 1, -1, 0.5, 0, 49, 80 },
	{ ANIM_0253, 0, -1, 0.5, 1, 22, -1 },
	{ ANIM_0253, 1, -1, 0.5, 1, 22, -1 },
	{ ANIM_0254, 0, -1, 0.5, 0, 52, 75 },
	{ ANIM_0254, 1, -1, 0.5, 0, 52, 75 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsHumanGun[] = {
	{ ANIM_DEATH_001A, 0, -1, 0.5, 0, 55, 39 },
	{ ANIM_DEATH_001A, 1, -1, 0.5, 0, 55, 39 },
	{ ANIM_DEATH_001C, 0, -1, 0.5, 1, 29, -1 },
	{ ANIM_DEATH_001C, 1, -1, 0.5, 1, 29, -1 },
	{ ANIM_DEATH_0021, 0, -1, 0.5, 0, 97, 64 },
	{ ANIM_DEATH_0021, 1, -1, 0.5, 0, 97, 64 },
	{ ANIM_DEATH_0022, 0, -1, 0.5, 0, 94, 66 },
	{ ANIM_DEATH_0022, 1, -1, 0.5, 0, 94, 66 },
	{ ANIM_DEATH_0023, 0, -1, 0.5, 0, 31, -1 },
	{ ANIM_DEATH_0023, 1, -1, 0.5, 0, 31, -1 },
	{ ANIM_DEATH_0024, 0, -1, 0.5, 0, 36, -1 },
	{ ANIM_DEATH_0024, 1, -1, 0.5, 0, 36, -1 },
	{ ANIM_DEATH_0025, 0, -1, 0.5, 0, 28, -1 },
	{ ANIM_DEATH_0025, 1, -1, 0.5, 0, 28, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_AnimTableHumanSlumped[] = {
	{ ANIM_DEATH_WALL_SLUMP, 0, -1, 0.5, 0, 67, 54 },
	{ ANIM_DEATH_WALL_SLUMP, 1, -1, 0.5, 0, 67, 54 },
	{ ANIM_0257, 0, -1, 0.5, 0, 15, 80 },
	{ ANIM_0257, 1, -1, 0.5, 0, 15, 80 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanLfoot[] = {
	{ ANIM_HIT_LF_ARM_LIMP, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_STUMBLE_BK, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanLshin[] = {
	{ ANIM_HIT_LF_ARM_LIMP, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_STUMBLE_BK, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_00BC, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_00BD, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanLthigh[] = {
	{ ANIM_HIT_LF_ARM_LIMP, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_STUMBLE_BK, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_STOMACH_LONG, 1, 20, 0.4, 0, -1, -1 },
	{ ANIM_00BA, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRfoot[] = {
	{ ANIM_HIT_STUMBLE_BK, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_LF_ARM_LIMP, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_0236, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRshin[] = {
	{ ANIM_HIT_STUMBLE_BK, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_LF_ARM_LIMP, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_00BE, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRthigh[] = {
	{ ANIM_HIT_STUMBLE_BK, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_LF_ARM_LIMP, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_STOMACH_LONG, 0, 20, 0.4, 0, -1, -1 },
	{ ANIM_00BF, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanPelvis[] = {
	{ ANIM_DEATH_0022, 0, 20, 0.5,  0, -1, -1 },
	{ ANIM_DEATH_0022, 1, 20, 0.5,  0, -1, -1 },
	{ ANIM_DEATH_001A, 0, 15, 0.5,  0, -1, -1 },
	{ ANIM_DEATH_001A, 1, 15, 0.5,  0, -1, -1 },
	{ ANIM_DEATH_0023, 0, 10, 0.25, 0, -1, -1 },
	{ ANIM_DEATH_0023, 1, 10, 0.25, 0, -1, -1 },
	{ ANIM_00DA, 0, -1, 0.5,  0, -1, -1 },
	{ ANIM_00F4, 0, -1, 0.5,  0, -1, -1 },
	{ 0,      0, -1, 0.5,  0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanHead[] = {
	{ ANIM_DEATH_0022, 0, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_0022, 1, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 0, 15, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 1, 15, 0.5, 0, -1, -1 },
	{ ANIM_00F8, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_00FB, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_0101, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_0113, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanLhand[] = {
	{ ANIM_HIT_HAND_LF, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_HAND_RT, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B8, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanForearm[] = {
	{ ANIM_HIT_FOREARM_LF, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_FOREARM_RT, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B4, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_021B, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanLbicep[] = {
	{ ANIM_HIT_SHOULDER_LF, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_SHOULDER_RT, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_0022,      0, 20, 0.5, 0, -1, -1 },
	{ ANIM_HIT_ARM_LF,      0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_ARM_LF2,     0, -1, 0.5, 0, -1, -1 },
	{ ANIM_021C, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B5, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRhand[] = {
	{ ANIM_HIT_HAND_RT, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_HAND_LF, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B9, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRforearm[] = {
	{ ANIM_HIT_FOREARM_RT, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_FOREARM_LF, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanRbicep[] = {
	{ ANIM_HIT_SHOULDER_RT,  0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_SHOULDER_LF,  1, -1, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_0022, 1, 20, 0.5, 0, -1, -1 },
	{ ANIM_0190, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_SHOULDER_RT2, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_HIT_SHOULDER_RT3, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B6, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_00B7, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanTorso[] = {
	{ ANIM_DEATH_0022, 0, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_0022, 1, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 0, 15, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 1, 15, 0.5, 0, -1, -1 },
	{ ANIM_0114, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_0130, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsHumanGun[] = {
	{ ANIM_DEATH_0022, 0, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_0022, 1, 20, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 0, 15, 0.5, 0, -1, -1 },
	{ ANIM_DEATH_001A, 1, 15, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLfoot[] = {
	{ ANIM_0337, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLshin[] = {
	{ ANIM_0337, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLthigh[] = {
	{ ANIM_0337, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 1, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRfoot[] = {
	{ ANIM_0337, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRshin[] = {
	{ ANIM_0337, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRthigh[] = {
	{ ANIM_0337, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033C, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033B, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarPelvis[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarHead[] = {
	{ ANIM_0339, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_0338, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033A, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLhand[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLforearm[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarLbicep[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRhand[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRforearm[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarRbicep[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarTorso[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarGun[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_DeathAnimsSkedarTail[] = {
	{ ANIM_0336, 0, -1, 0.5, 0, -1, -1 },
	{ ANIM_033D, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLfoot[] = {
	{ ANIM_038E, 0, -1, 1,   0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLshin[] = {
	{ ANIM_0390, 0, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLthigh[] = {
	{ ANIM_0390, 0, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRfoot[] = {
	{ ANIM_038E, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRshin[] = {
	{ ANIM_0390, 1, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRthigh[] = {
	{ ANIM_0390, 1, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarPelvis[] = {
	{ ANIM_038C, 0, -1, 0.9, 0, -1, -1 },
	{ ANIM_038C, 1, -1, 0.9, 0, -1, -1 },
	{ ANIM_0341, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarHead[] = {
	{ ANIM_0341, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLhand[] = {
	{ ANIM_0343, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLforearm[] = {
	{ ANIM_038B, 0, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarLbicep[] = {
	{ ANIM_038B, 0, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRhand[] = {
	{ ANIM_0343, 1, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRforearm[] = {
	{ ANIM_038B, 1, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarRbicep[] = {
	{ ANIM_038B, 1, -1, 0.9, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarTorso[] = {
	{ ANIM_038C, 0, -1, 0.9, 0, -1, -1 },
	{ ANIM_038C, 1, -1, 0.9, 0, -1, -1 },
	{ ANIM_0341, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarGun[] = {
	{ ANIM_0343, 0, -1, 0.5, 0, -1, -1 },
	{ 0,      0, -1, 0.5, 0, -1, -1 },
};

struct animtablerow g_InjuryAnimsSkedarTail[] = {
	{ ANIM_0341, 0, -1, 0.5, 0, -1,  -1  },
	{ 0,      0, -1, 0.5, 0, -1,  -1  },
};

struct animtablerow g_SpecialDieAnims[] = {
	{ ANIM_SPECIALDIE_FALLBACK,    0, -1, 0.5, 0, 149, 175 },
	{ ANIM_SPECIALDIE_ROLL1,       0, -1, 0.5, 0, 115, 152 },
	{ ANIM_SPECIALDIE_ROLL2,       0, -1, 0.5, 0, 115, 152 },
	{ ANIM_SPECIALDIE_ROLL3,       0, -1, 0.5, 0, 115, 152 },
	{ ANIM_SPECIALDIE_OVERRAILING, 0, -1, 0.5, 0, 83,  99  },
	{ ANIM_022B,                   0, -1, 0.5, 0, 0,   0   },
	{ ANIM_022C,                   0, -1, 0.5, 0, 0,   0   },
	{ ANIM_022D,                   0, -1, 0.5, 0, 0,   0   },
	{ ANIM_022E,                   0, -1, 0.5, 0, 0,   0   },
	{ 0,                           0, -1, 0.5, 0, -1,  -1  },
};

struct animtable g_AnimTablesHuman[] = {
	{ 0,                NULL,                      NULL,                       0, 0 },
	{ HITPART_LFOOT,    g_DeathAnimsHumanLfoot,    g_InjuryAnimsHumanLfoot,    0, 0 },
	{ HITPART_LSHIN,    g_DeathAnimsHumanLshin,    g_InjuryAnimsHumanLshin,    0, 0 },
	{ HITPART_LTHIGH,   g_DeathAnimsHumanLthigh,   g_InjuryAnimsHumanLthigh,   0, 0 },
	{ HITPART_RFOOT,    g_DeathAnimsHumanRfoot,    g_InjuryAnimsHumanRfoot,    0, 0 },
	{ HITPART_RSHIN,    g_DeathAnimsHumanRshin,    g_InjuryAnimsHumanRshin,    0, 0 },
	{ HITPART_RTHIGH,   g_DeathAnimsHumanRthigh,   g_InjuryAnimsHumanRthigh,   0, 0 },
	{ HITPART_PELVIS,   g_DeathAnimsHumanPelvis,   g_InjuryAnimsHumanPelvis,   0, 0 },
	{ HITPART_HEAD,     g_DeathAnimsHumanHead,     g_InjuryAnimsHumanHead,     0, 0 },
	{ HITPART_LHAND,    g_DeathAnimsHumanLhand,    g_InjuryAnimsHumanLhand,    0, 0 },
	{ HITPART_LFOREARM, g_DeathAnimsHumanForearm,  g_InjuryAnimsHumanForearm,  0, 0 },
	{ HITPART_LBICEP,   g_DeathAnimsHumanLbicep,   g_InjuryAnimsHumanLbicep,   0, 0 },
	{ HITPART_RHAND,    g_DeathAnimsHumanRhand,    g_InjuryAnimsHumanRhand,    0, 0 },
	{ HITPART_RFOREARM, g_DeathAnimsHumanRforearm, g_InjuryAnimsHumanRforearm, 0, 0 },
	{ HITPART_RBICEP,   g_DeathAnimsHumanRbicep,   g_InjuryAnimsHumanRbicep,   0, 0 },
	{ HITPART_TORSO,    g_DeathAnimsHumanTorso,    g_InjuryAnimsHumanTorso,    0, 0 },
	{ HITPART_GUN,      g_DeathAnimsHumanGun,      g_InjuryAnimsHumanGun,      0, 0 },
	{ HITPART_HAT,      NULL,                      NULL,                       0, 0 },
	{ -1,               NULL,                      NULL,                       0, 0 },
};

struct animtable g_AnimTablesSkedar[] = {
	{ 0,                NULL,                       NULL,                        0, 0 },
	{ HITPART_LFOOT,    g_DeathAnimsSkedarLfoot,    g_InjuryAnimsSkedarLfoot,    0, 0 },
	{ HITPART_LSHIN,    g_DeathAnimsSkedarLshin,    g_InjuryAnimsSkedarLshin,    0, 0 },
	{ HITPART_LTHIGH,   g_DeathAnimsSkedarLthigh,   g_InjuryAnimsSkedarLthigh,   0, 0 },
	{ HITPART_RFOOT,    g_DeathAnimsSkedarRfoot,    g_InjuryAnimsSkedarRfoot,    0, 0 },
	{ HITPART_RSHIN,    g_DeathAnimsSkedarRshin,    g_InjuryAnimsSkedarRshin,    0, 0 },
	{ HITPART_RTHIGH,   g_DeathAnimsSkedarRthigh,   g_InjuryAnimsSkedarRthigh,   0, 0 },
	{ HITPART_PELVIS,   g_DeathAnimsSkedarPelvis,   g_InjuryAnimsSkedarPelvis,   0, 0 },
	{ HITPART_HEAD,     g_DeathAnimsSkedarHead,     g_InjuryAnimsSkedarHead,     0, 0 },
	{ HITPART_LHAND,    g_DeathAnimsSkedarLhand,    g_InjuryAnimsSkedarLhand,    0, 0 },
	{ HITPART_LFOREARM, g_DeathAnimsSkedarLforearm, g_InjuryAnimsSkedarLforearm, 0, 0 },
	{ HITPART_LBICEP,   g_DeathAnimsSkedarLbicep,   g_InjuryAnimsSkedarLbicep,   0, 0 },
	{ HITPART_RHAND,    g_DeathAnimsSkedarRhand,    g_InjuryAnimsSkedarRhand,    0, 0 },
	{ HITPART_RFOREARM, g_DeathAnimsSkedarRforearm, g_InjuryAnimsSkedarRforearm, 0, 0 },
	{ HITPART_RBICEP,   g_DeathAnimsSkedarRbicep,   g_InjuryAnimsSkedarRbicep,   0, 0 },
	{ HITPART_TORSO,    g_DeathAnimsSkedarTorso,    g_InjuryAnimsSkedarTorso,    0, 0 },
	{ HITPART_GUN,      g_DeathAnimsSkedarGun,      g_InjuryAnimsSkedarGun,      0, 0 },
	{ HITPART_HAT,      g_DeathAnimsSkedarHead,     g_InjuryAnimsSkedarHead,     0, 0 },
	{ HITPART_TAIL,     g_DeathAnimsSkedarTail,     g_InjuryAnimsSkedarTail,     0, 0 },
	{ -1,               NULL,                       NULL,                        0, 0 },
};

struct animtable g_AnimTablesDrCaroll[] = {
	{ 0,  NULL, NULL, 0, 0 },
	{ -1, NULL, NULL, 0, 0 },
};

struct animtable g_AnimTablesEyespy[] = {
	{ 0,  NULL, NULL, 0, 0 },
	{ -1, NULL, NULL, 0, 0 },
};

struct animtable g_AnimTablesRobot[] = {
	{ 0,  NULL, NULL, 0, 0 },
	{ -1, NULL, NULL, 0, 0 },
};

struct animtable *g_AnimTablesByRace[] = {
	g_AnimTablesHuman,
	g_AnimTablesSkedar,
	g_AnimTablesDrCaroll,
	g_AnimTablesEyespy,
	g_AnimTablesRobot,
};

/**
 * A yeet anim is an animation config for a chr being launched by an explosion.
 */
struct yeetanim {
	int16_t animnum;
	bool flip;
	float speed;
	float startframe;
	float thudframe;
	float endframe;
};

struct yeetanim g_YeetAnimsHuman[] = {
	/* 0*/ { ANIM_YEET_FWD_0082, 0, 0.5, 9,  29, -1 },
	/* 1*/ { ANIM_YEET_FWD_0082, 1, 0.5, 9,  29, -1 },
	/* 2*/ { ANIM_YEET_FWD_008A, 0, 0.5, 11, 31, -1 },
	/* 3*/ { ANIM_YEET_FWD_008A, 1, 0.5, 11, 31, -1 },
	/* 4*/ { ANIM_YEET_ROLL_FWD, 0, 0.5, 6,  27, -1 },
	/* 5*/ { ANIM_YEET_ROLL_FWD, 1, 0.5, 6,  27, -1 },
	/* 6*/ { ANIM_YEET_FWD_RT2,  0, 0.5, 29, 48, -1 },
	/* 7*/ { ANIM_YEET_FWD_RT2,  1, 0.5, 29, 48, -1 },
	/* 8*/ { ANIM_YEET_FWD_RT3,  0, 0.5, 29, 49, -1 },
	/* 9*/ { ANIM_YEET_FWD_RT3,  1, 0.5, 29, 49, -1 },
	/*10*/ { ANIM_YEET_FWD_RT4,  0, 0.5, 19, 42, -1 },
	/*11*/ { ANIM_YEET_FWD_RT4,  1, 0.5, 19, 42, -1 },
	/*12*/ { ANIM_YEET_RT,       0, 0.5, 0,  60, -1 },
	/*13*/ { ANIM_YEET_RT,       1, 0.5, 0,  60, -1 },
	/*14*/ { ANIM_YEET_FWD_RT,   0, 0.5, 6,  29, -1 },
	/*15*/ { ANIM_YEET_FWD_RT,   1, 0.5, 6,  29, -1 },
	/*16*/ { ANIM_YEET_BK_LF,    0, 0.5, 8,  25, -1 },
	/*17*/ { ANIM_YEET_BK_LF,    1, 0.5, 8,  25, -1 },
	/*18*/ { ANIM_YEET_BK,       0, 0.5, 8,  25, -1 },
	/*19*/ { ANIM_YEET_BK,       1, 0.5, 8,  25, -1 },
	/*20*/ { ANIM_YEET_BK_LF2,   0, 0.5, 12, 29, -1 },
	/*21*/ { ANIM_YEET_BK_LF2,   1, 0.5, 12, 29, -1 },
	/*22*/ { ANIM_YEET_LF,       0, 0.5, 22, 41, -1 },
	/*23*/ { ANIM_YEET_LF,       1, 0.5, 22, 41, -1 },
	/*24*/ { 0,                  0, 0.5, 0,  0,  -1 },
};

int8_t g_YeetAnimIndexesHumanAngle0[] = { 0,  1,  2,  3, 4, 5 };
int8_t g_YeetAnimIndexesHumanAngle1[] = { 7,  9,  11          };
int8_t g_YeetAnimIndexesHumanAngle2[] = { 6,  8,  10          };
int8_t g_YeetAnimIndexesHumanAngle3[] = { 12, 15, 0           };
int8_t g_YeetAnimIndexesHumanAngle4[] = { 13, 14, 0           };
int8_t g_YeetAnimIndexesHumanAngle5[] = { 18, 19, 20, 21      };
int8_t g_YeetAnimIndexesHumanAngle6[] = { 16, 22              };
int8_t g_YeetAnimIndexesHumanAngle7[] = { 17, 23              };

struct yeetanim g_YeetAnimsSkedar[] = {
	/* 0*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 1*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 2*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 3*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 4*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 5*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 6*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 7*/ { ANIM_033F, 0, 0.5, 0, -1, -1 },
	/* 8*/ { 0,         0, 0.5, 0, 0,  -1 },
};

int8_t g_YeetAnimIndexesSkedarAngle0[] = { 0 };
int8_t g_YeetAnimIndexesSkedarAngle1[] = { 1 };
int8_t g_YeetAnimIndexesSkedarAngle2[] = { 2 };
int8_t g_YeetAnimIndexesSkedarAngle3[] = { 3 };
int8_t g_YeetAnimIndexesSkedarAngle4[] = { 4 };
int8_t g_YeetAnimIndexesSkedarAngle5[] = { 7 };
int8_t g_YeetAnimIndexesSkedarAngle6[] = { 5 };
int8_t g_YeetAnimIndexesSkedarAngle7[] = { 6 };

struct yeetanimindexlist {
	int8_t *indexes;
	int count;
};

struct yeetanimindexlist g_YeetAnimIndexesByRaceAngle[][8] = {
	{
		{ g_YeetAnimIndexesHumanAngle0, 6 },
		{ g_YeetAnimIndexesHumanAngle1, 3 },
		{ g_YeetAnimIndexesHumanAngle3, 3 },
		{ g_YeetAnimIndexesHumanAngle6, 2 },
		{ g_YeetAnimIndexesHumanAngle5, 4 },
		{ g_YeetAnimIndexesHumanAngle7, 2 },
		{ g_YeetAnimIndexesHumanAngle4, 3 },
		{ g_YeetAnimIndexesHumanAngle2, 3 },
	}, {
		{ g_YeetAnimIndexesSkedarAngle0, 1 },
		{ g_YeetAnimIndexesSkedarAngle1, 1 },
		{ g_YeetAnimIndexesSkedarAngle3, 1 },
		{ g_YeetAnimIndexesSkedarAngle6, 1 },
		{ g_YeetAnimIndexesSkedarAngle5, 1 },
		{ g_YeetAnimIndexesSkedarAngle7, 1 },
		{ g_YeetAnimIndexesSkedarAngle4, 1 },
		{ g_YeetAnimIndexesSkedarAngle2, 1 },
	},
};

struct attackanimconfig var800656c0[] = {
	{ ANIM_RIFLE_SHOOT_0002, 28, 0, 0, 0, -1, 23, 54, -1, -1, 18, 54, 0.87252569198608, -0.52351540327072, 1.0470308065414, -0.34901028871536, 1.6000000238419, 1.7999999523163 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,               0,                 0,               0               },
};

struct attackanimgroup var80065750 = { var800656c0, 0xffffffff };

struct attackanimconfig var80065758[] = {
	{ ANIM_RIFLE_IDLE_TO_FIRE, 37, 0, 0, 0, -1, 30, 81, -1, -1, 25, 81, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 1.6000000238419, 1.75 },
	{ ANIM_RIFLE_SHOOT_0003, 27, 0, 0, 0, -1, 22, 61, -1, -1, 17, 61, 0.87252569198608, -0.26175770163536, 0.69802057743073, -0.69802057743073, 2,               1    },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0,               0    },
};

struct attackanimgroup var80065830 = { var80065758, 0xffffffff };

struct attackanimconfig var80065838[] = {
	{ ANIM_RIFLE_IDLE_TO_FIRE, 37, 0, 0, 0, -1, 30, 81, -1, -1, 25, 81, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 1.6000000238419, 1.75 },
	{ ANIM_RIFLE_SHOOT_0003, 27, 0, 0, 0, -1, 22, 61, -1, -1, 17, 61, 0.87252569198608, -0.26175770163536, 0.69802057743073, -0.69802057743073, 2,               1    },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0,               0    },
};

struct attackanimgroup var80065910 = { var80065838, 0xffffffff };

struct attackanimconfig var80065918[] = {
	{ ANIM_RIFLE_SHOOT_LF_0004, 19, 0, 1.5707963705063, 0, -1, 19, 61, -1, -1, 14, 61, 0.87252569198608, -0.34901028871536, 0.43626284599304, -1.0470308065414, 2.5, 2.5 },
	{ 0, 0,  0, 0,               0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                0,   0   },
};

struct attackanimgroup var800659a8 = { var80065918, 0xffffffff };

struct attackanimconfig var800659b0[] = {
	{ ANIM_RIFLE_SHOOT_RT_0006, 27, 0, 0, 0, -1, 39, 74, -1, -1, 34, 74, 0.87252569198608, -0.69802057743073, 0.7852731347084, -0.69802057743073, 1.5, 1.5 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,               0,                 0,   0   },
};

struct attackanimgroup var80065a40 = { var800659b0, 0xffffffff };

struct attackanimconfig var80065a48[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80065ad8 = { var80065a48, 0xffffffff };

struct attackanimgroup *g_StandHeavyAttackAnims[][32] = {
	{
		// RACE_HUMAN
		&var80065750, &var80065830, &var80065830, &var80065830,
		&var80065830, &var80065830, &var80065830, &var80065830,
		&var80065830, &var80065830, &var800659a8, &var800659a8,
		&var800659a8, &var800659a8, &var800659a8, &var800659a8,
		&var80065a40, &var80065a40, &var80065a40, &var80065a40,
		&var80065a40, &var80065a40, &var80065910, &var80065910,
		&var80065910, &var80065910, &var80065910, &var80065910,
		&var80065910, &var80065910, &var80065910, &var80065750,
	}, {
		// RACE_SKEDAR
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
		&var80065ad8, &var80065ad8, &var80065ad8, &var80065ad8,
	},
};

struct attackanimconfig var80065be0[] = {
	{ ANIM_PISTOL_FIRE_0041, 26, 0, 0, 12, 140, 58, 92, 60, 79, 20, 120, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 0, 0 },
	{ ANIM_PISTOL_FIRE_0044, 0,  0, 0, 17, 100, 25, 87, 30, 55, 20, 93,  0.87252569198608, -0.69802057743073, 0.69802057743073, -1.0470308065414,  0, 0 },
	{ ANIM_PISTOL_FIRE_0045, 0,  0, 0, 12, 64,  19, 51, 24, 46, 14, 58,  0.87252569198608, -0.69802057743073, 0.52351540327072, -0.7852731347084,  0, 0 },
	{ ANIM_PISTOL_FIRE_0046, 22, 0, 0, 4,  69,  22, 49, 22, 33, 8,  58,  0.87252569198608, -0.69802057743073, 0.43626284599304, -0.7852731347084,  0, 0 },
	{ 0, 0,  0, 0, 0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80065d48 = { var80065be0, 0xffffffff };

struct attackanimconfig var80065d50[] = {
	{ ANIM_PISTOL_FIRE_0041, 26, 0, 0, 12, 140, 58, 92, 60, 79, 20, 120, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 0, 0 },
	{ ANIM_PISTOL_FIRE_0046, 22, 0, 0, 4,  69,  22, 49, 22, 33, 8,  58,  0.87252569198608, -0.69802057743073, 0.43626284599304, -0.7852731347084,  0, 0 },
	{ 0, 0,  0, 0, 0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80065e28 = { var80065d50, 0xffffffff };

struct attackanimconfig var80065e30[] = {
	{ ANIM_PISTOL_FIRE_0041, 26, 0, 0,               12, 140, 58, 92, 60, 79, 20, 120, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 0, 0 },
	{ ANIM_PISTOL_FIRE_0046, 22, 0, 0,               4,  69,  22, 49, 22, 33, 8,  58,  0.87252569198608, -0.69802057743073, 0.43626284599304, -0.7852731347084,  0, 0 },
	{ ANIM_PISTOL_FIRE_0049, 0,  0, 1.5707963705063, 7,  130, 45, 93, 56, 73, 26, 107, 0.87252569198608, -0.69802057743073, 0.34901028871536, -0.52351540327072, 0, 0 },
	{ ANIM_PISTOL_FIRE_004A, 15, 0, 1.5707963705063, 5,  76,  20, 31, 31, 38, 15, 49,  0.87252569198608, -0.69802057743073, 0.52351540327072, -1.0470308065414,  0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80065f98 = { var80065e30, 0xffffffff };

struct attackanimconfig var80065fa0[] = {
	{ ANIM_PISTOL_FIRE_0041, 26, 0, 0,               12, 140, 58, 92,  60, 79, 20, 120, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 0, 0 },
	{ ANIM_PISTOL_FIRE_0046, 22, 0, 0,               4,  69,  22, 49,  22, 33, 8,  58,  0.87252569198608, -0.69802057743073, 0.43626284599304, -0.7852731347084,  0, 0 },
	{ ANIM_PISTOL_FIRE_0047, 0,  0, 4.7123889923096, 7,  139, 54, 105, 61, 88, 26, 120, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.61076802015305, 0, 0 },
	{ ANIM_PISTOL_FIRE_0048, 19, 0, 4.7123889923096, 4,  79,  21, 50,  26, 42, 10, 64,  0.87252569198608, -0.69802057743073, 0.69802057743073, -0.61076802015305, 0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,   0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066108 = { var80065fa0, 0xffffffff };

struct attackanimconfig var80066110[] = {
	{ ANIM_PISTOL_FIRE_004A, 19, 0, 1.5707963705063, 5, 76, 20, 31, 31, 38, 15, 49, 0.87252569198608, -0.69802057743073, 0.52351540327072, -1.0470308065414, 0, 0 },
	{ 0, 0,  0, 0,               0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                0, 0 },
};

struct attackanimgroup var800661a0 = { var80066110, 0xffffffff };

struct attackanimconfig var800661a8[] = {
	{ ANIM_PISTOL_FIRE_0048, 19, 0, 4.7123889923096, 4, 79, 21, 50, 26, 42, 10, 64, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.61076802015305, 0, 0 },
	{ 0, 0,  0, 0,               0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066238 = { var800661a8, 0xffffffff };

struct attackanimconfig var80066240[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var800662d0 = { var80066240, 0xffffffff };

struct attackanimgroup *g_StandLightAttackAnims[][32] = {
	{
		// RACE_HUMAN
		&var80065d48, &var80065d48, &var80065e28, &var80065e28,
		&var80065e28, &var80065f98, &var80065f98, &var80065f98,
		&var80065f98, &var80065f98, &var800661a0, &var800661a0,
		&var800661a0, &var800661a0, &var800661a0, &var800661a0,
		&var80066238, &var80066238, &var80066238, &var80066238,
		&var80066238, &var80066238, &var80066108, &var80066108,
		&var80066108, &var80066108, &var80066108, &var80065e28,
		&var80065e28, &var80065e28, &var80065d48, &var80065d48,
	}, {
		// RACE_SKEDAR
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
		&var800662d0, &var800662d0, &var800662d0, &var800662d0,
	},
};


struct attackanimconfig var800663d8[] = {
	{ ANIM_AIM_DUAL_PISTOLS, 26, 0, 0, 7, 92, 28, 68, -1, -1, 11, 73, 0.87252569198608, -0.69802057743073, 0.69802057743073, -0.69802057743073, 0, 0 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066468 = { var800663d8, 0xffffffff };

struct attackanimconfig var80066470[] = {
	{ ANIM_DUAL_PISTOLS_SWING_LF,  26, 0, 1.5707963705063, 9,  112, 38, 87, -1, -1, 19, 98, 0.87252569198608, -0.69802057743073, 0.43626284599304, -0.43626284599304, 0, 0 },
	{ ANIM_DUAL_PISTOLS_SWING_LF2, 25, 0, 1.5707963705063, 10, 112, 32, 86, -1, -1, 19, 97, 0.87252569198608, -0.69802057743073, 0.43626284599304, -0.43626284599304, 0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066548 = { var80066470, 0xffffffff };

struct attackanimconfig var80066550[] = {
	{ ANIM_DUAL_PISTOLS_SWING_RT,  39, 0, 4.7123889923096, 22, 127, 44, 102, -1, -1, 28, 112, 0.87252569198608, -0.69802057743073, 0.43626284599304, -0.43626284599304, 0, 0 },
	{ ANIM_DUAL_PISTOLS_SWING_RT2, 39, 0, 4.7123889923096, 23, 130, 46, 100, -1, -1, 30, 110, 0.87252569198608, -0.69802057743073, 0.43626284599304, -0.43626284599304, 0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,   0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066628 = { var80066550, 0xffffffff };

struct attackanimconfig var80066630[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var800666c0 = { var80066630, 0xffffffff };

struct attackanimgroup *g_StandDualAttackAnims[][32] = {
	{
		// RACE_HUMAN
		&var80066468, &var80066468, &var80066468, &var80066468,
		&var80066468, &var80066548, &var80066548, &var80066548,
		&var80066548, &var80066548, &var80066548, &var80066548,
		&var80066548, &var80066548, &var80066548, &var80066548,
		&var80066628, &var80066628, &var80066628, &var80066628,
		&var80066628, &var80066628, &var80066628, &var80066628,
		&var80066628, &var80066628, &var80066628, &var80066468,
		&var80066468, &var80066468, &var80066468, &var80066468,
	}, {
		// RACE_SKEDAR
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
		&var800666c0, &var800666c0, &var800666c0, &var800666c0,
	},
};

struct attackanimconfig var800667c8[] = {
	{ ANIM_RIFLE_KNEEL_SHOOT_0007, 27, 0, 0, 0, -1, 35, 75, -1, -1, 31, 75, 0.87252569198608, -0.69802057743073, 0.90742671489716, -0.69802057743073, 1.5,             1.5             },
	{ ANIM_RIFLE_KNEEL_SHOOT_0008, 24, 0, 0, 0, -1, 46, 98, -1, -1, 41, 98, 0.87252569198608, -0.52351540327072, 1.1342834234238,  -0.69802057743073, 1.6000000238419, 1.6000000238419 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0,               0               },
};

struct attackanimgroup var800668a0 = { var800667c8, 0xffffffff };

struct attackanimconfig var800668a8[] = {
	{ ANIM_RIFLE_KNEEL_SHOOT_LF, 26, 0, 0, 0, -1, 34, 87, -1, -1, 29, 87, 0.87252569198608, -0.52351540327072, 0.69802057743073, -0.95977824926376, 1.6000000238419, 2 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0,               0 },
};

struct attackanimgroup var80066938 = { var800668a8, 0xffffffff };

struct attackanimconfig var80066940[] = {
	{ ANIM_RIFLE_KNEEL_SHOOT_RT, 28, 0, 0, 0, -1, 36, 88, -1, -1, 31, 88, 0.87252569198608, -0.69802057743073, 0.87252569198608, -0.43626284599304, 1.6000000238419, 1.5 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0,               0   },
};

struct attackanimgroup var800669d0 = { var80066940, 0xffffffff };

struct attackanimconfig var800669d8[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066a68 = { var800669d8, 0xffffffff };

struct attackanimgroup *g_KneelHeavyAttackAnims[][32] = {
	{
		&var800668a0, &var800668a0, &var800668a0, &var800668a0,
		&var800668a0, &var800668a0, &var800668a0, &var800668a0,
		&var800668a0, &var800668a0, &var80066938, &var80066938,
		&var80066938, &var80066938, &var80066938, &var80066938,
		&var800669d0, &var800669d0, &var800669d0, &var800669d0,
		&var800669d0, &var800669d0, &var800668a0, &var800668a0,
		&var800668a0, &var800668a0, &var800668a0, &var800668a0,
		&var800668a0, &var800668a0, &var800668a0, &var800668a0,
	}, {
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
		&var80066a68, &var80066a68, &var80066a68, &var80066a68,
	},
};

struct attackanimconfig var80066b70[] = {
	{ ANIM_KNEEL_SHOOT_PISTOL_RT_004B, 25, 0, 0, 12, 132, 55, 87, 67, 87, 26, 111, 0.87252569198608, -0.69802057743073, 0.61076802015305, -0.7852731347084, 0, 0 },
	{ ANIM_KNEEL_SHOOT_PISTOL_RT_004C, 26, 0, 0, 8,  89,  31, 63, 41, 51, 21, 80,  0.87252569198608, -0.69802057743073, 0.34901028871536, -1.1342834234238, 0, 0 },
	{ 0, 0,  0, 0, 0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                0, 0 },
};

struct attackanimgroup var80066c48 = { var80066b70, 0xffffffff };

struct attackanimconfig var80066c50[] = {
	{ ANIM_KNEEL_SHOOT_PISTOL_LF_004F, 47, 0, 1.5707963705063, 7,  128, 33, 86, 47, 74, 23, 106, 0.87252569198608, -0.52351540327072,  0.52351540327072, -0.7852731347084,  0,   0 },
	{ ANIM_KNEEL_SHOOT_PISTOL_LF_0050, 18, 0, 1.5707963705063, 7,  78,  28, 52, 35, 45, 15, 66,  0.87252569198608, -0.087252572178841, 0.69802057743073, -0.7852731347084,  1.5, 1 },
	{ ANIM_KNEEL_SHOOT_PISTOL_LF_0051, 20, 0, 1.5707963705063, 13, 92,  37, 67, 42, 55, 25, 84,  0.87252569198608, -0.52351540327072,  0.34901028871536, -0.69802057743073, 0,   0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                  0,                0,                 0,   0 },
};

struct attackanimgroup var80066d70 = { var80066c50, 0xffffffff };

struct attackanimconfig var80066d78[] = {
	{ ANIM_KNEEL_SHOOT_PISTOL_RT_004D, 28, 0, 4.7123889923096, 15, 124, 38, 97, 60, 84, 20, 106, 0.87252569198608, -0.69802057743073, 0.52351540327072, -0.87252569198608, 0, 0 },
	{ ANIM_KNEEL_SHOOT_PISTOL_RT_004E, 23, 0, 4.7123889923096, 0,  85,  32, 38, 38, 60, 14, 71,  0.87252569198608, -0.69802057743073, 0.61076802015305, -0.95977824926376, 0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066e50 = { var80066d78, 0xffffffff };

struct attackanimconfig var80066e58[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80066ee8 = { var80066e58, 0xffffffff };

struct attackanimgroup *g_KneelLightAttackAnims[][32] = {
	{
		&var80066c48, &var80066c48, &var80066c48, &var80066c48,
		&var80066c48, &var80066c48, &var80066c48, &var80066c48,
		&var80066c48, &var80066c48, &var80066d70, &var80066d70,
		&var80066d70, &var80066d70, &var80066d70, &var80066d70,
		&var80066e50, &var80066e50, &var80066e50, &var80066e50,
		&var80066e50, &var80066e50, &var80066c48, &var80066c48,
		&var80066c48, &var80066c48, &var80066c48, &var80066c48,
		&var80066c48, &var80066c48, &var80066c48, &var80066c48,
	}, {
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
		&var80066ee8, &var80066ee8, &var80066ee8, &var80066ee8,
	},
};

struct attackanimconfig var80066ff0[] = {
	{ ANIM_KNEEL_DUAL_PISTOLS, 22, 0, 0, 10, 111, 34, 87, -1, -1, 17, 104, 0.87252569198608, -0.69802057743073, 0.61076802015305, -0.7852731347084, 0, 0 },
	{ ANIM_KNEEL_DUAL_PISTOLS_CROSSED, 25, 0, 0, 9,  92,  33, 62, -1, -1, 18, 69,  0.87252569198608, -0.69802057743073, 0.61076802015305, -0.7852731347084, 0, 0 },
	{ 0, 0,  0, 0, 0,  -1,  0,  0,  0,  0,  0,  0,   0,                0,                 0,                0,                0, 0 },
};

struct attackanimgroup var800670c8 = { var80066ff0, 0xffffffff };

struct attackanimconfig var800670d0[] = {
	{ ANIM_KNEEL_DUAL_PISTOLS_LF, 28, 0, 1.5707963705063, 15, 108, 34, 73, -1, -1, 17, 93, 0.87252569198608, -0.69802057743073, 0.52351540327072, -0.7852731347084, 0,   0 },
	{ ANIM_KNEEL_DUAL_PISTOLS_LF2, 19, 0, 1.5707963705063, 3,  95,  30, 64, -1, -1, 14, 71, 0.87252569198608, -0.69802057743073, 0.52351540327072, -0.7852731347084, 1.5, 1 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                0,   0 },
};

struct attackanimgroup var800671a8 = { var800670d0, 0xffffffff };

struct attackanimconfig var800671b0[] = {
	{ ANIM_KNEEL_DUAL_PISTOLS_RT,  31, 0, 4.7123889923096, 14, 111, 40, 83, -1, -1, 21, 94, 0.87252569198608, -0.69802057743073, 0.52351540327072, -0.7852731347084, 0, 0 },
	{ ANIM_KNEEL_DUAL_PISTOLS_RT2, 26, 0, 4.7123889923096, 7,  89,  34, 60, -1, -1, 20, 68, 0.87252569198608, -0.69802057743073, 0.52351540327072, -0.7852731347084, 0, 0 },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                0, 0 },
};

struct attackanimgroup var80067288 = { var800671b0, 0xffffffff };

struct attackanimconfig var80067290[] = {
	{ ANIM_034A, 20, 0, 0, 0, -1, 25, 50, -1, -1, 10, 50, 0.34901028871536, -0.34901028871536, 0.52351540327072, -0.52351540327072, 1, 1 },
	{ 0, 0,  0, 0, 0, -1, 0,  0,  0,  0,  0,  0,  0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80067320 = { var80067290, 0xffffffff };

struct attackanimgroup *g_KneelDualAttackAnims[][32] = {
	{
		&var800670c8, &var800670c8, &var800670c8, &var800670c8,
		&var800670c8, &var800670c8, &var800670c8, &var800670c8,
		&var800670c8, &var800670c8, &var800671a8, &var800671a8,
		&var800671a8, &var800671a8, &var800671a8, &var800671a8,
		&var80067288, &var80067288, &var80067288, &var80067288,
		&var80067288, &var80067288, &var800670c8, &var800670c8,
		&var800670c8, &var800670c8, &var800670c8, &var800670c8,
		&var800670c8, &var800670c8, &var800670c8, &var800670c8,
	}, {
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
		&var80067320, &var80067320, &var80067320, &var80067320,
	},
};

struct attackanimconfig g_RollAttackAnims[] = {
	{ ANIM_RIFLE_SOMERSAULT_000B,                       76, 0, 0, 20, -1,  98, 161, -1, -1, 93, 161, 0.87252569198608, -0.52351540327072,  0.69802057743073, -0.69802057743073, 1.7000000476837, 2               },
	{ ANIM_RIFLE_SOMERSAULT_000C,                       58, 0, 0, 10, -1,  77, 104, -1, -1, 72, 104, 0.87252569198608, -0.34901028871536,  0.61076802015305, -0.69802057743073, 1.5499999523163, 1.5             },
	{ ANIM_RIFLE_SOMERSAULT_000D,                       61, 0, 0, 10, -1,  83, 128, -1, -1, 78, 128, 0.87252569198608, -0.52351540327072,  0.87252569198608, -0.52351540327072, 1.2000000476837, 1.2999999523163 },
	{ ANIM_RIFLE_SOMERSAULT_0027,         63, 0, 0,               10, -1,  73, 114, -1, -1, 68, 114, 0.87252569198608, -0.52351540327072,  0.61076802015305, -0.61076802015305, 1.6499999761581, 1.5             },
	{ ANIM_RIFLE_SOMERSAULT_000B,                       76, 0, 0, 20, 76,  98, 161, -1, -1, 93, 161, 0.87252569198608, -0.52351540327072,  0.69802057743073, -0.69802057743073, 1.7000000476837, 2               },
	{ ANIM_RIFLE_SOMERSAULT_000C,                       58, 0, 0, 10, 63,  77, 104, -1, -1, 72, 104, 0.87252569198608, -0.34901028871536,  0.61076802015305, -0.69802057743073, 1.5499999523163, 1.5             },
	{ ANIM_RIFLE_SOMERSAULT_000D,                       61, 0, 0, 10, 56,  83, 128, -1, -1, 78, 128, 0.87252569198608, -0.52351540327072,  0.87252569198608, -0.52351540327072, 1.2000000476837, 1.2999999523163 },
	{ ANIM_RIFLE_SOMERSAULT_0027, 63, 0, 0,                       10, 50,  73, 114, -1, -1, 68, 114, 0.87252569198608, -0.52351540327072,  0.61076802015305, -0.61076802015305, 1.6499999761581, 1.5             },
	{ ANIM_PISTOL_FIRE_0045,              0,  0, 0,               7,  64,  19, 51,  24, 46, 14, 58,  0.87252569198608, -0.69802057743073,  0.52351540327072, -0.7852731347084,  0,               0               },
	{ ANIM_PISTOL_FIRE_004A,              0,  0, 1.5707963705063, 14, 76,  26, 31,  31, 38, 15, 49,  0.87252569198608, -0.69802057743073,  0.52351540327072, -1.0470308065414,  0,               0               },
	{ ANIM_KNEEL_SHOOT_PISTOL_RT_004C,    26, 0, 0,               25, 89,  41, 63,  41, 51, 21, 80,  0.87252569198608, -0.69802057743073,  0.34901028871536, -1.1342834234238,  0,               0               },
	{ ANIM_KNEEL_SHOOT_PISTOL_LF_0050,    18, 0, 1.5707963705063, 11, 78,  33, 52,  35, 45, 15, 66,  0.87252569198608, -0.087252572178841, 0.69802057743073, -0.7852731347084,  1.5,             1               },
	{ ANIM_AIM_DUAL_PISTOLS,              26, 0, 0,               7,  92,  28, 68,  -1, -1, 11, 73,  0.87252569198608, -0.69802057743073,  0.69802057743073, -0.69802057743073, 0,               0               },
	{ ANIM_DUAL_PISTOLS_SWING_LF,         26, 0, 1.5707963705063, 9,  112, 38, 87,  -1, -1, 19, 98,  0.87252569198608, -0.69802057743073,  0.43626284599304, -0.43626284599304, 0,               0               },
	{ ANIM_KNEEL_DUAL_PISTOLS,                          22, 0, 0, 10, 11,  34, 87,  -1, -1, 17, 104, 0.87252569198608, -0.69802057743073,  0.61076802015305, -0.7852731347084,  0,               0               },
	{ ANIM_KNEEL_DUAL_PISTOLS_LF,         28, 0, 1.5707963705063, 15, 108, 34, 73,  -1, -1, 17, 93,  0.87252569198608, -0.69802057743073,  0.52351540327072, -0.7852731347084,  0,               0               },
	{ ANIM_AIM_DUAL_PISTOLS,                            26, 0, 0, 7,  92,  28, 68,  -1, -1, 11, 73,  0.87252569198608, -0.69802057743073,  0.69802057743073, -0.69802057743073, 0,               0               },
	{ ANIM_DUAL_PISTOLS_SWING_LF2,        25, 0, 1.5707963705063, 10, 112, 32, 86,  -1, -1, 19, 97,  0.87252569198608, -0.69802057743073,  0.43626284599304, -0.43626284599304, 0,               0               },
	{ ANIM_KNEEL_DUAL_PISTOLS_CROSSED,    25, 0, 0,               9,  92,  33, 62,  -1, -1, 18, 69,  0.87252569198608, -0.69802057743073,  0.61076802015305, -0.7852731347084,  0,               0               },
	{ ANIM_KNEEL_DUAL_PISTOLS_LF2,        19, 0, 1.5707963705063, 3,  95,  30, 64,  -1, -1, 14, 71,  0.87252569198608, -0.69802057743073,  0.52351540327072, -0.7852731347084,  1.5,             1               },
	{ 0, 0,  0, 0,               0,  -1,  0,  0,   0,  0,  0,  0,   0,                0,                  0,                0,                 0,               0               },
};

struct attackanimconfig g_WalkAttackAnims[] = {
	{ ANIM_WALK_RIFLE_POINTED,             0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 1.3999999761581, 1.2999999523163 },
	{ ANIM_JOG_RIFLE_POINTED,              0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 1.1000000238419, 1.2000000476837 },
	{ ANIM_WALK_AIM_PISTOL_FWD,            0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
	{ ANIM_RUN_AIM_PISTOL_FWD_0055,        0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
	{ ANIM_WALK_FWD_AIMING_GUNS,           0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
	{ ANIM_RUN_FWD_AIMING_GUNS_006E,       0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
	{ ANIM_WALK_FWD_ARMS_CROSSED,          0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
	{ ANIM_RUN_FWD_ARMS_CROSSED_006F,      0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0,               0               },
#ifndef PLATFORM_N64
	// ensure this is terminated
	{ 0, 0, 0, 0,               0, -1, 0, 0, 0,  0,  0, 0, 0,                0,                 0,                0,                 0, 0 },
#endif
};

struct attackanimconfig var80067c50[] = {
	{ ANIM_RUN_AIM_PISTOL_LF, 0, 0, 1.5707963705063, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0, 0 },
	{ ANIM_RUN_AIM_PISTOL_RT, 0, 0, 4.7123889923096, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0, 0 },
	{ 0, 0, 0, 0,               0, -1, 0, 0, 0,  0,  0, 0, 0,                0,                 0,                0,                 0, 0 },
};

struct attackanimconfig var80067d28[] = { ANIM_SNIPING_GETDOWN, 0, 0, 0, 0, 236, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0, 0 };

struct attackanimconfig var80067d70[] = {
	{ ANIM_SNIPING_ONGROUND, 0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0, 0 },
	{ ANIM_SNIPING_GETUP, 0, 0, 0, 0, -1, 0, 0, -1, -1, 0, 0, 0.87252569198608, -0.52351540327072, 0.52351540327072, -0.52351540327072, 0, 0 },
	{ 0, 0, 0, 0, 0, -1, 0, 0, 0,  0,  0, 0, 0,                0,                 0,                0,                 0, 0 },
};

struct attackanimgroup var80067e48 = { var80067d28, 0xffffffff };

struct attackanimgroup *g_LieAttackAnims = &var80067e48;

uint32_t g_StageFlags = 0;

struct chrdata *g_BgChrs = NULL;
int16_t *g_BgChrnums = NULL;
int g_NumBgChrs = 0;

int16_t *g_TeamList = NULL;
int16_t *g_SquadronList = NULL;

struct animsmovement humanmoveanims[] = {
	{ ANIM_RIFLE_PATROL,                0 },
	{ ANIM_RUNNING_TWOHANDGUN,          0 },
	{ ANIM_SPRINT_RIFLE,                0 },
	{ ANIM_WALK_FWD_006B,               0 },
	{ ANIM_RUNNING_ONEHANDGUN,          0 },
	{ ANIM_RUN_WITH_PISTOL_005A,        0 },
	{ ANIM_WALK_FWD_0072,               0 },
	{ ANIM_JOG_0073,                    0 },
	{ ANIM_RUN_WITH_PISTOL_005A,        0 },
	{ ANIM_WALK_FWD_AIMING_GUNS,        0 },
	{ ANIM_WALK_RIFLE_POINTED,          0 },
	{ ANIM_JOG_RIFLE_POINTED,           0 },
	{ ANIM_WALK_AIM_PISTOL_FWD,         0 },
	{ ANIM_RUN_AIM_PISTOL_FWD_0055,     0 },
	{ ANIM_RUN_FWD_AIMING_GUNS_006E,    0 },
	{ ANIM_RUN_FWD_ARMS_CROSSED_006F,   0 },
	{ ANIM_RUN_AIM_PISTOL_LF,           0 },
	{ ANIM_RUN_AIM_PISTOL_RT,           0 },
	{ ANIM_WALK_FWD_ARMS_CROSSED,       0 },
	{ ANIM_RUNNING_ONEHANDGUN,          0 },
	{ ANIM_020A,                        0 },
	{ ANIM_020D,                        0 },
	{ ANIM_01F9,                        0 },
	{ ANIM_01F8,                        0 },
	{ ANIM_021D,                        0 },
	{ ANIM_WALK_FORWARD01,              0 },
	{ ANIM_RIFLE_WALK,                  0 },
	{ ANIM_WALK_FWD_001B,               0 },
	{ ANIM_RUN_FWD_001D,                0 },
	{ ANIM_RUN_FWD_001E,                0 },
	{ ANIM_FEMALE_WALK,                 0 },
	{ ANIM_FEMALE_JOG,                  0 },
	{ ANIM_FEMALE_RUN,                  0 },
	{ ANIM_JOG_COVER_FACE,              0 },
	{ -1 },
};

struct animsmovement skedarmoveanims[] = {
	{ ANIM_0392, 0 },
	{ ANIM_0393, 0 },
	{ ANIM_SKEDAR_RUNNING, 0 },
	{ -1 },
};

struct animsmovement var80067fa4[] = {
	{ ANIM_015F, 0 },
	{ ANIM_0160, 0 },
	{ -1 },
};

struct animsmovement var80067fbc[] = {
	{ ANIM_015F, 0 },
	{ -1 },
};

struct animsmovement var80067fcc[] = {
	{ ANIM_0238, 0 },
	{ -1 },
};

struct animsmovement *g_MoveAnims[] = {
	humanmoveanims,
	skedarmoveanims,
	var80067fa4,
	var80067fbc,
	var80067fcc,
};

float func0f02dff0(int16_t animnum)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_MoveAnims); i++) {
		int j = 0;
		int16_t thisanimnum = g_MoveAnims[i][j].animnum;

		while (thisanimnum >= 0) {
			if (thisanimnum == animnum) {
				return g_MoveAnims[i][j].value;
			}

			j++;
			thisanimnum = g_MoveAnims[i][j].animnum;
		}
	}

	return 1.0f;
}

bool chrGoPosIsWaiting(struct chrdata *chr)
{
	static int16_t list1[] = { ANIM_TWO_GUN_HOLD, ANIM_STAND, -1 };
	static int16_t list2[] = { ANIM_00C0, -1 };
	static int16_t list3[] = { ANIM_013E, -1 };
	static int16_t list4[] = { ANIM_013E, -1 };
	static int16_t list5[] = { ANIM_0237, -1 };
	static int16_t *waitableanims[] = { list1, list2, list3, list4, list5 };

	if (chr->aibot) {
		if (chr->actiontype == ACT_STAND
				|| (chr->actiontype == ACT_GOPOS && (chr->act_gopos.flags & GOPOSFLAG_WAITING))) {
			return true;
		}
	} else {
		int16_t animnum = modelGetAnimNum(chr->model);
		int i;

		for (i = 0; i < ARRAYCOUNT(waitableanims); i++) {
			int16_t thisanimnum;
			int j;

			for (j = 0; (thisanimnum = waitableanims[i][j]) >= 0; j++) {
				if (thisanimnum == animnum) {
					return true;
				}
			}
		}
	}

	return false;
}

bool weaponIsOneHanded(struct prop *prop)
{
	if (prop) {
		struct weaponobj *weapon = prop->weapon;
		return weaponHasFlag(weapon->weaponnum, WEAPONFLAG_ONEHANDED);
	}

	return false;
}

/**
 * Returns a value between min and max based on the chr's speedrating property.
 *
 * chr->speedrating is between 0 and 100. The result is scaled between min and
 * max accordingly.
 *
 * This function also applies the PD mode reaction speed, but the PD mode
 * reaction speed is always zero because PD doesn't have it in the settings.
 * It was used in GE but disabled in PD.
 */
float chrGetRangedSpeed(struct chrdata *chr, float min, float max)
{
	float speedrating = chr->speedrating;

	speedrating = pdmodeGetEnemyReactionSpeed() * (100.0f - speedrating) + speedrating;

	return (max - min) * speedrating * 0.01f + min;
}

/**
 * Calculates a percentage of how slow the chr is.
 *
 * percentage is expected to be between 0 and 100.
 * chr->speedrating is between 0 and 100.
 *
 * This function takes the difference between the speedrating and the max (100),
 * then multiplies that amount by the given percentage and returns it.
 *
 * For example, if the chr's speedrating is 10 (out of 100) and the given
 * percentage is 50, the result will be 45.
 */
int chrGetPercentageOfSlowness(struct chrdata *chr, int percentage)
{
	int speedrating = chr->speedrating;
	int extra = pdmodeGetEnemyReactionSpeed() * (100 - speedrating);

	speedrating = extra + speedrating;

	return (100 - speedrating) * percentage / 100;
}

float chrGetRangedArghSpeed(struct chrdata *chr, float min, float max)
{
	float arghrating = chr->arghrating;

	arghrating = pdmodeGetEnemyReactionSpeed() * (100.0f - arghrating) + arghrating;

	return (max - min) * arghrating * 0.01f + min;
}

float chrGetAttackEntityRelativeAngle(struct chrdata *chr, int attackflags, int entityid)
{
	float angle;
	struct coord pos;
	RoomNum rooms[8];

	if (attackflags & ATTACKFLAG_AIMFORWARD) {
		return 0;
	}

	if (attackflags & ATTACKFLAG_AIMATDIRECTION) {
		angle = entityid * (M_TAU / 65536);
		angle -= chrGetInverseTheta(chr);

		if (angle < 0) {
			angle += M_TAU;
		}

		return angle;
	}

	chrGetAttackEntityPos(chr, attackflags, entityid, &pos, rooms);

	return chrGetAngleToPos(chr, &pos);
}

float chrGetAttackEntityDistance(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	if (attackflags & ATTACKFLAG_AIMATTARGET) {
		return chrGetDistanceToTarget(chr);
	}

	if (attackflags & ATTACKFLAG_AIMATCHR) {
		return chrGetDistanceToChr(chr, entityid);
	}

	if (attackflags & ATTACKFLAG_AIMATPAD) {
		return chrGetDistanceToPad(chr, entityid);
	}

	return 0;
}

void func0f02e3dc(struct coord *a, struct coord *b,struct coord *c, struct coord *d, struct coord *dst)
{
	float value = d->z * (b->x - a->x) - (b->z - a->z) * d->x;

	if (value != 0) {
		float tmp = ((b->z - a->z) * (c->x - a->x) + (a->z - c->z) * (b->x - a->x)) / value;
		dst->x = d->x * tmp + c->x;
		dst->y = d->y * tmp + c->y;
		dst->z = d->z * tmp + c->z;
	} else if (d->x == 0 && d->z == 0) {
		dst->x = c->x;
		dst->y = c->y;
		dst->z = c->z;
	} else {
		dst->x = a->x;
		dst->y = a->y;
		dst->z = a->z;
	}
}

void func0f02e4f8(struct coord *arg0, struct coord *arg1, struct coord *dst)
{
	struct coord sp2c;
	struct coord sp20;

	cdGetEdge(&sp2c, &sp20);
	func0f02e3dc(&sp2c, &sp20, arg0, arg1, dst);
}

float func0f02e550(struct prop *prop, float arg1, float arg2, uint32_t cdtypes, float ymax, float ymin)
{
	struct coord sp5c;
	struct coord sp50;
	struct chrdata *chr = prop->chr;
	float result;
	struct coord sp3c;
	float xdiff;
	float zdiff;

	sp5c.x = sinf(arg1);
	sp5c.y = 0;
	sp5c.z = cosf(arg1);

	sp50.x = prop->pos.x + sp5c.f[0] * arg2;
	sp50.y = prop->pos.y;
	sp50.z = prop->pos.z + sp5c.f[2] * arg2;

	chrSetPerimEnabled(chr, false);

	if (cdExamCylMove03(&prop->pos, prop->rooms, &sp50, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION) {
		result = arg2;
	} else {
		cdGetPos(&sp3c);

		xdiff = sp3c.x - prop->pos.x;
		zdiff = sp3c.z - prop->pos.z;

		result = sqrtf(xdiff * xdiff + zdiff * zdiff);
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

float func0f02e684(struct prop *prop, float arg1, float arg2)
{
	float ymax;
	float ymin;
	float radius;

	chrGetBbox(prop, &radius, &ymax, &ymin);

	return func0f02e550(prop, arg1, arg2, CDTYPE_ALL, ymax, ymin);
}

void chrChooseStandAnimation(struct chrdata *chr, float mergetime)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	int race = CHRRACE(chr);
	int prevanimnum = modelGetAnimNum(chr->model);

	if (chr->actiontype == ACT_GOPOS) {
		chr->act_gopos.flags |= GOPOSFLAG_WAITING;
	}

	if (chr->aibot) {
		return;
	}

	if (race == RACE_EYESPY) {
		modelSetAnimation(chr->model, ANIM_013E, 0, 0, 0, mergetime);
	} else if (race == RACE_HUMAN) {
		if (prevanimnum == ANIM_SNIPING_GETDOWN
				|| prevanimnum == ANIM_SNIPING_GETUP
				|| prevanimnum == ANIM_SNIPING_ONGROUND) {
			modelSetAnimation(chr->model, ANIM_SNIPING_GETUP, chr->model->anim->flip, -1, chrGetRangedSpeed(chr, 0.5, 0.8), 16);
		} else if ((leftgun && rightgun) || (!leftgun && !rightgun)
				|| weaponIsOneHanded(leftgun)
				|| weaponIsOneHanded(rightgun)) {
			modelSetAnimation(chr->model, ANIM_STAND, rngRandom() % 2, 0, 0.25, mergetime);
			modelSetAnimLooping(chr->model, 0, 16);
		} else if (rightgun || leftgun) {
			modelSetAnimation(chr->model, ANIM_TWO_GUN_HOLD, leftgun != NULL, 0, 0.25, mergetime);
			modelSetAnimLooping(chr->model, 0, 16);
			modelSetAnimEndFrame(chr->model, 120);
		}
	} else if (race == RACE_SKEDAR) {
		modelSetAnimation(chr->model, ANIM_00C0, rngRandom() % 2, 0, 0.5, mergetime);
	} else if (race == RACE_DRCAROLL) {
		modelSetAnimation(chr->model, ANIM_013E, 0, 0, 0.5, mergetime);
	} else if (race == RACE_ROBOT) {
		modelSetAnimation(chr->model, ANIM_0237, 0, 0, 0.5, mergetime);
	}
}

void func0f02e9a0(struct chrdata *chr, float mergetime)
{
	float limit = 127;
	float fsleep;

	chrStopFiring(chr);
	chr->actiontype = ACT_STAND;
	chr->act_stand.prestand = false;
	chr->act_stand.flags = 0;
	chr->act_stand.entityid = 0;
	chr->act_stand.reaim = 0;
	chr->act_stand.turning = TURNSTATE_OFF;
	chr->act_stand.checkfacingwall = false;
	chr->act_stand.wallcount = rngRandom() % 120 + 180; // 180 to 299
	chr->act_stand.mergetime = mergetime;
	chr->act_stand.playwalkanim = false;

	fsleep = mergetime;

	if (chr->model->anim->playspeed != 1.0f) {
		fsleep *= 1.0f / chr->model->anim->playspeed;
	}

	if (fsleep > limit) {
		fsleep = limit;
	}

	chr->sleep = fsleep;

	if (modelIsAnimMerging(chr->model) && !chr->aibot) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrChooseStandAnimation(chr, mergetime);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrStand(struct chrdata *chr)
{
	int race = CHRRACE(chr);
	float result;

	if (race != RACE_EYESPY) {
		chrStopFiring(chr);

		if (race == RACE_HUMAN && chr->actiontype == ACT_KNEEL) {
			chrStopFiring(chr);

			chr->actiontype = ACT_STAND;
			chr->act_stand.prestand = true;
			chr->act_stand.flags = 0;
			chr->act_stand.entityid = 0;
			chr->act_stand.reaim = 0;
			chr->act_stand.turning = TURNSTATE_OFF;
			chr->act_stand.checkfacingwall = false;
			chr->act_stand.wallcount = rngRandom() % 120 + 180;
			chr->sleep = 0;
			chr->act_stand.playwalkanim = false;

			if (chr->aibot == NULL) {
				if (modelGetAnimNum(chr->model) == ANIM_KNEEL_SHOOT_PISTOL_RT_004B) {
					result = chrGetRangedSpeed(chr, 0.5, 0.8);
					modelSetAnimation(chr->model, ANIM_KNEEL_SHOOT_PISTOL_RT_004B,
							chr->model->anim->flip, 109, result, 16);
					modelSetAnimEndFrame(chr->model, 140);
				} else {
					result = chrGetRangedSpeed(chr, 0.5, 0.8);
					modelSetAnimation(chr->model, ANIM_RIFLE_KNEEL_SHOOT_0008,
							chr->model->anim->flip, 120, result, 16);
					modelSetAnimEndFrame(chr->model, 151);
				}
			}
		} else if (race == RACE_DRCAROLL || race == RACE_ROBOT) {
			chr->actiontype = ACT_STAND;
			chr->act_stand.prestand = true;
			chr->act_stand.flags = 0;
			chr->act_stand.entityid = 0;
			chr->act_stand.reaim = 0;
			chr->act_stand.turning = TURNSTATE_OFF;
			chr->act_stand.checkfacingwall = false;
			chr->act_stand.wallcount = rngRandom() % 120 + 180;
			chr->sleep = 0;
			chr->act_stand.playwalkanim = false;

			func0f02e9a0(chr, 16);
		} else {
			func0f02e9a0(chr, 16);
		}
	}
}

bool chrFaceCover(struct chrdata *chr)
{
	struct cover cover;

	if (!coverUnpack(chr->cover, &cover)) {
		return false;
	}

	chrStand(chr);
	chr->act_stand.reaim = 0;
	chr->act_stand.flags = ATTACKFLAG_AIMATDIRECTION;
	chr->act_stand.turning = TURNSTATE_TURNING;
	chr->act_stand.entityid = atan2f(-cover.look->x, -cover.look->z) * 10432.039f;

	return true;
}

void func0f02ed28(struct chrdata *chr, float mergetime)
{
	func0f02e9a0(chr, mergetime);

	chr->act_stand.checkfacingwall = true;
}

void chrStop(struct chrdata *chr)
{
	chrStand(chr);

	chr->act_stand.checkfacingwall = true;
}

void chrKneelChooseAnimation(struct chrdata *chr)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);

	if (chr->aibot == NULL) {
		if ((leftgun && rightgun)
				|| (!leftgun && !rightgun)
				|| weaponIsOneHanded(leftgun)
				|| weaponIsOneHanded(rightgun)) {
			bool flip = rngRandom() % 2;
			modelSetAnimation(chr->model, ANIM_KNEEL_SHOOT_PISTOL_RT_004B, flip, 0, chrGetRangedSpeed(chr, 0.5, 0.8), 16);
			modelSetAnimEndFrame(chr->model, 28);
		} else if (rightgun || leftgun) {
			modelSetAnimation(chr->model, ANIM_RIFLE_KNEEL_SHOOT_0008, leftgun != NULL, 0, chrGetRangedSpeed(chr, 0.5, 0.8), 16);
			modelSetAnimEndFrame(chr->model, 27);
		}
	}
}

void chrKneel(struct chrdata *chr)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_KNEEL;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrKneelChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrStartAlarmChooseAnimation(struct chrdata *chr)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip = false;

	if (leftgun && !rightgun) {
		flip = true;
	} else if ((leftgun && rightgun) || (!leftgun && !rightgun)) {
		flip = rngRandom() % 2;
	}

	modelSetAnimation(chr->model, ANIM_TALKING_003D, flip, 40, 1, 16);
	modelSetAnimEndFrame(chr->model, 82);
}

void chrStartAlarm(struct chrdata *chr)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_STARTALARM;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrStartAlarmChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrThrowGrenadeChooseAnimation(struct chrdata *chr)
{
	uint32_t rand = rngRandom();

	if (chr->act_throwgrenade.needsequip) {
		if (rand % 3 == 0) {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_CROUCHING, chr->act_throwgrenade.hand != 0, 0, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		} else if (rand % 3 == 1) {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_NOPIN, chr->act_throwgrenade.hand != 0, 0, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		} else {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_STANDING, chr->act_throwgrenade.hand != 0, 0, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		}
	} else {
		if (rand % 3 == 0) {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_CROUCHING, chr->act_throwgrenade.hand != 0, 5, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		} else if (rand % 3 == 1) {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_NOPIN, chr->act_throwgrenade.hand != 0, 6, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		} else {
			modelSetAnimation(chr->model, ANIM_THROWGRENADE_STANDING, chr->act_throwgrenade.hand != 0, 84, chrGetRangedSpeed(chr, 0.5, 1.2), 16);
		}
	}

	modelSetAnimEndFrame(chr->model, -1);
}

void chrThrowGrenade(struct chrdata *chr, int hand, int needsequip)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_THROWGRENADE;
	chr->act_throwgrenade.hand = hand;
	chr->act_throwgrenade.needsequip = needsequip;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrThrowGrenadeChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrSurprisedChooseAnimation(struct chrdata *chr)
{
	if (chr->act_surprised.type == 1) {
		struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
		struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
		int flip = 0;

		if (leftgun != NULL && rightgun == NULL) {
			flip = 1;
		} else if ((leftgun != NULL && rightgun != NULL) || (leftgun == NULL && rightgun == NULL)) {
			flip = rngRandom() & 1;
		}

		modelSetAnimation(chr->model, ANIM_TAUNT, flip, 10, chrGetRangedSpeed(chr, 0.6f, 0.96000003f), 16);
		modelSetAnimEndFrame(chr->model, 52);
	} else if (chr->act_surprised.type == 2) {
		modelSetAnimation(chr->model, ANIM_SURRENDER_002E, rngRandom() & 1, 0, chrGetRangedSpeed(chr, 0.35f, 0.56f), 16);
		modelSetAnimEndFrame(chr->model, 7);
	} else {
		uint32_t part = rngRandom() % 3;
		modelSetAnimation(chr->model, ANIM_SURVEY, rngRandom() & 1, 17, 0.6f, 16);

		if (part == 0) {
			modelSetAnimEndFrame(chr->model, chrGetRangedSpeed(chr, 38, 8));
		} else if (part == 1) {
			modelSetAnimEndFrame(chr->model, chrGetRangedSpeed(chr, 66, 8));
		} else {
			modelSetAnimEndFrame(chr->model, chrGetRangedSpeed(chr, 96, 8));
		}
	}
}

void chrDoSurprisedOneHand(struct chrdata *chr)
{
	struct prop *prop = chrGetTargetProp(chr);
	float angle = chrGetAngleToPos(chr, &prop->pos);

	if (angle < DEG2RAD(10) || angle > DEG2RAD(350)) {
		chrStopFiring(chr);
		chr->actiontype = ACT_SURPRISED;
		chr->act_surprised.type = 1;
		chr->sleep = 0;

		if (modelIsAnimMerging(chr->model)) {
			chr->hidden |= CHRHFLAG_NEEDANIM;
		} else {
			chrSurprisedChooseAnimation(chr);
			chr->hidden &= ~CHRHFLAG_NEEDANIM;
		}
	} else if (!chrIsStopped(chr)) {
		chrStand(chr);
	}
}

void chrDoSurprisedSurrender(struct chrdata *chr)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_SURPRISED;
	chr->act_surprised.type = 2;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrSurprisedChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrDoSurprisedLookAround(struct chrdata *chr)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_SURPRISED;
	chr->act_surprised.type = 3;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrSurprisedChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrSurrenderChooseAnimation(struct chrdata *chr)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);

	if (rightgun || leftgun) {
		modelSetAnimation(chr->model, ANIM_SURRENDER_002F, rngRandom() & 1, 0, 0.5, 16);
		modelSetAnimLooping(chr->model, 40, 16);

		if (leftgun) {
			objSetDropped(leftgun, DROPTYPE_SURRENDER);
		}

		if (rightgun) {
			objSetDropped(rightgun, DROPTYPE_SURRENDER);
		}

		chr->hidden |= CHRHFLAG_DROPPINGITEM;
	} else {
		modelSetAnimation(chr->model, ANIM_SURRENDER_002E, rngRandom() & 1, 0, 0.5, 16);
		modelSetAnimLooping(chr->model, 30, 16);
	}

	chrDropConcealedItems(chr);
}

void chrSurrender(struct chrdata *chr)
{
	uint32_t action = ACT_SURRENDER;

	if (chr->actiontype != action) {
		chrStopFiring(chr);
		chr->actiontype = action;
		chr->sleep = action;

		if (modelIsAnimMerging(chr->model)) {
			chr->hidden |= CHRHFLAG_NEEDANIM;
		} else {
			chrSurrenderChooseAnimation(chr);
			chr->hidden &= ~CHRHFLAG_NEEDANIM;
		}
	}
}

void chrSidestepChooseAnimation(struct chrdata *chr)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip = false;
	bool allowflip = false;
	uint32_t race = CHRRACE(chr);

	if (leftgun && rightgun) {
		flip = rngRandom() % 2;
		allowflip = rngRandom() % 2;
	} else {
		if (weaponIsOneHanded(leftgun) == false
				&& weaponIsOneHanded(rightgun) == false
				&& (leftgun || rightgun)) {
			flip = (leftgun != 0);
			allowflip = rngRandom() % 2;
		}
	}

	if (race == RACE_HUMAN) {
		if (allowflip == false) {
			if (chr->act_sidestep.side) {
				modelSetAnimation(chr->model, ANIM_JUMP_RT, true, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
				modelSetAnimEndFrame(chr->model, 36);
			} else {
				modelSetAnimation(chr->model, ANIM_JUMP_RT, false, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
				modelSetAnimEndFrame(chr->model, 36);
			}
		} else {
			if ((chr->act_sidestep.side && !flip) || (chr->act_sidestep.side == 0 && flip)) {
				modelSetAnimation(chr->model, ANIM_SIDESTEP_RIFLE_LF, flip, 5, chrGetRangedSpeed(chr, 0.7, 1.12), 16);
				modelSetAnimEndFrame(chr->model, 34);
			} else {
				modelSetAnimation(chr->model, ANIM_SIDESTEP_RIFLE_RT, flip, 5, chrGetRangedSpeed(chr, 0.7, 1.12), 16);
				modelSetAnimEndFrame(chr->model, 32);
			}
		}
	} else if (race == RACE_SKEDAR) {
		if (chr->act_sidestep.side) {
			modelSetAnimation(chr->model, ANIM_0328, false, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
			modelSetAnimEndFrame(chr->model, 27);
		} else {
			modelSetAnimation(chr->model, ANIM_0328, true, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
			modelSetAnimEndFrame(chr->model, 27);
		}
	}
}

void chrSidestep(struct chrdata *chr, bool side)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_SIDESTEP;
	chr->act_sidestep.side = side;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrSidestepChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrJumpOutChooseAnimation(struct chrdata *chr)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip = false;

	if (leftgun && !rightgun) {
		flip = true;
	} else if ((leftgun && rightgun) || (!leftgun && !rightgun)
			|| weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
		flip = rngRandom() % 2;
	}

	if ((chr->act_jumpout.side && !flip) || (chr->act_jumpout.side == 0 && flip)) {
		modelSetAnimation(chr->model, ANIM_JUMP_RT, true, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
		modelSetAnimEndFrame(chr->model, 36);
	} else {
		modelSetAnimation(chr->model, ANIM_JUMP_RT, false, 5, chrGetRangedSpeed(chr, 0.55, 0.88000005), 16);
		modelSetAnimEndFrame(chr->model, 36);
	}
}

void chrJumpOut(struct chrdata *chr, bool side)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_JUMPOUT;
	chr->act_jumpout.side = side;
	chr->sleep = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrJumpOutChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrRunPosChooseAnimation(struct chrdata *chr)
{
	float xdiff = chr->prop->pos.x - chr->act_runpos.pos.x;
	float ydiff = chr->prop->pos.y - chr->act_runpos.pos.y;
	float zdiff = chr->prop->pos.z - chr->act_runpos.pos.z;
	float distance = sqrtf(xdiff * xdiff + zdiff * zdiff);
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool heavy = true;
	bool flip;
	int race = CHRRACE(chr);

	if ((leftgun && rightgun) || (!leftgun && !rightgun)) {
		heavy = false;
		flip = rngRandom() % 2;
	} else if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
		heavy = false;
		flip = (bool)leftgun != false;
	} else {
		flip = (bool)leftgun != false;
	}

	if (race == RACE_HUMAN) {
		if (heavy) {
			float mult = 0.5;
			chr->act_runpos.eta60 = 1.0f / (func0f02dff0(ANIM_RUNNING_TWOHANDGUN) * mult) * distance;
			modelSetAnimation(chr->model, ANIM_RUNNING_TWOHANDGUN, flip, 0, mult, 16);
		} else {
			float mult = 0.5;
			chr->act_runpos.eta60 = 1.0f / (func0f02dff0(ANIM_RUNNING_ONEHANDGUN) * mult) * distance;
			modelSetAnimation(chr->model, ANIM_RUNNING_ONEHANDGUN, flip, 0, mult, 16);
		}
	} else if (race == RACE_SKEDAR) {
		float mult = 0.5;
		chr->act_runpos.eta60 = 1.0f / (func0f02dff0(ANIM_SKEDAR_RUNNING) * mult) * distance;
		modelSetAnimation(chr->model, ANIM_SKEDAR_RUNNING, flip, 0, mult, 16);
	}
}

void chrRunToPos(struct chrdata *chr, struct coord *pos)
{
	chrStopFiring(chr);
	chr->actiontype = ACT_RUNPOS;
	chr->act_runpos.pos.x = pos->x;
	chr->act_runpos.pos.y = pos->y;
	chr->act_runpos.pos.z = pos->z;
	chr->sleep = 0;
	chr->act_runpos.neardist = 30;
	chr->act_runpos.turnspeed = 0;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrRunPosChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrAttackStand(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip;
	struct attackanimgroup **animgroup;
	bool firing[] = {false, false};
	int race = CHRRACE(chr);

	if (leftgun && rightgun) {
		struct prop *leftgun2 = chrGetHeldUsableProp(chr, HAND_LEFT);
		struct prop *rightgun2 = chrGetHeldUsableProp(chr, HAND_RIGHT);

		if (leftgun2 && rightgun2) {
			flip = rngRandom() % 2;

			if (rngRandom() % 3 == 0) {
				animgroup = g_StandLightAttackAnims[race];
				firing[HAND_LEFT] = flip;
				firing[HAND_RIGHT] = !flip;
			} else {
				animgroup = g_StandDualAttackAnims[race];
				firing[HAND_LEFT] = true;
				firing[HAND_RIGHT] = true;
			}
		} else {
			flip = (bool)rightgun2 == false;
			animgroup = g_StandLightAttackAnims[race];
			firing[HAND_LEFT] = (bool)rightgun2 == false;
			firing[HAND_RIGHT] = !flip;
		}
	} else {
		if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
			flip = (bool)leftgun != false;
			animgroup = g_StandLightAttackAnims[race];
			firing[HAND_LEFT] = (bool)leftgun != false;
			firing[HAND_RIGHT] = !flip;
		} else {
			flip = (bool)leftgun != false;
			animgroup = g_StandHeavyAttackAnims[race];
			firing[HAND_LEFT] = (bool)leftgun != false;
			firing[HAND_RIGHT] = !flip;
		}
	}

	chrAttack(chr, animgroup, flip, firing, attackflags, entityid, true);
}

void chrAttackLie(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	struct prop *gun = chrGetHeldProp(chr, HAND_RIGHT);
	bool firing[2] = {false, false};

	if (attackflags & ATTACKFLAG_AIMONLY) {
		firing[1] = false;
		firing[0] = false;
	} else {
		bool tmp = gun == NULL;
		firing[1] = tmp;
		firing[0] = (bool)!tmp;
	}

	chrAttack(chr, &g_LieAttackAnims, gun == NULL, firing, attackflags, entityid, false);
}

void chrAttackKneel(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	int flip;
	struct attackanimgroup **animgroup;
	bool firing[2] = {false, false};
	int race = CHRRACE(chr);
	struct prop *leftgun2;
	struct prop *rightgun2;

	if (leftgun && rightgun) {
		leftgun2 = chrGetHeldUsableProp(chr, HAND_LEFT);
		rightgun2 = chrGetHeldUsableProp(chr, HAND_RIGHT);

		if (leftgun2 && rightgun2) {
			flip = rngRandom() % 2;

			if (rngRandom() % 3 == 0) {
				animgroup = g_KneelLightAttackAnims[race];
				firing[HAND_LEFT] = flip;
				firing[HAND_RIGHT] = !flip;
			} else {
				animgroup = g_KneelDualAttackAnims[race];
				firing[HAND_LEFT] = true;
				firing[HAND_RIGHT] = true;
			}
		} else {
			flip = (bool)rightgun2 == false;
			animgroup = g_KneelLightAttackAnims[race];
			firing[HAND_LEFT] = (bool)rightgun2 == false;
			firing[HAND_RIGHT] = !flip;
		}
	} else {
		if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
			flip = (bool)leftgun != false;
			animgroup = g_KneelLightAttackAnims[race];
			firing[HAND_LEFT] = (bool)leftgun != false;
			firing[HAND_RIGHT] = !flip;
		} else {
			flip = (bool)leftgun != false;
			animgroup = g_KneelHeavyAttackAnims[race];
			firing[HAND_LEFT] = (bool)leftgun != false;
			firing[HAND_RIGHT] = !flip;
		}
	}

	chrAttack(chr, animgroup, flip, firing, attackflags, entityid, false);
}

void chrAttackWalkChooseAnimation(struct chrdata *chr)
{
	if (chr->aibot == NULL) {
		modelSetAnimation(chr->model, chr->act_attackwalk.animcfg->animnum,
				chr->act_attackwalk.flip, chr->act_attackwalk.animcfg->unk10, 0.5, 16);
	}
}

void chrAttackWalk(struct chrdata *chr, bool run)
{
	struct attackanimconfig *animcfg;
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip;
	bool firing[] = {false, false};
	bool everytick[] = {false, false};
	bool singleshot[] = {false, false};
	int i;
	struct prop *prop;
	struct weaponobj *weapon;

	if (leftgun && rightgun) {
		struct prop *leftgun2 = chrGetHeldUsableProp(chr, HAND_LEFT);
		struct prop *rightgun2 = chrGetHeldUsableProp(chr, HAND_RIGHT);
		int style = 0;

		if (leftgun2 && rightgun2) {
			flip = rngRandom() % 2;
			style = rngRandom() % 3;
		} else {
			flip = (bool)rightgun2 == false;
		}

		if (style == 0) {
			if (run) {
				animcfg = &g_WalkAttackAnims[3];
			} else {
				animcfg = &g_WalkAttackAnims[2];
			}

			if (flip) {
				firing[HAND_LEFT] = true;
			} else {
				firing[HAND_RIGHT] = true;
			}
		} else if (style == 1) {
			if (run) {
				animcfg = &g_WalkAttackAnims[5];
			} else {
				animcfg = &g_WalkAttackAnims[4];
			}

			firing[HAND_LEFT] = firing[HAND_RIGHT] = true;
		} else {
			if (run) {
				animcfg = &g_WalkAttackAnims[7];
			} else {
				animcfg = &g_WalkAttackAnims[6];
			}

			firing[HAND_LEFT] = firing[HAND_RIGHT] = true;
		}
	} else {
		if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
			flip = (bool)leftgun != false;

			if (run) {
				animcfg = &g_WalkAttackAnims[3];
			} else {
				animcfg = &g_WalkAttackAnims[2];
			}

			if (flip) {
				firing[HAND_LEFT] = true;
			} else {
				firing[HAND_RIGHT] = true;
			}
		} else {
			flip = (bool)leftgun != false;

			if (run) {
				animcfg = &g_WalkAttackAnims[1];
			} else {
				animcfg = &g_WalkAttackAnims[0];
			}

			if (flip) {
				firing[HAND_LEFT] = true;
			} else {
				firing[HAND_RIGHT] = true;
			}
		}
	}

	for (i = 0; i < 2; i++) {
		if (firing[i]) {
			prop = chrGetHeldProp(chr, i);
			weapon = prop->weapon;

			if (weaponGetNumTicksPerShot(weapon->weaponnum, weapon->gunfunc) < 1) {
				everytick[i] = true;
			}

			if (weapon->weaponnum == WEAPON_ROCKETLAUNCHER
					|| weapon->weaponnum == WEAPON_ROCKETLAUNCHER_34
					|| weapon->weaponnum == WEAPON_SLAYER
					|| weapon->weaponnum == WEAPON_DEVASTATOR
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_DY357MAGNUM
						&& chr->headnum != HEAD_JONATHAN
						&& chr->headnum != HEAD_CHRIST)
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_DY357LX)
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_SHOTGUN)) {
				singleshot[i] = true;
			}
		}
	}

	chr->actiontype = ACT_ATTACKWALK;

	chr->act_attackwalk.frame60count = 0;
	chr->act_attackwalk.frame60max = rngRandom() % (int)(400 * g_AttackWalkDurationScale) + TICKS(120);
	chr->act_attackwalk.facedtarget = false;
	chr->act_attackwalk.animcfg = animcfg;
	chr->act_attackwalk.nextshot60 = 0;
	chr->act_attackwalk.nextgun = rngRandom() % 2;
	chr->act_attackwalk.firegun[HAND_LEFT] = firing[HAND_LEFT];
	chr->act_attackwalk.firegun[HAND_RIGHT] = firing[HAND_RIGHT];
	chr->act_attackwalk.everytick[HAND_LEFT] = everytick[HAND_LEFT];
	chr->act_attackwalk.everytick[HAND_RIGHT] = everytick[HAND_RIGHT];
	chr->act_attackwalk.singleshot[HAND_LEFT] = singleshot[HAND_LEFT];
	chr->act_attackwalk.singleshot[HAND_RIGHT] = singleshot[HAND_RIGHT];
	chr->act_attackwalk.turnspeed = 0;
	chr->act_attackwalk.flip = flip;

	chr->sleep = 0;
	chr->chrflags &= ~CHRCFLAG_INJUREDTARGET;

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrAttackWalkChooseAnimation(chr);

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrAttackRollChooseAnimation(struct chrdata *chr)
{
	modelSetAnimation(chr->model, chr->act_attack.animcfg->animnum, chr->act_attack.flip,
			chr->act_attack.animcfg->unk10, chrGetRangedSpeed(chr, 0.5, 0.8), 16);

	if (chr->act_attack.onehanded == false) {
		if (chr->act_attack.dorecoil) {
			if (chr->act_attack.animcfg->unk24 >= 0) {
				modelSetAnimEndFrame(chr->model, chr->act_attack.animcfg->unk24);
			} else {
				modelSetAnimEndFrame(chr->model, chr->act_attack.animcfg->unk1c);
			}
		} else {
			if (chr->act_attack.animcfg->unk20 >= 0) {
				modelSetAnimEndFrame(chr->model, chr->act_attack.animcfg->unk20);
			} else if (chr->act_attack.animcfg->unk14 >= 0) {
				modelSetAnimEndFrame(chr->model, chr->act_attack.animcfg->unk14);
			}
		}
	}
}

void chrAttackRoll(struct chrdata *chr, bool toleft)
{
	struct attackanimconfig *animcfg;
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	bool flip;
	bool onehanded = false;
	struct prop *prop;
	struct weaponobj *weapon;
	bool dorecoil = true;
	bool firing[] = {false, false};
	int i;
	bool dooneburst = false;
	bool everytick[] = {false, false};
	bool singleshot[] = {false, false};

	if (leftgun && rightgun) {
		struct prop *leftgun2 = chrGetHeldUsableProp(chr, HAND_LEFT);
		struct prop *rightgun2 = chrGetHeldUsableProp(chr, HAND_RIGHT);

		if (leftgun2 && rightgun2) {
			flip = rngRandom() % 2;
			onehanded = true;

			if (rngRandom() % 3 == 0) {
				firing[HAND_LEFT] = flip;
				firing[HAND_RIGHT] = !flip;
			} else {
				firing[HAND_LEFT] = true;
				firing[HAND_RIGHT] = true;
			}
		} else {
			flip = (bool)rightgun2 == false;
			onehanded = true;
			firing[HAND_LEFT] = flip;
			firing[HAND_RIGHT] = !flip;
		}
	} else {
		if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
			flip = (bool)leftgun != false;
			onehanded = true;
			firing[HAND_LEFT] = flip;
			firing[HAND_RIGHT] = !flip;
		} else {
			flip = (bool)leftgun != false;
			firing[HAND_LEFT] = flip;
			firing[HAND_RIGHT] = !flip;
		}
	}

	if ((toleft && !flip) || (!toleft && flip)) {
		// Roll to left
		if (rngRandom() % 2) {
			animcfg = &g_RollAttackAnims[0];
		} else {
			animcfg = &g_RollAttackAnims[2];
		}
	} else {
		// Roll to right
		if (rngRandom() % 2) {
			animcfg = &g_RollAttackAnims[1];
		} else {
			animcfg = &g_RollAttackAnims[3];
		}
	}

	if (onehanded) {
		// Move the animation pointer to the one-handed version of the same
		// animation. The one-handed versions are in the array immediately after
		// the four heavy-weapon versions.
		animcfg += 4;
	}

	for (i = 0; i < 2; i++) {
		if (firing[i]) {
			prop = chrGetHeldProp(chr, i);
			weapon = prop->weapon;

			if (weaponGetNumTicksPerShot(weapon->weaponnum, weapon->gunfunc) < 1) {
				everytick[i] = true;

				if (weapon->weaponnum == WEAPON_LASER) {
					dorecoil = false;
				}
			} else {
				dorecoil = false;
				dooneburst = true;
			}

			if (weapon->weaponnum == WEAPON_ROCKETLAUNCHER
					|| weapon->weaponnum == WEAPON_ROCKETLAUNCHER_34
					|| weapon->weaponnum == WEAPON_SLAYER
					|| weapon->weaponnum == WEAPON_DEVASTATOR
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_DY357MAGNUM
						&& chr->headnum != HEAD_JONATHAN
						&& chr->headnum != HEAD_CHRIST)
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_DY357LX)
					|| (
						!g_Vars.normmplayerisrunning
						&& weapon->weaponnum == WEAPON_SHOTGUN)) {
				singleshot[i] = true;
			}
		}
	}

	chr->actiontype = ACT_ATTACKROLL;

	chr->act_attack.animcfg = animcfg;
	chr->act_attack.fired = false;
	chr->act_attack.nextgun = rngRandom() % 2;
	chr->act_attack.firegun[HAND_LEFT] = firing[HAND_LEFT];
	chr->act_attack.firegun[HAND_RIGHT] = firing[HAND_RIGHT];
	chr->act_attack.everytick[HAND_LEFT] = everytick[HAND_LEFT];
	chr->act_attack.everytick[HAND_RIGHT] = everytick[HAND_RIGHT];
	chr->act_attack.singleshot[HAND_LEFT] = singleshot[HAND_LEFT];
	chr->act_attack.singleshot[HAND_RIGHT] = singleshot[HAND_RIGHT];
	chr->act_attack.dorecoil = dorecoil;
	chr->act_attack.dooneburst = dooneburst;
	chr->act_attack.onehanded = onehanded;
	chr->act_attack.pausecount = 0;
	chr->act_attack.numshots = 0;
	chr->act_attack.turning = TURNSTATE_TURNING;

	if (singleshot[HAND_LEFT] || singleshot[HAND_RIGHT]) {
		if (singleshot[HAND_LEFT] && singleshot[HAND_RIGHT]) {
			chr->act_attack.maxshots = 2;
		} else {
			chr->act_attack.maxshots = 1;
		}
	} else {
		chr->act_attack.maxshots = (rngRandom() % 4) + 2;

		if (firing[HAND_RIGHT] && firing[HAND_LEFT]) {
			chr->act_attack.maxshots += (rngRandom() % 4) + 2;
		}
	}

	chr->act_attack.flags = ATTACKFLAG_AIMATTARGET;
	chr->act_attack.entityid = 0;
	chr->act_attack.standing = true;
	chr->act_attack.reaim = 0;
	chr->act_attack.lastfire60 = 0;
	chr->act_attack.lastontarget60 = 0;
	chr->act_attack.flip = flip;

	chr->sleep = 0;

	// @bug: CHRCFLAG_INJUREDTARGET is not unset here. This means if the chr
	// does an attack that hits the target, then does a roll attack which
	// misses, their AI list will incorrectly read the roll attack as a hit
	// provided it didn't read and clear the flag between the attacks.
	// It usually (always?) does though, so this isn't really an issue.

	if (modelIsAnimMerging(chr->model)) {
		chr->hidden |= CHRHFLAG_NEEDANIM;
	} else {
		chrAttackRollChooseAnimation(chr);

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrStartAnim(struct chrdata *chr, int animnum, float startframe, float endframe, uint8_t chranimflags, int merge, float speed)
{
	if (chr && chr->model) {
		if (chranimflags & CHRANIMFLAG_REVERSE) {
			speed = -speed;
		}

		if (CHRRACE(chr) != RACE_DRCAROLL) {
			chrStopFiring(chr);
		}

		chr->actiontype = ACT_ANIM;

		chr->act_anim.movewheninvis = (chranimflags & CHRANIMFLAG_MOVEWHENINVIS) != 0;
		chr->act_anim.pauseatend = (chranimflags & CHRANIMFLAG_PAUSEATEND) != 0;
		chr->act_anim.completed = (chranimflags & CHRANIMFLAG_COMPLETED) != 0;
		chr->act_anim.slowupdate = (chranimflags & CHRANIMFLAG_SLOWUPDATE) != 0;
		chr->act_anim.lockpos = (chranimflags & CHRANIMFLAG_LOCKPOS) != 0;
		chr->act_anim.ishitanim = false;
		chr->act_anim.animnum = animnum;
		chr->act_anim.flip = (chranimflags & CHRANIMFLAG_FLIP) != 0;
		chr->act_anim.startframe = startframe;
		chr->act_anim.endframe = endframe;
		chr->act_anim.speed = speed;
		chr->act_anim.blend = merge;

		chr->sleep = chr->act_anim.slowupdate ? merge : 0;

		if (merge > 0 && modelIsAnimMerging(chr->model)) {
			chr->hidden |= CHRHFLAG_NEEDANIM;
		} else {
			modelSetAnimation(chr->model, animnum, (chranimflags & CHRANIMFLAG_FLIP) != 0, startframe, speed, merge);

			if (endframe >= 0) {
				modelSetAnimEndFrame(chr->model, endframe);
			}

			chr->hidden &= ~CHRHFLAG_NEEDANIM;
		}
	}
}

void chrBeginDead(struct chrdata *chr)
{
	if (chr->actiontype != ACT_DEAD) {
		chrStopFiring(chr);

		if (chr->cover != -1) {
			coverSetInUse(chr->cover, false);
			chr->cover = -1;
		}

		chr->actiontype = ACT_DEAD;
		chr->act_dead.fadetimer60 = chr->aibot ? 0 : -1;
		chr->act_dead.fadenow = false;
		chr->act_dead.fadewheninvis = false;
		chr->act_dead.invistimer60 = 0;
		chr->act_dead.notifychrindex = 0;
		chr->sleep = 0;

		if (chr->race == RACE_DRCAROLL) {
			chr->drcarollimage_left = DRCAROLLIMAGE_STATIC;
			chr->drcarollimage_right = DRCAROLLIMAGE_STATIC;
		}
	}
}

void func0f031254(struct chrdata *chr)
{
	struct model *model = chr->model;
	struct attackanimconfig *animcfg = chr->act_attack.animcfg;

	if (chr->act_attack.flags & ATTACKFLAG_AIMONLY) {
		if (animcfg->unk20 >= 0 && animcfg->unk20 < animcfg->unk18) {
			modelSetAnimEndFrame(model, animcfg->unk20);
		} else {
			modelSetAnimEndFrame(model, animcfg->unk18);
		}
	} else if (chr->act_attack.dorecoil) {
		if (animcfg->unk20 >= 0) {
			modelSetAnimEndFrame(model, animcfg->unk20);
		} else {
			modelSetAnimEndFrame(model, animcfg->unk18);
		}
	} else {
		if (animcfg->unk20 >= 0) {
			modelSetAnimEndFrame(model, animcfg->unk20);
		} else if (animcfg->unk14 >= 0) {
			modelSetAnimEndFrame(model, animcfg->unk14);
		} else {
			modelSetAnimEndFrame(model, -1);
		}
	}
}

/**
 * This function implements attack behaviour common to all the attack types,
 * such as stand, kneel and lie.
 */
void chrAttack(struct chrdata *chr, struct attackanimgroup **animgroups, bool flip, bool *firing, uint32_t attackflags, int entityid, bool standing)
{
	struct model *model = chr->model;
	int i;
	float angle;
	struct attackanimconfig *animcfg;
	struct prop *prop;
	struct weaponobj *weapon;
	int groupindex;
	bool dooneburst = false;
	int index;
	bool everytick[] = {false, false};
	bool singleshot[] = {false, false};
	bool dorecoil = true;
	int race = CHRRACE(chr);
	uint8_t sniping = false;

	if (race != RACE_DRCAROLL && race != RACE_EYESPY && race != RACE_ROBOT) {
		chr->actiontype = ACT_ATTACK;

		if (&animgroups[0] == &g_LieAttackAnims) {
			sniping = true;

			if (modelGetAnimNum(chr->model) != ANIM_SNIPING_ONGROUND) {
				// Getting up or getting down
				animcfg = var80067d28;
				modelSetAnimation(model, animcfg->animnum, flip, animcfg->unk10, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);
				modelSetAnimEndFrame(model, 236);
			} else {
				animcfg = var80067d70;
			}
		} else {
			// Non-sniping animations: Choose animation based on angle to target
			angle = chrGetAttackEntityRelativeAngle(chr, attackflags, entityid);

			if (flip) {
				groupindex = (M_TAU - angle) * 5.0937690734863f + 0.5f;
			} else {
				groupindex = angle * 5.0937690734863f + 0.5f;
			}

			if (groupindex < 0 || groupindex > 31) {
				groupindex = 0;
			}

			index = rngRandom() % animgroups[groupindex]->len;
			animcfg = &animgroups[groupindex]->animcfg[index];
		}

		for (i = 0; i < 2; i++) {
			if (firing[i]) {
				prop = chrGetHeldProp(chr, i);

				if (prop == NULL) {
					chrChooseStandAnimation(chr, 16);
					return;
				}

				weapon = prop->weapon;

				if (weaponGetNumTicksPerShot(weapon->weaponnum, weapon->gunfunc) < 1) {
					// Note: the only weapon that can enter this branch is the laser
					everytick[i] = true;

					if (weapon->weaponnum == WEAPON_LASER) {
						dorecoil = false;
					}
				} else {
					dorecoil = false;
					dooneburst = true;
				}

				// There's an easter egg here: Any guard with Chris T's head
				// (Foster from the firing range) can fire multiple shots with
				// the magnum.
				if (weapon->weaponnum == WEAPON_ROCKETLAUNCHER
						|| weapon->weaponnum == WEAPON_ROCKETLAUNCHER_34
						|| weapon->weaponnum == WEAPON_SLAYER
						|| weapon->weaponnum == WEAPON_DEVASTATOR
						|| (
							!g_Vars.normmplayerisrunning
							&& weapon->weaponnum == WEAPON_DY357MAGNUM
							&& chr->headnum != HEAD_JONATHAN
							&& chr->headnum != HEAD_CHRIST)
						|| (
							!g_Vars.normmplayerisrunning
							&& weapon->weaponnum == WEAPON_DY357LX)
						|| (
							!g_Vars.normmplayerisrunning
							&& weapon->weaponnum == WEAPON_SHOTGUN)) {
					singleshot[i] = true;
				}
			}
		}

		chr->act_attack.turning = TURNSTATE_TURNING;
		chr->act_attack.animcfg = animcfg;
		chr->act_attack.fired = false;
		chr->act_attack.nextgun = rngRandom() % 2;
		chr->act_attack.firegun[HAND_LEFT] = firing[HAND_LEFT];
		chr->act_attack.firegun[HAND_RIGHT] = firing[HAND_RIGHT];
		chr->act_attack.everytick[HAND_LEFT] = everytick[HAND_LEFT];
		chr->act_attack.everytick[HAND_RIGHT] = everytick[HAND_RIGHT];
		chr->act_attack.singleshot[HAND_LEFT] = singleshot[HAND_LEFT];
		chr->act_attack.singleshot[HAND_RIGHT] = singleshot[HAND_RIGHT];
		chr->act_attack.dorecoil = dorecoil;
		chr->act_attack.dooneburst = dooneburst;
		chr->act_attack.pausecount = 0;
		chr->act_attack.numshots = 0;

		if (singleshot[HAND_LEFT] || singleshot[HAND_RIGHT]) {
			if (singleshot[HAND_LEFT] && singleshot[HAND_RIGHT]) {
				chr->act_attack.maxshots = 2;
			} else {
				chr->act_attack.maxshots = 1;
			}
		} else {
			if (attackflags & ATTACKFLAG_SINGLESHOT) {
				chr->act_attack.maxshots = 1;
			} else {
				chr->act_attack.maxshots = (rngRandom() % 4) + 2;
			}

			// @bug: ATTACKFLAG_SINGLESHOT is not respected here if both guns
			// are firing.
			if (firing[HAND_RIGHT] && firing[HAND_LEFT]) {
				chr->act_attack.maxshots += (rngRandom() % 4) + 2;
			}
		}

		chr->act_attack.flags = attackflags;
		chr->act_attack.entityid = entityid;
		chr->act_attack.standing = standing;
		chr->act_attack.reaim = 0;
		chr->act_attack.lastfire60 = 0;
		chr->act_attack.lastontarget60 = 0;
		chr->act_attack.flip = flip;

		chr->sleep = 0;
		chr->chrflags &= ~CHRCFLAG_INJUREDTARGET;

		if (!sniping && !chr->aibot) {
			if (modelIsAnimMerging(chr->model)) {
				chr->hidden |= CHRHFLAG_NEEDANIM;
			} else {
				modelSetAnimation(model, animcfg->animnum, flip, animcfg->unk10, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);
				func0f031254(chr);

				chr->hidden &= ~CHRHFLAG_NEEDANIM;
			}
		}
	}
}

void chrAttackAmount(struct chrdata *chr, uint32_t attackflags, uint32_t entityid, uint32_t maxshots)
{
	struct prop *prop = chrGetHeldProp(chr, HAND_RIGHT);
	struct attackanimgroup **things = NULL;
	bool firing[] = {false, false};
	uint32_t race = CHRRACE(chr);

	if (prop) {
		things = weaponIsOneHanded(prop) ? g_StandLightAttackAnims[race] : g_StandHeavyAttackAnims[race];

		firing[1] = false;
		firing[0] = true;
	}

	chrAttack(chr, things, false, firing, attackflags, entityid, false);

	chr->actiontype = ACT_ATTACKAMOUNT;
	chr->act_attack.numshots = 0;
	chr->act_attack.maxshots = maxshots;
	chr->act_attack.dooneburst = false;
}

int g_DrCarollDyingTimer = 10;
uint8_t g_DrCarollShieldSndThreshold = 50;

/**
 * Given a perfectly alive chr, make them begin the process of dying or being
 * knocked unconscious.
 *
 * This function handles:
 * - Eyespy destruction
 * - Transitioning to ACT_DIE
 * - Choosing and applying death animations
 * - Updating kill statistics
 * - Dropping items
 */
void chrBeginDeath(struct chrdata *chr, struct coord *dir, float relangle, int hitpart, struct gset *gset, bool knockout, int aplayernum)
{
	bool overridden = false;
	bool instant;
	int index = -1;
	int animnum;
	struct prop *prop = chr->prop;
	struct model *model = chr->model;
	int race = CHRRACE(chr);
	bool wasknockedout = false;
	int prevplayernum;
	int i;
	int buddyplayernum;
	struct eyespy *eyespy;
	int objectivenum;
	float impactforce1;
	float impactforce2;
	float impactforce3;

	// If chr was previously knocked out, they are now dead so decrease KO counter
	if (chr->actiontype == ACT_DRUGGEDCOMINGUP
			|| chr->actiontype == ACT_DRUGGEDDROP
			|| chr->actiontype == ACT_DRUGGEDKO) {
		if (chr->actiontype == ACT_DRUGGEDKO) {
			wasknockedout = true;
		}

		mpstatsDecrementTotalKnockoutCount();
	}

	// Handle eyespy then return early
	if (race == RACE_EYESPY) {
		prevplayernum = g_Vars.currentplayernum;
		buddyplayernum = -1;
		eyespy = chrToEyespy(chr);
		objectivenum = -1;

		// Figure out which playernum has the eyespy that's being destroyed,
		// and the buddy's playernum if applicable. Note that the player count
		// can only be 1 or 2 here.
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (eyespy == g_Vars.players[i]->eyespy) {
				setCurrentPlayerNum(i);
			} else {
				buddyplayernum = i;
			}
		}

		if (g_Vars.currentplayer->eyespy) {
			// Stop using eyespy if active
			if (g_Vars.currentplayer->eyespy->active) {
				g_Vars.currentplayer->eyespy->active = false;
				g_Vars.currentplayer->devicesactive &= ~DEVICE_EYESPY;
			}

			// Destroy the eyespy
			chr->hidden |= CHRHFLAG_DELETING;

			explosionCreateSimple(g_Vars.currentplayer->eyespy->prop,
					&g_Vars.currentplayer->eyespy->prop->pos,
					g_Vars.currentplayer->eyespy->prop->rooms, EXPLOSIONTYPE_EYESPY, 0);
			invRemoveItemByNum(WEAPON_EYESPY);

			psStopSound(g_Vars.currentplayer->eyespy->prop, PSTYPE_GENERAL, 0xffff);
			g_Vars.currentplayer->eyespy = NULL;
			setCurrentPlayerNum(prevplayernum);

			// For Investigation and G5 Building, set a stage flag to show that
			// the eyespy is destroyed. The scripting in those stages checks for
			// this flag and fails the objective if set.
			switch (stageGetIndex(g_Vars.stagenum)) {
			case STAGEINDEX_INVESTIGATION:
				objectivenum = 0;
				break;
			case STAGEINDEX_G5BUILDING:
				if (lvGetDifficulty() == DIFF_A) {
					objectivenum = 2;
				} else if (lvGetDifficulty() == DIFF_SA) {
					objectivenum = 2;
				} else {
					objectivenum = 2;
				}
				break;
			}

			// But don't set the flag if the coop buddy still has an eyespy
			if (objectivenum >= 0 && buddyplayernum >= 0) {
				setCurrentPlayerNum(buddyplayernum);

				if (g_Vars.currentplayer->eyespy) {
					objectivenum = -1;
				}

				setCurrentPlayerNum(prevplayernum);
			}

			if (objectivenum >= 0 && objectiveCheck(objectivenum) != OBJECTIVE_COMPLETE) {
				g_StageFlags |= STAGEFLAG_EYESPY_DESTROYED;
			}
		}

		setCurrentPlayerNum(prevplayernum);
		return;
	}

	// instant = whether to merge into death animation or switch to it instantly
	instant = chr->actiontype == ACT_ARGH && chr->act_argh.lvframe60 == g_Vars.lvframe60;

	for (i = 0; g_AnimTablesByRace[race][i].hitpart != -1; i++) {
		if (g_AnimTablesByRace[race][i].hitpart == hitpart) {
			index = i;
			break;
		}
	}

	// Set up chr's new action
	chr->blurdrugamount = 0;

	chrStopFiring(chr);
	chrUncloak(chr, true);

	chr->chrflags &= ~CHRCFLAG_HIDDEN;
	chr->actiontype = (knockout == true ? ACT_DRUGGEDDROP : ACT_DIE);
	chr->act_die.notifychrindex = 0;
	chr->act_die.timeextra = 0;
	chr->act_die.drcarollimagedelay = TICKS(45);
	chr->act_die.thudframe1 = -1;
	chr->act_die.thudframe2 = -1;

	if (chr->race == RACE_DRCAROLL) {
		chr->drcarollimage_left = (int)((rngRandom() % 400) * 0.01f) + 1;
		chr->drcarollimage_right = (int)((rngRandom() % 400) * 0.01f) + 1;
	}

	chr->sleep = 0;

	// Handle robots and Dr Caroll then return early
	if (race == RACE_ROBOT || race == RACE_DRCAROLL) {
		impactforce1 = gsetGetImpactForce(gset) * 0.5f;

		if (impactforce1 <= 0) {
			impactforce1 = 3;
		}

		if (impactforce1 != 0.0f) {
			chr->elapseextra = 0;
			chr->timeextra = impactforce1 * 15;
			chr->extraspeed.x = dir->x * impactforce1;
			chr->extraspeed.y = dir->y * impactforce1;
			chr->extraspeed.z = dir->z * impactforce1;
		}

		if (race == RACE_DRCAROLL) {
			g_DrCarollDyingTimer = 0;

			chr->soundtimer = 0;
			chr->voicebox = VOICEBOX_MALE1;

			modelSetAnimation(chr->model, ANIM_0164, false, 0, 0.5f, 16);
		}

		return;
	}

	// Handle humans and Skedar
	if (race == RACE_HUMAN) {
		animnum = modelGetAnimNum(chr->model);

		// Chrs in lying-down sniping poses don't use standard death animations
		if (animnum == ANIM_SNIPING_GETDOWN
				|| animnum == ANIM_SNIPING_GETUP
				|| animnum == ANIM_SNIPING_ONGROUND) {
			modelSetAnimation(chr->model, ANIM_SNIPING_DIE, false, 0, 0.5f, 16);
		} else {
			// Consider making the chr do an animation where they slump against
			// a wall or object which is behind them.
			if ((relangle < 1.5705462694168f || relangle > 4.7116389274597f)
					&& rngRandom() % 20 == 0
					&& chr->specialdie == SPECIALDIE_NONE) {
				float angle1;
				float angle2 = chrGetInverseTheta(chr);
				float fval1;
				float fval2;

				angle1 = angle2 + 3.3155977725983f;
				angle2 += 2.966587305069f;

				if (angle1 >= M_TAU) {
					angle1 -= M_TAU;
				}

				if (angle2 >= M_TAU) {
					angle2 -= M_TAU;
				}

				fval1 = func0f02e684(prop, angle1, 150);
				fval2 = func0f02e684(prop, angle2, 150);

				if (fval1 < 150 && fval2 < 150
						&& fval1 - fval2 < 10 && fval1 - fval2 > -10
						&& !wasknockedout) {
					struct animtablerow *row = &g_AnimTableHumanSlumped[rngRandom() % 4];

					chr->act_die.thudframe1 = row->thudframe1;
					chr->act_die.thudframe2 = row->thudframe2;

					modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);

					if (row->endframe >= 0) {
						modelSetAnimEndFrame(model, row->endframe);
					}

					chr->radius = 10;
					chr->chrflags &= ~CHRCFLAG_HAS_SPECIAL_DEATH_ANIMATION;

					overridden = true;
				}
			}

			// Consider making the chr do a fall forward animation.
			// The player must be behind the chr for it to happen.
			if (relangle > 2.3558194637299f && relangle < 3.9263656139374f
					&& rngRandom() % 5 < 2
					&& chr->specialdie == SPECIALDIE_NONE) {
				struct animtablerow *row;

				struct animtablerow rows[] = {
					{ 0x005b, 0, -1, 0.6, 0, 27, -1 },
					{ 0x0255, 0, -1, 0.5, 0, 25, -1 },
				};

				bool flip;

				overridden = true;

				if (hitpart == HITPART_LBICEP || hitpart == HITPART_RBICEP) {
					row = &rows[0];

					if (hitpart == HITPART_LBICEP) {
						flip = true;
					} else {
						flip = false;
					}
				} else {
					row = &rows[1];
					flip = rngRandom() % 2;
				}

				chr->act_die.thudframe1 = row->thudframe1;
				chr->act_die.thudframe2 = row->thudframe2;

				modelSetAnimationWithMerge(model, row->animnum, flip, 0, row->speed, 16, !instant);

				if (row->endframe >= 0) {
					modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, row->endframe, 8));
				} else {
					modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, animGetNumFrames(row->animnum) - 1, 8));
				}

				chr->chrflags &= ~CHRCFLAG_HAS_SPECIAL_DEATH_ANIMATION;
			}

			if (!overridden && index >= 0) {
				// Handle specialdie animations or choose a random one if not
				// using specialdie
				if (g_AnimTablesByRace[race][index].deathanims
						&& g_AnimTablesByRace[race][index].deathanimcount > 0
						&& !wasknockedout) {
					struct animtablerow *row;

					if (chr->specialdie == SPECIALDIE_NONE) {
						int tmp = rngRandom() % g_AnimTablesByRace[race][index].deathanimcount;
						row = &g_AnimTablesByRace[race][index].deathanims[tmp];
					} else if (chr->specialdie == SPECIALDIE_ONCHAIR) {
						row = &g_SpecialDieAnims[chr->specialdie + rngRandom() % 2];

						// chr->myspecial is the tag number of the chr's chair
						if (chr->myspecial >= 0) {
							struct defaultobj *obj = objFindByTagId(chr->myspecial);
							obj->flags3 &= ~OBJFLAG3_PUSHABLE;
							obj->flags |= OBJFLAG_INVINCIBLE;

							// The original source likely didn't have the brackets here,
							// but I'm including them to show the logic that's actually
							// being used. There is no bug here, as obj and obj->prop
							// are always set at this point so these checks are
							// unnecessary.
							if ((obj && obj->prop && obj->modelnum == MODEL_DD_REDARM)
									|| obj->modelnum == MODEL_DD_REDSOFA) {
								row = &g_SpecialDieAnims[chr->specialdie - 1];
							}
						}
					} else {
						row = &g_SpecialDieAnims[chr->specialdie - 1];
					}

					chr->act_die.thudframe1 = row->thudframe1;
					chr->act_die.thudframe2 = row->thudframe2;

					if (chr->specialdie == SPECIALDIE_NONE) {
						modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);
					} else {
						modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 30, !instant);
					}

					if (row->endframe >= 0) {
						modelSetAnimEndFrame(model, row->endframe);
					}

					impactforce2 = gsetGetImpactForce(gset);

					if (impactforce2 <= 0 && (chr->chrflags & CHRCFLAG_DIEWITHFORCE)) {
						impactforce2 = 6;
					}

					if (row->unk10 && impactforce2 > 0) {
						chr->act_die.timeextra = impactforce2 * 15;
						chr->act_die.elapseextra = 0;
						chr->act_die.extraspeed.x = dir->x * impactforce2;
						chr->act_die.extraspeed.y = dir->y * impactforce2;
						chr->act_die.extraspeed.z = dir->z * impactforce2;
					}

					chr->chrflags &= ~CHRCFLAG_HAS_SPECIAL_DEATH_ANIMATION;
				}
			}
		}
	} else if (race == RACE_SKEDAR) {
		struct animtablerow *row;

		if (relangle > 2.3558194637299f && relangle < 3.9263656139374f) {
			// Player is behind the Skedar - use specific set of anims
			row = &g_AnimTablesByRace[race][1 + (rngRandom() % 6)].deathanims[rngRandom() % 3];

			chr->act_die.thudframe1 = row->thudframe1;
			chr->act_die.thudframe2 = row->thudframe2;

			modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);

			if (row->endframe >= 0) {
				modelSetAnimEndFrame(model, row->endframe);
			}
		} else {
			// Normal Skedar death
			if (index >= 0
					&& g_AnimTablesByRace[race][index].deathanims != NULL
					&& g_AnimTablesByRace[race][index].deathanimcount > 0) {
				int tmp = rngRandom() % g_AnimTablesByRace[race][index].deathanimcount;
				row = &g_AnimTablesByRace[race][index].deathanims[tmp];
			} else {
				row = &g_AnimTablesByRace[race][0].deathanims[0];
			}

			chr->act_die.thudframe1 = row->thudframe1;
			chr->act_die.thudframe2 = row->thudframe2;

			modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);

			if (row->endframe >= 0) {
				modelSetAnimEndFrame(model, row->endframe);
			}

			impactforce3 = gsetGetImpactForce(gset);

			if (impactforce3 <= 0 && (chr->chrflags & CHRCFLAG_DIEWITHFORCE)) {
				impactforce3 = 6;
			}

			if (row->unk10 != 0 && impactforce3 > 0) {
				chr->act_die.timeextra = impactforce3 * 15;
				chr->act_die.elapseextra = 0;
				chr->act_die.extraspeed.x = dir->x * impactforce3;
				chr->act_die.extraspeed.y = dir->y * impactforce3;
				chr->act_die.extraspeed.z = dir->z * impactforce3;
			}
		}
	}

	// Handle multiplayer stats and kill count
	if (g_Vars.mplayerisrunning) {
		mpstatsRecordDeath(aplayernum, mpPlayerGetIndex(chr));
	} else if (aplayernum >= 0) {
		int prevplayernum = g_Vars.currentplayernum;
		setCurrentPlayerNum(aplayernum);
		mpstatsRecordPlayerKill();
		setCurrentPlayerNum(prevplayernum);
	}

	if (chr->chrflags & CHRCFLAG_KILLCOUNTABLE) {
		mpstatsIncrementTotalKillCount();
	}

	// Drop items
	if (race == RACE_HUMAN || race == RACE_SKEDAR) {
		if (chr->weapons_held[0] && (chr->weapons_held[0]->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
			objSetDropped(chr->weapons_held[0], DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}

		if (chr->weapons_held[1] && (chr->weapons_held[1]->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
			objSetDropped(chr->weapons_held[1], DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}

		chrDropConcealedItems(chr);
	}
}

void chrBeginArgh(struct chrdata *chr, float angle, int hitpart)
{
	bool doneanim = false;
	int instant;
	int index = -1;
	struct model *model = chr->model;
	int i;
	int race = CHRRACE(chr);
	int animnum = modelGetAnimNum(chr->model);

	if (animnum == ANIM_SNIPING_GETDOWN
			|| animnum == ANIM_SNIPING_GETUP
			|| animnum == ANIM_SNIPING_ONGROUND) {
		chrFlinchBody(chr);
		return;
	}

	if (race == RACE_EYESPY || chr->aibot) {
		return;
	}

	if (race == RACE_DRCAROLL) {
		chr->actiontype = ACT_ARGH;
		chr->act_argh.notifychrindex = 0;
		chr->act_argh.lvframe60 = g_Vars.lvframe60;

		chr->sleep = 0;

		modelSetAnimation(chr->model, ANIM_0163, false, 0, 0.5f, 16);

		chr->drcarollimage_left = DRCAROLLIMAGE_X;
		chr->drcarollimage_right = DRCAROLLIMAGE_X;
		return;
	}

	instant = chr->actiontype == ACT_ARGH && chr->act_argh.lvframe60 == g_Vars.lvframe60;

	for (i = 0; g_AnimTablesByRace[race][i].hitpart != -1; i++) {
		if (g_AnimTablesByRace[race][i].hitpart == hitpart) {
			index = i;

			if (chr->hitpart == 0) {
				chr->hitpart = hitpart;
			}

			break;
		}
	}

	// If shot in the butt from behind, 2 in 5 chance of doing a special anim
	if (race == RACE_HUMAN
			&& hitpart == HITPART_PELVIS
			&& angle > 2.3558194637299f
			&& angle < 3.9263656139374f
			&& rngRandom() % 5 < 2) {
		struct animtablerow *row;
		struct animtablerow rows[] = {
			{ 0x013b, 0, -1, 0.5, 0, -1, -1 },
			{ 0x013c, 0, -1, 0.5, 0, -1, -1 },
			{ 0x013f, 0, -1, 0.5, 0, -1, -1 },
			{ 0x0142, 0, -1, 0.5, 0, -1, -1 },
			{ 0x0145, 0, -1, 0.5, 0, -1, -1 },
			{ 0x0148, 0, -1, 0.5, 0, -1, -1 },
			{ 0x0036, 0, -1, 0.5, 0, -1, -1 },
			{ 0x0037, 0, -1, 0.5, 0, -1, -1 },
		};

		chrStopFiring(chr);

		chr->actiontype = ACT_ARGH;
		chr->act_argh.notifychrindex = 0;
		chr->act_argh.lvframe60 = g_Vars.lvframe60;
		chr->sleep = 0;

		row = &rows[rngRandom() % 8];

		modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);

		if (row->endframe >= 0) {
			modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, row->endframe, 8));
			doneanim = true;
		} else {
			modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, animGetNumFrames(row->animnum) - 1, 8));
			doneanim = true;
		}
	}

	if (!doneanim
			&& index >= 0
			&& g_AnimTablesByRace[race][index].injuryanims
			&& g_AnimTablesByRace[race][index].injuryanimcount > 0) {
		// If shot in a hand that's holding a gun, remap the hit location to the
		// forearm because the hand injury animations assume the hand is empty.
		struct prop *lgun = chrGetHeldProp(chr, HAND_LEFT);
		struct prop *rgun = chrGetHeldProp(chr, HAND_RIGHT);
		int rowindex;
		struct animtablerow *row;

		if (race == RACE_HUMAN) {
			if (index == 9 && lgun) { // left hand
				index = 10; // left forearm
			} else if (index == 12 && rgun) { // right hand
				index = 13; // right forearm
			}
		}

		// Select a random animation for this hit location and apply it
		rowindex = rngRandom() % g_AnimTablesByRace[race][index].injuryanimcount;

		row = &g_AnimTablesByRace[race][index].injuryanims[rowindex];

		chrStopFiring(chr);

		chr->actiontype = ACT_ARGH;
		chr->act_argh.notifychrindex = 0;
		chr->act_argh.lvframe60 = g_Vars.lvframe60;
		chr->sleep = 0;

		modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, !instant);

		if (row->endframe >= 0) {
			modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, row->endframe, 8));
		} else {
			modelSetAnimEndFrame(model, chrGetRangedArghSpeed(chr, animGetNumFrames(row->animnum) - 1, 8));
		}
	}
}

void chrReactToDamage(struct chrdata *chr, struct coord *vector, float angle, int hitpart, struct gset *gset, int aplayernum)
{
	int race = CHRRACE(chr);
	bool knockedout = false;
	int animnum = modelGetAnimNum(chr->model);

	if (chr->actiontype == ACT_DRUGGEDKO)
	{
		knockedout = true;
	}

	if (race == RACE_EYESPY) {
		float strength = gsetGetImpactForce(gset);
		struct eyespy *eyespy = chrToEyespy(chr);

		if (eyespy) {
			if (strength <= 0) {
				strength = 6;
			}

			strength *= 4;

			eyespy->hit = EYESPYHIT_DAMAGE;
			eyespy->vel.x += vector->x * strength;
			eyespy->vel.z += vector->z * strength;
		}
	}

	if (chr->damage >= chr->maxdamage) {
		chrBeginDeath(chr, vector, angle, hitpart, gset, false, aplayernum);
	} else if (animnum == ANIM_SNIPING_GETDOWN
			|| animnum == ANIM_SNIPING_GETUP
			|| animnum == ANIM_SNIPING_ONGROUND) {
		chrFlinchBody(chr);
	} else if (race == RACE_EYESPY) {
		// empty
	} else if (race == RACE_DRCAROLL || race == RACE_ROBOT) {
		float strength = gsetGetImpactForce(gset);

		if (race == RACE_DRCAROLL) {
			strength *= 0.5f;
		}

		if (strength <= 0) {
			strength = 6;
		}

		if (strength > 0) {
			chr->elapseextra = 0;
			chr->timeextra = strength * 15;
			chr->extraspeed.x = vector->x * strength;
			chr->extraspeed.y = vector->y * strength;
			chr->extraspeed.z = vector->z * strength;
		}

		if (race == RACE_DRCAROLL) {
			chrBeginArgh(chr, 0, 0);
		}
	} else if (!knockedout) {
		chrBeginArgh(chr, angle, hitpart);
	}
}

/**
 * Launch a chr away from the given pos (for explosions).
 */
/**
 * Launch a chr away from the given pos (for explosions).
 */
void chrYeetFromPos(struct chrdata *chr, struct coord *exppos, float force)
{
	if (!chr || !chr->model || !chr->prop || !exppos) {
		return;
	}

	struct model *model = chr->model;
	struct prop *prop = chr->prop;
	int race = CHRRACE(chr);

	// Skip characters that shouldn't be yeeted
	if (race == RACE_DRCAROLL || race == RACE_EYESPY || race == RACE_ROBOT) {
		return;
	}

	float faceangle = chrGetInverseTheta(chr);
	float latangle = atan2f(prop->pos.x - exppos->x, prop->pos.z - exppos->z);

	struct coord delta = {
		prop->pos.x - exppos->x,
		prop->pos.y - exppos->y,
		prop->pos.z - exppos->z
	};

	// Avoid divide-by-zero
	if (delta.x == 0.0f && delta.y == 0.0f && delta.z == 0.0f) {
		delta.z = 1.0f;
	}

	float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
	float speed = 0.625f * force / sqrtf(distSq);

	delta.x *= speed;
	delta.y *= speed;
	delta.z *= speed;

	chr->fallspeed = delta;

	float angletoexplosion = latangle - faceangle;

	if (latangle < faceangle) {
		angletoexplosion += M_TAU;
	}

	int angleindex = angletoexplosion * 1.2734422683716f + 0.5f;

	if (angleindex >= 8) {
		angleindex = 0;
	}

	int count = g_YeetAnimIndexesByRaceAngle[race][angleindex].count;
	if (count == 0) {
		return;
	}

	int subindex = rngRandom() % count;
	int animindex = g_YeetAnimIndexesByRaceAngle[race][angleindex].indexes[subindex];

	struct yeetanim *row = NULL;

	if (race == RACE_HUMAN) {
		row = &g_YeetAnimsHuman[animindex];
	} else if (race == RACE_SKEDAR) {
		row = &g_YeetAnimsSkedar[animindex];
	} else {
		// Unknown race
		return;
	}

	// Set death animation and state
	chrStopFiring(chr);
	chrUncloak(chr, true);

	chr->chrflags &= ~CHRCFLAG_HIDDEN;
	chr->actiontype = ACT_DIE;
	chr->act_die.notifychrindex = 0;
	chr->act_die.thudframe1 = row->thudframe;
	chr->act_die.thudframe2 = -1;
	chr->act_die.timeextra = 0;
	chr->act_die.drcarollimagedelay = TICKS(45);

	if (chr->race == RACE_DRCAROLL) {
		chr->drcarollimage_left = 1 + (int)((rngRandom() % 400) * 0.01f);
		chr->drcarollimage_right = 1 + (int)((rngRandom() % 400) * 0.01f);
	}

	chr->sleep = 0;

	modelSetAnimation(model, row->animnum, row->flip, row->startframe, row->speed, 8);

	if (row->endframe >= 0.0f) {
		modelSetAnimEndFrame(model, row->endframe);
	}
}

int gsetGetBlurAmount(struct gset *gset)
{
	int amount = TICKS(1000);

	if (g_Vars.normmplayerisrunning == false) {
		amount = TICKS(250);
	}

	if (gset->weaponnum == WEAPON_TRANQUILIZER) {
		amount = TICKS(2000);
	}

	if (gset->weaponnum == WEAPON_BOLT) {
		amount = TICKS(5000);
	}

	if (gset->weaponnum == WEAPON_NBOMB) {
		amount = TICKS(100);
	}

	return amount;
}

void chrKnockOut(struct chrdata *chr, float angle, int hitpart, struct gset *gset)
{
	if (chr->actiontype != ACT_DRUGGEDCOMINGUP
			&& chr->actiontype != ACT_DRUGGEDDROP
			&& chr->actiontype != ACT_DRUGGEDKO) {

		if (mpstatsGetTotalKnockoutCount() < 2) {
			chr->chrflags |= CHRCFLAG_KEEPCORPSEKO;
		}

		mpstatsIncrementTotalKnockoutCount();

		chr->actiontype = ACT_DRUGGEDCOMINGUP;
		chr->act_druggedcomingup.timer60 = 0;
	}
}

/**
 * Return true if the chr's current animation would be too awkward to transition
 * into an injury animation or if they're already in the PREARGH action state,
 * and set dst to the anim frame number where the chr will become available for
 * transition to an injury animation.
 *
 * The attack roll animation is the only one which is too awkward to transition.
 */
bool chrIsAnimPreventingArgh(struct chrdata *chr, float *dst)
{
	bool result = false;
	int race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_EYESPY || chr->aibot) {
		return false;
	}

	if (race == RACE_HUMAN) {
		int animnum = modelGetAnimNum(chr->model);
		float endframe;

		if (animnum == ANIM_SNIPING_GETDOWN
				|| animnum == ANIM_SNIPING_GETUP
				|| animnum == ANIM_SNIPING_ONGROUND) {
			chrFlinchBody(chr);
		} else if (chr->actiontype == ACT_ATTACKROLL
				&& modelGetAnimNum(chr->model) == chr->act_attack.animcfg->animnum) {
			if (chr->act_attack.onehanded) {
				if (chr->act_attack.animcfg == &g_RollAttackAnims[4]
						|| chr->act_attack.animcfg == &g_RollAttackAnims[5]
						|| chr->act_attack.animcfg == &g_RollAttackAnims[6]
						|| chr->act_attack.animcfg == &g_RollAttackAnims[7]) {
					endframe = chr->act_attack.animcfg->unk04 - 8;

					if (chr->act_attack.animcfg->unk14 < chr->act_attack.animcfg->unk04) {
						endframe = chr->act_attack.animcfg->unk14;
					}

					if (endframe > modelGetCurAnimFrame(chr->model)) {
						*dst = endframe;
						result = true;
					}
				}
			} else {
				endframe = chr->act_attack.animcfg->unk04 - 8;

				if (endframe > modelGetCurAnimFrame(chr->model)) {
					*dst = endframe;
					result = true;
				}
			}
		}
	}

	if (chr->actiontype == ACT_PREARGH) {
		result = true;
	}

	return result;
}

void chrChoke(struct chrdata *chr, int choketype)
{
	bool male = false;
	int16_t soundnum = -1;
	int race = CHRRACE(chr);
	int playernum;
	int allowoverride = false;

	static int nextindexdrcaroll = 0;
	static int nextindexmaian = 0;
	static int nextindexskedar = 0;
	static int nextindexshock = 0;
	static int nextindexmale = 0;
	static int nextindexfemale = 0;

	if (race == RACE_EYESPY || race == RACE_ROBOT) {
		return;
	}

	if (chr->prop->type == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(chr->prop);

		if (g_Vars.players[playernum]->isdead) {
			return;
		}
	}

	if (g_HeadsAndBodies[chr->headnum].ismale) {
		male = true;
	}

	if (race == RACE_DRCAROLL) {
		int16_t sounds[] = {
			SFX_ARGH_DRCAROLL_0240,
			SFX_ARGH_DRCAROLL_024C,
			SFX_ARGH_DRCAROLL_0250,
			SFX_ARGH_DRCAROLL_0251,
			SFX_ARGH_DRCAROLL_0259,
			SFX_ARGH_DRCAROLL_025A,
		};

		if (g_DrCarollDyingTimer > TICKS(10)) {
			g_DrCarollDyingTimer = 0;

			soundnum = sounds[nextindexdrcaroll];
			nextindexdrcaroll++;

			if (nextindexdrcaroll >= ARRAYCOUNT(sounds)) {
				nextindexdrcaroll = 0;
			}
		}
	} else if (chr->headnum == HEAD_THEKING
			|| chr->headnum == HEAD_ELVIS
			|| chr->headnum == HEAD_MAIAN_S
			|| chr->headnum == HEAD_ELVIS_GOGS) {
		int16_t sounds[] = {
			SFX_ARGH_MAIAN_05DF,
			SFX_ARGH_MAIAN_05E0,
			SFX_ARGH_MAIAN_05E1
		};

		soundnum = sounds[rngRandom() % 3];
		nextindexmaian++;

		if (nextindexmaian >= ARRAYCOUNT(sounds)) {
			nextindexmaian = 0;
		}
	} else if (race == RACE_SKEDAR) {
		if (chr->bodynum == BODY_MINISKEDAR) {
			int16_t sounds[] = {
				SFX_SKEDAR_ROAR_0536,
				SFX_SKEDAR_ROAR_0537,
				SFX_SKEDAR_ROAR_0538,
				SFX_SKEDAR_ROAR_0539,
				SFX_SKEDAR_ROAR_053A,
			};

			soundnum = sounds[rngRandom() % 5];
			nextindexskedar++;

			if (nextindexskedar >= ARRAYCOUNT(sounds)) {
				nextindexskedar = 0;
			}
		} else {
			int16_t sounds[] = {
				SFX_SKEDAR_ROAR_052D,
				SFX_SKEDAR_ROAR_052E,
				SFX_SKEDAR_ROAR_052F,
			};

			soundnum = sounds[rngRandom() % 3];
			nextindexskedar++;

			if (nextindexskedar >= ARRAYCOUNT(sounds)) {
				nextindexskedar = 0;
			}
		}
	} else if (chr->headnum == HEAD_DDSHOCK) {
		int16_t sounds[] = {
			SFX_ARGH_MALE_0086,
			SFX_ARGH_MALE_0088,
			SFX_ARGH_MALE_008A,
			SFX_ARGH_MALE_008C,
			SFX_ARGH_MALE_008E,
			SFX_ARGH_MALE_0090,
			SFX_ARGH_MALE_0092,
			SFX_ARGH_MALE_0094,
			SFX_ARGH_MALE_0096,
			SFX_ARGH_MALE_0098,
			SFX_ARGH_MALE_009A,
			SFX_ARGH_MALE_009C,
			SFX_ARGH_MALE_009E,
			SFX_ARGH_MALE_0087,
		};

		soundnum = sounds[nextindexshock];
		nextindexshock++;

		if (nextindexshock >= ARRAYCOUNT(sounds)) {
			nextindexshock = 0;
		}

		allowoverride = true;
	} else if (male) {
		int16_t sounds[] = {
			SFX_ARGH_MALE_0086,
			SFX_ARGH_MALE_0087,
			SFX_ARGH_MALE_0088,
			SFX_ARGH_MALE_0089,
			SFX_ARGH_MALE_008A,
			SFX_ARGH_MALE_008B,
			SFX_ARGH_MALE_008C,
			SFX_ARGH_MALE_008D,
			SFX_ARGH_MALE_008E,
			SFX_ARGH_MALE_008F,
			SFX_ARGH_MALE_0090,
			SFX_ARGH_MALE_0091,
			SFX_ARGH_MALE_0092,
			SFX_ARGH_MALE_0093,
			SFX_ARGH_MALE_0094,
			SFX_ARGH_MALE_0095,
			SFX_ARGH_MALE_0096,
			SFX_ARGH_MALE_0097,
			SFX_ARGH_MALE_0098,
			SFX_ARGH_MALE_0099,
			SFX_ARGH_MALE_009A,
			SFX_ARGH_MALE_009B,
			SFX_ARGH_MALE_009C,
			SFX_ARGH_MALE_009D,
			SFX_ARGH_MALE_009E,
		};

		soundnum = sounds[nextindexmale];
		nextindexmale++;

		allowoverride = true;

		if (nextindexmale >= ARRAYCOUNT(sounds)) {
			nextindexmale = 0;
		}
	} else if (chr->headnum == HEAD_DARK_COMBAT
			|| chr->headnum == HEAD_DARK_FROCK
			|| chr->headnum == HEAD_DARKAQUA
			|| chr->headnum == HEAD_DARK_SNOW) {
		int16_t sounds[] = {
			SFX_ARGH_JO_02AA,
			SFX_ARGH_JO_02AB,
			SFX_ARGH_JO_02AC,
			SFX_ARGH_JO_02AD,
			SFX_ARGH_JO_02AE,
			SFX_ARGH_JO_02AF,
			SFX_ARGH_JO_02B0,
			SFX_ARGH_JO_02B1,
			SFX_ARGH_JO_02B2,
			SFX_ARGH_JO_02B3,
		};

		soundnum = sounds[rngRandom() % 10];
		allowoverride = true;
	} else {
		int16_t sounds[] = {
			SFX_ARGH_FEMALE_000D,
			SFX_ARGH_FEMALE_000E,
			SFX_ARGH_FEMALE_000F,
		};

		soundnum = sounds[nextindexfemale];
		nextindexfemale++;

		if (nextindexfemale >= ARRAYCOUNT(sounds)) {
			nextindexfemale = 0;
		}

		allowoverride = true;
	}

	if (allowoverride) {
		if (choketype == CHOKETYPE_GURGLE) {
			int sounds[] = {
				SFX_M1_CHOKING,
				SFX_GURGLE_05B1,
				SFX_GURGLE_05B2,
			};

			if ((rngRandom() % 8) == 0) {
				soundnum = sounds[rngRandom() % 3];
			}

			chr->soundgap = 10;
			chr->soundtimer = 0;
		} else if (choketype == CHOKETYPE_COUGH) {
			if (male) {
				if ((rngRandom() % 2) == 0) {
					soundnum = SFX_COUGH_04AF;
				} else {
					soundnum = SFX_COUGH_04B0;
				}
			} else {
				int index = rngRandom() % 4;
				int sounds[] = {
					SFX_COUGH_05AB,
					SFX_COUGH_05AC,
					SFX_COUGH_05AD,
					SFX_COUGH_05AE,
				};

				soundnum = sounds[index];
			}

			chr->soundgap = 10;
			chr->soundtimer = 0;
		}
	}

	if (soundnum >= 0) {
		if (chr->prop->type == PROPTYPE_PLAYER) {
			if (g_Vars.players[playernum]->chokehandle == NULL) {
				sndStart(var80095200, soundnum, &g_Vars.players[playernum]->chokehandle, -1, -1, -1, -1, -1);
			}
		} else {
			psStopSound(chr->prop, PSTYPE_CHRTALK, 0);

			if (!psPropHasSoundWithContext(chr->prop, PSTYPE_CHRCHOKE)) {
				psCreate(NULL, chr->prop, soundnum, -1,
						-1, 0, 0, PSTYPE_CHRCHOKE, NULL, -1, NULL, -1, -1, -1, -1);
			}
		}
	}
}

float chrGetShield(struct chrdata *chr)
{
	return chr->cshield;
}

void chrSetShield(struct chrdata *chr, float amount)
{
	if (amount < 0) {
		amount = 0;
	}

	chr->cshield = amount;

	if ((chr->hidden & CHRHFLAG_INFINITESHIELD) && chr->cshield < 1) {
		chr->cshield = 1;
	}

	if (chr->prop->type == PROPTYPE_PLAYER) {
		int playernum = playermgrGetPlayerNumByProp(chr->prop);

		if (playernum >= 0) {
			int prevplayernum = g_Vars.currentplayernum;
			setCurrentPlayerNum(playernum);
			playerDisplayHealth();
			g_Vars.currentplayerstats->armourcount += amount * 0.125f;
			setCurrentPlayerNum(prevplayernum);
		}
	}
}

bool func0f034080(struct chrdata *chr, struct modelnode *node, struct prop *prop, struct model *model, int side, int16_t *arg5)
{
	if (chrGetShield(chr) > 0) {
		if (node && (node->type & 0xff) == MODELNODETYPE_BBOX) {
			shieldhitCreate(chr->prop, chrGetShield(chr), prop, node, model, side, arg5);
		}

		return true;
	}

	return false;
}

/**
 * Damage the chr, bypassing any shield.
 *
 * Used for knife poison, nbomb damage, Investigation radioactivity and Escape gas.
 */
void chrDamageByMisc(struct chrdata *chr, float damage, struct coord *vector, struct gset *gset, struct prop *prop)
{
	chrDamage(chr, damage, vector, gset, prop, HITPART_GENERAL,
			false,     // damageshield
			NULL,      // prop2
			NULL,      // node
			NULL,      // model
			-1,        // side
			NULL,      // arg11
			false,     // explosion
			NULL);     // explosionpos
}

void chrDamageByLaser(struct chrdata *chr, float damage, struct coord *vector, struct gset *gset, struct prop *prop)
{
	chrDamage(chr, damage, vector, gset, prop, HITPART_GENERAL,
			true,      // damageshield
			chr->prop, // prop2
			NULL,      // node
			NULL,      // model
			-1,        // side
			NULL,      // arg11
			false,     // explosion
			NULL);     // explosionpos
}

void func0f0341dc(struct chrdata *chr, float damage, struct coord *vector, struct gset *gset, struct prop *prop, int hitpart, struct prop *prop2, struct modelnode *node, struct model *model, int side, int16_t *arg10)
{
	chrDamage(chr, damage, vector, gset, prop, hitpart,
			true,      // damageshield
			prop2,     // prop2
			node,      // node
			model,     // model
			side,      // side
			arg10,     // arg11
			false,     // explosion
			NULL);     // explosionpos
}

/**
 * Used for punching, but also used by AI commands to make chrs take damage.
 */
void chrDamageByImpact(struct chrdata *chr, float damage, struct coord *vector, struct gset *gset, struct prop *prop, int hitpart)
{
	struct modelnode *node = NULL;
	struct model *model = NULL;
	int side = 0;

	if (chrGetShield(chr) >= 0 && chr->model) {
		chrCalculateShieldHit(chr, &chr->prop->pos, vector, &node, &hitpart, &model, &side);
	}

	chrDamage(chr, damage, vector, gset, prop, hitpart,
			true,      // damageshield
			chr->prop, // prop2
			node,      // node
			model,     // model
			side,      // side
			NULL,      // arg11
			false,     // explosion
			NULL);     // explosionpos
}

void chrDamageByExplosion(struct chrdata *chr, float damage, struct coord *vector, struct prop *prop, struct coord *explosionpos)
{
	chrDamage(chr, damage, vector, NULL, prop, HITPART_GENERAL,
			true,      // damageshield
			chr->prop, // prop2
			NULL,      // node
			NULL,      // model
			-1,        // side
			NULL,      // arg11
			true,      // explosion
			explosionpos);
}

void playerUpdateDamageStats(struct prop *attacker, struct prop *victim, float damage)
{
	int playernum;

	if (attacker && attacker->type == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(attacker);

		if (playernum >= 0) {
			g_Vars.playerstats[playernum].damtransmitted += damage;
		}
	}

	if (victim && victim->type == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(victim);

		if (playernum >= 0) {
			g_Vars.playerstats[playernum].damreceived += damage;
		}
	}
}

/**
 * Handle a chr being damaged.
 *
 * Calculates and damage based on many factors and applies it to the chr,
 * killing them if needed.
 *
 * The chr can be a player, aibot or regular chr. The chr can also be of any
 * race (human, Skedar, Dr Caroll, robot or eyespy).
 *
 * chr - the chr being damaged
 * damage - the base amount of damage to deal, prior to scaling factors
 * vector - position of the attacker?
 * gset - gun settings struct
 * aprop - the attacker's prop struct
 * hitpart - "i've been hit" value, ie. the body part (see HITPART constants)
 * damageshield - false if attack should bypass shield if any
 * prop2 - ?
 * node - if shielded, model node (of type bbox) which was hit
 * model - if shielded, model of chr
 * side - if shielded, side of the model node's bounding box which was hit (0-5)
 * arg11 - ?
 * explosion - true if damage is coming from an explosion
 * explosionpos - position of said explosion
 */
void chrDamage(struct chrdata *chr, float damage, struct coord *vector, struct gset *gset,
		struct prop *aprop, int hitpart, bool damageshield, struct prop *prop2,
		struct modelnode *node, struct model *model, int side, int16_t *arg11,
		bool explosion, struct coord *explosionpos)
{
	bool onehitko = false;
	int race = CHRRACE(chr);
	float shield;
	bool makedizzy;
	bool ismelee;
	struct prop *vprop = chr->prop;
	float headshotdamagescale = 1;
	bool usedshield = false;
	bool showshield = false;
	bool showdamage = false;
	struct gset gset2 = {0};
	float explosionforce = damage;
	float healthscale = 1;
	float armourscale = 1;
	bool isshoot = true;
	bool forceapplydamage = false;
	struct weaponfunc *func;
	float amount;
	bool canchoke = true;
	int aplayernum = -1;
	int choketype = CHOKETYPE_NONE;

	if (hitpart == HITPART_HEAD) {
		choketype = CHOKETYPE_GURGLE;
	}

	if (gset) {
		if (gset->weaponnum == WEAPON_COMBATKNIFE) {
			if (gset->weaponfunc == FUNC_2) {
				canchoke = false;
			}

			if (gset->weaponfunc == FUNC_POISON) {
				choketype = CHOKETYPE_COUGH;
			}
		} else if (gset->weaponnum == WEAPON_TRANQUILIZER) {
			if (gset->weaponfunc == FUNC_SECONDARY) {
				choketype = CHOKETYPE_GURGLE;
			}
		}
	}

	// Don't damage if in CI training outside of training session
	if (chr->prop == g_Vars.currentplayer->prop
			&& g_Vars.currentplayer->training == false
			&& mainGetStageNum() == STAGE_CITRAINING) {
		return;
	}

	// Don't damage if attacker was anti and chr is non-interactable by anti
	if (g_Vars.antiplayernum >= 0
			&& aprop
			&& aprop == g_Vars.anti->prop
			&& (chr->hidden & CHRHFLAG_ANTINONINTERACTABLE)) {
		return;
	}

	// Don't damage if coop and friendly fire is off (human buddy)
	if (g_Vars.coopplayernum >= 0
			&& g_Vars.coopfriendlyfire == false
			&& aprop
			&& aprop != vprop
			&& aprop->type == PROPTYPE_PLAYER
			&& vprop->type == PROPTYPE_PLAYER) {
		return;
	}

	// Don't damage if coop and friendly fire is off (AI buddy)
	if (g_MissionConfig.iscoop
			&& g_Vars.coopfriendlyfire == false
			&& aprop
			&& aprop != vprop
			&& (aprop->type == PROPTYPE_PLAYER || aprop->type == PROPTYPE_CHR)
			&& chr->team == TEAM_ALLY
			&& aprop->chr->team == TEAM_ALLY) {
		return;
	}

	// Don't allow coop AI to kill or destroy anything
	// which anti wouldn't be able to
	if (g_MissionConfig.iscoop
			&& aprop
			&& aprop != vprop
			&& aprop->type == PROPTYPE_CHR
			&& aprop->chr->team == TEAM_ALLY
			&& (chr->hidden & CHRHFLAG_ANTINONINTERACTABLE)) {
		return;
	}

#ifndef PLATFORM_N64
	// Don't damage if team multiplayer and friendly fire is off
	if (g_Vars.mplayerisrunning
			&& (g_MpSetup.options & MPOPTION_TEAMSENABLED)
			&& !(g_MpSetup.options & MPOPTION_FRIENDLYFIRE)
			&& aprop
			&& aprop != vprop
			&& (aprop->type == PROPTYPE_PLAYER || aprop->type == PROPTYPE_CHR)
			&& chr->team == aprop->chr->team) {
		return;
	}
#endif

	if (gset == NULL) {
		gset = &gset2;
	}

	func = gsetGetWeaponFunction(gset);
	ismelee = func && (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE;
	makedizzy = race != RACE_DRCAROLL && gsetHasFunctionFlags(gset, FUNCFLAG_MAKEDIZZY);

	if (chr->prop == g_Vars.currentplayer->prop && g_Vars.currentplayer->invincible) {
		return;
	}

	if (ismelee) {
		isshoot = false;
	}

	// Set a flag on the victim that makes them switch to their "shot" AI list
	chr->chrflags |= CHRCFLAG_TRIGGERSHOTLIST;

	// Set a flag on the attacker so their AI scripting can tell that they've
	// hit their target
	if (aprop
			&& aprop->type == PROPTYPE_CHR
			&& chrGetTargetProp(aprop->chr) == chr->prop) {
		aprop->chr->chrflags |= CHRCFLAG_INJUREDTARGET;
	}

	// Disarm only hurts the victim in solo missions and if the victim is an NPC
	if (gsetHasFunctionFlags(gset, FUNCFLAG_DISARM)
			&& gset->weaponnum == WEAPON_UNARMED
			&& (vprop->type == PROPTYPE_PLAYER || g_Vars.normmplayerisrunning)) {
		damage = 0;
	}

	// Apply damage scaling based on difficulty settings
	if (g_Vars.mplayerisrunning == false) {
		// Solo
		if (explosion) {
			if (vprop->type == PROPTYPE_PLAYER) {
				damage *= g_ExplosionDamageTxScale;
			}
		} else if (aprop && aprop->type == PROPTYPE_PLAYER) {
			// Player is attacking
			damage *= g_PlayerDamageTxScale;
			headshotdamagescale = 25;
		} else if (aprop && aprop->type == PROPTYPE_CHR && vprop->type == PROPTYPE_PLAYER) {
			// Chr is attacking player
			damage *= g_PlayerDamageRxScale * pdmodeGetEnemyDamage();
		}

		if (vprop->type != PROPTYPE_PLAYER) {
			damage /= pdmodeGetEnemyHealth();
		}

		if (vprop->type == PROPTYPE_PLAYER) {
			healthscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->healthscale;
			armourscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->armourscale;
		}
	} else if (g_Vars.coopplayernum >= 0) {
		// Co-op
		if (explosion) {
			if (vprop->type == PROPTYPE_PLAYER) {
				damage *= g_ExplosionDamageTxScale;
			}
		} else if (aprop && aprop->type == PROPTYPE_PLAYER && vprop->type != PROPTYPE_PLAYER) {
			damage *= g_PlayerDamageTxScale;
			headshotdamagescale = 25;
		} else if (aprop && aprop->type == PROPTYPE_CHR && vprop->type == PROPTYPE_PLAYER) {
			damage *= g_PlayerDamageRxScale * pdmodeGetEnemyDamage();
		}

		if (vprop->type != PROPTYPE_PLAYER) {
			damage /= pdmodeGetEnemyHealth();
		}

		if (vprop->type == PROPTYPE_PLAYER) {
			healthscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->healthscale;
			armourscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->armourscale;
		}
	} else if (g_Vars.antiplayernum >= 0) {
		// Anti
		if (explosion) {
			if (vprop == g_Vars.bond->prop) {
				damage *= g_ExplosionDamageTxScale;
			}
		} else if (aprop && aprop == g_Vars.bond->prop) {
			damage *= g_PlayerDamageTxScale;
			headshotdamagescale = 25;
		} else if (aprop && aprop != g_Vars.bond->prop && vprop == g_Vars.bond->prop) {
			damage *= g_PlayerDamageRxScale * pdmodeGetEnemyDamage();
		}

		if (vprop != g_Vars.bond->prop) {
			damage /= pdmodeGetEnemyHealth();
		}

		if (vprop == g_Vars.bond->prop) {
			healthscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->healthscale;
			armourscale = g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->armourscale;
		}

		// Anti shooting other enemies is lethal
		if (aprop && aprop == g_Vars.anti->prop && vprop != g_Vars.bond->prop) {
			damage *= 100;
		}
	} else {
		// Normal multiplayer
		if (vprop->type == PROPTYPE_PLAYER) {
			int prevplayernum = g_Vars.currentplayernum;
			setCurrentPlayerNum(playermgrGetPlayerNumByProp(vprop));
			damage *= g_Vars.currentplayerstats->damagescale;
			setCurrentPlayerNum(prevplayernum);
		}
	}

	// Apply rumble
	if (vprop->type == PROPTYPE_PLAYER) {
		int prevplayernum = g_Vars.currentplayernum;

		int contpad1;
		int contpad2;

		setCurrentPlayerNum(playermgrGetPlayerNumByProp(vprop));

		joyGetContpadNumsForPlayer(g_Vars.currentplayernum, &contpad1, &contpad2);

		if (contpad1 >= 0) {
			pakRumble(contpad1, 0.25f, -1, -1);
		}

		if (contpad2 >= 0) {
			pakRumble(contpad2, 0.25f, -1, -1);
		}

		setCurrentPlayerNum(prevplayernum);
	}

	// Find the attacker's player number if possible
	// (includes MP aibots, not applicable for solo chrs)
	if (g_Vars.mplayerisrunning) {
		if (aprop && (aprop->type == PROPTYPE_PLAYER || aprop->type == PROPTYPE_CHR)) {
			aplayernum = mpPlayerGetIndex(aprop->chr);
		}
	} else {
		if (aprop && aprop->type == PROPTYPE_PLAYER) {
			aplayernum = playermgrGetPlayerNumByProp(aprop);
		}
	}

	// If using the shotgun, scale the damage based on distance
	if (aprop && aprop->type == PROPTYPE_CHR && gset->weaponnum == WEAPON_SHOTGUN) {
		float xdiff = aprop->pos.x - vprop->pos.x;
		float ydiff = aprop->pos.y - vprop->pos.y;
		float zdiff = aprop->pos.z - vprop->pos.z;
		float sqdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

		if (sqdist < 200 * 200) {
			damage *= 4.0f + (int)(rngRandom() % 3); // 4, 5 or 6
		} else if (sqdist < 400 * 400) {
			damage *= 3.0f + (int)(rngRandom() % 2); // 3 or 4
		} else if (sqdist < 800 * 800) {
			damage *= 2.0f + (int)(rngRandom() % 2); // 2 or 3
		} else if (sqdist < 1600 * 1600) {
			damage *= 1.0f + (int)(rngRandom() % 2); // 1 or 2
		}
	}

	// damageshield is an argument to this function,
	// but is forced on if using the Farsight.
	if (gset && gset->weaponnum == WEAPON_FARSIGHT) {
		damageshield = true;
		damage *= 10;
	}

	// Handle shield damage
	if (damageshield) {
		shield = chrGetShield(chr);

		if (chr->aibot && chr->aibot->config->type == BOTTYPE_TURTLE) {
			armourscale = 4;
		}

		if (shield > 0) {
			if (g_Vars.normmplayerisrunning) {
				damage = damage * mpHandicapToDamageScale(g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].handicap);
			}

			chr->chrflags |= CHRCFLAG_SHIELDDAMAGED;

			if (prop2 && node && chr->model) {
				func0f034080(chr, node, prop2, model, side, arg11);
			} else {
				shieldhitCreate(chr->prop, chrGetShield(chr), NULL, NULL, NULL, 0, 0);
			}

			if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_ONEHITKILLS)) {
				damage = 0;
				chrSetShield(chr, 0);
			} else if (shield >= damage / armourscale) {
				// Has enough shield to sustain the damage
				shield -= damage / armourscale;
				damage = 0;
				chrSetShield(chr, shield);
			} else {
				// Shield is now gone
				damage = 0;
				chrSetShield(chr, 0);
			}

			showshield = true;
			usedshield = true;
		}
	}

	// Handle hat shots. This is left over from GE, as hats don't exist in PD
	if (damage > 0 && hitpart == HITPART_HAT && chr->weapons_held[2]) {
		int type = hatGetType(chr->weapons_held[2]);

		if (type == HATTYPE_CLOTH) {
			// Hat remains on head and damages the chr (eg. Moonraker Elite)
			hitpart = HITPART_HEAD;
		} else if (type != HATTYPE_METAL) {
			// Normal hat
			damage = 0;
			objSetDropped(chr->weapons_held[2], DROPTYPE_HAT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		} else {
			// Metal helmets don't fall off and make a metallic chink noise when shot
			uint16_t sounds[] = { SFX_HIT_METAL_807B, SFX_HIT_METAL_8079, SFX_HATHIT_807C };
			damage = 0;

			psCreate(NULL, chr->prop, sounds[rngRandom() % 3], -1,
					-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
		}
	}

	// Handle incrementing player shot count
	if (aprop && aprop->type == PROPTYPE_PLAYER && !explosion) {
		bool alreadydead = false;
		int prevplayernum = g_Vars.currentplayernum;
		setCurrentPlayerNum(playermgrGetPlayerNumByProp(aprop));

		// ACT_DIE is not checked here, so it would appear that shooting
		// a chr as they're dying will increment the shots hit count
		if (chr && chr->actiontype == ACT_DEAD) {
			alreadydead = true;
		}

		if (vprop->type == PROPTYPE_PLAYER && g_Vars.players[playermgrGetPlayerNumByProp(vprop)]->isdead) {
			alreadydead = true;
		}

		if (!alreadydead && hitpart) {
			switch (hitpart) {
			case HITPART_HEAD:
				mpstatsIncrementPlayerShotCount2(gset, SHOTREGION_HEAD);
				break;
			case HITPART_GUN:
				mpstatsIncrementPlayerShotCount2(gset, SHOTREGION_GUN);
				break;
			case HITPART_HAT:
				mpstatsIncrementPlayerShotCount2(gset, SHOTREGION_HAT);
				break;
			case HITPART_PELVIS:
			case HITPART_TORSO:
				mpstatsIncrementPlayerShotCount2(gset, SHOTREGION_BODY);
				break;
			default:
				mpstatsIncrementPlayerShotCount2(gset, SHOTREGION_LIMB);
				break;
			}
		}

		setCurrentPlayerNum(prevplayernum);
	}

	// If the chr is invincible, make them flinch then we're done
	if (chr->chrflags & CHRCFLAG_INVINCIBLE) {
		chrFlinchBody(chr);
		return;
	}

	// If chr is dying or already dead, consider making their head flinch
	// then we're done
	if (chr->actiontype == ACT_DIE || chr->actiontype == ACT_DEAD) {
		if (hitpart == HITPART_HEAD && chr->actiontype == ACT_DIE && race != RACE_SKEDAR && isshoot) {
			struct coord pos;
			pos.x = vprop->pos.x - vector->x;
			pos.y = vprop->pos.y - vector->y;
			pos.z = vprop->pos.z - vector->z;
			chrFlinchHead(chr, chrGetAngleToPos(chr, &pos));
		}

		return;
	}

	// At this point the chr is known to be alive before they were shot
	{
		struct prop *weapon;
		struct coord sp9c;
		float angle;

		sp9c.x = vprop->pos.x - vector->x;
		sp9c.y = vprop->pos.y - vector->y;
		sp9c.z = vprop->pos.z - vector->z;
		angle = chrGetAngleToPos(chr, &sp9c);

		// Knife in the back to an unalerted chr is lethal
		if (gset->weaponnum == WEAPON_COMBATKNIFE
				&& gset->weaponfunc == FUNC_PRIMARY
				&& angle > 2.0940616130829f
				&& angle < 4.1881237030029f
				&& (chr->alertness < 100 || chr->lastseetarget60 == 0)) {
			damage *= 1000;
		}

		// Punching and pistol whipping is less effective from the front
		if (gsetHasFunctionFlags(gset, FUNCFLAG_BLUNTIMPACT)) {
			if (angle < 1.0470308065414f || angle > 5.2351541519165f) {
				damage *= 0.4f;
			} else if (angle < 2.0940616130829f || angle > 4.1881237030029f) {
				damage *= 0.7f;
			} else if (chr->alertness < 100) {
				onehitko = true;
			}

			if (chrGetHeldProp(chr, HAND_RIGHT) == NULL
					&& chrGetHeldProp(chr, HAND_LEFT) == NULL
					&& (chr->gunprop == NULL || chr->actiontype == ACT_SURRENDER || chr->actiontype == ACT_SURPRISED)) {
				// Chr is unarmed and has no hope of getting their gun
				onehitko = true;
			}

			forceapplydamage = true;
		}

		if (hitpart == HITPART_GENERAL) {
			// Halve the damage because it's doubled for torso below
			hitpart = HITPART_TORSO;
			damage *= 0.5f;
		} else if (hitpart == HITPART_GENERALHALF) {
			// Likewise, quarter it here so it becomes half below
			hitpart = HITPART_TORSO;
			damage *= 0.25f;
		}

		// Hits to a Skedar's tail are 10x more lethal
		if (race == RACE_SKEDAR && hitpart == HITPART_TAIL) {
			damage *= 10;
		}

		// Apply damage multipliers based on which body parts were hit,
		// and flinch head if shot in the head
		if (hitpart == HITPART_HEAD) {
			if (race == RACE_SKEDAR) {
				damage += damage;
				chrFlinchHead(chr, angle);
			} else {
				damage *= 4;

				if (isshoot && !usedshield) {
					chrFlinchHead(chr, angle);
					damage *= headshotdamagescale;

					if (gset->weaponnum == WEAPON_COMBATKNIFE && gset->weaponfunc != FUNC_POISON) {
						damage += damage;
					}
				}
			}
		} else if (hitpart == HITPART_TORSO) {
			// Double damage for torso hits
			damage += damage;
		} else if (hitpart == HITPART_GUN) {
			// No damage for gun hits
			damage = 0;
			makedizzy = false;
		} else if (hitpart == HITPART_HAT) {
			// No damage for hat hits
			damage = 0;
			makedizzy = false;
		}

		// Handle situations where the player is the one being shot, then return
		if (vprop->type == PROPTYPE_PLAYER) {
			int prevplayernum = g_Vars.currentplayernum;
			setCurrentPlayerNum(playermgrGetPlayerNumByProp(vprop));

			if (g_Vars.normmplayerisrunning) {
				damage /= mpHandicapToDamageScale(g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].handicap);
			}

			if (g_Vars.currentplayer->isdead == false && !g_PlayerInvincible) {
				float boostscale;

				// Handle player losing gun
				if (gsetHasFunctionFlags(gset, FUNCFLAG_DISARM)) {
					bgunDisarm(aprop);
				}

				// Handle player dizziness
				if (makedizzy && g_Vars.currentplayer->invincible == false) {
					float blurscale = 1;
					struct chrdata *achr = NULL;

					if (aprop) {
						achr = aprop->chr;

						if (achr && achr->bodynum == BODY_MINISKEDAR) {
							blurscale = 4;
						}
					}

					if (!achr
							|| !achr->aibot
							|| !gsetHasFunctionFlags(gset, FUNCFLAG_00400000)
							|| chr->blurdrugamount < TICKS(4500)) {
						chr->blurdrugamount += gsetGetBlurAmount(gset) * blurscale;
					}

					chr->blurnumtimesdied = 0;
				}

				// Handle player damage
				if (g_Vars.currentplayer->invincible == false && damage > 0) {
					float statsamount = amount = damage * 0.125f;

					if (statsamount > g_Vars.currentplayer->bondhealth) {
						statsamount = g_Vars.currentplayer->bondhealth;
					}

					if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_ONEHITKILLS)) {
						statsamount = g_Vars.currentplayer->bondhealth;
					}

					playerUpdateDamageStats(aprop, vprop, statsamount);
					playerDisplayHealth();

					if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_ONEHITKILLS)) {
						g_Vars.currentplayer->bondhealth = 0;
					}

					g_Vars.currentplayer->bondhealth -= amount / healthscale;

					chr->lastattacker = (aprop ? aprop->chr : NULL);

					showdamage = true;

					if (g_Vars.currentplayer->training == false
							&& g_Vars.currentplayer->bondhealth <= 0) {
						playerDieByShooter(aplayernum, false);
						chr->blurnumtimesdied++;
					}

					if (!lvIsPaused() && canchoke) {
						chrChoke(chr, choketype);
					}

					if (g_Vars.currentplayer->haschrbody) {
						chrFlinchBody(chr);
					}
				}

				// Handle player boost
				if (ismelee && gset->weaponnum == WEAPON_REAPER) {
					boostscale = 0.1f;
				} else if (g_Vars.normmplayerisrunning) {
					boostscale = 0.75f;
				} else {
					boostscale = 1;
				}

				g_Vars.currentplayer->bondshotspeed.x += vector->x * boostscale;
				g_Vars.currentplayer->bondshotspeed.z += vector->z * boostscale;

				if (showdamage) {
					playerDisplayDamage();
				}

				if (showshield) {
					playerDisplayShield();
				}

				if (g_Vars.normmplayerisrunning && aprop && aprop->type == PROPTYPE_PLAYER) {
					playerCheckIfShotInBack(prevplayernum, vector->x, vector->z);
				}
			}

			setCurrentPlayerNum(prevplayernum);
			return;
		}

		// This check is pointless - a similar check and return exists earlier
		if (chr->actiontype == ACT_DIE || chr->actiontype == ACT_DEAD) {
			return;
		}

		// At this point we know we're dealing with a NPC being shot, and the
		// NPC was alive prior to being shot.

		// Handle aibot/chr losing gun
		if (gsetHasFunctionFlags(gset, FUNCFLAG_DISARM)
				&& ((chr->flags & CHRFLAG0_CANLOSEGUN) || chr->aibot)) {
			if (chr->aibot) {
				botDisarm(chr, aprop);
			} else {
				weapon = chrGetHeldProp(chr, HAND_RIGHT);

				if (weapon) {
					chr->gunprop = weapon;
					objSetDropped(weapon, DROPTYPE_DEFAULT);
					chr->hidden |= CHRHFLAG_DROPPINGITEM;
				}

				weapon = chrGetHeldProp(chr, HAND_LEFT);

				if (weapon) {
					chr->gunprop = weapon;
					objSetDropped(weapon, DROPTYPE_DEFAULT);
					chr->hidden |= CHRHFLAG_DROPPINGITEM;
				}
			}
		}

		// Handle chr damage
		if (chr->damage < chr->maxdamage
				|| (!g_Vars.normmplayerisrunning && chr->actiontype != ACT_PREARGH)) {
			float sp80 = 0;

			chr->numarghs++;

			// Handle chr dizziness and psychosis
			if (makedizzy && race != RACE_DRCAROLL && race != RACE_ROBOT) {
				if (gsetHasFunctionFlags(gset, FUNCFLAG_PSYCHOSIS)) {
					chr->hidden |= CHRHFLAG_PSYCHOSISED;
				} else {
					chr->blurdrugamount += gsetGetBlurAmount(gset);
					chr->blurnumtimesdied = 0;

					if (!chr->aibot && chr->blurdrugamount >= TICKS(5000)) {
						onehitko = true;
					}
				}
			}

			// Handle chr boost
			if (chr->aibot) {
				float boostscale;

				if (ismelee && gset->weaponnum == WEAPON_REAPER) {
					boostscale = 0.1f;
				} else {
					boostscale = 0.75f;
				}

				chr->aibot->shotspeed.x += vector->x * boostscale;
				chr->aibot->shotspeed.z += vector->z * boostscale;
			}

			if (gset->weaponnum == WEAPON_UNARMED) {
				sp80 = 2;
			}

			if (gset->weaponnum == WEAPON_TRANQUILIZER || gset->weaponnum == WEAPON_PSYCHOSISGUN) {
				forceapplydamage = true;
			}

			// Handle one-hit knockouts
			if (onehitko && chr->aibot == NULL && race == RACE_HUMAN) {
				chrKnockOut(chr, angle, hitpart, gset);
				psStopSound(chr->prop, PSTYPE_CHRTALK, 0);

				if (canchoke) {
					chrChoke(chr, choketype);
				}

				if (gset->weaponnum == WEAPON_UNARMED && chr->actiontype != ACT_DRUGGEDKO) {
					return;
				}

				return;
			}

			// Handle applying damage to NPCs
			// Don't enter this branch if there is no damage to give,
			// or we are making a chr dizzy in solo mode (unless force is set)
			if (damage > 0 && (g_Vars.normmplayerisrunning || !makedizzy || forceapplydamage)) {
				float amount = damage;

				if (chr->damage + damage > chr->maxdamage) {
					amount = chr->maxdamage - chr->damage;
				}

				amount *= 0.125f;

				playerUpdateDamageStats(aprop, vprop, amount);

				chr->damage += damage;
				chr->lastattacker = (aprop ? aprop->chr : NULL);
				chr->chrflags |= CHRCFLAG_JUST_INJURED;

				if (chr->aibot) {
					if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_ONEHITKILLS)) {
						chr->damage = chr->maxdamage;
					}

					if (canchoke) {
						chrChoke(chr, choketype);
					}

					chrFlinchBody(chr);

					if (chr->damage >= chr->maxdamage) {
						chrDie(chr, aplayernum);
					}
				} else if (explosion) {
					// Chrs die instantly from explosion damage provided they
					// don't have any armour (the chr has armour if their
					// chr->damage is negative). Note that damage has already
					// been applied to the chr above, so a perfectly healthy chr
					// with no armour will already have a damage value > 0 at
					// this point.
					if (chr->damage > 0) {
						chr->damage = chr->maxdamage;

						if (race == RACE_DRCAROLL || race == RACE_EYESPY || race == RACE_ROBOT) {
							chrBeginDeath(chr, vector, angle, hitpart, gset, false, aplayernum);
						} else {
							chrYeetFromPos(chr, explosionpos, explosionforce);
						}

						if (canchoke) {
							chrChoke(chr, choketype);
						}

						if (g_Vars.mplayerisrunning) {
							mpstatsRecordDeath(aplayernum, mpPlayerGetIndex(chr));
						} else if (aprop && aprop->type == PROPTYPE_PLAYER) {
							int prevplayernum = g_Vars.currentplayernum;
							setCurrentPlayerNum(playermgrGetPlayerNumByProp(aprop));
							mpstatsRecordPlayerKill();
							setCurrentPlayerNum(prevplayernum);
						}

						if (chr->chrflags & CHRCFLAG_KILLCOUNTABLE) {
							mpstatsIncrementTotalKillCount();
						}

						if (chr->aibot == NULL) {
							chrDropConcealedItems(chr);
						}

						if (chr->aibot == NULL) {
							weapon = chr->weapons_held[HAND_RIGHT];

							if (weapon && (weapon->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
								objSetDropped(weapon, DROPTYPE_DEFAULT);
								chr->hidden |= CHRHFLAG_DROPPINGITEM;
							}

							weapon = chr->weapons_held[HAND_LEFT];

							if (weapon && (weapon->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
								objSetDropped(weapon, DROPTYPE_DEFAULT);
								chr->hidden |= CHRHFLAG_DROPPINGITEM;
							}
						}
					}
				} else {
					// Non-explosion damage to solo mode chr
					if (chr->actiontype != ACT_DRUGGEDKO && canchoke) {
						chrChoke(chr, choketype);
					}

					if (makedizzy && chr->damage >= chr->maxdamage) {
						chr->damage = chr->maxdamage - 0.1f;
						chrKnockOut(chr, angle, hitpart, gset);
					}

					// If chr has armour or the weapon doesn't stun
					if (chr->damage < 0 ||
							(gsetHasFunctionFlags(gset, FUNCFLAG_NOSTUN) && chr->damage < chr->maxdamage)) {
						float endframe = -1;

						if (!chrIsAnimPreventingArgh(chr, &endframe)) {
							chrFlinchBody(chr);
						}
					} else if (hitpart != HITPART_HAT) {
						// Cancel current animation and prepare for argh
						float endframe = -1;

						if (chrIsAnimPreventingArgh(chr, &endframe)) {
							if (endframe >= 0) {
								modelSetAnimEndFrame(chr->model, endframe);
							}

							chr->actiontype = ACT_PREARGH;
							chr->act_preargh.dir.x = vector->x;
							chr->act_preargh.dir.y = vector->y;
							chr->act_preargh.dir.z = vector->z;
							chr->act_preargh.relshotdir = angle;
							chr->act_preargh.hitpart = hitpart;
							chr->act_preargh.aplayernum = aplayernum;
							chr->act_preargh.gset.weaponnum = gset->weaponnum;
							chr->act_preargh.gset.unk0639 = gset->unk0639;
							chr->act_preargh.gset.unk063a = gset->unk063a;
							chr->act_preargh.gset.weaponfunc = gset->weaponfunc;

							chr->sleep = 0;
						} else {
							chrReactToDamage(chr, vector, angle, hitpart, gset, aplayernum);
						}
					}
				}

				if (sp80 > 0) {
					chr->timeextra = sp80 * 15;
					chr->elapseextra = 0;

					chr->extraspeed.x = prop2->pos.x - aprop->pos.x;
					chr->extraspeed.y = prop2->pos.y - aprop->pos.y;
					chr->extraspeed.z = prop2->pos.z - aprop->pos.z;

					utilsNormalizeF(&chr->extraspeed.x, &chr->extraspeed.y, &chr->extraspeed.z);

					chr->extraspeed.x *= sp80;
					chr->extraspeed.y *= sp80;
					chr->extraspeed.z *= sp80;
				}
			}
		}
	}
}

void chrDie(struct chrdata *chr, int aplayernum)
{
	if (chr->actiontype != ACT_DIE) {
		chrStopFiring(chr);
		chrUncloak(chr, true);

		chr->actiontype = ACT_DIE;
		chr->act_die.notifychrindex = 0;
		chr->sleep = 0;
		chr->blurnumtimesdied++;
		chr->act_die.thudframe1 = -1;
		chr->act_die.thudframe2 = -1;
		chr->act_die.timeextra = 0;

		chr->ailist = ailistFindById(GAILIST_AIBOT_DEAD);
		chr->aioffset = 0;

		mpstatsRecordDeath(aplayernum, mpPlayerGetIndex(chr));
		botinvDropAll(chr, chr->aibot->weaponnum);

		chr->aibot->hasbriefcase = false;
		chr->aibot->hascase = false;
		chr->aibot->unk04c_04 = false;
		chr->aibot->unk04c_03 = false;
		chr->aibot->hasuplink = false;
	}
}

bool func0f03645c(struct chrdata *chr, struct coord *arg1, RoomNum *arg2, struct coord *arg3, struct coord *arg4, int arg5)
{
	bool result = false;
	float ymax;
	float ymin;
	float radius;
	RoomNum rooms[8];
	struct prop *prop = chr->prop;

	chrGetBbox(prop, &radius, &ymax, &ymin);
	chrSetPerimEnabled(chr, false);

	if (cdTestCylMove04(arg1, arg2, arg3, rooms, arg5, 1, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION) {
		if (cdTestCylMove01(arg3, rooms, arg4, arg5, 1, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION) {
			result = true;
		}
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

bool func0f03654c(struct chrdata *chr, struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2, struct coord *vector, float arg6, uint32_t types)
{
	struct coord tmp;
	float a;
	float b;
	struct prop *prop;
	bool result = false;
	float ymax;
	float ymin;
	float radius;
	struct coord sp5c;
	struct coord sp50;
	RoomNum sp40[8];
	float mult;

	prop = chr->prop;

	chrSetPerimEnabled(chr, false);
	chrGetBbox(prop, &radius, &ymax, &ymin);

	if ((rooms2 && cdTestCylMove02(pos, rooms, pos2, rooms2, types, true, ymax - prop->pos.y, ymin - prop->pos.y))
			|| (rooms2 == NULL && cdTestCylMove01(pos, rooms, pos2, types, 1, ymax - prop->pos.y, ymin - prop->pos.y))) {
		if (vector == NULL) {
			vector = &tmp;

			tmp.x = pos2->x - pos->x;
			tmp.y = 0;
			tmp.z = pos2->z - pos->z;

			if (tmp.f[0] == 0 && tmp.f[2] == 0) {
				// @bug: Needs to call chrSetPerimEnabled(chr, true)
				// before returning
				return true;
			}

			mult = 1.0f / sqrtf(tmp.f[0] * tmp.f[0] + tmp.f[2] * tmp.f[2]);
			tmp.x *= mult;
			tmp.z *= mult;
		}

		a = vector->x * arg6;
		b = vector->z * arg6;

		sp5c.x = pos->x + b;
		sp5c.y = pos->y;
		sp5c.z = pos->z - a;

		sp50.x = pos2->x + b;
		sp50.y = pos2->y;
		sp50.z = pos2->z - a;

		if (cdTestCylMove04(pos, rooms, &sp5c, sp40, types, 1, ymax - prop->pos.y, ymin - prop->pos.y)
				&& cdTestCylMove01(&sp5c, sp40, &sp50, types, 1, ymax - prop->pos.y, ymin - prop->pos.y)) {
			sp5c.x = pos->x - b;
			sp5c.y = pos->y;
			sp5c.z = pos->z + a;

			sp50.x = pos2->x - b;
			sp50.y = pos2->y;
			sp50.z = pos2->z + a;

			if (cdTestCylMove04(pos, rooms, &sp5c, sp40, types, 1, ymax - prop->pos.y, ymin - prop->pos.y)
					&& cdTestCylMove01(&sp5c, sp40, &sp50, types, 1, ymax - prop->pos.y, ymin - prop->pos.y)) {
				result = true;
			}
		}
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

bool propchrHasClearLineToPos(struct prop *prop, struct coord *dstpos, struct coord *vector)
{
	return func0f03654c(prop->chr, &prop->pos, prop->rooms, dstpos, NULL, vector, prop->chr->radius * 1.2f, CDTYPE_ALL);
}

bool propchrHasClearLineInVector(struct prop *prop, struct coord *vector, float mult)
{
	struct coord dstpos;

	dstpos.x = vector->x * mult + prop->pos.x;
	dstpos.y = prop->pos.y;
	dstpos.z = vector->z * mult + prop->pos.z;

	return propchrHasClearLineToPos(prop, &dstpos, vector);
}

bool func0f036974(struct prop *prop, struct coord *pos)
{
	return func0f03654c(prop->chr, &prop->pos, prop->rooms, pos, NULL, NULL, prop->chr->radius * 1.2f, CDTYPE_ALL);
}

void chrGetSideVectorToTarget(struct chrdata *chr, bool side, struct coord *vector)
{
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);

	vector->x = 0;
	vector->y = 0;
	vector->z = 1;

	if (target) {
		float x = target->pos.x - prop->pos.x;
		float z = target->pos.z - prop->pos.z;
		float distance = sqrtf(x * x + z * z);

		if (distance > 0) {
			x = x / distance;
			z = z / distance;

			if (side) {
				vector->x = z;
				vector->y = 0;
				vector->z = -x;
			} else {
				vector->x = -z;
				vector->y = 0;
				vector->z = x;
			}
		}
	}
}

bool chrCanRollInDirection(struct chrdata *chr, bool side, float distance)
{
	struct prop *prop = chr->prop;
	struct coord vector;
	struct coord dstpos;

	chrGetSideVectorToTarget(chr, side, &vector);

	dstpos.x = vector.x * distance + prop->pos.x;
	dstpos.y = prop->pos.y;
	dstpos.z = vector.z * distance + prop->pos.z;

	return propchrHasClearLineToPos(prop, &dstpos, &vector);
}

void chrGetSideVector(struct chrdata *chr, bool side, struct coord *vector)
{
	float angle = chrGetInverseTheta(chr);

	if (side) {
		vector->x = cosf(angle);
		vector->y = 0;
		vector->z = -sinf(angle);
	} else {
		vector->x = -cosf(angle);
		vector->y = 0;
		vector->z = sinf(angle);
	}
}

bool chrCanJumpInDirection(struct chrdata *chr, bool side, float distance)
{
	struct prop *prop = chr->prop;
	struct coord vector;
	struct coord dstpos;

	chrGetSideVector(chr, side, &vector);

	dstpos.x = vector.x * distance + prop->pos.x;
	dstpos.y = prop->pos.y;
	dstpos.z = vector.z * distance + prop->pos.z;

	return propchrHasClearLineToPos(prop, &dstpos, &vector);
}

bool chrIsRoomOffScreen(struct chrdata *chr, struct coord *waypos, RoomNum *wayrooms)
{
	struct prop *prop = chr->prop;
	RoomNum sp7c[20];
	int i;
	RoomNum sp64[8];
	bool offscreen = true;
	RoomNum sp50[8];

	if ((chr->hidden & CHRHFLAG_CLOAKED) == 0 || USINGDEVICE(DEVICE_IRSCANNER)) {
		propUpdatePositionRooms(&prop->pos, prop->rooms, waypos, sp64, sp7c, 20);

		if (g_Vars.mplayerisrunning) {
			for (i = 0; sp7c[i] != -1; i++) {
				if (g_MpRoomVisibility[sp7c[i]] & 0x0f) {
					offscreen = false;
					break;
				}
			}
		} else {
			for (i = 0; sp7c[i] != -1; i++) {
				if (g_Rooms[sp7c[i]].flags & ROOMFLAG_ONSCREEN) {
					offscreen = false;
					break;
				}
			}
		}
	}

	if (offscreen) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			portalTraceLineThroughRooms(waypos, &g_Vars.players[i]->prop->pos, wayrooms, sp50, 0, 0);

			if (roomArrayIntersects(g_Vars.players[i]->prop->rooms, sp50)) {
				offscreen = false;
				break;
			}
		}
	}

	return offscreen;
}

void chrGoPosInitMagic(struct chrdata *chr, struct waydata *waydata, struct coord *padpos, struct coord *chrpos)
{
	float xdiff1 = padpos->x - chr->prop->pos.x;
	float zdiff1 = padpos->z - chr->prop->pos.z;

	float angle = atan2f(xdiff1, zdiff1);

	float xdiff2 = padpos->x - chrpos->x;
	float zdiff2 = padpos->z - chrpos->z;

	waydata->mode = WAYMODE_MAGIC;

	waydata->magictotal = sqrtf(xdiff1 * xdiff1 + zdiff1 * zdiff1);
	waydata->magicdone = waydata->magictotal - sqrtf(xdiff2 * xdiff2 + zdiff2 * zdiff2);

	chrSetLookAngle(chr, angle);
}

void chrGoPosGetCurWaypointInfoWithFlags(struct chrdata *chr, struct coord *pos, RoomNum *rooms, uint32_t *flags)
{
	struct waypoint *waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];
	struct pad pad;

	if (waypoint) {
		padUnpack(waypoint->padnum, PADFIELD_POS | PADFIELD_ROOM | PADFIELD_FLAGS, &pad);

		pos->x = pad.pos.x;
		pos->y = pad.pos.y;
		pos->z = pad.pos.z;

		rooms[0] = pad.room;
		rooms[1] = -1;

		if (flags) {
			*flags = pad.flags;
		}
	} else {
		pos->x = chr->act_gopos.endpos.x;
		pos->y = chr->act_gopos.endpos.y;
		pos->z = chr->act_gopos.endpos.z;

		rooms[0] = chr->act_gopos.endrooms[0];
		rooms[1] = -1;

		if (flags) {
			*flags = 0;
		}
	}
}

void chrGoPosGetCurWaypointInfo(struct chrdata *chr, struct coord *pos, RoomNum *rooms)
{
	chrGoPosGetCurWaypointInfoWithFlags(chr, pos, rooms, NULL);
}

float func0f0370a8(struct chrdata *chr)
{
	float result;

	if (chr->aibot) {
		result = botCalculateMaxSpeed(chr);
	} else {
		int16_t animnum = modelGetAnimNum(chr->model);
		result = func0f02dff0(animnum) * (chr->model->scale * 9.999999f);
	}

	return result;
}

int chrGoPosCalculateBaseTtl(struct chrdata *chr)
{
	float xdiff;
	float zdiff;
	struct coord pos;
	RoomNum rooms[8];
	float speed;

	chrGoPosGetCurWaypointInfo(chr, &pos, rooms);

	xdiff = pos.x - chr->prop->pos.x;
	zdiff = pos.z - chr->prop->pos.z;

	if (xdiff < 0) {
		xdiff = -xdiff;
	}

	if (zdiff < 0) {
		zdiff = -zdiff;
	}

	speed = func0f0370a8(chr);

	if (chr->aibot == NULL) {
		speed *= modelGetAbsAnimSpeed(chr->model);
	}

	if (speed < 0.001f) {
		speed = 0.001f;
	}

	return (xdiff + zdiff) / speed;
}

void chrGoPosClearRestartTtl(struct chrdata *chr)
{
	chr->act_gopos.restartttl = 0;
}

void chrGoPosConsiderRestart(struct chrdata *chr)
{
	if (chr->act_gopos.waydata.mode != WAYMODE_MAGIC
			&& chr->liftaction != LIFTACTION_WAITINGONLIFT
			&& chr->liftaction != LIFTACTION_WAITINGFORLIFT) {
		if (chr->act_gopos.restartttl == 0) {
			int value = chrGoPosCalculateBaseTtl(chr) * 2 + 300;

			if (value > 0xffff) {
				value = 0xffff;
			}

			chr->act_gopos.restartttl = value;
		} else if (chr->act_gopos.restartttl <= (uint16_t)g_Vars.lvupdate60) {
			if (chr->aibot) {
				botCheckFetch(chr);
			} else {
				chrGoToRoomPos(chr, &chr->act_gopos.endpos, chr->act_gopos.endrooms, chr->act_gopos.flags);
			}
		} else {
			chr->act_gopos.restartttl -= (uint16_t)g_Vars.lvupdate60;
		}
	}
}

void chrGoPosInitExpensive(struct chrdata *chr)
{
	struct coord pos;
	RoomNum rooms[8];

	chrGoPosGetCurWaypointInfo(chr, &pos, rooms);

	chr->act_gopos.waydata.mode = WAYMODE_INIT;
	chr->act_gopos.waydata.iter = 0;
	chr->act_gopos.waydata.gotaimpos = false;
	chr->act_gopos.waydata.aimpos.x = pos.x;
	chr->act_gopos.waydata.aimpos.y = pos.y;
	chr->act_gopos.waydata.aimpos.z = pos.z;

	chrGoPosClearRestartTtl(chr);
}

/**
 * Advance the chr's current waypoint index to the next one in the route.
 *
 * The waypoints array allows 6 waypoints and it's important that they have a
 * couple loaded in front of their current one. So if the index is moving too
 * far into the array, new pathfinding will be done and the array and index will
 * be reset.
 */
void chrGoPosAdvanceWaypoint(struct chrdata *chr)
{
	if (chr->act_gopos.curindex < 3) {
		chr->act_gopos.curindex++;
	} else {
		struct waypoint *from = chr->act_gopos.waypoints[chr->act_gopos.curindex];
		chr->act_gopos.curindex = 1;

		navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
		navFindRoute(from, chr->act_gopos.target, chr->act_gopos.waypoints, MAX_CHRWAYPOINTS);
		navSetSeed(0, 0);
	}

	chrGoPosInitExpensive(chr);
}

/**
 * Determines which step index the chr will be at given their current index, the
 * number of steps to take and in which direction (forward or back).
 *
 * Returns the step index and populates *forward with true or false depending on
 * whether the chr will be traversing the path in the forward direction at that
 * point.
 */
int chrPatrolCalculateStep(struct chrdata *chr, int *forward, int numsteps)
{
	int nextstep = chr->act_patrol.nextstep;
	int isforward = *forward;

	if (numsteps < 0) {
		isforward = !isforward;
		numsteps = -numsteps;
	}

	while (numsteps > 0) {
		numsteps--;

		if (isforward) {
			nextstep++;

			if (chr->act_patrol.path->pads[nextstep] < 0) {
				// Reached the end of the list
				if (chr->act_patrol.path->flags & PATHFLAG_CIRCULAR) {
					nextstep = 0;
				} else {
					isforward = false;
					nextstep -= 2;

					if (nextstep < 0) {
						nextstep = 0;
					}
				}
			}
		} else {
			nextstep--;

			if (nextstep < 0) {
				// Reached the start of the list
				if (chr->act_patrol.path->flags & PATHFLAG_CIRCULAR) {
					nextstep = chr->act_patrol.path->len - 1;
				} else {
					isforward = true;
					nextstep = 1;

					if (chr->act_patrol.path->len - 1 <= 0) {
						nextstep = chr->act_patrol.path->len - 1;
					}
				}
			}
		}
	}

	*forward = isforward;

	return nextstep;
}

/**
 * Determines which pad number the chr will be at given their current index and
 * the number of steps to take.
 */
int16_t chrPatrolCalculatePadNum(struct chrdata *chr, int numsteps)
{
	int *padnumptr;
	int forward = chr->act_patrol.forward;
	int step = chrPatrolCalculateStep(chr, &forward, numsteps);
	padnumptr = &chr->act_patrol.path->pads[step];

	return *padnumptr;
}

void chrPatrolGetCurWaypointInfoWithFlags(struct chrdata *chr, struct coord *pos, RoomNum *rooms, uint32_t *flags)
{
	int padnum = chrPatrolCalculatePadNum(chr, 0);
	struct pad pad;

	padUnpack(padnum, PADFIELD_POS | PADFIELD_ROOM | PADFIELD_FLAGS, &pad);

	pos->x = pad.pos.x;
	pos->y = pad.pos.y;
	pos->z = pad.pos.z;

	rooms[0] = pad.room;
	rooms[1] = -1;

	if (flags) {
		*flags = pad.flags;
	}
}

void chrPatrolGetCurWaypointInfo(struct chrdata *chr, struct coord *pos, RoomNum *rooms)
{
	chrPatrolGetCurWaypointInfoWithFlags(chr, pos, rooms, NULL);
}

void func0f037580(struct chrdata *chr)
{
	RoomNum rooms[8];

	chr->act_patrol.waydata.mode = WAYMODE_INIT;
	chr->act_patrol.waydata.iter = 0;
	chr->act_patrol.waydata.gotaimpos = false;

	chrPatrolGetCurWaypointInfo(chr, &chr->act_patrol.waydata.aimpos, rooms);
}

void func0f0375b0(struct chrdata *chr)
{
	int nextstep = chrPatrolCalculateStep(chr, &chr->act_patrol.forward, 1);

	chr->act_patrol.nextstep = nextstep;
	chr->patrolnextstep = nextstep;

	func0f037580(chr);
}

void chrNavTickMagic(struct chrdata *chr, struct waydata *waydata, float speed, struct coord *arg3, RoomNum *rooms)
{
	RoomNum sp118[8];
	float ymax;
	float ymin;
	float radius;
	float ground;
	uint16_t floorcol;
	uint8_t floortype;
	RoomNum floorroom;
	struct coord spf4;
	struct prop *prop = chr->prop;
	union modelrwdata *rwdata;
	struct waypoint *waypoint;
	struct coord spdc;
	RoomNum spcc[8];
	struct pad pad;
	struct coord sp5c;
	RoomNum sp4c[8];

	chr->invalidmove = 0;
	chr->lastmoveok60 = g_Vars.lvframe60;

	waydata->magicdone += speed * modelGetAbsAnimSpeed(chr->model) * g_Vars.lvupdate60freal;

	if (waydata->magicdone >= waydata->magictotal) {
		// Reached end of segment
		chrSetPerimEnabled(chr, false);
		roomsCopy(rooms, sp118);
		chrFindEnteredRooms(chr, arg3, sp118);

		ground = cdFindGroundInfoAtCyl(arg3, chr->radius, sp118, &floorcol, &floortype, 0, &floorroom, false, NULL);

		spf4.x = arg3->x;
		spf4.y = prop->pos.y - chr->ground + ground;
		spf4.z = arg3->z;

		roomsCopy(rooms, sp118);
		chrFindEnteredRooms(chr, &spf4, sp118);
		chrGetBbox(chr->prop, &radius, &ymax, &ymin);

		if (cdTestVolume(&spf4, chr->radius, sp118, CDTYPE_ALL, CHECKVERTICAL_YES, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION) {
			// Reached end of segment with no collision
			prop->pos.x = spf4.x;
			prop->pos.y = spf4.y;
			prop->pos.z = spf4.z;

			chr->ground = ground;
			chr->manground = ground;
			chr->sumground = ground * 10.0f;

			chr->floorcol = floorcol;
			chr->floortype = floortype;
			chr->floorroom = floorroom;

			propDeregisterRooms(prop);
			roomsCopy(sp118, prop->rooms);
			propRegisterRooms(prop);

			modelSetRootPosition(chr->model, &prop->pos);

			rwdata = modelGetNodeRwData(chr->model, chr->model->definition->rootnode);
			rwdata->chrinfo.ground = ground;

			chr->chrflags |= CHRCFLAG_FORCETOGROUND;

			if (chr->actiontype == ACT_PATROL) {
				func0f0375b0(chr);
				chrPatrolGetCurWaypointInfo(chr, &spdc, spcc);
				chrGoPosInitMagic(chr, waydata, &spdc, &prop->pos);
			} else if (chr->actiontype == ACT_GOPOS) {
				if (chr->act_gopos.waypoints[chr->act_gopos.curindex] == NULL) {
					// Reached the end of the route
					if (chr->act_gopos.flags & GOPOSFLAG_FORPATHSTART) {
						chrTryStartPatrol(chr);
					} else {
						if (chr->act_gopos.curindex >= 2) {
							waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex - 2];
							padUnpack(waypoint->padnum, PADFIELD_POS, &pad);
							chrSetLookAngle(chr, atan2f(prop->pos.x - pad.pos.x, prop->pos.z - pad.pos.z));
						}

						if (CHRRACE(chr) == RACE_HUMAN || CHRRACE(chr) == RACE_SKEDAR) {
							chrStop(chr);
						}
					}
				} else {
					// Advance to next segment, still using magic
					chrGoPosAdvanceWaypoint(chr);
					chrGoPosGetCurWaypointInfo(chr, &sp5c, sp4c);
					chrGoPosInitMagic(chr, waydata, &sp5c, &prop->pos);
				}
			}
		} else {
			// Collision
			waydata->magicdone = waydata->magictotal;

			if (chr->actiontype == ACT_PATROL) {
				chr->act_patrol.waydata.lastvisible60 = g_Vars.lvframe60;
				func0f037580(chr);
			} else {
				chr->act_gopos.waydata.lastvisible60 = g_Vars.lvframe60;
				chrGoPosInitExpensive(chr);
			}
		}

		chrSetPerimEnabled(chr, true);
	}
}

/**
 * Calculate the chr's position when using the magic method of navigating.
 *
 * The magic method is used when the chr is off-screen. It measures the distance
 * between two consecutive pads in the route and simply increments a distance
 * value along that segment on each tick, ignoring collisions. Meanwhile, the
 * chr's prop->pos is left as the original location where this segment started.
 *
 * The calculation is finding the fraction of the distance travelled in this
 * segment, then finding the position between the starting pos and the pad's
 * pos.
 *
 * The pos variable is used for both loading the next pad's position and for
 * returning the new position, which means there's less stack usage.
 */
void chrCalculatePosition(struct chrdata *chr, struct coord *pos)
{
	RoomNum rooms[8];
	float frac;

	if (chr->actiontype == ACT_PATROL && chr->act_patrol.waydata.mode == WAYMODE_MAGIC) {
		chrPatrolGetCurWaypointInfo(chr, pos, rooms);

		if (!(chr->act_patrol.waydata.magicdone >= chr->act_patrol.waydata.magictotal)
				&& chr->act_patrol.waydata.magictotal > 0) {
			frac = chr->act_patrol.waydata.magicdone / chr->act_patrol.waydata.magictotal;
			pos->x = (pos->x - chr->prop->pos.x) * frac + chr->prop->pos.x;
			pos->y = (pos->y - chr->prop->pos.y) * frac + chr->prop->pos.y;
			pos->z = (pos->z - chr->prop->pos.z) * frac + chr->prop->pos.z;
		}
	} else if (chr->actiontype == ACT_GOPOS && chr->act_gopos.waydata.mode == WAYMODE_MAGIC) {
		chrGoPosGetCurWaypointInfo(chr, pos, rooms);

		if (!(chr->act_gopos.waydata.magicdone >= chr->act_gopos.waydata.magictotal)
				&& chr->act_gopos.waydata.magictotal > 0) {
			frac = chr->act_gopos.waydata.magicdone / chr->act_gopos.waydata.magictotal;
			pos->x = (pos->x - chr->prop->pos.x) * frac + chr->prop->pos.x;
			pos->y = (pos->y - chr->prop->pos.y) * frac + chr->prop->pos.y;
			pos->z = (pos->z - chr->prop->pos.z) * frac + chr->prop->pos.z;
		}
	} else {
		pos->x = chr->prop->pos.x;
		pos->y = chr->prop->pos.y;
		pos->z = chr->prop->pos.z;
	}
}

void chrGoPosChooseAnimation(struct chrdata *chr)
{
	int gospeed = chr->act_gopos.flags & GOPOSMASK_SPEED;
	int male = g_HeadsAndBodies[chr->bodynum].ismale;
	struct prop *leftgun = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightgun = chrGetHeldProp(chr, HAND_RIGHT);
	int flip = false;
	int heavy;
	int race = CHRRACE(chr);
	int anim = -1;
	float speed = 0.5;
	float sp60 = 16;
	float animspeed = -1;
	float startframe = 16;

	if (chr->actiontype == ACT_GOPOS) {
		chr->act_gopos.flags &= ~GOPOSFLAG_WAITING;
	}

	if (race == RACE_EYESPY || chr->aibot) {
		return;
	}

	if (race == RACE_HUMAN || race == RACE_SKEDAR) {
		if ((leftgun && rightgun) || (!leftgun && !rightgun)) {
			heavy = false;
			flip = rngRandom() % 2;
		} else {
			if (weaponIsOneHanded(leftgun) || weaponIsOneHanded(rightgun)) {
				heavy = false;
				flip = (bool)leftgun != false;
			} else {
				heavy = true;
				flip = (bool)leftgun != false;
			}
		}

		if (race == RACE_SKEDAR) {
			if (gospeed == GOPOSFLAG_RUN) {
				anim = ANIM_SKEDAR_RUNNING;
			} else if (gospeed == GOPOSFLAG_JOG) {
				anim = ANIM_0393;
			} else if (gospeed == GOPOSFLAG_WALK) {
				anim = ANIM_0392;
			}
		} else {
			if (heavy) {
				if (gospeed == GOPOSFLAG_RUN) {
					// Human, heavy weapon, running
					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH
							|| chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_020A;
						speed = 0.4f;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP
							|| chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_020D;
						speed = 0.4f;
					} else {
						anim = ANIM_SPRINT_RIFLE;
						speed = 0.5f;
					}

					if (chr->chrflags & CHRCFLAG_RUNFASTER) {
						animspeed = 0.65f;
						startframe = 48;
					} else {
						animspeed = 0.5f;
						startframe = 48;
					}
				} else if (gospeed == GOPOSFLAG_JOG) {
					// Human, heavy weapon, jogging
					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH
							|| chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_01F9;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP
							|| chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_01F8;
					} else {
						anim = ANIM_RUNNING_TWOHANDGUN;
					}
				} else {
					// Human, heavy weapon, walking
					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH
							|| chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_01F9;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP
							|| chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_01F8;
					} else {
						if (rngRandom() % 2) {
							anim = ANIM_RIFLE_WALK;
						} else {
							anim = ANIM_RIFLE_PATROL;
						}
					}
				}
			} else {
				if (gospeed == GOPOSFLAG_RUN) {
					// Human, light weapon, running
					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH) {
						anim = ANIM_020A;
						flip = true;
						speed = 0.4f;
					} else if (chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_020A;
						speed = 0.4f;
						flip = false;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP) {
						anim = ANIM_020D;
						speed = 0.4f;
						flip = true;
					} else if (chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_020D;
						speed = 0.4;
						flip = false;
					} else if (male) {
						if (rngRandom() % 4 == 0) {
							speed = 0.5f;
							anim = ANIM_RUN_FWD_001E;
						} else {
							speed = 0.5f;
							anim = ANIM_RUN_WITH_PISTOL_005A;
							sp60 = 24;
						}
					} else {
						if (rngRandom() % 2) {
							anim = ANIM_FEMALE_RUN;
						} else {
							anim = ANIM_RUN_WITH_PISTOL_005A;
						}

						speed = 0.5f;
					}

					if (chr->chrflags & CHRCFLAG_RUNFASTER) {
						animspeed = 0.65;
						startframe = 48;
					} else {
						animspeed = 0.5;
						startframe = 48;
					}
				} else if (gospeed == GOPOSFLAG_JOG) {
					// Human, light weapon, jogging
					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH) {
						anim = ANIM_01F9;
						flip = false;
					} else if (chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_01F9;
						flip = true;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP) {
						anim = ANIM_01F8;
						flip = false;
					} else if (chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_01F8;
						flip = true;
					} else if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_CHICAGO) {
						anim = ANIM_JOG_COVER_FACE;
					} else if (male) {
						if (rngRandom() % 2) {
							anim = ANIM_RUN_FWD_001D;
							animspeed = 1.0f;
						} else {
							anim = ANIM_RUNNING_ONEHANDGUN;
							animspeed = 1.0f;
						}
					} else {
						if (chr->myaction != MA_PANIC) {
							if (rngRandom() % 2) {
								anim = ANIM_FEMALE_JOG;
							} else {
								anim = ANIM_JOG_0073;
								animspeed = 1.0f;
							}
						} else {
							anim = ANIM_021D;
						}
					}
				} else {
					// Human, light weapon, walking
					int anims[] = {ANIM_WALK_FWD_006B, ANIM_WALK_FWD_001B, ANIM_WALK_FORWARD01};

					if (chr->hitpart == HITPART_LFOOT
							|| chr->hitpart == HITPART_LSHIN
							|| chr->hitpart == HITPART_LTHIGH) {
						anim = ANIM_01F9;
						flip = false;
					} else if (chr->hitpart == HITPART_RFOOT
							|| chr->hitpart == HITPART_RSHIN
							|| chr->hitpart == HITPART_RTHIGH) {
						anim = ANIM_01F9;
						flip = true;
					} else if (chr->hitpart == HITPART_LHAND
							|| chr->hitpart == HITPART_LFOREARM
							|| chr->hitpart == HITPART_LBICEP) {
						anim = ANIM_01F8;
						flip = false;
					} else if (chr->hitpart == HITPART_RHAND
							|| chr->hitpart == HITPART_RFOREARM
							|| chr->hitpart == HITPART_RBICEP) {
						anim = ANIM_01F8;
						flip = true;
					} else if (male) {
						anim = anims[rngRandom() % 3];
					} else {
						if (rngRandom() % 2) {
							anim = ANIM_FEMALE_WALK;
						} else {
							anim = ANIM_WALK_FWD_0072;
						}
					}
				}
			}
		}
	} else if (race == RACE_DRCAROLL) {
		if (gospeed == GOPOSFLAG_RUN) {
			anim = ANIM_0160;
		} else if (gospeed == GOPOSFLAG_WALK) {
			anim = ANIM_015F;
		} else {
			anim = ANIM_015F;
		}
	} else if (race == RACE_ROBOT) {
		anim = ANIM_0238;
	}

	if (anim >= 0) {
		modelSetAnimation(chr->model, anim, flip, 0, speed, sp60);

		if (animspeed > 0) {
			modelSetAnimSpeed(chr->model, animspeed, startframe);
		}
	}
}

bool chrGoToRoomPos(struct chrdata *chr, struct coord *pos, RoomNum *room, uint32_t goposflags)
{
	struct prop *prop = chr->prop;
	struct waypoint *nextwaypoint;
	struct waypoint *lastwaypoint;
	struct waypoint *waypoints[MAX_CHRWAYPOINTS];
	int i;
	struct coord curwppos;
	RoomNum curwprooms[8];
	int isgopos = chr->actiontype == ACT_GOPOS
		&& (chr->act_gopos.flags & GOPOSMASK_SPEED) == (goposflags & 0xff & GOPOSMASK_SPEED)
		&& !chrGoPosIsWaiting(chr);
	int ismagic = isgopos && chr->act_gopos.waydata.mode == WAYMODE_MAGIC;
	struct coord prevpos;
	int numwaypoints = 0;

	for (i = 0; chr->prop->rooms[i] != -1; i++) {
		chr->oldrooms[i] = chr->prop->rooms[i];
	}

	chr->oldrooms[i] = -1;

	if (isgopos && ismagic && chr->act_gopos.waypoints[chr->act_gopos.curindex]) {
		nextwaypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];
	} else {
		nextwaypoint = waypointFindClosestToPos(&prop->pos, prop->rooms);
	}

	lastwaypoint = waypointFindClosestToPos(pos, room);

	if (nextwaypoint && lastwaypoint) {
		navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
		numwaypoints = navFindRoute(nextwaypoint, lastwaypoint, waypoints, MAX_CHRWAYPOINTS);
		navSetSeed(0, 0);
	}

	if (numwaypoints > 1) {
		if (isgopos && ismagic) {
			chrCalculatePosition(chr, &prevpos);
		} else {
			prevpos.x = prop->pos.x;
			prevpos.y = prop->pos.y;
			prevpos.z = prop->pos.z;
		}

		chrStopFiring(chr);

		chr->actiontype = ACT_GOPOS;
		chr->act_gopos.endpos.x = pos->x;
		chr->act_gopos.endpos.y = pos->y;
		chr->act_gopos.endpos.z = pos->z;
		roomsCopy(room, chr->act_gopos.endrooms);

		chr->act_gopos.target = lastwaypoint;
		chr->act_gopos.curindex = 0;
		chr->act_gopos.flags = goposflags | GOPOSFLAG_INIT;
		chr->act_gopos.turnspeed = 0;
		chr->unk32c_21 = false;
		chr->act_gopos.waydata.age = rngRandom() % 100;
		chr->act_gopos.waydata.gotaimposobj = 0;

		if (!isgopos) {
			chr->act_gopos.waydata.lastvisible60 = -1;
		}

		for (i = 0; i < MAX_CHRWAYPOINTS; i++) {
			chr->act_gopos.waypoints[i] = waypoints[i];
		}

		chrGoPosInitExpensive(chr);
		chr->goposforce = -1;
		chr->sleep = 0;
		chr->liftaction = 0;
		chr->act_gopos.flags &= ~(GOPOSFLAG_DUCK | GOPOSFLAG_CROUCH | GOPOSFLAG_WAITING);
		chrGoPosGetCurWaypointInfo(chr, &curwppos, curwprooms);

		if ((!isgopos || ismagic)
				&& g_Vars.normmplayerisrunning == false
				&& (prop->flags & (PROPFLAG_ONANYSCREENPREVTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONTHISSCREENTHISTICK)) == 0
				&& chrIsRoomOffScreen(chr, &curwppos, curwprooms)
				&& chr->inlift == false) {
			chrGoPosInitMagic(chr, &chr->act_gopos.waydata, &curwppos, &prevpos);
		}

		if (chr->act_gopos.waydata.mode != WAYMODE_MAGIC
				&& modelIsAnimMerging(chr->model) && !chr->aibot) {
			chr->hidden |= CHRHFLAG_NEEDANIM;
			return true;
		} else {
			if (!isgopos) {
				chrGoPosChooseAnimation(chr);
			}

			chr->hidden &= ~CHRHFLAG_NEEDANIM;
			return true;
		}
	}

	return false;
}

struct path *pathFindById(uint32_t path_id)
{
	int i = 0;

	for (i = 0; g_StageSetup.paths[i].pads; i++) {
		if (path_id == g_StageSetup.paths[i].id) {
			return &g_StageSetup.paths[i];
		}
	}

	return NULL;
}

void chrPatrolChooseAnimation(struct chrdata *chr)
{
	struct prop *leftprop = chrGetHeldProp(chr, HAND_LEFT);
	struct prop *rightprop = chrGetHeldProp(chr, HAND_RIGHT);
	int flip;
	bool heavy;
	int race = CHRRACE(chr);
	int ismale = g_HeadsAndBodies[chr->bodynum].ismale;
	float speed;

	if (race == RACE_EYESPY) {
		// empty
	} else if (race == RACE_HUMAN || race == RACE_SKEDAR) {
		if ((leftprop && rightprop) || (!leftprop && !rightprop)) {
			// No weapon, or double weapons
			heavy = false;
			flip = rngRandom() % 2;
		} else {
			// Single weapon
			if (weaponIsOneHanded(leftprop) || weaponIsOneHanded(rightprop)) {
				heavy = false;
				flip = ((bool)leftprop != false);
			} else {
				heavy = true;
				flip = ((bool)leftprop != false);
			}
		}

		if (race == RACE_SKEDAR) {
			modelSetAnimation(chr->model, ANIM_0392, flip, 0, 0.25f, 16);
		} else {
			speed = 0.5f * func0f02dff0(ANIM_RIFLE_PATROL) / func0f02dff0(ANIM_WALK_FWD_006B);

			if (heavy) {
				modelSetAnimation(chr->model, rngRandom() % 2 ? ANIM_RIFLE_WALK : ANIM_RIFLE_PATROL, flip, 0, speed, 16);
			} else if (ismale) {
				int anims[] = { ANIM_WALK_FWD_006B, ANIM_WALK_FWD_001B, ANIM_WALK_FORWARD01 };
				modelSetAnimation(chr->model, anims[rngRandom() % 3], flip, 0, speed, 16);
			} else {
				modelSetAnimation(chr->model, rngRandom() % 2 ? ANIM_FEMALE_WALK : ANIM_WALK_FWD_0072, flip, 0, speed, 16);
			}
		}
	} else if (race == RACE_DRCAROLL) {
		modelSetAnimation(chr->model, ANIM_015F, false, 0, 0.5f, 16);
	} else if (race == RACE_ROBOT) {
		modelSetAnimation(chr->model, ANIM_0238, false, 0, 0.5f, 16);
	}
}

void chrStartPatrol(struct chrdata *chr, struct path *path)
{
	int i;
	float dist;
	float xdiff;
	float zdiff;
	int *padnumptr;
	struct pad pad;
	struct coord nextpos;
	RoomNum nextrooms[8];
	RoomNum rooms[8];
	float ymax;
	float ymin;
	float radius;
	float bestdistance = 0;
	int nextstep = -1;
	struct prop *prop = chr->prop;
	RoomNum sp60[2];

	if (CHRRACE(chr) != RACE_EYESPY) {
		// Do some kind of collision test with the pad to resume from...
		// maybe a line of sight check?
		if (chr->patrolnextstep >= 0 && chr->patrolnextstep < path->len) {
			padnumptr = &path->pads[chr->patrolnextstep];
			padUnpack(*padnumptr, PADFIELD_POS | PADFIELD_ROOM, &pad);

			rooms[0] = pad.room;
			rooms[1] = -1;

			chrGetBbox(prop, &radius, &ymax, &ymin);

			chrSetPerimEnabled(chr, false);

			if (cdTestCylMove04(&prop->pos, prop->rooms, &pad.pos, rooms, CDTYPE_BG, 1,
						ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION) {
				nextstep = chr->patrolnextstep;
			}

			chrSetPerimEnabled(chr, true);
		}

		// If the pad to resume from is not in sight, find the closest pad
		// to the chr's current position and start from there.
		if (nextstep < 0) {
			for (i = 0; path->pads[i] >= 0; i++) {
				padnumptr = &path->pads[i];
				padUnpack(*padnumptr, PADFIELD_POS, &pad);

				xdiff = pad.pos.x - prop->pos.x;
				zdiff = pad.pos.z - prop->pos.z;
				dist = xdiff * xdiff + zdiff * zdiff;

				if (nextstep < 0 || dist < bestdistance) {
					bestdistance = dist;
					nextstep = i;
				}
			}
		}

		padnumptr = &path->pads[nextstep];
		padUnpack(*padnumptr, PADFIELD_POS | PADFIELD_ROOM, &pad);

		rooms[0] = pad.room;
		rooms[1] = -1;

		// If chr has line of sight to the pad then begin the patrol,
		// otherwise use gopos to get to the starting pad
		if (func0f03654c(chr, &prop->pos, prop->rooms, &pad.pos, rooms, NULL,
					chr->radius * 1.2f, CDTYPE_PATHBLOCKER | CDTYPE_BG) != CDRESULT_COLLISION) {
			chrStopFiring(chr);

			chr->actiontype = ACT_PATROL;
			chr->act_patrol.path = path;
			chr->act_patrol.nextstep = nextstep;
			chr->act_patrol.forward = true;

			chr->act_patrol.waydata.age = rngRandom() % 100;
			chr->act_patrol.waydata.gotaimposobj = 0;
			chr->act_patrol.waydata.lastvisible60 = -1;

			chr->act_patrol.turnspeed = 0;

			func0f037580(chr);

			chr->sleep = 0;
			chr->liftaction = LIFTACTION_NOTUSINGLIFT;
			chr->patrolnextstep = chr->act_patrol.nextstep;

			chrPatrolGetCurWaypointInfo(chr, &nextpos, nextrooms);

			if (!g_Vars.normmplayerisrunning
					&& (chr->prop->flags & (PROPFLAG_ONTHISSCREENTHISTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONANYSCREENPREVTICK)) == 0
					&& chrIsRoomOffScreen(chr, &nextpos, nextrooms)
					&& !chr->inlift) {
				chrGoPosInitMagic(chr, &chr->act_patrol.waydata, &nextpos, &prop->pos);
			}

			// @bug: This should be act_patrol rather than act_gopos.
			// It's actually reading act_patrol.waydata.aimposobj.y which is a
			// float, so it's taking its binary representation and comparing it
			// with integer 6.
			if (chr->act_gopos.waydata.mode != WAYMODE_MAGIC && modelIsAnimMerging(chr->model)) {
				chr->hidden |= CHRHFLAG_NEEDANIM;
			} else {
				chrPatrolChooseAnimation(chr);
				chr->hidden &= ~CHRHFLAG_NEEDANIM;
			}
		} else {
			sp60[0] = pad.room;
			sp60[1] = -1;

			chrGoToRoomPos(chr, &pad.pos, sp60, GOPOSFLAG_FORPATHSTART);
		}
	}
}

void chrRecordLastVisibleTargetTime(struct chrdata *chr)
{
	chr->lastvisibletarget60 = g_Vars.lvframe60;
}

bool chrHasLosToEntity(struct chrdata *chr, struct coord *chrpos, RoomNum *chrrooms, bool allowextraheight, uint32_t attackflags, uint32_t entityid)
{
	bool result = false;
	struct coord targetpos;
	RoomNum targetrooms[8];
	struct prop *targetprop;
	struct chrdata *targetchr;
	uint32_t types;
	struct prop *weaponprop;

	if (attackflags & ATTACKFLAG_AIMFORWARD) {
		result = true;
	} else {
		types = CDTYPE_DOORSWITHOUTFLAG | CDTYPE_ALL;
		weaponprop = chrGetHeldProp(chr, HAND_RIGHT);

		if (weaponprop == NULL) {
			weaponprop = chrGetHeldProp(chr, HAND_LEFT);
		}

		if (weaponprop) {
			struct weaponobj *weapon = weaponprop->weapon;

			if (weapon->weaponnum == WEAPON_ROCKETLAUNCHER
					|| weapon->weaponnum == WEAPON_SLAYER
					|| weapon->weaponnum == WEAPON_ROCKETLAUNCHER_34) {
				types = CDTYPE_DOORSWITHOUTFLAG | CDTYPE_OBJSIMMUNETOEXPLOSIONS | CDTYPE_ALL;
			} else {
				types = CDTYPE_DOORSWITHOUTFLAG | CDTYPE_OBJSIMMUNETOGUNFIRE | CDTYPE_ALL;
			}
		}

		chrGetAttackEntityPos(chr, attackflags, entityid, &targetpos, targetrooms);
		chrSetPerimEnabled(chr, false);

		if ((attackflags & ATTACKFLAG_AIMATTARGET)) {
			targetprop = chrGetTargetProp(chr);

			if (targetprop->type != PROPTYPE_PLAYER || g_Vars.bondvisible) {
				propSetPerimEnabled(targetprop, false);

				if (allowextraheight && (chr->chrflags & CHRCFLAG_LOSEXTRAHEIGHT)) {
					struct coord frompos;
					RoomNum fromrooms[8];

					frompos.x = chrpos->x;
					frompos.y = chrpos->y + 70;
					frompos.z = chrpos->z;

					propFindRoomsContainingNewPosSimple(chrpos, chrrooms, &frompos, fromrooms);

					if (cdTestLos05(&frompos, fromrooms, &targetpos, targetrooms, types, GEOFLAG_BLOCK_SHOOT)) {
						chrRecordLastVisibleTargetTime(chr);
						result = true;
					}
				} else {
					if (cdTestLos05(chrpos, chrrooms, &targetpos, targetrooms, types, GEOFLAG_BLOCK_SHOOT)) {
						chrRecordLastVisibleTargetTime(chr);
						result = true;
					}
				}

				propSetPerimEnabled(targetprop, true);
			}
		} else if (attackflags & ATTACKFLAG_AIMATCHR) {
			targetchr = chrFindById(chr, entityid);

			if (!targetchr || !targetchr->prop) {
				targetchr = chr;
			}

			chrSetPerimEnabled(targetchr, false);

			if (cdTestLos05(chrpos, chrrooms, &targetpos, targetrooms, types, GEOFLAG_BLOCK_SHOOT)) {
				result = true;
			}

			chrSetPerimEnabled(targetchr, true);
		} else if (attackflags & ATTACKFLAG_AIMATPAD) {
			if (cdTestLos05(chrpos, chrrooms, &targetpos, targetrooms, types, GEOFLAG_BLOCK_SHOOT)) {
				result = true;
			}
		}

		chrSetPerimEnabled(chr, true);
	}

	return result;
}

bool chrHasLosToAttackTarget(struct chrdata *chr, struct coord *pos, RoomNum *rooms, bool allowextraheight)
{
	uint32_t attackflags = ATTACKFLAG_AIMATTARGET;
	uint32_t entityid = 0;

	if (chr->actiontype == ACT_ATTACK) {
		attackflags = chr->act_attack.flags;
		entityid = chr->act_attack.entityid;
	}

	return chrHasLosToEntity(chr, pos, rooms, allowextraheight, attackflags, entityid);
}

bool chrHasLosToChr(struct chrdata *chr, struct chrdata *target, RoomNum *room)
{
	bool cansee = false;
	RoomNum sp88[] = {-1, 0, 0, 0, 0, 0, 0, 0};

	if (!botIsTargetInvisible(chr, target)) {
		struct prop *prop = chr->prop;
		struct coord pos;
		RoomNum rooms[8];

		pos.x = prop->pos.x;
		pos.y = chr->ground + chr->height - 20;
		pos.z = prop->pos.z;

		chrSetPerimEnabled(chr, false);
		chrSetPerimEnabled(target, false);

		propUpdatePositionRoomsSimple(&prop->pos, prop->rooms, &pos, rooms);

		if (cdTestLos07(&pos, rooms, &target->prop->pos, target->prop->rooms, sp88,
					CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG | CDTYPE_AIOPAQUE,
					GEOFLAG_BLOCK_SIGHT)) {
			cansee = true;
		}

		chrSetPerimEnabled(chr, true);
		chrSetPerimEnabled(target, true);
	}

	if (room) {
		*room = sp88[0];
	}

	return cansee;
}

bool chrHasLosToTarget(struct chrdata *chr)
{
	bool cansee;
	struct prop *prop;

	cansee = false;
	prop = chrGetTargetProp(chr);

	if (prop && prop->chr) {
		cansee = chrHasLosToChr(chr, prop->chr, NULL);

		if (cansee) {
			chrRecordLastVisibleTargetTime(chr);
		}
	}

	return cansee;
}

bool chrHasLosToPos(struct chrdata *chr, struct coord *pos, RoomNum *rooms)
{
	struct prop *prop = chr->prop;
	bool result = false;
	struct coord eyepos;
	RoomNum chrrooms[8];

	eyepos.x = prop->pos.x;
	eyepos.y = chr->ground + chr->height - 20;
	eyepos.z = prop->pos.z;

	chrSetPerimEnabled(chr, false);
	propUpdatePositionRoomsSimple(&prop->pos, prop->rooms, &eyepos, chrrooms);

	if (cdTestLos05(&eyepos, chrrooms, pos, rooms,
				CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG | CDTYPE_AIOPAQUE,
				GEOFLAG_BLOCK_SIGHT)) {
		result = true;
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

/**
 * Wasteful because this function calculates angles then does nothing with them.
 * chrHasLosToPos is called with the same arguments regardless.
 */
bool chrHasLosToPosWasteful(struct chrdata *chr, struct coord *pos, RoomNum *rooms)
{
	float facingangle = chrGetInverseTheta(chr);
	float posangle = atan2f(pos->x - chr->prop->pos.x, pos->z - chr->prop->pos.z);
	float diffangle = posangle - facingangle;

	if (posangle < facingangle) {
		diffangle += M_TAU;
	}

	if ((diffangle < 1.7450513839722f || diffangle > 4.5371336936951f)
			&& chrHasFlag(chr, CHRFLAG1_NOOP_00200000, BANK_1) == false) {
		return chrHasLosToPos(chr, pos, rooms);
	}

	return chrHasLosToPos(chr, pos, rooms);
}

bool chrHasLosToProp(struct chrdata *chr, struct prop *prop)
{
	bool result;

	propSetPerimEnabled(prop, false);
	result = chrHasLosToPosWasteful(chr, &prop->pos, prop->rooms);
	propSetPerimEnabled(prop, true);

	return result;
}

void chrRecordLastSeeTargetTime(struct chrdata *chr)
{
	chr->lastseetarget60 = g_Vars.lvframe60;
}

void chrRecordLastHearTargetTime(struct chrdata *chr)
{
	chr->hidden |= CHRHFLAG_IS_HEARING_TARGET;
	chr->lastheartarget60 = g_Vars.lvframe60;
}

bool chrIsStopped(struct chrdata *chr)
{
	int16_t anim = modelGetAnimNum(chr->model);

	if (anim == ANIM_SNIPING_GETDOWN || anim == ANIM_SNIPING_GETUP) {
		return false;
	}

	if (anim == ANIM_SNIPING_ONGROUND && chr->act_attack.numshots >= chr->act_attack.maxshots) {
		chrStopFiring(chr);
		return true;
	}

	if (chr->actiontype == ACT_ROBOTATTACK && chr->act_robotattack.finished) {
		return true;
	}

	if (chr->actiontype == ACT_ATTACKAMOUNT && chr->act_attack.numshots >= chr->act_attack.maxshots) {
		return true;
	}

	if (chr->actiontype == ACT_STAND
			&& !chr->act_stand.prestand
			&& !chr->act_stand.reaim
			&& chr->act_stand.turning != 1) {
		return true;
	}

	if (chr->actiontype == ACT_ANIM) {
		if (chr->act_anim.completed
				|| (modelGetAnimSpeed(chr->model) >= 0 && modelGetCurAnimFrame(chr->model) >= modelGetAnimEndFrame(chr->model))
				|| (modelGetAnimSpeed(chr->model) < 0 && modelGetCurAnimFrame(chr->model) <= 0)) {
			return true;
		}
	} else if (chr->actiontype == ACT_PATROL) {
		return true;
	}

	return false;
}

bool chrCheckCanSeeTarget(struct chrdata *chr)
{
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);
	float sqdistance;
	float invtheta = chrGetInverseTheta(chr);

	float x = target->pos.x - prop->pos.x;
	float y = target->pos.y - prop->pos.y;
	float z = target->pos.z - prop->pos.z;

	float angletotarget = atan2f(x, z);
	float angle = angletotarget - invtheta;
	bool result = false;

	if (angletotarget < invtheta) {
		angle += M_TAU;
	}

	sqdistance = x * x + y * y + z * z;

	if ((sqdistance < chr->visionrange * chr->visionrange * 10000.0f && (angle < 1.9195564985275f || angle > 4.3626284599304f))
			|| (sqdistance < 40000.0f && (angle < 1.9195564985275f || angle > 4.3626284599304f))) {
		result = false;

		if (sqdistance < envGetSquaredFogMax()) {
			float tmp;
			int iVar8 = (sqrtf(sqdistance) * 0.0018749999580905f);
			int tmp2;

			if (angle > 0.7852731347084f && angle < 5.4969120025635f) {
				tmp = angle;

				if (tmp > M_PI) {
					tmp = M_TAU - angle;
				}

				tmp -= 0.7852731347084f;
				tmp2 = tmp * 3.8203268051147f;
				iVar8 *= 1 + tmp2;
			}

			iVar8 = chrGetPercentageOfSlowness(chr, iVar8) + 1;
			result = rngRandom() % iVar8 == 0;
		}
	}

	if (result) {
		result = chrHasLosToTarget(chr);
	}

	if (result) {
		chrRecordLastSeeTargetTime(chr);
	}

	return result;
}

bool chrIsReadyForOrders(struct chrdata *chr)
{
	if (chr->onladder) {
		return false;
	}

	switch ((int) chr->actiontype) {
	case ACT_DIE:
	case ACT_DEAD:
	case ACT_PREARGH:
	case ACT_DRUGGEDDROP:
	case ACT_DRUGGEDKO:
	case ACT_DRUGGEDCOMINGUP:
		return false;
	case ACT_ARGH:
		if ((chr->chrflags & CHRCFLAG_CANCHANGEACTDURINGARGH) == 0) {
			return false;
		}
		break;
	case ACT_ROBOTATTACK:
		if (!chr->act_robotattack.finished) {
			return false;
		}
		break;
	}

	return true;
}

bool chrIsDead(struct chrdata *chr)
{
	if (!chr || chr->actiontype == ACT_DIE || chr->actiontype == ACT_DEAD) {
		return true;
	}

	if (chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		uint32_t playernum = playermgrGetPlayerNumByProp(chr->prop);

		if (g_Vars.players[playernum]->isdead) {
			return true;
		}
	}

	return false;
}

bool chrTrySidestep(struct chrdata *chr)
{
	uint8_t race = CHRRACE(chr);

	if ((race == RACE_HUMAN || race == RACE_SKEDAR)
			&& chrIsReadyForOrders(chr)) {
		struct prop *prop = chr->prop;
		struct prop *target = chrGetTargetProp(chr);
		float a = chrGetInverseTheta(chr);
		float b = atan2f(target->pos.x - prop->pos.x, target->pos.z - prop->pos.z);
		float angle = b - a;

		if (b < a) {
			angle += M_TAU;
		}

		if (angle < 0.7852731347084f || angle > 5.4969120025635f
				|| (angle > 2.3558194637299f && angle < 3.9263656139374f)) {
			bool side = (rngRandom() % 2) == 0;

			if (chrCanJumpInDirection(chr, side, 100)) {
				chrSidestep(chr, side);
				return true;
			}

			if (chrCanJumpInDirection(chr, !side, 100)) {
				chrSidestep(chr, !side);
				return true;
			}
		}
	}

	return false;
}

bool chrTryJumpOut(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		struct prop *prop = chr->prop;
		struct prop *target = chrGetTargetProp(chr);

		float a = chrGetInverseTheta(chr);
		float b = atan2f(target->pos.x - prop->pos.x, target->pos.z - prop->pos.z);
		float angle = b - a;

		if (b < a) {
			angle += M_TAU;
		}
		
		if (angle < DEG2RAD(45) || angle > DEG2RAD(315)
				|| (angle > DEG2RAD(135) && angle < DEG2RAD(225))) {
			bool side = (rngRandom() % 2) == 0;

			if (chrCanJumpInDirection(chr, side, 200)) {
				chrJumpOut(chr, side);
				return true;
			}

			if (chrCanJumpInDirection(chr, !side, 200)) {
				chrJumpOut(chr, !side);
				return true;
			}
		}
	}

	return false;
}

bool chrTryRunSideways(struct chrdata *chr)
{
	uint32_t race = CHRRACE(chr);

	if ((race == RACE_HUMAN || race == RACE_SKEDAR)
			&& chrIsReadyForOrders(chr)
			&& g_Vars.lvframe60 - chr->lastwalk60 > TICKS(180)) {
		struct prop *prop = chr->prop;
		float distance = 200.0f + RANDOMFRAC() * 200.0f;
		struct coord vector;
		struct coord dstpos;

		chrGetSideVectorToTarget(chr, rngRandom() % 2 == 0, &vector);

		dstpos.x = vector.x * distance + prop->pos.x;
		dstpos.y = prop->pos.y;
		dstpos.z = vector.z * distance + prop->pos.z;

		if (propchrHasClearLineToPos(prop, &dstpos, &vector)) {
			chrRunToPos(chr, &dstpos);
			return true;
		}

		vector.x = -vector.x;
		vector.z = -vector.z;

		dstpos.x = vector.x * distance + prop->pos.x;
		dstpos.y = prop->pos.y;
		dstpos.z = vector.z * distance + prop->pos.z;

		if (propchrHasClearLineToPos(prop, &dstpos, &vector)) {
			chrRunToPos(chr, &dstpos);
			return true;
		}
	}

	return false;
}

bool chrTryAttackWalk(struct chrdata *chr)
{
	uint32_t race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_EYESPY) {
		return false;
	}

	if (chrIsReadyForOrders(chr)) {
		struct prop *prop = chr->prop;

		if (chrHasLosToAttackTarget(chr, &prop->pos, prop->rooms, false)
				&& (chrGetHeldUsableProp(chr, 0) || chrGetHeldUsableProp(chr, 1))
				&& g_Vars.lvframe60 - chr->lastwalk60 > TICKS(120)) {
			struct prop *target = chrGetTargetProp(chr);
			float x = target->pos.x - prop->pos.x;
			float y = target->pos.y - prop->pos.y;
			float z = target->pos.z - prop->pos.z;

			if (race == RACE_HUMAN && x * x + y * y + z * z >= 1000 * 1000) {
				chrAttackWalk(chr, false);
				return true;
			}
		}
	}

	return false;
}

bool chrTryAttackRun(struct chrdata *chr)
{
	uint32_t race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_EYESPY) {
		return false;
	}

	if (chrIsReadyForOrders(chr)) {
		struct prop *prop = chr->prop;

		if (chrHasLosToAttackTarget(chr, &prop->pos, prop->rooms, false)
				&& (chrGetHeldUsableProp(chr, 0) || chrGetHeldUsableProp(chr, 1))
				&& g_Vars.lvframe60 - chr->lastwalk60 > TICKS(180)) {
			struct prop *target = chrGetTargetProp(chr);
			float x = target->pos.x - prop->pos.x;
			float y = target->pos.y - prop->pos.y;
			float z = target->pos.z - prop->pos.z;

			if (race == RACE_HUMAN && x * x + y * y + z * z >= 1000 * 1000) {
				chrAttackWalk(chr, true);
				return true;
			}
		}
	}

	return false;
}

bool chrTryAttackRoll(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		struct prop *prop = chr->prop;

		if (chrHasLosToAttackTarget(chr, &prop->pos, prop->rooms, false) &&
				(chrGetHeldUsableProp(chr, 0) || chrGetHeldUsableProp(chr, 1))) {
			struct prop *target = chrGetTargetProp(chr);
			float x = target->pos.x - prop->pos.x;
			float y = target->pos.y - prop->pos.y;
			float z = target->pos.z - prop->pos.z;
			float sqdistance = x * x + y * y + z * z;

			if (sqdistance >= 200 * 200) {
				bool toleft = (rngRandom() % 2) == 0;

				if (chrCanRollInDirection(chr, toleft, 200)) {
					chrAttackRoll(chr, toleft);
					return true;
				}

				if (chrCanRollInDirection(chr, !toleft, 200)) {
					chrAttackRoll(chr, !toleft);
					return true;
				}
			}
		}
	}

	return false;
}

bool chrTryAttackAmount(struct chrdata *chr, uint32_t arg1, uint32_t arg2, uint8_t lower, uint8_t upper)
{
	uint8_t race = CHRRACE(chr);

	if ((race == RACE_HUMAN || race == RACE_SKEDAR)
			&& chrIsReadyForOrders(chr)
			&& chr->weapons_held[0]) {
		int quantity;
		float percentage;
		struct weaponobj *weapon = chr->weapons_held[0]->weapon;
		struct inventory_ammo *ammo;

		quantity = 1;

		if (upper > 100) {
			upper = 100;
		}

		if (lower < upper) {
			percentage = ((rngRandom() % (upper - lower)) + (uint32_t)lower) * 0.01f;
		} else {
			percentage = 0;
		}

		ammo = weaponGetAmmoByFunction(weapon->weaponnum, 0);

		if (ammo) {
			quantity = ammo->clipsize * percentage;
		}

		if (quantity < 1) {
			quantity = 1;
		}

		chr->aimendrshoulder = 0;
		chr->aimendlshoulder = 0;
		chr->aimendback = 0;
		chr->aimendsideback = 0;
		chr->aimendcount = 10;

		chrAttackAmount(chr, arg1, arg2, quantity);

		return true;
	}

	return false;
}

bool chrTryAttackStand(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	int race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_EYESPY) {
		return false;
	}

	if (chrIsReadyForOrders(chr)) {
		if (race == RACE_ROBOT) {
			robotAttack(chr);
			return true;
		}

		if (race == RACE_HUMAN || race == RACE_SKEDAR) {
			if (chrGetHeldUsableProp(chr, 0) ||
					(chrGetHeldUsableProp(chr, 1))) {
				chrAttackStand(chr, attackflags, entityid);
				return true;
			}
		}
	}

	return false;
}

bool chrTryAttackKneel(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	int race = CHRRACE(chr);

	if (race == RACE_HUMAN || race == RACE_SKEDAR) {
		if (chrIsReadyForOrders(chr) && (chrGetHeldUsableProp(chr, 0) || chrGetHeldUsableProp(chr, 1))) {
			chrAttackKneel(chr, attackflags, entityid);
			return true;
		}
	}

	return false;
}

bool chrTryAttackLie(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	int race = CHRRACE(chr);

	if (race == RACE_HUMAN || race == RACE_SKEDAR) {
		if (chrIsReadyForOrders(chr) && (chrGetHeldUsableProp(chr, 0) || chrGetHeldUsableProp(chr, 1))) {
			chrAttackLie(chr, attackflags, entityid);
			return true;
		}
	}

	return false;
}

bool chrTryModifyAttack(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	int race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_EYESPY) {
		return false;
	}

	if (chr->actiontype == ACT_ATTACK
			&& (chr->act_attack.flags & (ATTACKFLAG_AIMONLY | ATTACKFLAG_DONTTURN))) {
		chr->act_attack.flags = attackflags;
		chr->act_attack.entityid = entityid;
		// infinitely shooting guard fix from 1964GEPD
		struct anim *anim = chr->model->anim;
		if (anim && anim->framea == anim->frameb) {
				anim->frameb++;
		}
		func0f031254(chr);
		return true;
	}

	return false;
}

bool chrFaceEntity(struct chrdata *chr, uint32_t attackflags, int entityid)
{
	if (chrIsReadyForOrders(chr)) {
		if (chr->actiontype != ACT_STAND) {
			chrStand(chr);
		}

		if (attackflags != chr->act_stand.flags || entityid != chr->act_stand.entityid) {
			chr->act_stand.flags = attackflags;
			chr->act_stand.entityid = entityid;
			chr->act_stand.reaim = 0;
			chr->act_stand.checkfacingwall = false;

			if (attackflags == ATTACKFLAG_AIMATTARGET && entityid == 1) {
				chr->act_stand.playwalkanim = true;
				chr->act_stand.entityid = 0;
			}
		}

		return true;
	}

	return false;
}

bool chrGoToPad(struct chrdata *chr, int padnum, uint32_t goposflags)
{
	if (padnum >= 0
			&& chrIsReadyForOrders(chr)
			&& (g_NumChrsSeenPlayerRecently2 <= 8
				|| (chr->hidden & CHRHFLAG_BASICGUARD) == 0
				|| (chr->flags & CHRFLAG0_ACTIVATEALARM))
				) {
		padnum = chrResolvePadId(chr, padnum);

		if (padnum >= 0)
		{
			RoomNum rooms[2];
			struct pad pad;

			padUnpack(padnum, PADFIELD_ROOM | PADFIELD_POS, &pad);

			rooms[0] = pad.room;
			rooms[1] = -1;

			if (chrGoToRoomPos(chr, &pad.pos, rooms, goposflags)) {
				return true;
			}
		}
	}

	return false;
}

bool chrSetPath(struct chrdata *chr, uint32_t path_id)
{
	chr->path = path_id;
	return true;
}

bool chrTryStartPatrol(struct chrdata *chr)
{
	struct path *path = pathFindById(chr->path);

	if (path && chrIsReadyForOrders(chr)) {
		chrStartPatrol(chr, path);
		return true;
	}

	return false;
}

bool chrTrySurrender(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		chrSurrender(chr);
		return true;
	}

	return false;
}

bool chrFadeOut(struct chrdata *chr)
{
	chrBeginDead(chr);
	chrFadeCorpse(chr);

	return true;
}

bool chrGoToTarget(struct chrdata *chr, uint32_t goposflags)
{
	if (chrIsReadyForOrders(chr)) {
		if (
				g_NumChrsSeenPlayerRecently2 <= 8
				|| (chr->hidden & CHRHFLAG_BASICGUARD) == 0
				|| (chr->flags & CHRFLAG0_ACTIVATEALARM)
				) {
			struct prop *prop = chrGetTargetProp(chr);

			if (chrGoToRoomPos(chr, &prop->pos, prop->rooms, goposflags)) {
				return true;
			}
		}
	}

	return false;
}

bool chrGoToChr(struct chrdata *chr, uint32_t dst_chrnum, uint32_t goposflags)
{
	if (chrIsReadyForOrders(chr)) {
		if (
				g_NumChrsSeenPlayerRecently2 <= 8
				|| (chr->hidden & CHRHFLAG_BASICGUARD) == 0
				|| (chr->flags & CHRFLAG0_ACTIVATEALARM)
				) {
			struct chrdata *dstchr = chrFindById(chr, dst_chrnum);

			if (dstchr && dstchr->prop && chrGoToRoomPos(chr, &dstchr->prop->pos, dstchr->prop->rooms, goposflags)) {
				return true;
			}
		}
	}

	return false;
}

bool chrGoToProp(struct chrdata *chr, struct prop *prop, uint32_t goposflags)
{
	if (chrIsReadyForOrders(chr) && prop) {
		if (chrGoToRoomPos(chr, &prop->pos, prop->rooms, goposflags)) {
			return true;
		}
	}

	return false;
}

bool chrGoToPos(struct chrdata *chr, struct coord *pos, uint32_t goposflags)
{
	RoomNum inrooms[21];
	RoomNum aboverooms[21];

	if (chrIsReadyForOrders(chr)) {
		if (g_NumChrsSeenPlayerRecently2 < 9
				|| (chr->hidden & CHRHFLAG_BASICGUARD) == 0
				|| (chr->flags & CHRCFLAG_NEVERSLEEP))
		{
			RoomNum *rooms = NULL;

			bgFindRoomsByPos(pos, inrooms, aboverooms, 20, NULL);

			if (inrooms[0] != -1) {
				rooms = inrooms;
			} else if (aboverooms[0] != -1) {
				rooms = aboverooms;
			}

			if (rooms != NULL && chrGoToRoomPos(chr, pos, rooms, goposflags)) {
				return true;
			}
		}
	}

	return false;
}

int func0f03aca0(struct chrdata *chr, float arg1, uint8_t arg2)
{
	float somefloat;

	if (!arg2) {
		arg1 -= chrGetDistanceToCoord(chr, &chr->runfrompos);
	}

	if (arg1 < 0) {
		chrAssignCoverByCriteria(g_Vars.chrdata,
				COVERCRITERIA_FURTHEREST
				| COVERCRITERIA_DISTTOTARGET
				| COVERCRITERIA_ONLYNEIGHBOURINGROOMS
				| COVERCRITERIA_ROOMSFROMME, 0);
		return chrGoToCover(chr, GOPOSFLAG_RUN);
	}

	somefloat = arg1 - 2000;

	if (somefloat < 2000) {
		somefloat = 2000;
	}

	if (!chrAssignCoverAwayFromDanger(chr, somefloat, arg1 + 10000)) {
		chrAssignCoverByCriteria(g_Vars.chrdata,
				COVERCRITERIA_FURTHEREST
				| COVERCRITERIA_DISTTOTARGET
				| COVERCRITERIA_ONLYNEIGHBOURINGROOMS
				| COVERCRITERIA_ROOMSFROMME, 0);
	}

	return chrGoToCover(chr, GOPOSFLAG_RUN);
}

bool chrTryStop(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_EYESPY) {
		func0f02e9a0(chr, 0);
		return true;
	}

	if (chrIsReadyForOrders(chr)) {
		chrStop(chr);
		return true;
	}

	return false;
}

bool chrTrySurprisedOneHand(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		chrDoSurprisedOneHand(chr);
		return true;
	}

	return false;
}

bool chrTrySurprisedSurrender(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		chrDoSurprisedSurrender(chr);
		return true;
	}

	return false;
}

bool chrTrySurprisedLookAround(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		chrDoSurprisedLookAround(chr);
		return true;
	}

	return false;
}

bool chrTryKneel(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		chrKneel(chr);
		return true;
	}

	return false;
}

bool chrTryStartAnim(struct chrdata *chr, int animfnum, float startframe, float endframe, uint8_t chranimflags, int merge, float speed)
{
	if (chrIsReadyForOrders(chr)) {
		chrStartAnim(chr, animfnum, startframe, endframe, chranimflags, merge, speed);
		return true;
	}

	return false;
}

bool chrTryRunFromTarget(struct chrdata *chr)
{
	struct prop *target = chrGetTargetProp(chr);
	struct prop *prop = chr->prop;
	float ymax;
	float ymin;
	float radius;
	struct coord dst;
	RoomNum rooms[8];
	struct coord diff;
	float distance;

	if (chrIsReadyForOrders(chr)) {
		if (!target) {
			return false;
		}

		diff.x = target->pos.x - prop->pos.x;
		diff.z = target->pos.z - prop->pos.z;
		diff.y = 0;

		distance = sqrtf(diff.f[0] * diff.f[0] + diff.f[2] * diff.f[2]);

		// Scale diff into range -1 to +1
		diff.x *= (1 / distance);
		diff.z *= (1 / distance);

		// Set dst to 1000 units in the opposite direction
		dst.x = prop->pos.x - diff.x * 1000;
		dst.z = prop->pos.z - diff.z * 1000;
		dst.y = prop->pos.y;

		propGetBbox(prop, &radius, &ymax, &ymin);

		// If dst runs into a wall, set it to closest valid spot
		if (cdExamCylMove03(&prop->pos, prop->rooms, &dst,
					CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG,
					1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION) {
			cdGetPos(&dst);
		}

		// Adjust dst to be two chr widths closer to avoid collision with wall
		dst.x = dst.x + (diff.x * radius + diff.x * radius);
		dst.z = dst.z + (diff.z * radius + diff.z * radius);

		if (func0f036974(prop, &dst)) {
			uint32_t speed = GOPOSFLAG_RUN;

			if (CHRRACE(chr) == RACE_HUMAN) {
				float dist = chrGetDistanceToCoord(chr, &dst);

				if (dist > 100) {
					speed = GOPOSFLAG_RUN;
				} else if (dist < 50) {
					speed = GOPOSFLAG_WALK;
				} else {
					speed = GOPOSFLAG_JOG;
				}
			}

			propUpdatePositionRoomsSimple(&prop->pos, prop->rooms, &dst, rooms);
			chrGoToRoomPos(chr, &dst, rooms, speed);

			return true;
		}
	}

	return false;
}

/**
 * Attempt to make the chr find a suitable prop in their current room to use as
 * cover and run to it. However, a bug prevents this from working.
 */
bool chrGoToCoverProp(struct chrdata *chr)
{
	struct prop *targetprop = chrGetTargetProp(chr);
	struct prop *chrprop = chr->prop;
	int16_t i;
	int16_t propnums[258];
	struct defaultobj *obj;
	int16_t numprops;
	int16_t startindex;

	if (targetprop == NULL) {
		return false;
	}

	if (chrIsReadyForOrders(chr)) {
		roomGetProps(chrprop->rooms, propnums, 256);

		for (numprops = 0; propnums[numprops] >= 0; numprops++);

		if (numprops > 0) {
			startindex = rngRandom() % numprops;
		}

		for (i = 0; i < numprops; i++) {
			struct prop *prop = &g_Vars.props[propnums[(i + startindex) % numprops]];
			obj = prop->obj;

			if (prop->type == PROPTYPE_OBJ
					&& (obj->hidden2 & OBJH2FLAG_DESTROYED) == 0
					&& (obj->hidden & OBJHFLAG_00008000) == 0
					&& (obj->hidden & OBJHFLAG_OCCUPIEDCHAIR) == 0) {
				float targetdist = propGetDistanceToProp(targetprop, prop);
				float chrdist = propGetDistanceToProp(chrprop, prop);

				// @bug: This condition cannot pass
				// (should be chrdist < targetdist + targetdist)
				//if (chrdist < 300 && chrdist > targetdist + targetdist && targetdist > 800) {
				if (chrdist < 300 && chrdist < targetdist + targetdist + targetdist && targetdist > 800) { // Ben's comment: fixing this, although no character actually makes use of it
					float propymax;
					float propymin;
					float propradius;
					float chrymax;
					float chrymin;
					float chrradius;
					float propheight;
					float chrheight;
					struct coord dstpos;
					RoomNum dstrooms[8];

					propGetBbox(prop, &propradius, &propymax, &propymin);
					propGetBbox(chrprop, &chrradius, &chrymax, &chrymin);

					propheight = propymax - propymin;
					chrheight = chrymax - chrymin;

					if (propheight > chrheight * 0.4f && propheight < chrheight * 0.9f) {
						propSetPerimEnabled(prop, false);

						if (cdTestLos04(&chrprop->pos, chrprop->rooms, &prop->pos, CDTYPE_DOORS | CDTYPE_BG)) {
							propSetPerimEnabled(prop, true);

							dstpos.x = prop->pos.x - (targetprop->pos.x - prop->pos.x) / targetdist * (propradius * 1.25f + chrradius);
							dstpos.z = prop->pos.z - (targetprop->pos.z - prop->pos.z) / targetdist * (propradius * 1.25f + chrradius);
							dstpos.y = prop->pos.y;

							if (func0f036974(chrprop, &dstpos)) {
								uint32_t speed = GOPOSFLAG_JOG;

								if (CHRRACE(chr) == RACE_HUMAN) {
									if (chrdist > 1500) {
										speed = GOPOSFLAG_RUN;
									} else if (chrdist < 100) {
										speed = GOPOSFLAG_WALK;
									}
								}

								propUpdatePositionRoomsSimple(&chrprop->pos, chrprop->rooms, &dstpos, dstrooms);
								chrGoToRoomPos(chr, &dstpos, dstrooms, speed);

								chr->proppreset1 = prop - g_Vars.props;
								obj->hidden |= OBJHFLAG_OCCUPIEDCHAIR;

								return true;
							}
						}
					}

					propSetPerimEnabled(prop, true);
				}
			}
		}
	}

	return false;
}

bool chrTryStartAlarm(struct chrdata *chr, int pad_id)
{
	if (CHRRACE(chr) == RACE_HUMAN && chrIsReadyForOrders(chr)) {
		pad_id = chrResolvePadId(chr, pad_id);

		if (pad_id >= 0)
		{
			struct defaultobj *obj = objFindByPadNum(pad_id);

			if (obj && objIsHealthy(obj)) {
				chrStartAlarm(chr);
				return true;
			}
		}
	}

	return false;
}

bool chrConsiderGrenadeThrow(struct chrdata *chr, uint32_t attackflags, uint32_t entityid)
{
	bool done = false;

	if (CHRRACE(chr) == RACE_HUMAN &&
			chr->grenadeprob > (rngRandom() % 255) &&
			chrGetDistanceToTarget(chr) > 200 &&
			chrIsReadyForOrders(chr)) {
		struct prop *target = chrGetTargetProp(chr);
		struct coord pos;

		if (target) {
			pos.x = target->pos.x;
			pos.y = target->pos.y;
			pos.z = target->pos.z;
		}

		if (target && cdTestLos04(&chr->prop->pos, chr->prop->rooms, &pos,
					CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
			struct prop *leftprop = chrGetHeldProp(chr, HAND_LEFT);
			struct prop *rightprop = chrGetHeldProp(chr, HAND_RIGHT);
			struct weaponobj *weapon;

			// If grenade is equipped in either hand, use it
			if (rightprop) {
				weapon = rightprop->weapon;

				if (weapon->weaponnum == WEAPON_GRENADE || weapon->weaponnum == WEAPON_NBOMB) {
					chrThrowGrenade(chr, 0, false);
					chr->act_throwgrenade.flags = attackflags;
					chr->act_throwgrenade.entityid = entityid;
					done = true;
				}
			}

			if (!done && leftprop) {
				weapon = leftprop->weapon;

				if (weapon->weaponnum == WEAPON_GRENADE || weapon->weaponnum == WEAPON_NBOMB) {
					chrThrowGrenade(chr, 1, false);
					chr->act_throwgrenade.flags = attackflags;
					chr->act_throwgrenade.entityid = entityid;
					done = true;
				}
			}

			// Grenade not equipped, and using a single weapon
			if (!done && (leftprop == NULL || rightprop == NULL)) {
				struct prop *prop;
				uint32_t flags = 0;

				if (rightprop) {
					flags = OBJFLAG_WEAPON_LEFTHANDED;
				}

				if (stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MBR) {
					prop = chrGiveWeapon(chr, MODEL_CHRGRENADE, WEAPON_NBOMB, flags);
				} else {
					prop = chrGiveWeapon(chr, MODEL_CHRGRENADE, WEAPON_GRENADE, flags);
				}

				if (prop) {
					weapon = prop->weapon;
					weapon->base.hidden |= OBJHFLAG_GONE;
					chrThrowGrenade(chr, rightprop == NULL ? 0 : 1, true);
					chr->act_throwgrenade.flags = attackflags;
					chr->act_throwgrenade.entityid = entityid;
					done = true;
				}
			}
		}
	}

	return done;
}

bool chrDropItem(struct chrdata *chr, uint32_t modelnum, uint32_t weaponnum)
{
	struct weaponobj *weapon;
	uint8_t race = CHRRACE(chr);

	if (race == RACE_DRCAROLL || race == RACE_ROBOT) {
		return false;
	}

	weapon = weaponCreateProjectileFromWeaponNum(modelnum, (uint8_t)weaponnum, chr);

	if (weapon && weapon->base.prop) {
		modelSetScale(weapon->base.model, weapon->base.model->scale);
		propReparent(weapon->base.prop, chr->prop);
		weapon->timer240 = TICKS(720);
		objSetDropped(weapon->base.prop, DROPTYPE_DEFAULT);
		chr->hidden |= CHRHFLAG_DROPPINGITEM;

		return true;
	}

	return false;
}

void chrPunchInflictDamage(struct chrdata *chr, int damage, int range, uint8_t reverse)
{
	struct prop *targetprop = chrGetTargetProp(chr);
	struct gset gset = {WEAPON_UNARMED, 0, 0, FUNC_PRIMARY};
	struct coord vector;

	if (chr->aibot) {
		gset.weaponnum = chr->aibot->weaponnum;
		gset.weaponfunc = chr->aibot->gunfunc;
	}

	if (chrIsTargetInFov(chr, 20, reverse)
			&& chrGetDistanceToTarget(chr) < range
			&& cdTestLos04(&chr->prop->pos, chr->prop->rooms, &targetprop->pos,
				CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
		vector.x = targetprop->pos.x - chr->prop->pos.x;
		vector.y = 0;
		vector.z = targetprop->pos.z - chr->prop->pos.z;

		utilsNormalizeF(&vector.x, &vector.y, &vector.z);

		bgunPlayPropHitSound(&gset, targetprop, -1);

		if (targetprop->type == PROPTYPE_PLAYER || targetprop->type == PROPTYPE_CHR) {
			chrDamageByImpact(targetprop->chr, gsetGetDamage(&gset) * damage, &vector, &gset, chr->prop, 200);
		}
	}

	weaponPlayWhooshSound(gset.weaponnum, chr->prop);
}

struct punchanim {
	int animnum;
	uint32_t damage;
	int16_t hitframe;
	float endframe;
};

struct punchanim g_HumanPunchAnims[] = {
	{ 0x027c, 5, 20, 60 },
	{ 0x027d, 5, 20, 31 },
	{ 0x027e, 5, 20, 48 },
	{ 0x027f, 5, 20, 69 },
	{ 0x0212, 5, 20, 64 },
	{ 0x0213, 5, 20, 52 },
	{ 0x0214, 5, 20, 51 },
	{ 0x020e, 5, 20, 53 },
	{ 0x020f, 5, 20, 89 },
	{ 0x0210, 5, 20, 71 },
	{ 0x0215, 5, 20, 62 },
	{ 0x0211, 5, 20, 72 },
};

struct punchanim g_SkedarPunchAnims[] = {
	{ 0x034c, 15, 25, 100 },
	{ 0x034d, 15, 25, -1  },
	{ 0x0395, 15, 25, -1  },
	{ 0x0346, 15, 25, -1  },
	{ 0x0347, 15, 25, -1  },
	{ 0x034f, 15, 25, -1  },
};

/**
 * Make the chr try to punch or kick.
 *
 * The function is only ever called with reverse = 0. If non-zero, it would
 * cause Skedar to kick behind them.
 *
 * Note that the final human anim can't be used because the modulus value is too
 * short by one.
 */
bool chrTryPunch(struct chrdata *chr, uint8_t reverse)
{
	struct punchanim *anims = NULL;
	int race = CHRRACE(chr);
	int animindex;
	uint32_t chranimflags = (rngRandom() % 256 > 128) ? CHRANIMFLAG_FLIP : 0;
	int chrhitradius;
	int playerhitradius;
	float startframe;
	bool ok;

	if (race == RACE_HUMAN) {
		anims = g_HumanPunchAnims;
		startframe = 10;
		chrhitradius = 120;
		playerhitradius = 120;
		//animindex = rngRandom() % 11;
		animindex = rngRandom() % 12; // Ben's comment: let's try fixing this...
	} else if (race == RACE_SKEDAR) {
		anims = g_SkedarPunchAnims;
		chrhitradius = 200;
		playerhitradius = 200;
		startframe = 20;

		if (reverse) {
			// Skedar kick behind
			animindex = 5;
		} else if (!chr->weapons_held[HAND_RIGHT] && !chr->weapons_held[HAND_LEFT]) {
			// Unarmed: Only use indexes 0 or 1
			animindex = rngRandom() % 2;
		} else {
			// Allow indexes 0-4, but if 3 or 4 then flip the anim based on
			// which hand is holding the gun
			animindex = rngRandom() % 5;

			if (animindex >= 3) {
				if (!chr->weapons_held[HAND_RIGHT] || !chr->weapons_held[HAND_LEFT]) {
					if (chr->weapons_held[HAND_RIGHT]) {
						chranimflags = 0;
					} else if (chr->weapons_held[HAND_LEFT]) {
						chranimflags = CHRANIMFLAG_FLIP;
					}
				}
			}
		}
	}

	if (chrHasFlag(chr, CHRFLAG1_ADJUSTPUNCHSPEED, BANK_1)) {
		if (chrHasFlag(chr, CHRFLAG0_CHUCKNORRIS, BANK_0)) {
			// Fast punch
			ok = chrTryStartAnim(chr, anims[animindex].animnum, startframe, anims[animindex].endframe, chranimflags, 16, 1.5f);
		} else {
			// Slow punch
			ok = chrTryStartAnim(chr, anims[animindex].animnum, startframe, anims[animindex].endframe - 25, chranimflags, 16, 0.5f);
		}

		chr->dodgerating = 0;
	} else {
		// Normal punch
		ok = chrTryStartAnim(chr, anims[animindex].animnum, startframe, anims[animindex].endframe, chranimflags, 16, 0.85f);
		chr->dodgerating = 0;
	}

	if (ok) {
		struct prop *targetprop = chrGetTargetProp(chr);

		if (targetprop->type == PROPTYPE_EYESPY || targetprop->type == PROPTYPE_PLAYER) {
			chr->act_anim.hitradius = playerhitradius;
		} else {
			chr->act_anim.hitradius = chrhitradius;
		}

		chr->act_anim.ishitanim = true;
		chr->act_anim.hitframe = anims[animindex].hitframe;

		if (g_Vars.normmplayerisrunning) {
			chr->act_anim.hitdamage = 1;
		} else if (chrHasFlag(chr, CHRFLAG1_ADJUSTPUNCHSPEED, BANK_1) && chrHasFlag(chr, CHRFLAG0_CHUCKNORRIS, BANK_0)) {
			chr->act_anim.hitdamage = (uint16_t)anims[animindex].damage * (float)chr->morale + (uint16_t)anims[animindex].damage * (float)chr->morale;
		} else if (chrHasFlag(chr, CHRFLAG1_PUNCHHARDER, BANK_1)) {
			chr->act_anim.hitdamage = (uint16_t)anims[animindex].damage * 6;
		} else {
			chr->act_anim.hitdamage = anims[animindex].damage;
		}

		chr->act_anim.reverse = reverse;
		chr->chrflags &= ~CHRCFLAG_INJUREDTARGET;

		return true;
	}

	return false;
}

/**
 * Verifies that the given chr struct is actually an eyespy and returns the
 * eyespy struct.
 *
 * Eyespys have their own chr struct, even though they aren't a chr. Iterating
 * the player list is required because the only pointer to an eyespy is via the
 * player struct.
 */
struct eyespy *chrToEyespy(struct chrdata *chr)
{
	if (chr && chr->prop) {
		if (CHRRACE(chr) == RACE_EYESPY) {
			int playercount = PLAYERCOUNT();
			int i;

			for (i = 0; i < playercount; i++) {
				if (g_Vars.players[i]->eyespy && chr->prop == g_Vars.players[i]->eyespy->prop) {
					return g_Vars.players[i]->eyespy;
				}
			}

			return NULL;
		}
	}

	return NULL;
}

void chrTickStand(struct chrdata *chr)
{
	int race;
	int i;
	int j;
	struct prop *leftgun;
	struct prop *rightgun;
	float angle;
	float sp74[8];
	float sp70;
	float sp6c;
	int tmp;
	int index;
	int sp44[8];

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrChooseStandAnimation(chr, chr->act_stand.mergetime);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (modelGetAnimNum(chr->model) == ANIM_SNIPING_GETUP) {
		if (modelGetCurAnimFrame(chr->model) >= modelGetAnimEndFrame(chr->model)) {
			chrChooseStandAnimation(chr, 8);
			chr->act_stand.prestand = 0;
		}

		return;
	}

	if (chr->sleep > 0) {
		return;
	}

	race = CHRRACE(chr);

	if (race == RACE_EYESPY) {
		return;
	}

	if (chr->act_stand.prestand) {
		if (modelGetCurAnimFrame(chr->model) >= modelGetAnimEndFrame(chr->model)) {
			chrChooseStandAnimation(chr, 8);
			chr->act_stand.prestand = 0;
		}

		chr->sleep = 0;
		return;
	}

	if (!chr->aibot && (race == RACE_HUMAN || race == RACE_SKEDAR) && chr->act_stand.flags > 0) {
		if (chr->act_stand.reaim) {
			chr->act_stand.turning = chrTurn(chr, chr->act_stand.turning, modelGetNumAnimFrames(chr->model) - 1, 1, 0);

			if (chr->act_stand.turning != TURNSTATE_TURNING) {
				chrChooseStandAnimation(chr, 8);
				chr->act_stand.reaim = false;

				if (chr->act_stand.flags & ATTACKFLAG_AIMATDIRECTION) {
					chr->act_stand.flags = 0;
				}
			}
		} else {
			float relangle = chrGetAttackEntityRelativeAngle(chr, chr->act_stand.flags, chr->act_stand.entityid);

			if ((relangle > 0.34901028871536f && relangle < 5.9331746101379f)
					|| (relangle > 0.17450514435768f && relangle < 6.1076798439026f && !chr->act_stand.playwalkanim)) {
				leftgun = chrGetHeldProp(chr, HAND_LEFT);
				rightgun = chrGetHeldProp(chr, HAND_RIGHT);

				chr->act_stand.reaim = true;
				chr->act_stand.turning = TURNSTATE_TURNING;

				if (race == RACE_HUMAN) {
					if ((leftgun && rightgun)
							|| (!leftgun && !rightgun)
							|| weaponIsOneHanded(leftgun)
							|| weaponIsOneHanded(rightgun)) {
						modelSetAnimation(chr->model, ANIM_WALK_FWD_006B, rngRandom() % 2, 0, 0.5f, 16);
						modelSetAnimEndFrame(chr->model, animGetNumFrames(ANIM_WALK_FWD_006B) - 1);
					} else {
						if (rightgun || leftgun) {
							modelSetAnimation(chr->model, ANIM_RIFLE_PATROL, leftgun != NULL, 0, 0.5f, 16);
							modelSetAnimEndFrame(chr->model, animGetNumFrames(ANIM_RIFLE_PATROL) - 1);
						}
					}
				} else if (race == RACE_SKEDAR) {
					modelSetAnimation(chr->model, ANIM_0392, rngRandom() % 2, 0, 0.5f, 16);
					modelSetAnimEndFrame(chr->model, animGetNumFrames(ANIM_0392) - 1);
				}
			} else if (chr->act_stand.flags & ATTACKFLAG_AIMATDIRECTION) {
				chr->act_stand.flags = 0;
			}
		}

		chr->sleep = 0;
		return;
	}

	if (chr->aibot) {
		return;
	}

	if (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) {
		chr->sleep = 0;
	} else {
		chr->sleep = 14 + (rngRandom() % 5);
	}

	if (chr->act_stand.checkfacingwall == false) {
		return;
	}

	if (chr->chrflags & CHRCFLAG_CANFACEWALL) {
		chr->act_stand.checkfacingwall = false;
		return;
	}

	chr->act_stand.wallcount -= chr->sleep;

	if (chr->act_stand.wallcount >= 0) {
		return;
	}

	sp6c = sp70 = chrGetInverseTheta(chr);

	for (i = 0; i < ARRAYCOUNT(sp74); i++) {
		sp6c += 0.7852731347084f;

		if (sp6c >= M_TAU) {
			sp6c -= M_TAU;
		}

		sp74[i] = func0f02e550(chr->prop, sp6c, 1000, CDTYPE_BG, 0, 1);
	}

	for (i = 0; i < ARRAYCOUNT(sp44); i++) {
		sp44[i] = i;
	}

	for (i = 0; i < ARRAYCOUNT(sp44) - 1; i++) {
		index = i;

		for (j = index + 1; j < ARRAYCOUNT(sp44); j++) {
			if (sp74[sp44[j]] < sp74[sp44[index]]) {
				index = j;
			}
		}

		j = sp44[i];
		sp44[i] = sp44[index];
		sp44[index] = j;
	}

	index = -1;

	if (sp74[0] < 490) {
		if (sp74[sp44[4]] < 200) {
			index = 7;
		} else if (sp44[0] == 0 || sp44[1] == 0 || sp44[2] == 0) {
			if ((sp44[3] == 4 || sp44[4] == 4) && (rngRandom() % 3) == 0) {
				if (sp44[3] == 4) {
					index = 3;
				} else {
					index = 4;
				}
			} else {
				index = 5 + rngRandom() % 3;
			}
		} else if ((sp44[0] == 1 || sp44[0] == 7) && sp44[5] && sp44[6] && sp44[7]) {
			index = 5 + rngRandom() % 3;
		}
	}

	if (index >= 0) {
		i = sp44[index];
		angle = sp70 + i * 0.7852731347084f;

		if (angle >= M_TAU) {
			angle -= M_TAU;
		}

		chrFaceEntity(chr, ATTACKFLAG_AIMATDIRECTION, angle * 10432.0390625f);
	} else {
		chr->act_stand.checkfacingwall = false;
	}
}

void chrTickKneel(struct chrdata *chr)
{
	chr->sleep = 0;

	if ((chr->hidden & CHRHFLAG_NEEDANIM) && !modelIsAnimMerging(chr->model)) {
		chrKneelChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}
}

void chrTickAnim(struct chrdata *chr)
{
	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		modelSetAnimation(chr->model, chr->act_anim.animnum, chr->act_anim.flip,
				chr->act_anim.startframe, chr->act_anim.speed, chr->act_anim.blend);

		if (chr->act_anim.endframe >= 0) {
			modelSetAnimEndFrame(chr->model, chr->act_anim.endframe);
		}

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (!chr->act_anim.pauseatend && modelGetCurAnimFrame(chr->model) >= modelGetAnimEndFrame(chr->model)) {
		chrStand(chr);
	}

	if (chr->act_anim.ishitanim && modelGetCurAnimFrame(chr->model) >= (int)chr->act_anim.hitframe) {
		chr->act_anim.ishitanim = false;
		chrPunchInflictDamage(chr, chr->act_anim.hitdamage, chr->act_anim.hitradius, chr->act_anim.reverse);
	}

	// Play sneezing sound
	if (CHRRACE(chr) == RACE_HUMAN
			&& modelGetAnimNum(chr->model) == ANIM_SNEEZE
			&& modelGetCurAnimFrame(chr->model) >= 42
			&& (g_Vars.lvframenum % 2) == 0
			&& chrGetDistanceToCurrentPlayer(chr) < 800) {
				//debug_log("Sneeze!\n", 0);
		psCreate(NULL, chr->prop, SFX_0037, -1,
				-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
	}

	if (chr->sleep <= 0 && chr->act_anim.slowupdate) {
		chr->sleep = 14 + (rngRandom() % 5);
	}

	if (modelGetAnimNum(chr->model) == ANIM_RELOAD_0209) {
		chrSetFiring(chr, HAND_RIGHT, false);
		chrSetFiring(chr, HAND_LEFT, false);
	}
}

void chrTickSurrender(struct chrdata *chr)
{
	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrSurrenderChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (chr->sleep <= 0) {
		if (CHRRACE(chr) == RACE_HUMAN) {
			struct model *model = chr->model;
			chr->sleep = 16;

			if (modelGetAnimNum(model) == ANIM_SURRENDER_002F && modelGetCurAnimFrame(model) >= 80.0f) {
				struct coord coord = {0, 0, 0};
				float value = chrGetInverseTheta(chr);
				coord.x = -sinf(value);
				coord.z = -cosf(value);

				if (!propchrHasClearLineInVector(chr->prop, &coord, 20)) {
					modelSetAnimation(chr->model, ANIM_SURRENDER_002E, rngRandom() & 1, 30, 0.5, 16);
					modelSetAnimLooping(chr->model, 30, 16);
				}
			}
		}
	}
}

void chrFadeCorpse(struct chrdata *chr)
{
	if (chr->actiontype == ACT_DEAD || chr->actiontype == ACT_DRUGGEDKO) {
		chr->act_dead.fadenow = true;
	}
}

void chrFadeCorpseWhenOffScreen(struct chrdata *chr)
{
	if (chr->actiontype == ACT_DEAD) {
		chr->act_dead.fadewheninvis = true;
	}
}

void chrTickDead(struct chrdata *chr)
{
	struct aibot *aibot = chr->aibot;

	// If fade is active, handle it
	if (chr->act_dead.fadetimer60 >= 0) {
		chr->act_dead.fadetimer60 += g_Vars.lvupdate60;

		if (chr->act_dead.fadetimer60 >= TICKS(90)) {
			// Fade finished
			chr->fadealpha = 0;

			if (aibot) {
				botSpawn(chr, true);
			} else {
				chr->hidden |= CHRHFLAG_DELETING;
			}
		} else {
			// Still fading
			chr->fadealpha = (TICKS(90) - chr->act_dead.fadetimer60) * 255 / TICKS(90);
		}
	} else {
		// If fade has been triggered (this can happen when the corpse is on
		// screen and there's lots of other chrs around)
		if (chr->act_dead.fadenow) {
			chr->act_dead.fadetimer60 = 0;
			chrDropItemsForOwnerReap(chr);
		}

		if (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) {
			// Keep corpse for now
			chr->act_dead.invistimer60 = 0;
		} else {
			chr->act_dead.invistimer60 += g_Vars.lvupdate60;
		}

		if (chr->act_dead.fadewheninvis && chr->act_dead.invistimer60 >= TICKS(120)) {
			// Remove corpse (off-screen)
			if (aibot == NULL) {
				chr->hidden |= CHRHFLAG_DELETING;
			}

			chr->fadealpha = 0;

			chrDropItemsForOwnerReap(chr);
		}
	}

	if (aibot == NULL) {
		chr->ailist = NULL;
	}
}

/**
 * This function is called when a chr is injured or killed. It iterates other
 * chrs within 2000 units who can see the chr and updates their chrseeshot or
 * chrseedie properties.
 *
 * The search stops once 7 chrs have been found in range, regardless of whether
 * they can see the chr, presumably to avoid doing too many expensive line of
 * sight checks in one tick. The last iterated chr index is stored and continued
 * from there next time the function is called. The function is called on
 * subsequent ticks while the chr is still in their injured or dying action.
 */
void chrAlertOthersOfInjury(struct chrdata *chr, bool dying)
{
	int index = 0;
	int numinrange = 0;
	int numchrs = chrsGetNumSlots();

	if (g_Vars.antiplayernum >= 0 && chr->prop == g_Vars.anti->prop) {
		return;
	}

	if (chr->actiontype == ACT_ARGH) {
		index = chr->act_argh.notifychrindex;
	} else if (chr->actiontype == ACT_DIE || chr->actiontype == ACT_DRUGGEDDROP) {
		index = chr->act_die.notifychrindex;
	} else if (chr->actiontype == ACT_DEAD) {
		index = chr->act_dead.notifychrindex;
	}

	// notifychrindex can be < 0, so avoid reading g_ChrSlots when that is the case
	for (; index >= 0 && index < numchrs && numinrange < 7; index++) {
		struct chrdata *loopchr = &g_ChrSlots[index];

		if (loopchr->model && loopchr->prop && (loopchr->prop->flags & PROPFLAG_ENABLED)) {
			float xdiff = loopchr->prop->pos.x - chr->prop->pos.x;
			float ydiff = loopchr->prop->pos.y - chr->prop->pos.y;
			float zdiff = loopchr->prop->pos.z - chr->prop->pos.z;

			if (xdiff * xdiff + ydiff * ydiff + zdiff * zdiff < 4000000.0f) {
				numinrange++;

				if (chrHasLosToPosWasteful(loopchr, &chr->prop->pos, chr->prop->rooms)) {
					if (dying == false) {
						loopchr->chrseeshot = chr->chrnum;
					} else {
						loopchr->chrseedie = chr->chrnum;
					}
				}
			}
		}
	}

	if (chr->actiontype == ACT_ARGH) {
		chr->act_argh.notifychrindex = index;
	} else if (chr->actiontype == ACT_DIE || chr->actiontype == ACT_DRUGGEDDROP) {
		chr->act_die.notifychrindex = index;
	} else if (chr->actiontype == ACT_DEAD) {
		chr->act_dead.notifychrindex = index;
	}
}

void chrTickDie(struct chrdata *chr)
{
	struct model *model = chr->model;
	uint32_t race = CHRRACE(chr);

	uint16_t thuds[] = {
		SFX_THUD_808D,
		SFX_THUD_808E,
		SFX_THUD_808F,
		SFX_THUD_8090,
		SFX_THUD_8091,
		SFX_THUD_8092,
		SFX_THUD_8093,
		SFX_THUD_8094,
		SFX_THUD_8095,
		SFX_THUD_8096,
		SFX_THUD_8097,
	};

	uint16_t specialdiesounds[] = {
		SFX_M1_NOOO, // "Noooo!"
		SFX_M1_SCREAM, // Death scream
		SFX_M2_NOOO, // "Noooo!"
		SFX_M2_NOOO, // "Noooo!"
		SFX_M1_SCREAM, // Death scream
		SFX_THUD_8092,
		SFX_THUD_8093,
		SFX_THUD_8094,
		SFX_THUD_8095,
		SFX_THUD_8096,
		SFX_THUD_8097,
	};

	static int thudindex = 0;

	if (race == RACE_EYESPY) {
		return;
	}

	if (race == RACE_ROBOT) {
		struct prop *prop = chr->prop;
		psStopSound(prop, PSTYPE_GENERAL, 0xffff);
		explosionCreateSimple(prop, &prop->pos, prop->rooms, EXPLOSIONTYPE_8, g_Vars.currentplayernum);
		chr->hidden |= CHRHFLAG_DELETING;
		return;
	}

	if (race == RACE_DRCAROLL) {
		struct prop *prop = chr->prop;

		if (g_DrCarollDyingTimer > TICKS(120) && chr->voicebox) {
			// Play speech
			uint16_t phrases[] = {
				SFX_DRCAROLL_SYSTEMS_FAILURE,
				SFX_DRCAROLL_YOU_GO_ON,
				SFX_DRCAROLL_I_CANT_MAKE_IT,
				SFX_DRCAROLL_IM_DYING,
				SFX_DRCAROLL_GOODBYE,
				SFX_DRCAROLL_YOU_WERE_SUPPOSED,
			};

			psCreate(NULL, chr->prop, phrases[rngRandom() % 5], -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
			chr->voicebox = 0;
		}

		// Change images randomly
		if (chr->act_die.drcarollimagedelay > 0) {
			chr->act_die.drcarollimagedelay -= g_Vars.lvupdate60;
		} else {
			chr->act_die.drcarollimagedelay = (rngRandom() % TICKS(1000)) * 0.01f + 5.0f;
			chr->drcarollimage_left = 1 + (int)((rngRandom() % 400) * 0.01f);
			chr->drcarollimage_right = 1 + (int)((rngRandom() % 400) * 0.01f);
		}

		if (g_DrCarollDyingTimer > TICKS(310)) {
			// Explode
			psStopSound(prop, PSTYPE_GENERAL, 0xffff);
			explosionCreateSimple(prop, &prop->pos, prop->rooms, EXPLOSIONTYPE_8, g_Vars.currentplayernum);
			chrBeginDead(chr);
		} else if (chr->soundtimer > (int)g_DrCarollShieldSndThreshold) {
			// Play shield damage sound
			chr->soundtimer = 0;
			g_DrCarollShieldSndThreshold -= 5;
			psCreate(NULL, prop, SFX_SHIELD_DAMAGE, -1,
					-1, PSFLAG_0400, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
			sparksCreate(prop->rooms[0], prop, &prop->pos, NULL, 0, SPARKTYPE_ELECTRICAL);
		}

		return;
	}

	// Human or Skedar
	// If due, play thud 1 sound
	if (chr->act_die.thudframe1 >= 0 && modelGetCurAnimFrame(model) >= chr->act_die.thudframe1) {
		if (chr->specialdie == 0) {
			psCreate(NULL, chr->prop, thuds[thudindex], -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
		} else if (chr->specialdie != SPECIALDIE_OVERRAILING) {
			psCreate(NULL, chr->prop, specialdiesounds[chr->specialdie - 1], -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
		}

		thudindex++;

		if (thudindex > 10) {
			thudindex = 0;
		}

		chr->act_die.thudframe1 = -1;
	}

	// If due, play thud 2 sound
	if (chr->act_die.thudframe2 >= 0 && modelGetCurAnimFrame(model) >= chr->act_die.thudframe2) {
		if (chr->specialdie < 5) {
			psCreate(NULL, chr->prop, SFX_THUD_808E, -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
		} else {
			psCreate(NULL, chr->prop, thuds[thudindex], -1,
					-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
		}

		thudindex++;

		if (thudindex > 10) {
			thudindex = 0;
		}

		chr->act_die.thudframe2 = -1;
	}

	// Check for end of death animation and switch to ACT_DEAD
	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		if (CHRRACE(chr) == RACE_HUMAN && modelGetAnimNum(model) == ANIM_DEATH_STOMACH_LONG) {
			modelSetAnimation(model, ANIM_DEATH_KNOCKED_BK, !modelIsFlipped(model), 50, 0.3, animGetNumFrames(ANIM_DEATH_KNOCKED_BK) - 51.0f);
			modelSetAnimSpeed(model, 0.5, animGetNumFrames(ANIM_DEATH_KNOCKED_BK) - 51.0f);
			return;
		}

		chrBeginDead(chr);
	}

	chrAlertOthersOfInjury(chr, true);
}

void chrTickDruggedComingUp(struct chrdata *chr)
{
	uint16_t thuds[] = {
		SFX_THUD_808D,
		SFX_THUD_808E,
		SFX_THUD_808F,
		SFX_THUD_8090,
		SFX_THUD_8091,
		SFX_THUD_8092,
		SFX_THUD_8093,
		SFX_THUD_8094,
		SFX_THUD_8095,
		SFX_THUD_8096,
		SFX_THUD_8097,
	};

	chr->act_druggedcomingup.timer60 += g_Vars.lvupdate60;

	if (chr->act_druggedcomingup.timer60 > 0) {
		struct animtablerow *row;
		int race = CHRRACE(chr);
		struct model *model = chr->model;
		int i = 0;
		bool done = false;
		struct prop *weapon;

		chrUncloak(chr, true);

		chr->actiontype = ACT_DRUGGEDDROP;

		while (!done) {
			if (i >= 0
					&& g_AnimTablesByRace[race][i].deathanims != NULL
					&& g_AnimTablesByRace[race][i].deathanimcount > 0) {
				int index = rngRandom() % g_AnimTablesByRace[race][i].deathanimcount;
				row = &g_AnimTablesByRace[race][i].deathanims[index];

				chr->act_die.thudframe1 = row->thudframe1;
				chr->act_die.thudframe2 = row->thudframe2;

				modelSetAnimationWithMerge(model, row->animnum, row->flip, 0, row->speed, 16, true);

				if (row->endframe >= 0) {
					modelSetAnimEndFrame(model, row->endframe);
				}

				done = true;
			}

			if (!done) {
				i++;

				if (g_AnimTablesByRace[race][i].hitpart == -1) {
					done = true;
				}
			}
		}

		weapon = chr->weapons_held[HAND_RIGHT];

		if (weapon && (weapon->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
			objSetDropped(weapon, DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}

		weapon = chr->weapons_held[HAND_LEFT];

		if (weapon && (weapon->obj->flags & OBJFLAG_AIUNDROPPABLE) == 0) {
			objSetDropped(weapon, DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}

		chrDropConcealedItems(chr);
	}
}

void chrTickDruggedDrop(struct chrdata *chr)
{
	struct model *model = chr->model;

	uint16_t thuds[] = {
		SFX_THUD_808D,
		SFX_THUD_808E,
		SFX_THUD_808F,
		SFX_THUD_8090,
		SFX_THUD_8091,
		SFX_THUD_8092,
		SFX_THUD_8093,
		SFX_THUD_8094,
		SFX_THUD_8095,
		SFX_THUD_8096,
		SFX_THUD_8097,
	};

	static int thudindex = 0;

	// If due, play thud 1 sound
	if (chr->act_die.thudframe1 >= 0 && modelGetCurAnimFrame(model) >= chr->act_die.thudframe1) {
		psCreate(NULL, chr->prop, thuds[thudindex], -1,
				-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);

		thudindex++;

		if (thudindex > 10) {
			thudindex = 0;
		}

		chr->act_die.thudframe1 = -1;
	}

	// If due, play thud 2 sound
	if (chr->act_die.thudframe2 >= 0 && modelGetCurAnimFrame(model) >= chr->act_die.thudframe2) {
		psCreate(NULL, chr->prop, thuds[thudindex], -1,
				-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);

		thudindex++;

		if (thudindex > 10) {
			thudindex = 0;
		}

		chr->act_die.thudframe2 = -1;
	}

	// If falling animation finished, assign ACT_DRUGGEDKO
	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chr->actiontype = ACT_DRUGGEDKO;
		chr->act_dead.fadetimer60 = chr->aibot ? 0 : -1;
		chr->act_dead.fadenow = false;
		chr->act_dead.fadewheninvis = false;
		chr->act_dead.invistimer60 = 0;
		chr->act_dead.notifychrindex = 0;
		chr->sleep = 0;
	}

	chrAlertOthersOfInjury(chr, true);
}

void chrTickDruggedKo(struct chrdata *chr)
{
	bool reap = false;

	// If fade is active, handle it
	if (chr->act_dead.fadetimer60 >= 0) {
		chr->act_dead.fadetimer60 += g_Vars.lvupdate60;

		if (chr->act_dead.fadetimer60 >= TICKS(90)) {
			reap = true;
		} else {
			chr->fadealpha = (TICKS(90) - chr->act_dead.fadetimer60) * 255 / TICKS(90);
		}
	} else if ((chr->chrflags & CHRCFLAG_KEEPCORPSEKO) == 0) {
		if (chr->act_dead.fadenow) {
			chr->act_dead.fadetimer60 = 0;
		}

		if (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) {
			chr->act_dead.invistimer60 = 0;
		} else {
			chr->act_dead.invistimer60 += g_Vars.lvupdate60;
		}

		if (chr->act_dead.fadewheninvis && chr->act_dead.invistimer60 >= TICKS(120)) {
			reap = true;
		}
	}

	if (reap) {
		chr->fadealpha = 0;
		chr->hidden |= CHRHFLAG_DELETING;
		chrDropItemsForOwnerReap(chr);
	}
}

void chrTickArgh(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chrRecordLastSeeTargetTime(chr);

		if (CHRRACE(chr) == RACE_HUMAN && modelGetAnimNum(model) == ANIM_DEATH_STOMACH_LONG) {
			func0f02ed28(chr, 26);
		} else {
			if (chr->race == RACE_DRCAROLL) {
				chr->drcarollimage_left = DRCAROLLIMAGE_EYESDEFAULT;
				chr->drcarollimage_right = DRCAROLLIMAGE_EYESDEFAULT;
			}

			chrStop(chr);
		}
	}

	chrAlertOthersOfInjury(chr, false);
}

void chrTickPreArgh(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		struct coord dir;
		dir.x = chr->act_preargh.dir.x;
		dir.y = chr->act_preargh.dir.y;
		dir.z = chr->act_preargh.dir.z;

		chrReactToDamage(chr, &dir,
				chr->act_preargh.relshotdir,
				chr->act_preargh.hitpart,
				&chr->act_preargh.gset,
				chr->act_preargh.aplayernum);
	}
}

void chrTickSidestep(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrSidestepChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chrRecordLastSeeTargetTime(chr);
		func0f02ed28(chr, 10);
	}
}

void chrTickJumpOut(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrJumpOutChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chrRecordLastSeeTargetTime(chr);
		chrStop(chr);
	}
}

void chrTickTest(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chrStand(chr);
	}
}

void chrTickStartAlarm(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrStartAlarmChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (modelGetCurAnimFrame(model) >= 60) {
		alarmActivate();
	}

	if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
		chrStop(chr);
	}
}

void chrTickSurprised(struct chrdata *chr)
{
	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrSurprisedChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (CHRRACE(chr) == RACE_HUMAN) {
		struct model *model = chr->model;

		if (modelGetCurAnimFrame(model) >= modelGetAnimEndFrame(model)) {
			if (modelGetAnimNum(model) == ANIM_SURRENDER_002E) {
				func0f02ed28(chr, 26);
			} else if (modelGetAnimNum(model) == ANIM_TAUNT) {
				func0f02ed28(chr, 26);
			} else {
				chrStop(chr);
			}
		}
	}
}

void chrUpdateFireslot(struct chrdata *chr, int handnum, bool withsound, bool withbeam, struct coord *from, struct coord *to)
{
	struct prop *weaponprop;
	struct weaponobj *weapon;
	int weaponnum;
	bool playsound = false;
	uint8_t duration;
	uint16_t soundnum;

	weaponprop = chrGetHeldProp(chr, handnum);

	if (weaponprop) {
		weapon = weaponprop->weapon;
		weaponnum = weapon->weaponnum;
		duration = gsetGetFireslotDuration(&weapon->gset);
		soundnum = gsetGetSingleShootSound(&weapon->gset);

		if (chr->fireslots[handnum] < 0) {
			chr->fireslots[handnum] = bgunAllocateFireslot();
		}

		if (chr->fireslots[handnum] >= 0) {
			struct fireslot *fireslot = &g_Fireslots[chr->fireslots[handnum]];

			if (withsound) {
				if (duration > 0) {
					if (chr->hidden2 & CHRH2FLAG_FIRESOUNDDONE)
					{
						playsound = false;
					} else {
						if (g_Vars.lvframe60 > fireslot->endlvframe) {
							playsound = true;
						} else {
							playsound = false;
						}
					}
				} else {
					playsound = true;
				}
			}

			if (playsound) {
				psCreate(NULL, chr->prop, soundnum, -1, -1,
						PSFLAG_0400, PSFLAG2_PRINTABLE, PSTYPE_CHRSHOOT, NULL, -1, NULL, -1, -1, -1, -1);
				fireslot->endlvframe = (uint32_t)g_Vars.lvframe60 + duration;
				chr->hidden2 |= CHRH2FLAG_FIRESOUNDDONE;
			}

			if (withbeam) {
				beamCreate(&fireslot->beam, weaponnum, from, to);
			}
		}
	}
}

/**
 * Returns the chr's turn angle difference to 360 degrees, in radians.
 */
float chrGetInverseTheta(struct chrdata *chr)
{
	if (chr->aibot) {
		return chr->aibot->lookangle;
	}

	if (chr->model == NULL && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		struct player *player = g_Vars.players[playermgrGetPlayerNumByProp(chr->prop)];
		float angle = (360.0f - player->vv_theta) * 0.017450513318181f;

		if (angle >= M_TAU) {
			angle -= M_TAU;
		} else if (angle < 0) {
			angle += M_TAU;
		}

		return angle;
	}

	return modelGetChrRotY(chr->model);
}

void chrSetLookAngle(struct chrdata *chr, float angle)
{
	if (chr->aibot) {
		chr->aibot->lookangle = angle;
	} else {
		modelSetChrRotY(chr->model, angle);
	}
}

float chrGetRotY(struct chrdata *chr)
{
	if (chr->aibot) {
		return chr->aibot->roty;
	} else {
		return modelGetChrRotY(chr->model);
	}
}

void chrSetRotY(struct chrdata *chr, float roty)
{
	if (chr->aibot) {
		chr->aibot->roty = roty;
	} else {
		modelSetChrRotY(chr->model, roty);
	}
}

float chrGetAimAngle(struct chrdata *chr)
{
	float angle = chrGetInverseTheta(chr) + chr->aimsideback;
	float offset = 0;

	if (angle >= M_TAU) {
		angle -= M_TAU;
	} else if (angle < 0) {
		angle += M_TAU;
	}

	if (chr->aibot) {
		if (chr->aibot->attackanimconfig) {
			// empty
		}
	} else if (chr->actiontype == ACT_ATTACK
			|| chr->actiontype == ACT_ATTACKROLL
			|| chr->actiontype == ACT_BOT_ATTACKSTAND
			|| chr->actiontype == ACT_BOT_ATTACKKNEEL
			|| chr->actiontype == ACT_BOT_ATTACKSTRAFE) {
		offset = chr->act_attack.animcfg->unk0c;
	} else if (chr->prop->type == PROPTYPE_PLAYER) {
		offset += g_Vars.players[playermgrGetPlayerNumByProp(chr->prop)]->angleoffset;
	}

	if (offset) {
		if (chr->model->anim->flip) {
			offset = M_TAU - offset;
		}

		angle += offset;

		if (angle >= M_TAU) {
			angle -= M_TAU;
		} else if (angle < M_TAU) {
			angle += M_TAU;
		}
	}

	return angle;
}

float chrGetPitchAngle(struct chrdata *chr)
{
	float sum = chr->aimuprshoulder + chr->aimupback;

	if (sum < 0) {
		sum += M_TAU;
	}

	return sum;
}

/**
 * Turn the chr slightly towards their target.
 */
int chrTurn(struct chrdata *chr, int turning, float endanimframe, float speed, float toleranceangle)
{
	if (turning != TURNSTATE_OFF) {
		struct model *model = chr->model;
		float curframe = modelGetCurAnimFrame(model);
		float finalangle = chrGetInverseTheta(chr);
		float remainingangle;
		float increment = M_TAU / 100.0f * speed * g_Vars.lvupdate60f * model->anim->playspeed;

		if (chr->aibot) {
			struct prop *target = chrGetTargetProp(chr);
			remainingangle = chrGetAngleToPos(chr, &target->pos);
		} else if (chr->actiontype == ACT_ATTACK
				|| chr->actiontype == ACT_BOT_ATTACKSTAND
				|| chr->actiontype == ACT_BOT_ATTACKKNEEL
				|| chr->actiontype == ACT_BOT_ATTACKSTRAFE) {
			remainingangle = chrGetAttackEntityRelativeAngle(chr, chr->act_attack.flags, chr->act_attack.entityid);
		} else if (chr->actiontype == ACT_STAND) {
			remainingangle = chrGetAttackEntityRelativeAngle(chr, chr->act_stand.flags, chr->act_stand.entityid);
		} else if (chr->actiontype == ACT_THROWGRENADE) {
			remainingangle = chrGetAttackEntityRelativeAngle(chr, chr->act_throwgrenade.flags, chr->act_throwgrenade.entityid);
		} else {
			struct prop *target = chrGetTargetProp(chr);
			remainingangle = chrGetAngleToPos(chr, &target->pos);
		}

		remainingangle -= toleranceangle;

		if (remainingangle < 0) {
			remainingangle += M_TAU;
		}

		if (increment > remainingangle || M_TAU - increment < remainingangle) {
			// Close enough to stop
			finalangle += remainingangle;

			if (finalangle >= M_TAU) {
				finalangle -= M_TAU;
			}

			chrSetLookAngle(chr, finalangle);
			turning = TURNSTATE_ONTARGET;
		} else if (remainingangle < M_PI) {
			// Turning in one direction
			finalangle += increment;

			if (finalangle >= M_TAU) {
				finalangle -= M_TAU;
			}

			chrSetLookAngle(chr, finalangle);
		} else {
			// Turning in the other direction
			finalangle -= increment;

			if (finalangle < 0) {
				finalangle += M_TAU;
			}

			chrSetLookAngle(chr, finalangle);
		}

		if (curframe >= endanimframe) {
			turning = TURNSTATE_OFF;
		}
	}

	return turning;
}

/**
 * Calculates and sets a character's final aiming direction for an attack.
 *
 * This function:
 * - Computes aim rotation toward the target (both horizontal and vertical),
 * - Adjusts aim based on visibility flags and distance,
 * - Handles special targeting behavior for players (e.g., eye height compensation),
 * - Incorporates aim "wobble" to simulate human-like inaccuracies,
 * - Clamps aim within animation limits defined by animcfg,
 * - Returns whether the resulting aim is within acceptable bounds.
 *
 * Used by both AI bots and scripted NPCs during attack sequences.
 */

 bool chrCalcAim(struct chrdata *chr, struct attackanimconfig *animcfg, bool firingleft, bool firingright, float arg4)
{
	float shootrotx = 0.0f;
	float aimendsideback = 0.0f;
	uint32_t flags = ATTACKFLAG_AIMATTARGET;
	int entityid = 0;
	bool result = true;

	if (!chr->aibot) {
		if (chr->actiontype == ACT_ATTACK
				|| chr->actiontype == ACT_BOT_ATTACKSTAND
				|| chr->actiontype == ACT_BOT_ATTACKKNEEL
				|| chr->actiontype == ACT_BOT_ATTACKSTRAFE) {
			flags = chr->act_attack.flags;
			entityid = chr->act_attack.entityid;
		} else if (chr->actiontype == ACT_STAND) {
			flags = chr->act_stand.flags;
			entityid = chr->act_stand.entityid;
		}
	}

	if ((flags & ATTACKFLAG_AIMFORWARD) == 0) {
		float sp178;
		float sp174;
		float sp170;
		struct prop *chrprop = chr->prop;
		struct prop *targetprop = chrGetTargetProp(chr);
		float sqdist;
		bool holdturn;
		struct coord targetpos;
		RoomNum targetrooms[8];

		sp178 = targetprop->pos.x - chrprop->pos.x;
		sp174 = targetprop->pos.y - chrprop->pos.y;
		sp170 = targetprop->pos.z - chrprop->pos.z;

		sqdist = sp178 * sp178 + sp174 * sp174 + sp170 * sp170;

		if (chr->aibot) {
			holdturn = false;
		} else if (flags & ATTACKFLAG_AIMATTARGET) {
			if (flags & ATTACKFLAG_DONTTURN) {
				holdturn = true;
			} else {
				holdturn = chrHasLosToTarget(chr);
			}
		} else {
			holdturn = true;
		}

		if ((flags & ATTACKFLAG_AIMATTARGET) && targetprop->type == PROPTYPE_PLAYER) {
			float eyeheight = g_Vars.players[playermgrGetPlayerNumByProp(targetprop)]->vv_eyeheight;

			targetpos.x = targetprop->pos.x;
			targetpos.y = targetprop->pos.y;
			targetpos.z = targetprop->pos.z;

			if (chr->aibot) {
				sp174 -= eyeheight * (0.4f + (0.05f * RANDOMFRAC() * arg4));
			} else if (chr->chrflags & CHRCFLAG_LOSEXTRAHEIGHT) {
				if (sqdist < 400.0f * 400.0f) {
					if (chrprop->pos.y < targetpos.y - 2.0f * eyeheight) {
						sp174 -= eyeheight * (0.55f + 0.1f * RANDOMFRAC() * arg4);
					} else if (chrprop->pos.y > targetpos.y - eyeheight * 0.5f) {
						sp174 -= eyeheight * (0.15f + 0.1f * RANDOMFRAC() * arg4);
					} else {
						sp174 = (RANDOMFRAC() * 0.1f * arg4 + 1.0f) * 40.0f;
					}
				} else {
					sp174 += eyeheight * (0.025f - 0.05f * RANDOMFRAC() * arg4);
				}
			} else if (sqdist > 1000.0f * 1000.0f) {
				if ((rngRandom() % 3) == 0) {
					sp174 += eyeheight * (0.05f + 0.1f * RANDOMFRAC() * arg4);
				} else {
					sp174 -= eyeheight * (0.05f + 0.55f * RANDOMFRAC() * arg4);
				}
			} else {
				if (chrprop->pos.y < targetpos.y - eyeheight) {
					sp174 -= eyeheight * (0.55f + 0.1f * RANDOMFRAC() * arg4);
				} else if (chrprop->pos.y > targetpos.y - eyeheight * 0.5f) {
					sp174 -= eyeheight * (0.15f + 0.1f * RANDOMFRAC() * arg4);
				} else {
					sp174 = (RANDOMFRAC() * 0.1f * arg4 - 0.05f) * eyeheight;
				}
			}
		} else {
			struct coord chrpos;

			modelGetRootPosition(chr->model, &chrpos);
			chrGetAttackEntityPos(chr, flags, entityid, &targetpos, targetrooms);

			sp178 = targetpos.x - chrpos.x;
			sp174 = targetpos.y - chrpos.y;
			sp170 = targetpos.z - chrpos.z;
		}

		if ((flags & ATTACKFLAG_NOVERTICAL) == 0) {
			shootrotx = atan2f(sp174, sqrtf(sp178 * sp178 + sp170 * sp170));

			if (shootrotx >= M_PI) {
				shootrotx -= M_TAU;
			}
		}

		if (holdturn) {
			float aimangle = chrGetAimAngle(chr);
			struct prop *gunprop;
			struct modelnode *posnode;
			struct model *gunmodel;
			struct coord sp118;
			int sp114;
			struct modelnode *burstnode;
			struct modelrwdata_chrinfo *chrrwdata;
			Mtx spc8;
			struct modelrodata_chrgunfire *burstrodata;
			struct coord spb8;
			Mtxf sp70;
			Mtx *sp6c;
			struct coord sp60;
			struct coord sp54;
			struct coord sp48;
			float anglev;

			if (flags & ATTACKFLAG_AIMATTARGET) {
				if (firingright) {
					gunprop = chrGetHeldProp(chr, HAND_RIGHT);
				} else {
					gunprop = chrGetHeldProp(chr, HAND_LEFT);
				}

				if (PLAYERCOUNT() == 1
						&& gunprop
						&& (gunprop->flags & PROPFLAG_ONANYSCREENPREVTICK)
						&& sqdist < 1000.0f * 1000.0f) {
					struct defaultobj *gun = gunprop->obj;
					gunmodel = gun->model;
					sp114 = 0;
					burstnode = modelGetPart(gunmodel->definition, MODELPART_CHRGUN_GUNFIRE);

					if (burstnode) {
				    	Mtx *sp108 = (Mtx*)modelFindNodeMtx(gunmodel, burstnode, 0);
						burstrodata = &burstnode->rodata->chrgunfire;
						Mtx *spb4 = camGetProjectionMtxForPlayers((uint8_t *)sp108);

						if (spb4) {
							mtxApplyAffineTransformInPlace(spb4, &spc8);

							spb8.x = burstrodata->pos.x;
							spb8.y = burstrodata->pos.y;
							spb8.z = burstrodata->pos.z;

							mtx4TransformVecInPlace(&spc8, &spb8);

							sp114 = 1;
							sp118.x = spb8.x;
							sp118.y = spb8.y;
							sp118.z = spb8.z;
						}
					} else {
						posnode = modelGetPart(gunmodel->definition, MODELPART_CHRGUN_0001);

						if (posnode) {
							Mtx *spb0 = (Mtx*)modelFindNodeMtx(gunmodel, posnode, 0);
							sp6c = camGetProjectionMtxForPlayers((uint8_t *)spb0);

							if (sp6c) {
								mtxApplyAffineTransformInPlace(sp6c, (Mtx*)&sp70);

								sp114 = 1;
								sp118.x = sp70.m[3][0];
								sp118.y = sp70.m[3][1];
								sp118.z = sp70.m[3][2];
							}
						}
					}

					if (sp114) {
						sp54.x = sinf(aimangle);
						sp54.y = 0.0f;
						sp54.z = cosf(aimangle);

						sp48.x = chrprop->pos.x - sp170;
						sp48.y = chrprop->pos.y;
						sp48.z = chrprop->pos.z + sp178;

						func0f02e3dc(&chrprop->pos, &sp48, &sp118, &sp54, &sp60);

						sp178 = targetpos.x - sp60.x;
						sp170 = targetpos.z - sp60.z;
					}
				}
			}

			anglev = atan2f(sp178, sp170);
			aimendsideback = anglev - aimangle;

			if (anglev < aimangle) {
				aimendsideback += M_TAU;
			}

			chrrwdata = modelGetNodeRwData(chr->model, chr->model->definition->rootnode);

			if (chrrwdata->unk5c > 0.0f) {
				aimendsideback -= chrrwdata->unk5c * chrrwdata->unk58;

				if (aimendsideback < 0.0f) {
					aimendsideback += M_TAU;
				}

				if (aimendsideback >= M_TAU) {
					aimendsideback -= M_TAU;
				}
			}

			if (!chr->aibot
					&& (flags & ATTACKFLAG_AIMATTARGET)
					&& ((flags & (ATTACKFLAG_AIMONLY | ATTACKFLAG_DONTTURN)) == 0)
					&& targetprop->type == PROPTYPE_PLAYER) {
				aimendsideback += chrGetAimLimitAngle(sqdist) * 0.5f * sinf((((int) (g_Vars.lvframe60 * chr->model->anim->playspeed) + chr->chrnum) % 60) * 0.10470308f);

				if (aimendsideback < 0.0f) {
					aimendsideback += M_TAU;
				}

				if (aimendsideback >= M_TAU) {
					aimendsideback -= M_TAU;
				}
			}

			if (aimendsideback >= M_PI) {
				aimendsideback -= M_TAU;
			}

			aimendsideback += chr->aimsideback;

			if (animcfg) {
				if (chr->model->anim->flip) {
					if (aimendsideback < -animcfg->unk38) {
						aimendsideback = -animcfg->unk38;
						result = false;
					} else if (aimendsideback > -animcfg->unk3c) {
						aimendsideback = -animcfg->unk3c;
						result = false;
					}
				} else {
					if (aimendsideback > animcfg->unk38) {
						aimendsideback = animcfg->unk38;
						result = false;
					} else if (aimendsideback < animcfg->unk3c) {
						aimendsideback = animcfg->unk3c;
						result = false;
					}
				}
			}
		}
	}

	chrCalculateAimEndProperties(chr, animcfg, firingleft, firingright, shootrotx);

	chr->aimendsideback = aimendsideback;
	chr->aimendcount = 10;

	return result;
}

void chrCalculateAimEndProperties(struct chrdata *chr, struct attackanimconfig *animcfg, bool firingleft, bool firingright, float shootrotx)
{
	float aimfreeshoulder = 0;
	float aimendback = 0;
	float aimgunshoulder = shootrotx;

	if (animcfg != NULL) {
		if (shootrotx > animcfg->unk30) {
			aimendback = shootrotx - animcfg->unk30;
			aimgunshoulder = animcfg->unk30;
		} else if (shootrotx < animcfg->unk34) {
			aimendback = shootrotx - animcfg->unk34;
			aimgunshoulder = animcfg->unk34;
		}

		if (aimgunshoulder > 0) {
			aimfreeshoulder = animcfg->unk40 * aimgunshoulder;
		} else {
			aimfreeshoulder = animcfg->unk44 * aimgunshoulder;
		}
	}

	if (firingright) {
		chr->aimendrshoulder = aimgunshoulder;

		if (firingleft) {
			chr->aimendlshoulder = aimgunshoulder;
		} else {
			chr->aimendlshoulder = aimfreeshoulder;
		}
	} else {
		chr->aimendrshoulder = aimfreeshoulder;
		chr->aimendlshoulder = aimgunshoulder;
	}

	chr->aimendback = aimendback;
}

void chrResetAimEndProperties(struct chrdata *chr)
{
	chr->aimendcount = 10;
	chr->aimendrshoulder = 0;
	chr->aimendlshoulder = 0;
	chr->aimendback = 0;
	chr->aimendsideback = 0;
}

void chrSetFiring(struct chrdata *chr, int hand, bool firing)
{
	struct prop *prop = chrGetHeldProp(chr, hand);

	chr->prop->forcetick = firing ? true : false;

	if (prop) {
		weaponSetGunfireVisible(prop, firing, chr->prop->rooms[0]);
	}
}

bool chrIsGunfireVisible(struct chrdata *chr, int hand)
{
	struct prop *prop = chrGetHeldProp(chr, hand);

	if (prop) {
		return weaponIsGunfireVisible(prop);
	}

	return false;
}

void chrStopFiring(struct chrdata *chr)
{
	uint8_t race = CHRRACE(chr);

	if (race != RACE_DRCAROLL && race != RACE_EYESPY && chr->aibot == NULL) {
		chrSetFiring(chr, HAND_RIGHT, false);
		chrSetFiring(chr, HAND_LEFT, false);

		chrResetAimEndProperties(chr);

		chr->fireslots[0] = bgunFreeFireslot(chr->fireslots[0]);
		chr->fireslots[1] = bgunFreeFireslot(chr->fireslots[1]);
	}
}

void chrSetHandFiring(struct chrdata *chr, int hand, bool firing)
{
	if (firing) {
		if (hand == HAND_LEFT) {
			chr->hidden |= CHRHFLAG_FIRINGLEFT;
		} else {
			chr->hidden |= CHRHFLAG_FIRINGRIGHT;
		}
	} else {
		if (hand == HAND_LEFT) {
			chr->hidden &= ~CHRHFLAG_FIRINGLEFT;
		} else {
			chr->hidden &= ~CHRHFLAG_FIRINGRIGHT;
		}
	}

	if (!firing) {
		chrSetFiring(chr, hand, false);
	}
}

float chrGetAimLimitAngle(float sqdist)
{
	if (sqdist > 1600 * 1600) {
		return 0.018752790987492f;
	}

	if (sqdist > 800 * 800) {
		return 0.03761787340045f;
	}

	if (sqdist > 400 * 400) {
		return 0.07478791475296f;
	}

	if (sqdist > 200 * 200) {
		return 0.14957582950592f;
	}

	return 0.2512874007225f;
}

/**
 * Calculate whether a chr's shot hits their target on this tick.
 *
 * Each chr maintains a shotbondsum property which is a float ranging from
 * 0 to 1. Each time this function is called, the value is increased slightly
 * based on the chr's accuracy and other factors. Once it reaches 1, the target
 * is considered to be hit and the value is reset to 0.
 *
 * The function writes to the angleokptr argument if the angle to their target
 * is within range, and writes to the hit argument to indicate if the target is
 * being hit or not.
 */
void chrCalculateHit(struct chrdata *chr, bool *angleokptr, bool *hit, struct gset *gset)
{
	struct prop *prop;
	struct prop *target;
	float xdist;
	float ydist;
	float zdist;
	float angletotarget;
	float angleaiming;
	float anglediff;
	float limitangle;
	bool angleok;
	float taperdist;
	float sqdist;

	taperdist = 300;

	// Check that the chr's aim angle is within an acceptable range to their
	// target.
	prop = chr->prop;
	target = chrGetTargetProp(chr);

	xdist = target->pos.x - prop->pos.x;
	ydist = target->pos.y - prop->pos.y;
	zdist = target->pos.z - prop->pos.z;

	angletotarget = atan2f(xdist, zdist);
	angleaiming = chrGetAimAngle(chr);
	anglediff = angletotarget - angleaiming;

	sqdist = xdist * xdist + ydist * ydist + zdist * zdist;
	limitangle = chrGetAimLimitAngle(sqdist);

	if (anglediff < 0) {
		anglediff += M_TAU;
	}

	angleok = anglediff < limitangle || anglediff > M_TAU - limitangle;

	*angleokptr = angleok;
	*hit = false;

	// Determine the distance at which accuracy starts to taper off
	switch (gset->weaponnum) {
	case WEAPON_FALCON2:
	case WEAPON_FALCON2_SILENCER:
	//case WEAPON_FALCON2_SANDS:
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
	case WEAPON_SHOTGUN:
	case WEAPON_REAPER:
	case WEAPON_DEVASTATOR:
	case WEAPON_ROCKETLAUNCHER:
	case WEAPON_SLAYER:
	case WEAPON_COMBATKNIFE:
	case WEAPON_CROSSBOW:
	case WEAPON_TRANQUILIZER:
	case WEAPON_LASER:
	case WEAPON_GRENADE:
	case WEAPON_NBOMB:
	case WEAPON_TIMEDMINE:
	case WEAPON_PROXIMITYMINE:
	case WEAPON_REMOTEMINE:
	case WEAPON_COMBATBOOST:
	case WEAPON_PP9I:
	case WEAPON_CC13:
	case WEAPON_KL01313:
	case WEAPON_KF7SPECIAL:
	case WEAPON_ZZT:
	case WEAPON_DMC:
	case WEAPON_AR53:
	case WEAPON_RCP45:
	case WEAPON_PSYCHOSISGUN:
	default:
		// Use default distance (300)
		break;
	case WEAPON_FALCON2_SCOPE:
	case WEAPON_MAGSEC4:
		taperdist = 600;
		break;
	case WEAPON_SNIPERRIFLE:
	case WEAPON_FARSIGHT:
		taperdist = 1200;
		break;
	}

	if (angleok) {
		float dist = sqrtf(xdist * xdist + ydist * ydist + zdist * zdist);
		float accuracy = 0.16f;

		// Decrease accuracy if further than taperdist
		if (dist > taperdist) {
			accuracy *= taperdist / dist;
		}

		// Scale accuracy up or down based on chr's accuracyrating
		if (chr->accuracyrating > 0) {
			accuracy *= 1 + chr->accuracyrating * 0.1f;
		} else if (chr->accuracyrating < 0) {
			if (chr->accuracyrating <= -100) {
				accuracy = 0;
			} else {
				accuracy *= (chr->accuracyrating + 100) * 0.01f;
			}
		}

		// Apply PD mode enemy accuracy setting (default 1 which is no op)
		if (pdmodeGetEnemyAccuracy() <= 1) {
			accuracy *= pdmodeGetEnemyAccuracy();
		} else {
			accuracy *= 9 / (10.001f - pdmodeGetEnemyAccuracy());
		}

		// Apply difficulty multiplier (solo A = 0.6, SA = 0.8, PA = 1.175)
		accuracy *= g_EnemyAccuracyScale;

		// If the weapon fires more than once per tick, double the value to
		// account for it. No weapons meet this criteria, however.
		if (weaponGetNumTicksPerShot(gset->weaponnum, gset->weaponfunc) <= 0) {
			accuracy += accuracy;
		}

		// Shotgun doubles the value due to more bullets
		if (gset->weaponnum == WEAPON_SHOTGUN) {
			accuracy += accuracy;
		}

		chr->shotbondsum += accuracy;

		if (chr->hidden & CHRHFLAG_PERFECTACCURACY) {
			chr->shotbondsum += 1.1f;
		}

		if (chr->shotbondsum >= 1) {
			*hit = true;
			chr->shotbondsum = 0;
		}
	}
}

/**
 * If the chr's gun is on screen, populate gunpos with the world coordinates of
 * their gun and return true.
 *
 * If the chr's gun is off screen, return false without populating gunpos.
 */
bool chrGetGunPos(struct chrdata *chr, int handnum, struct coord *gunpos)
{
	struct prop *weaponProp = chrGetHeldProp(chr, handnum);
	struct defaultobj *weaponObj;
	struct model *weaponModel;
	bool success = false;

	struct modelnode *gunfireNode;
	struct modelnode *fallbackNode;
	Mtx *gunfireNodeMtx;
	Mtx transformedMtx;
	struct modelrodata_chrgunfire *gunfireData;

	Mtx *fallbackNodeMtx;
	Mtx fallbackTransformed;

	if (!weaponProp) {
		return false;
	}

	weaponObj = weaponProp->obj;
	weaponModel = weaponObj->model;

	// Only proceed if both the character and the weapon are visible this frame
	if ((chr->prop->flags & PROPFLAG_ONTHISSCREENTHISTICK) &&
		(weaponProp->flags & PROPFLAG_ONTHISSCREENTHISTICK)) {

		// Try to get the main gun part (MODELPART_0000)
		gunfireNode = modelGetPart(weaponModel->definition, MODELPART_0000);

		if (gunfireNode) {
			gunfireNodeMtx = (Mtx *)modelFindNodeMtx(weaponModel, gunfireNode, 0);
			gunfireData = &gunfireNode->rodata->chrgunfire;

			// Start with the local offset
			gunpos->x = gunfireData->pos.x;
			gunpos->y = gunfireData->pos.y;
			gunpos->z = gunfireData->pos.z;

			// Transform to world space
			mtxApplyAffineTransform(camGetProjectionMtx(), gunfireNodeMtx, &transformedMtx);
			mtx4TransformVecInPlace(&transformedMtx, gunpos);
			success = true;

		} else {
			// Fallback: Use MODELPART_0001's matrix position
			fallbackNode = modelGetPart(weaponModel->definition, MODELPART_0001);

			if (fallbackNode) {
				fallbackNodeMtx = (Mtx *)modelFindNodeMtx(weaponModel, fallbackNode, 0);
				mtxApplyAffineTransform(camGetProjectionMtx(), fallbackNodeMtx, &fallbackTransformed);

				gunpos->x = fallbackTransformed[3][0];
				gunpos->y = fallbackTransformed[3][1];
				gunpos->z = fallbackTransformed[3][2];
				success = true;
			}
		}
	}

	return success;
}

/**
 * For a shielded chr, determine which model node has been shot and populate the
 * last 4 pointer arguments with information about the hit.
 *
 * Shield hits are calculated using the bounding box of each body part.
 * This gives a bit of padding around the chr which can register as a hit.
 *
 * outNode    - will be populated with a pointer to the bbox model node.
 * outHitPart - will be populated with the HITPART constant value.
 * outModel   - will be populated with a pointer to the chr's model.
 * outSide    - will be populated with an index in the range 0-5 which
 *              represents which side of the node's bounding box was hit.
 */
void chrCalculateShieldHit(struct chrdata *chr, struct coord *hitPos, struct coord *dirVector,
		struct modelnode **outNode, int *outHitPart, struct model **outModel, int *outSide)
{
	struct prop *prop = chr->prop;
	float bestvolume;
	struct modelnode *node;
	float volume;
	Mtx *modelMtx;

	if (prop->type != PROPTYPE_PLAYER || g_Vars.normmplayerisrunning || chrGetShield(chr) > 0) {
		if (prop->flags & (PROPFLAG_ONTHISSCREENTHISTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONANYSCREENPREVTICK)) {
			struct modelnode *closestNode = NULL;
			bestvolume = MAXFLOAT;
			Mtx* worldtoscreenmtx = camGetWorldToScreenMtx((uint8_t *)chr->model->matrices);

			if (worldtoscreenmtx) {
				struct coord hitPosScreen;
				struct coord dirVectorScreen;

				mtx4TransformVec(worldtoscreenmtx, hitPos, &hitPosScreen);
				mtx4RotateVec(worldtoscreenmtx, dirVector, &dirVectorScreen);

				node = chr->model->definition->rootnode;

				while (node) {
					if ((node->type & 0xff) == MODELNODETYPE_BBOX) {
						modelMtx = (Mtx*)modelFindNodeMtx(chr->model, node, 0);

						float x = (*modelMtx)[3][0] - hitPosScreen.x;
						float y = (*modelMtx)[3][1] - hitPosScreen.y;
						float z = (*modelMtx)[3][2] - hitPosScreen.z;

						volume = x * x + y * y + z * z;

						if (volume < bestvolume) {
							bestvolume = volume;
							closestNode = node;
						}
					}

					// Iterate all nodes recursively except headspot's chidlren
					if (node->child && (node->type & 0xff) != MODELNODETYPE_HEADSPOT) {
						node = node->child;
					} else {
						while (node) {
							if (node->next) {
								node = node->next;
								break;
							}

							node = node->parent;
						}
					}
				}

				if (closestNode != NULL)
				{
					struct modelrodata_bbox *rodata = &closestNode->rodata->bbox;

					*outHitPart = rodata->hitpart;
					*outNode = closestNode;
					*outModel = chr->model;
					*outSide = 0;

					modelMtx = (Mtx*)modelFindNodeMtx(chr->model, closestNode, 0);

					bestvolume = -2;

					float x = (dirVectorScreen.f[0] * (*modelMtx)[0][0]) + (dirVectorScreen.f[1] * (*modelMtx)[0][1]) + (dirVectorScreen.f[2] * (*modelMtx)[0][2]);
					float y = (dirVectorScreen.f[0] * (*modelMtx)[1][0]) + (dirVectorScreen.f[1] * (*modelMtx)[1][1]) + (dirVectorScreen.f[2] * (*modelMtx)[1][2]);
					float z = (dirVectorScreen.f[0] * (*modelMtx)[2][0]) + (dirVectorScreen.f[1] * (*modelMtx)[2][1]) + (dirVectorScreen.f[2] * (*modelMtx)[2][2]);

					float sides[6];
					sides[0] = x;
					sides[1] = -x;
					sides[2] = y;
					sides[3] = -y;
					sides[4] = z;
					sides[5] = -z;

					for (int i = 0; i < ARRAYCOUNT(sides); i++) {
						if (sides[i] > bestvolume) {
							bestvolume = sides[i];
							*outSide = i;
						}
					}

					return;
				}
			}
		}

		// If no node was found above, search the model for the torso bbox
		// and return that.
		node = chr->model->definition->rootnode;

		while (node) {

			if ((node->type & 0xff) == MODELNODETYPE_BBOX) {
				struct modelrodata_bbox *rodata = &node->rodata->bbox;

				if (rodata->hitpart == HITPART_TORSO) {
					*outHitPart = rodata->hitpart;
					*outNode = node;
					*outModel = chr->model;
					*outSide = 0;
					return;
				}
			}

			if (node->child) {
				node = node->child;
			} else {
				while (node) {
					if (node->next) {
						node = node->next;
						break;
					}

					node = node->parent;
				}
			}
		}
	}
}

/**
 * Calculates the trajectory for thrown items.
 */
void chrCalculateTrajectory(struct coord *fromPos, float velocityScalar, struct coord *aimPos, struct coord *outVelocity)
{
	// Apply scalar reduction to the base velocity input
	velocityScalar *= 0.6f;

	// Compute directional velocity deltas
	float dx = (aimPos->x - fromPos->x) * 0.01f;
	float dy = (aimPos->y - fromPos->y) * 0.01f;
	float dz = (aimPos->z - fromPos->z) * 0.01f;

	// Full 3D velocity magnitude and lateral (horizontal) component
	float totalSpeed = sqrtf(dx * dx + dy * dy + dz * dz);
	float lateralSpeed = sqrtf(dx * dx + dz * dz);

	float lateralFraction = lateralSpeed / totalSpeed;
	float angleToHorizontal = acosf(lateralFraction);

	// Flip angle based on direction
	if (dy < 0) {
		angleToHorizontal = -angleToHorizontal;
	}

	// Estimate launch pitch using gravity and vertical aim offset
	float gravity = 9.81f;
	float verticalRatio = (totalSpeed * gravity * lateralFraction * lateralFraction) / (velocityScalar * velocityScalar);
	verticalRatio += dy / totalSpeed;

	// Clamp asin input to [-1, 1]
	if (verticalRatio < -1.0f) verticalRatio = -1.0f;
	if (verticalRatio > 1.0f)  verticalRatio =  1.0f;

	// Adjust pitch angle based on ballistic arc
	float adjustedPitch = (asinf(verticalRatio) - angleToHorizontal) * 0.5f + angleToHorizontal;
	float cosPitch = cosf(adjustedPitch);
	float sinPitch = sinf(adjustedPitch);

	// Final direction vector
	outVelocity->x = (dx / lateralSpeed) * cosPitch;
	outVelocity->y = sinPitch;
	outVelocity->z = (dz / lateralSpeed) * cosPitch;
}

/**
 * Fire the chr's gun, check what was hit and do all the appropriate things
 * such as dealing damage, creating beams and sparks and playing sounds.
 *
 * This should be called on every frame while the chr is shooting.
 * The function takes care of the gun's fire rate.
 */
void chrTickShoot(struct chrdata *chr, int handnum)
{
	struct prop *chrprop = chr->prop;
	struct prop *gunprop;
	uint8_t isaibot = false;
	uint8_t normalshoot = true;

	if (chr->aibot) {
		isaibot = true;
	}

	gunprop = chrGetHeldProp(chr, handnum);

	if (gunprop) {
		bool firingthisframe = false;
		struct weaponobj *weapon = gunprop->weapon;
		struct gset gset;
		struct prop *targetprop = chrGetTargetProp(chr);
		uint32_t attackflags;
		bool shotdue;
		bool makebeam;
		struct coord gunpos;
		RoomNum gunrooms[8];
		struct coord hitpos;
		bool hitsomething;
		RoomNum hitrooms[8];
		bool queriedhitrooms;
		int tickspershot;

		gset = weapon->gset;
		attackflags = ATTACKFLAG_AIMATTARGET;

		if (chr->actiontype == ACT_ATTACK
				|| chr->actiontype == ACT_BOT_ATTACKSTAND
				|| chr->actiontype == ACT_BOT_ATTACKKNEEL
				|| chr->actiontype == ACT_BOT_ATTACKSTRAFE) {
			attackflags = chr->act_attack.flags;
		}

		shotdue = false;
		makebeam = false;
		hitsomething = false;
		queriedhitrooms = false;

		// Most guns can fire at most once every few ticks - even automatics.
		// The chr's firecount property tracks how many ticks have elapsed since
		// the last bullet, which is used to determine if another bullet should
		// be discharged on this tick.
		tickspershot = weaponGetNumTicksPerShot(gset.weaponnum, gset.weaponfunc);

		if (tickspershot <= 0) {
			shotdue = true;
			makebeam = true;
		} else {
			if (chr->aibot
					&& chr->aibot->weaponnum == WEAPON_REAPER
					&& chr->aibot->gunfunc == FUNC_PRIMARY) {
				float sp208 = (TICKS(90) - chr->aibot->reaperspeed[handnum]) * (1.0f / TICKS(18.0f));
				tickspershot *= 1 + sp208;
			}

			chr->firecount[handnum] += g_Vars.lvupdate60;

			if (chr->firecount[handnum] >= tickspershot) {
				chr->firecount[handnum] = 0;
				chr->unk32c_12 ^= 1 << handnum;

				shotdue = true;

				if ((chr->unk32c_12 & (1 << handnum)) || gset.weaponnum == WEAPON_LASER) {
					makebeam = true;
				}

				if (chr->actiontype == ACT_ATTACK) {
					if (modelGetAnimNum(chr->model) == ANIM_SNIPING_ONGROUND) {
						chr->act_attack.numshots++;
					}
				}
			}
		}

		if (shotdue) {
			float roty = chrGetAimAngle(chr);
			float rotx = chrGetPitchAngle(chr);
			bool extracdtypes = isaibot ? CDTYPE_PLAYERS : 0;

			firingthisframe = true;

			if (!chrGetGunPos(chr, handnum, &gunpos)) {
				// Gun is off screen - use a quick but inexact calculation
				gunpos.x = chrprop->pos.x;
				gunpos.y = chrprop->pos.y + 30;
				gunpos.z = chrprop->pos.z;

				if (handnum == HAND_LEFT) {
					gunpos.x += cosf(roty) * 10.0f;
					gunpos.z += -sinf(roty) * 10.0f;
				} else {
					gunpos.x += -cosf(roty) * 10.0f;
					gunpos.z += sinf(roty) * 10.0f;
				}
			}

			// Check that the chr isn't clipping their gun through anything such
			// as another chr or a closed door. If they are, the shot won't be
			// taken because that wouldn't be fair.
			// How nice of the developers to check for this!
			chrSetPerimEnabled(chr, false);

			if (cdTestLos10(&chrprop->pos, chrprop->rooms, &gunpos, gunrooms,
						CDTYPE_DOORS | CDTYPE_CHRS | CDTYPE_BG | CDTYPE_DOORSWITHOUTFLAG | extracdtypes,
						GEOFLAG_BLOCK_SHOOT) == CDRESULT_COLLISION) {
				firingthisframe = false;
			}

			chrSetPerimEnabled(chr, true);

			if (firingthisframe) {
				bool angleok = false;
				bool hitplayer = false;
				bool effective = true;
				int zero = 0;
				struct coord vector;
				float xdiff;
				float ydiff;
				float zdiff;
				float sqshotdist;
				struct prop *hitprop = NULL;
				uint32_t cdtypes = isaibot
					? CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_CHRS | CDTYPE_PATHBLOCKER | CDTYPE_BG | CDTYPE_DOORSWITHOUTFLAG | CDTYPE_PLAYERS
					: CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_CHRS | CDTYPE_PATHBLOCKER | CDTYPE_BG | CDTYPE_DOORSWITHOUTFLAG;
				bool isshootingeyespy = CHRRACE(targetprop->chr) == RACE_EYESPY && chrGetDistanceToTarget(chr) > 150;
				bool fudgeforeyespy = false;

				if (isshootingeyespy) {
					vector.x = targetprop->pos.x - gunpos.x;
					vector.y = targetprop->pos.y - gunpos.y;
					vector.z = targetprop->pos.z - gunpos.z;

					utilsNormalizeF(&vector.x, &vector.y, &vector.z);
					propSetPerimEnabled(targetprop, true);
				} else {
					vector.x = cosf(rotx) * sinf(roty);
					vector.y = sinf(rotx);
					vector.z = cosf(rotx) * cosf(roty);

					if (isaibot) {
						bgunCalculateBotShotSpread(&vector,
								chr->aibot->weaponnum, chr->aibot->gunfunc,
								chr->aibot->burstsdone[handnum], botGuessCrouchPos(chr),
								chr->weapons_held[0] && chr->weapons_held[1]);
					}
				}

				// Handle Farsight shots by aibots specially
				// because they can shoot through walls.
				if (chr->aibot && gset.weaponnum == WEAPON_FARSIGHT && !chr->aibot->targetinsight) {
					makebeam = true;

					// This function can never return 2 though...
					if (botactShootFarsight(chr + zero, 0, &vector, &gunpos) == 2) {
						normalshoot = rngRandom() % 255 > 200;
					}
				}

				// Check if the shot would hit anything
				hitpos.x = gunpos.x + vector.x * 65536.0f;
				hitpos.y = gunpos.y + vector.y * 65536.0f;
				hitpos.z = gunpos.z + vector.z * 65536.0f;

				chrSetPerimEnabled(chr, false);

				if (isaibot) {
					g_Vars.useperimshoot = true;
				}

				if (cdExamLos08(&gunpos, gunrooms, &hitpos, cdtypes, GEOFLAG_BLOCK_SHOOT) == CDRESULT_COLLISION) {
					hitsomething = true;
					cdGetPos(&hitpos);
					hitprop = cdGetObstacleProp();
				}

				chrSetPerimEnabled(chr, true);

				if (isaibot) {
					g_Vars.useperimshoot = false;
				}

				// Eyespy is small and hard to hit, so make it a 50/50 chance
				if (hitprop == NULL && isshootingeyespy) {
					fudgeforeyespy = rngRandom() % 100 > 50;

					if (fudgeforeyespy) {
						hitprop = targetprop;

						hitpos.x = hitprop->pos.x;
						hitpos.y = hitprop->pos.y;
						hitpos.z = hitprop->pos.z;
					}
				}

				xdiff = hitpos.x - gunpos.x;
				ydiff = hitpos.y - gunpos.y;
				zdiff = hitpos.z - gunpos.z;

				sqshotdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

				// Handle projectile launchers specially
				if (gset.weaponnum == WEAPON_ROCKETLAUNCHER
						|| gset.weaponnum == WEAPON_SLAYER
						|| (gset.weaponnum == WEAPON_SUPERDRAGON && gset.weaponfunc == FUNC_SECONDARY)
						|| gset.weaponnum == WEAPON_DEVASTATOR
						|| gset.weaponnum == WEAPON_CROSSBOW
						|| gset.weaponnum == WEAPON_ROCKETLAUNCHER_34) {
					makebeam = false;

					// Solo chrs won't fire their projectile weapon
					// in less than 4 metres of space
					if (isaibot || sqshotdist > 400.0f * 400.0f) {
						struct weaponobj *projectileobj;
						Mtxf identmtx;
						struct coord sp16c;
						float sp168;
						struct coord sp15c;
						Mtxf projectilemtx;
						Mtxf yrotmtx;
						struct weapon *weapondef = weaponFindById(gset.weaponnum);
						struct weaponfunc_shootprojectile *func = weapondef->functions[gset.weaponfunc];

						// Handle creating the projectile
						if (gset.weaponnum == WEAPON_ROCKETLAUNCHER
								|| gset.weaponnum == WEAPON_ROCKETLAUNCHER_34
								|| gset.weaponnum == WEAPON_SLAYER) {
							int rocketweaponnum = WEAPON_ROCKET;

							if (func->base.base.flags & FUNCFLAG_HOMINGROCKET) {
								rocketweaponnum = WEAPON_HOMINGROCKET;
							}

							projectileobj = weaponCreateProjectileFromWeaponNum(func->projectilemodelnum, rocketweaponnum, chr);
						} else if (gset.weaponnum == WEAPON_CROSSBOW) {
							projectileobj = weaponCreateProjectileFromWeaponNum(func->projectilemodelnum, WEAPON_BOLT, chr);

							if (projectileobj) {
								projectileobj->gunfunc = gset.weaponfunc;
							}
						} else if (gset.weaponnum == WEAPON_DEVASTATOR) {
							projectileobj = weaponCreateProjectileFromWeaponNum(func->projectilemodelnum, WEAPON_GRENADEROUND, chr);

							if (projectileobj) {
								projectileobj->gunfunc = gset.weaponfunc;
							}
						} else if (gset.weaponnum == WEAPON_SUPERDRAGON) {
							projectileobj = weaponCreateProjectileFromWeaponNum(func->projectilemodelnum, WEAPON_GRENADEROUND, chr);

							if (projectileobj) {
								projectileobj->gunfunc = FUNC_2;
							}
						} else {
							// Unreachable
							projectileobj = weaponCreateProjectileFromGset(func->projectilemodelnum, &gset, g_Vars.currentplayer->prop->chr);
						}

						if (projectileobj) {
							float spcc;

							sp168 = func->speed * (1.0f / 0.6f) / 60.0f;
							spcc = func->traveldist * (1.0f / 0.6f);

							// AI bots are a bit smarter than solo chrs
							// with regard to how they aim their projectiles
							if (isaibot && chrIsTargetInFov(chr, 30, 0)) {
								bool hasaimpos = false;
								struct coord aimpos;

								if (gset.weaponfunc == FUNC_PRIMARY &&
										(gset.weaponnum == WEAPON_ROCKETLAUNCHER
										 || gset.weaponnum == WEAPON_ROCKETLAUNCHER_34
										 || gset.weaponnum == WEAPON_SLAYER)) {
									if (targetprop->type == PROPTYPE_CHR || targetprop->type == PROPTYPE_PLAYER) {
										// Rockets - aim at target's feet
										aimpos.x = targetprop->pos.x;
										aimpos.y = targetprop->chr->manground;
										aimpos.z = targetprop->pos.z;

										vector.x = aimpos.x - gunpos.x;
										vector.y = aimpos.y - gunpos.y;
										vector.z = aimpos.z - gunpos.z;

										utilsNormalizeF(&vector.x, &vector.y, &vector.z);
										hasaimpos = true;
									}
								} else if ((gset.weaponnum == WEAPON_DEVASTATOR && gset.weaponfunc == FUNC_PRIMARY)
										|| gset.weaponnum == WEAPON_SUPERDRAGON) {
									if (targetprop->type == PROPTYPE_CHR || targetprop->type == PROPTYPE_PLAYER) {
										// Grenades - aim at target's feet
										aimpos.x = targetprop->pos.x;
										aimpos.y = targetprop->chr->manground;
										aimpos.z = targetprop->pos.z;

										chrCalculateTrajectory(&gunpos, spcc, &aimpos, &vector);
										hasaimpos = true;
									}
								} else if ((gset.weaponnum == WEAPON_DEVASTATOR && gset.weaponfunc == FUNC_SECONDARY)
										|| gset.weaponnum == WEAPON_CROSSBOW) {
									// Wall hugger grenade or crossbow - aim at target directly
									aimpos.x = targetprop->pos.x;
									aimpos.y = targetprop->pos.y;
									aimpos.z = targetprop->pos.z;

									if (targetprop->type == PROPTYPE_PLAYER) {
										aimpos.y -= 25;
									}

									chrCalculateTrajectory(&gunpos, spcc, &aimpos, &vector);
									hasaimpos = true;
								}

								if (hasaimpos) {
									float angle = chrGetAngleToPos(chr, &aimpos);
									float cos = cosf(angle);
									float sin = sinf(angle);
									float x = vector.f[0];
									float z = vector.f[2];

									vector.x = sin * z + cos * x;
									vector.z = cos * z - sin * x;
								}
							}

							// Calculate and projectile's matrix,
							// spawn position and speed
							mtxIdent((Mtx*)&identmtx);
							mtx4LoadXRotation(rotx, (Mtx*)&projectilemtx);
							mtx4LoadYRotation(roty, (Mtx*)&yrotmtx);
							mtxApplyAffineTransformInPlace((Mtx*)&yrotmtx, (Mtx*)&projectilemtx);

							sp15c.x = vector.x * sp168;
							sp15c.y = vector.y * sp168;
							sp15c.z = vector.z * sp168;

							sp16c.x = sp15c.f[0] * g_Vars.lvupdate60freal + vector.f[0] * spcc;
							sp16c.y = sp15c.f[1] * g_Vars.lvupdate60freal + vector.f[1] * spcc;
							sp16c.z = sp15c.f[2] * g_Vars.lvupdate60freal + vector.f[2] * spcc;

							projectileobj->timer240 = func->timer60;

							if (projectileobj->timer240 != -1) {
								projectileobj->timer240 *= 4;
							}

							bgunCreateXBowBolt(&projectileobj->base, &gunpos, gunrooms, &projectilemtx, &sp16c, &identmtx, chrprop, &gunpos);

							if (projectileobj->base.hidden & OBJHFLAG_PROJECTILE) {
								if (func->base.base.flags & FUNCFLAG_PROJECTILE_LIGHTWEIGHT) {
									projectileobj->base.projectile->flags |= PROJECTILEFLAG_LIGHTWEIGHT;
								} else if (func->base.base.flags & FUNCFLAG_PROJECTILE_POWERED) {
									projectileobj->base.projectile->flags |= PROJECTILEFLAG_POWERED;
								}

								projectileobj->base.projectile->unk010 = sp15c.x;
								projectileobj->base.projectile->unk014 = sp15c.y;
								projectileobj->base.projectile->unk018 = sp15c.z;

								projectileobj->base.projectile->pickuptimer240 = TICKS(240);
								projectileobj->base.projectile->bounciness = func->reflectangle;
								projectileobj->base.projectile->unk098 = func->unk50 * (1.0f / 0.6f);

								projectileobj->base.projectile->targetprop = chrGetTargetProp(chr);

								// Play sound
								if (func->soundnum > 0) {
									psCreate(NULL, projectileobj->base.prop, func->soundnum, -1,
											-1, 0, 0, PSTYPE_NONE, NULL, -1, NULL, -1, -1, -1, -1);
								}
							}
						}
					} else {
						firingthisframe = false;
					}

					normalshoot = false;
				} else if (gset.weaponnum == WEAPON_MAULER && isaibot && gset.weaponfunc == FUNC_SECONDARY) {
					gset.unk063a = (int) (chr->aibot->maulercharge[handnum] * 10.0f);
					chr->aibot->maulercharge[handnum] = 0;
				}

				if (normalshoot) {
					if (!isaibot) {
						if ((attackflags & ATTACKFLAG_AIMATTARGET)
								&& targetprop->type == PROPTYPE_PLAYER
								&& chrHasLosToAttackTarget(chr, &gunpos, gunrooms, false)
								&& chrCompareTeams(targetprop->chr, chr, COMPARE_ENEMIES)
								) {
							// Solo chr shooting at a player
							xdiff = targetprop->pos.x - gunpos.x - vector.x * 15.0f;
							ydiff = targetprop->pos.y - gunpos.y - vector.y * 15.0f;
							zdiff = targetprop->pos.z - gunpos.z - vector.z * 15.0f;

							if (xdiff * xdiff + ydiff * ydiff + zdiff * zdiff <= sqshotdist) {
								// Player has a chance of being hit
								chrCalculateHit(chr, &angleok, &hitplayer, &gset);

								// If the player was hit then turn off effective
								// (There's no need to check other props for
								// hits later on in this function)
								effective = !hitplayer;

								if (angleok
										&& (chr->actiontype == ACT_ATTACK
										 || chr->actiontype == ACT_ATTACKROLL
										 || chr->actiontype == ACT_BOT_ATTACKSTAND
										 || chr->actiontype == ACT_BOT_ATTACKKNEEL
										 || chr->actiontype == ACT_BOT_ATTACKSTRAFE)) {
									chr->act_attack.lastontarget60 = g_Vars.lvframe60;
								}
							}
						} else {
							// Solo chr shooting at something else
							if (chr->actiontype == ACT_ATTACK
									|| chr->actiontype == ACT_ATTACKROLL
									|| chr->actiontype == ACT_BOT_ATTACKSTAND
									|| chr->actiontype == ACT_BOT_ATTACKKNEEL
									|| chr->actiontype == ACT_BOT_ATTACKSTRAFE) {
								chr->act_attack.lastontarget60 = g_Vars.lvframe60;
							}
						}

						if (hitplayer) {
							float damage = gsetGetDamage(&gset);
							struct modelnode *node = NULL;
							struct model *model = NULL;
							int side = -1;
							int hitpart = HITPART_GENERAL;
							struct chrdata *targetchr = targetprop->chr;

							hitpos.x = targetprop->pos.x;
							hitpos.y = targetprop->pos.y;
							hitpos.z = targetprop->pos.z;

							if (rngRandom() % 2) {
								hitpos.y += 2 + rngRandom() % 10;
							} else {
								hitpos.y -= 2 + rngRandom() % 10;
							}

							bgunPlayPropHitSound(&gset, targetprop, -1);

							if (targetchr->model && chrGetShield(targetchr) > 0) {
								chrCalculateShieldHit(targetchr, &hitpos, &vector, &node, &hitpart, &model, &side);
							}

							func0f0341dc(targetchr, damage, &vector, &gset, chr->prop, HITPART_GENERAL, targetprop, node, model, side, NULL);
						} else if ((hitprop == NULL || (hitprop->type != PROPTYPE_CHR && hitprop->type != PROPTYPE_PLAYER))
								&& sqshotdist < 100.0f * 100.0f) {
							// Hit the background or something other than a
							// player or chr, and the shot distance was less
							// than 1 metre. Don't bother applying damage etc.
							effective = false;
						}
					}

					if (effective) {
						if (hitprop) {
							if (hitprop->type == PROPTYPE_PLAYER || hitprop->type == PROPTYPE_CHR) {
								// Hit a player or chr other than the one they
								// were aiming for
								if (isaibot
										|| fudgeforeyespy
										|| ((chr->chrflags & CHRCFLAG_NOFRIENDLYFIRE) && chrCompareTeams(hitprop->chr, chr, COMPARE_ENEMIES))) {
									struct modelnode *node = NULL;
									struct model *model = NULL;
									int side = -1;
									int hitpart = HITPART_GENERAL;
									float damage = gsetGetDamage(&gset);
									struct chrdata *hitchr = hitprop->chr;

									bgunPlayPropHitSound(&gset, hitprop, -1);

									if (hitchr->model && chrGetShield(hitchr) > 0) {
										chrCalculateShieldHit(hitchr, &hitpos, &vector, &node, &hitpart, &model, &side);
									}

									chrEmitSparks(hitchr, hitprop, hitpart, &hitpos, &vector, chr);
									func0f0341dc(hitchr, damage, &vector, &gset, chr->prop, HITPART_GENERAL, hitprop, node, model, side, NULL);
								} else {
									makebeam = false;
									firingthisframe = false;
								}
							} else if (hitprop->type == PROPTYPE_OBJ
									|| hitprop->type == PROPTYPE_WEAPON
									|| hitprop->type == PROPTYPE_DOOR) {
								// Hit an object
								struct defaultobj *hitobj = hitprop->obj;
								int playernum = -1;

								if (g_Vars.mplayerisrunning) {
									playernum = mpPlayerGetIndex(chr);
								}

								bgunPlayPropHitSound(&gset, hitprop, -1);
								propUpdatePositionRoomsSimple(&gunpos, gunrooms, &hitpos, hitrooms);
								queriedhitrooms = true;

								if (chrIsUsingPaintball(chr)) {
									sparksCreate(hitrooms[0], hitprop, &hitpos, NULL, NULL, SPARKTYPE_PAINT);
								} else {
									sparksCreate(hitrooms[0], hitprop, &hitpos, NULL, NULL, SPARKTYPE_DEFAULT);
								}

								if (g_MissionConfig.iscoop && chr->team == TEAM_ALLY
										&& (hitobj->flags2 & OBJFLAG2_IMMUNETOANTI)) {
									// Co-op can't damage mission critical objects
								} else {
									objTakeGunfire(hitobj, gsetGetDamage(&gset), &hitpos, gset.weaponnum, playernum);
								}
							}
						} else if (hitsomething) {
							// Hit the background
							propUpdatePositionRoomsSimple(&gunpos, gunrooms, &hitpos, hitrooms);
							queriedhitrooms = true;
							bgunPlayBgHitSound(&gset, &hitpos, -1, hitrooms);

							if (chrIsUsingPaintball(chr)) {
								sparksCreate(hitrooms[0], 0, &hitpos, NULL, NULL, SPARKTYPE_PAINT);
							} else {
								sparksCreate(hitrooms[0], 0, &hitpos, NULL, NULL, SPARKTYPE_DEFAULT);
							}
						}

						// Create explosion if using Phoenix
						if (gset.weaponnum == WEAPON_PHOENIX && gset.weaponfunc == FUNC_SECONDARY) {
							int playernum = chr->aibot ? mpPlayerGetIndex(chr) : g_Vars.currentplayernum;

							if (!queriedhitrooms) {
								propUpdatePositionRoomsSimple(&gunpos, gunrooms, &hitpos, hitrooms);
							}

							explosionCreateSimple(NULL, &hitpos, hitrooms, EXPLOSIONTYPE_PHOENIX, playernum);
						}
					}
				}

				if (isshootingeyespy) {
					propSetPerimEnabled(targetprop, false);
				}
			}
		}

		if (makebeam) {
			switch (gset.weaponnum) {
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
			case WEAPON_LASER:
			case WEAPON_PP9I:
			case WEAPON_CC13:
			case WEAPON_KL01313:
			case WEAPON_KF7SPECIAL:
			case WEAPON_ZZT:
			case WEAPON_DMC:
			case WEAPON_AR53:
			case WEAPON_RCP45:
				makebeam = true;
				break;
			default:
				makebeam = false;
				break;
			}
		}

		chrUpdateFireslot(chr, handnum, firingthisframe, firingthisframe && makebeam, &gunpos, &hitpos);

		if (isaibot) {
			if (firingthisframe) {
				if (chr->aibot->loadedammo[handnum] > 0) {
					chr->aibot->loadedammo[handnum]--;
				}
			}

			chrSetFiring(chr, handnum, firingthisframe && normalshoot);
		} else {
			chrSetFiring(chr, handnum, firingthisframe);
		}
	}
}

void func0f041a74(struct chrdata *chr)
{
	chr->hidden2 &= ~CHRH2FLAG_FIRESOUNDDONE;

	if (chr->actiontype == ACT_ROBOTATTACK) {
		if (chr->act_robotattack.firing[0]) {
			chr->prop->forcetick = true;
			projectileCreate(chr->prop, chr->roboguns[0], &chr->act_robotattack.pos[0],
					&chr->act_robotattack.dir[0], chr->act_robotattack.guntype[0], chrGetTargetProp(chr));
			chr->roboguns[1]->unk08 = g_Vars.lvframe60 + 2;
			chr->roboguns[1]->unk14 = chr->roboguns[0]->unk14;
		} else {
			chr->prop->forcetick = false;
		}

		if (chr->act_robotattack.firing[1]) {
			chr->prop->forcetick = true;
			projectileCreate(chr->prop, chr->roboguns[1], &chr->act_robotattack.pos[1],
					&chr->act_robotattack.dir[1], chr->act_robotattack.guntype[1], chrGetTargetProp(chr));
			chr->roboguns[0]->unk14 = chr->roboguns[1]->unk14;
		} else {
			chr->prop->forcetick = false;
		}

		beamTick(chr->roboguns[0]->beam);
		beamTick(chr->roboguns[1]->beam);
	} else if (chr->actiontype == ACT_ATTACKAMOUNT) {
		if (chr->act_attack.numshots < chr->act_attack.maxshots
				&& (chr->hidden & CHRHFLAG_FIRINGRIGHT)) {
			chrTickShoot(chr, HAND_RIGHT);
		}
	} else {
		if (chr->hidden & CHRHFLAG_FIRINGRIGHT) {
			chrTickShoot(chr, HAND_RIGHT);
			chr->hidden &= ~CHRHFLAG_FIRINGRIGHT;
		}

		if (chr->hidden & CHRHFLAG_FIRINGLEFT) {
			chrTickShoot(chr, HAND_LEFT);
			chr->hidden &= ~CHRHFLAG_FIRINGLEFT;
		}
	}
}

bool chrConfigureRoll(struct chrdata *chr)
{
	if (CHRRACE(chr) == RACE_HUMAN) {
		if (chr->act_attack.animcfg == &g_RollAttackAnims[2] || chr->act_attack.animcfg == &g_RollAttackAnims[3]) {
			struct model *model = chr->model;
			struct attackanimconfig *animcfg = &g_RollAttackAnims[1];
			bool flip = model->anim->flip;

			chr->act_attack.turning = TURNSTATE_OFF;
			chr->act_attack.animcfg = animcfg;
			chr->sleep = 0;

			modelSetAnimation(model, animcfg->animnum, flip, animcfg->unk1c, chrGetRangedSpeed(chr, 0.7f, 1.12f), 22);

			if (animcfg->unk14 >= 0) {
				modelSetAnimEndFrame(model, animcfg->unk14);
			}

			return true;
		}
	}

	return false;
}

void chrAttackAmountUpdateAnimation(struct chrdata *chr)
{
	struct model *model = chr->model;

	if (chr->act_attack.animcfg->unk24 > 0) {
		modelSetAnimation(model,
				modelGetAnimNum(model),
				model->anim->flip,
				chr->act_attack.animcfg->unk24,
				chrGetRangedSpeed(chr, 0.5f, 0.8f),
				8);
	} else {
		modelSetAnimation(model,
				modelGetAnimNum(model),
				model->anim->flip,
				chr->act_attack.animcfg->unk1c,
				chrGetRangedSpeed(chr, 0.5f, 0.8f),
				8);
	}

	if (chr->act_attack.animcfg->unk14 >= 0) {
		modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk14);
	}
}

void chrTickFire(struct chrdata *chr)
{
	struct model *model = chr->model;
	float curframe = modelGetCurAnimFrame(model);
	int i;

	if (modelGetAnimNum(model) == ANIM_SNIPING_GETDOWN) {
		return;
	}

	if (chr->act_attack.lastontarget60 < chr->act_attack.lastfire60 - TICKS(30)
			&& model->anim->animnum2 == 0
			&& curframe > chr->act_attack.animcfg->unk18 + 10
			&& curframe < chr->act_attack.animcfg->unk1c
			&& (chr->act_attack.animcfg->unk24 < 0 || curframe < chr->act_attack.animcfg->unk24)) {
		if (!chr->act_attack.dorecoil) {
			if (!chrConfigureRoll(chr)) {
				modelSetAnimation(model, modelGetAnimNum(model), model->anim->flip,
						chr->act_attack.animcfg->unk1c, chrGetRangedSpeed(chr, 0.5f, 0.8f), 8);

				if (chr->act_attack.animcfg->unk14 >= 0) {
					modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk14);
				}
			}
		} else {
			chrAttackAmountUpdateAnimation(chr);
		}

		chr->act_attack.numshots = chr->act_attack.maxshots + 1;

		curframe = modelGetCurAnimFrame(model);
	}

	if (curframe >= modelGetAnimEndFrame(model)) {
		if (modelGetAnimNum(model) != ANIM_SNIPING_ONGROUND
				&& (chr->act_attack.dooneburst || chr->act_attack.numshots > chr->act_attack.maxshots)) {
			if (!chrConfigureRoll(chr)) {
				if (chr->act_attack.flags & ATTACKFLAG_AIMATTARGET) {
					chrRecordLastSeeTargetTime(chr);
				}

				chrStop(chr);
				return;
			}
		} else if (chr->act_attack.numshots == chr->act_attack.maxshots) {
			chr->act_attack.numshots++;
			chrAttackAmountUpdateAnimation(chr);
		} else if (chr->act_attack.fired) {
			float f2 = 0.5f;
			float startframe;
			float endframe;
			float diff;

			if (chr->act_attack.dorecoil) {
				if (chr->act_attack.animcfg->unk20 > 0) {
					startframe = chr->act_attack.animcfg->unk20;
				} else {
					startframe = chr->act_attack.animcfg->unk18;
				}

				if (chr->act_attack.animcfg->unk24 > 0) {
					endframe = chr->act_attack.animcfg->unk24;
				} else {
					endframe = chr->act_attack.animcfg->unk1c;
				}
			} else {
				startframe = chr->act_attack.animcfg->unk18;

				if (chr->act_attack.animcfg->unk20 > 0) {
					endframe = chr->act_attack.animcfg->unk20;
				} else {
					endframe = chr->act_attack.animcfg->unk1c;
				}
			}

			diff = endframe - startframe;

			if (diff < 12) {
				f2 = diff * (1.0f / 24.0f);
			} else if (diff > 16) {
				f2 = diff * (1.0f / 32.0f);
			}

			if (chr->act_attack.everytick[HAND_RIGHT] && chr->act_attack.everytick[HAND_LEFT]) {
				f2 = f2 + f2;
			}

			chr->act_attack.fired = false;

			modelSetAnimation(model, modelGetAnimNum(model), model->anim->flip, startframe, f2, 8);
			modelSetAnimEndFrame(model, endframe);
		}

		curframe = modelGetCurAnimFrame(model);
	}

	if (modelGetAnimNum(model) != ANIM_SNIPING_ONGROUND && (chr->act_attack.flags & ATTACKFLAG_DONTTURN) == 0) {
		float f2 = chr->act_attack.animcfg->unk0c;
		float f12 = chr->act_attack.animcfg->unk04;

		if ((chr->act_attack.flags & ATTACKFLAG_AIMONLY) && f12 > modelGetAnimEndFrame(model)) {
			f12 = modelGetAnimEndFrame(model);
		}

		if (model->anim->flip) {
			f2 = M_TAU - f2;
		}

		chr->act_attack.turning = chrTurn(chr, chr->act_attack.turning, f12, chrGetRangedSpeed(chr, 1, 1.6f), f2);
	}

	if ((curframe > chr->act_attack.animcfg->unk28 && curframe < chr->act_attack.animcfg->unk2c)
			|| modelGetAnimNum(model) == ANIM_SNIPING_ONGROUND) {
		chrCalcAim(chr, chr->act_attack.animcfg, chr->act_attack.firegun[HAND_LEFT], chr->act_attack.firegun[HAND_RIGHT], 1);
	} else {
		chrResetAimEndProperties(chr);
	}

	for (i = 0; i < 2; i++) {
		if (chr->act_attack.firegun[i]) {
			if (chr->act_attack.everytick[i] == 0) {
				if (modelGetAnimNum(model) == ANIM_SNIPING_ONGROUND
						|| (curframe >= chr->act_attack.animcfg->unk18 && curframe < chr->act_attack.animcfg->unk1c)) {
					chrSetHandFiring(chr, i, true);

					chr->act_attack.lastfire60 = g_Vars.lvframe60;

					if (chr->actiontype == ACT_ATTACKROLL) {
						float f12 = chr->act_attack.animcfg->unk1c - chr->act_attack.animcfg->unk18;

						if (f12 < 30) {
							if (chr->act_attack.pausecount >= TICKS(60) - (int)(f12) * 2) {
								modelSetAnimSpeed(model, 0.5f, 0);
							} else {
								modelSetAnimSpeed(model, 0.1f, 0);
								chr->act_attack.pausecount += g_Vars.lvupdate60;
							}
						} else {
							modelSetAnimSpeed(model, 0.5f, 0);
						}
					} else {
						modelSetAnimSpeed(model, 0.5f, 0);
					}
				} else {
					chrSetHandFiring(chr, i, false);

					if (chr->actiontype == ACT_ATTACKROLL) {
						modelSetAnimSpeed(model, chrGetRangedSpeed(chr, 0.5f, 0.8f), 0);
					} else {
						modelSetAnimSpeed(model, chrGetRangedSpeed(chr, 0.5f, 0.8f), 0);
					}
				}
			} else if (modelGetAnimNum(model) == ANIM_SNIPING_ONGROUND
					|| ((!chr->act_attack.fired && (i == chr->act_attack.nextgun || !chr->act_attack.everytick[chr->act_attack.nextgun]))
						&& ((chr->act_attack.animcfg->unk20 >= 0 && curframe >= chr->act_attack.animcfg->unk20 && curframe <= chr->act_attack.animcfg->unk24)
							|| (chr->act_attack.animcfg->unk20 < 0 && curframe >= chr->act_attack.animcfg->unk18)))) {
				chr->act_attack.fired = true;
				chr->act_attack.nextgun = 1 - chr->act_attack.nextgun;
				chr->act_attack.numshots++;
				chr->act_attack.lastfire60 = g_Vars.lvframe60;

				chrSetHandFiring(chr, i, true);
			} else {
				chrSetHandFiring(chr, i, false);
			}
		} else {
			chrSetHandFiring(chr, i, false);
		}
	}
}

void chrTickAttackAmount(struct chrdata *chr)
{
	struct model *model = chr->model;
	float frame = modelGetCurAnimFrame(model);
	float unk0c = chr->act_attack.animcfg->unk0c;
	float unk04 = chr->act_attack.animcfg->unk04;

	chrTurn(chr, 1, unk04, chrGetRangedSpeed(chr, 1, 1.6f), unk0c);

	if (frame > chr->act_attack.animcfg->unk28
			&& frame < chr->act_attack.animcfg->unk2c) {
		chrCalcAim(chr, chr->act_attack.animcfg, false, true, 0.2f);
	} else {
		chrResetAimEndProperties(chr);
	}

	if (frame >= chr->act_attack.animcfg->unk18 && chr->act_attack.dooneburst == false) {
		chr->act_attack.dooneburst = true;
	}

	if (chr->act_attack.dooneburst) {
		if (chr->act_attack.numshots++ < chr->act_attack.maxshots) {
			chrSetHandFiring(chr, HAND_RIGHT, true);
		} else {
			chrAttackAmountUpdateAnimation(chr);
			chrSetHandFiring(chr, HAND_RIGHT, false);
		}
	} else {
		chrSetHandFiring(chr, HAND_RIGHT, false);
	}
}

/**
 * Sets the robot's muzzle flash to on or off.
 *
 * There are two muzzles, left and right, which is specified using the `right`
 * argument.
 */
void robotSetMuzzleFlash(struct chrdata *chr, bool right, bool visible)
{
	struct modelnode *node;
	union modelrwdata *rwdata;
	int partnum;

	if (right) {
		partnum = MODELPART_ROBOT_RGUNFIRE;
	} else {
		partnum = MODELPART_ROBOT_LGUNFIRE;
	}

	node = modelGetPart(chr->model->definition, partnum);

	if (node) {
		rwdata = modelGetNodeRwData(chr->model, node);
	}

	if (rwdata) {
		rwdata->chrgunfire.visible = visible;
	}
}

void robotAttack(struct chrdata *chr)
{
	uint32_t numshots = rngRandom() % 20;

	if (chr->roboguns[0] && chr->roboguns[1]) {
		chr->actiontype = ACT_ROBOTATTACK;

		chr->roboguns[0]->beam->age = -1;
		chr->roboguns[0]->unk00 = rngRandom() % 3;
		chr->roboguns[0]->unk01 = 0;
		chr->roboguns[0]->unk08 = -1;
		chr->roboguns[0]->unk0c = 0.85f;

		if ((lvGetDifficulty() == DIFF_PA) * 0.2f) {
			chr->roboguns[0]->unk10 = 2.0f;
		} else {
			chr->roboguns[0]->unk10 = 1.0f;
		}

		chr->roboguns[0]->unk14 = 0.0f;

		chr->act_robotattack.pos[0].x = 0.0f;
		chr->act_robotattack.pos[0].y = 0.0f;
		chr->act_robotattack.pos[0].z = 0.0f;
		chr->act_robotattack.dir[0].x = 0.0f;
		chr->act_robotattack.dir[0].y = 0.0f;
		chr->act_robotattack.dir[0].z = 0.0f;
		chr->act_robotattack.guntype[0] = WEAPON_WATCHLASER;
		chr->act_robotattack.firing[0] = false;

		chr->roboguns[1]->beam->age = -1;
		chr->roboguns[1]->unk00 = rngRandom() % 3;
		chr->roboguns[1]->unk01 = 0;
		chr->roboguns[1]->unk08 = -1;
		chr->roboguns[1]->unk0c = 0.85f;
		chr->roboguns[1]->unk10 = 0.2f;
		chr->roboguns[1]->unk14 = 0.0f;

		chr->act_robotattack.guntype[1] = WEAPON_WATCHLASER;
		chr->act_robotattack.firing[1] = false;
		chr->act_robotattack.finished = false;
		chr->act_robotattack.numshots[0] = numshots;
		chr->act_robotattack.numshots[1] = numshots;
		chr->act_robotattack.pos[1].x = 0.0f;
		chr->act_robotattack.pos[1].y = 0.0f;
		chr->act_robotattack.pos[1].z = 0.0f;
		chr->act_robotattack.dir[1].x = 0.0f;
		chr->act_robotattack.dir[1].y = 0.0f;
		chr->act_robotattack.dir[1].z = 0.0f;

		chrChooseStandAnimation(chr, 16);
	}
}

void func0f0429d8(struct chrdata *chr, float arg1, float arg2)
{
	struct prop *prop = chrGetTargetProp(chr);
	float distance = atan2f(prop->pos.x - chr->prop->pos.x, prop->pos.z - chr->prop->pos.z);
	float value = modelTweenRotAxis(arg2, distance, arg1);
	chrSetLookAngle(chr, value);
}

void chrTickRobotAttack(struct chrdata *chr)
{
	int i;
	float roty = 0.0f;
	float rotx = 0.0f;
	struct prop *targetprop = chrGetTargetProp(chr);
	bool firing;
	bool empty;
	float invtheta = chrGetInverseTheta(chr);
	struct act_robotattack *act = &chr->act_robotattack;

	func0f0429d8(chr, 0.085f, invtheta);

	if (chr->model->definition->skel != &g_SkelRobot) {
		act->finished = true;
		return;
	}

	for (i = 0; i < 2; i++) {
		empty = false;

		if (act->numshots[i] > 0) {
			chr->roboguns[i]->unk01 = !(chr->roboguns[i]->unk00 % 3);
			firing = !(chr->roboguns[i]->unk00 % 2);
		} else {
			chr->roboguns[i]->unk01 = 0;
			firing = false;
		}

		act->firing[i] = firing;

		if (act->numshots[0] <= 0 && act->numshots[1] <= 0) {
			empty = true;

			if (fabsf(chr->gunroty[0]) < 0.03f
					&& fabsf(chr->gunrotx[0]) < 0.03f
					&& fabsf(chr->gunroty[1]) < 0.03f
					&& fabsf(chr->gunrotx[1]) < 0.03f) {
				act->finished = true;
			}
		}

		if (empty);
		if ((float)empty);

		if (!empty) {
			float aimy;
			union modelrodata *rodata;
			struct coord spe4;
			Mtxf spa4;

			aimy = targetprop->pos.y - 20.0f;
			rodata = modelGetPartRodata(chr->model->definition, (i ? MODELPART_ROBOT_0000 : MODELPART_ROBOT_0001));

			act->pos[i].x = rodata->position.pos.x;
			act->pos[i].y = rodata->position.pos.y - 300.0f;
			act->pos[i].z = rodata->position.pos.z;

			mtx4LoadYRotation(invtheta, (Mtx*)&spa4);
			mtx4RotateVec((Mtx*)&spa4, &act->pos[i], &spe4);

			spe4.x *= chr->model->scale;
			spe4.y *= chr->model->scale;
			spe4.z *= chr->model->scale;

			act->pos[i].x = spe4.x + chr->prop->pos.x;
			act->pos[i].y = spe4.y + chr->prop->pos.y;
			act->pos[i].z = spe4.z + chr->prop->pos.z;

			roty = atan2f(targetprop->pos.x - act->pos[i].x, targetprop->pos.z - act->pos[i].z) - invtheta;

			if (roty < 0.0f) {
				roty += M_TAU;
			}

			if (roty > M_PI) {
				roty -= M_TAU;
			}

			if (roty < -0.524f) {
				roty = -0.524f;
			}

			if (roty > 0.524f) {
				roty = 0.524f;
			}

#define X() (targetprop->pos.x - act->pos[i].x)
#define Z() (targetprop->pos.z - act->pos[i].z)

			rotx = M_TAU - atan2f(aimy - act->pos[i].y, sqrtf(Z() * Z() + X() * X()));

			if (rotx > M_PI) {
				rotx -= M_TAU;
			}

			if (rotx < -0.524f) {
				rotx = -0.524f;
			}

			if (rotx > 0.524f) {
				rotx = 0.524f;
			}
		}

		chr->gunroty[i] += (roty - chr->gunroty[i]) * 0.15f;
		chr->gunrotx[i] += (rotx - chr->gunrotx[i]) * 0.15f;

		if (!empty) {
			if (firing) {
				float gunrotx = chr->gunrotx[i];
				float gunroty = chr->gunroty[i];

				if (gunrotx < 0.0f) {
					gunrotx += M_TAU;
				}

				if (gunroty < 0.0f) {
					gunroty += M_TAU;
				}

				gunroty += invtheta;

				if (gunroty >= M_TAU) {
					gunroty -= M_TAU;
				}

				act->dir[i].x = sinf(gunroty) * cosf(gunrotx);
				act->dir[i].y = -sinf(gunrotx);
				act->dir[i].z = cosf(gunroty) * cosf(gunrotx);

				robotSetMuzzleFlash(chr, i, true);

				act->numshots[i]--;
			}

			chr->roboguns[i]->unk00++;
		}
	}
}

void chrTickAttack(struct chrdata *chr)
{
	struct model *model = chr->model;
	float curframe = modelGetCurAnimFrame(model);

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		modelSetAnimation(model, chr->act_attack.animcfg->animnum, chr->act_attack.flip,
				chr->act_attack.animcfg->unk10, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);
		func0f031254(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (chr->act_attack.animcfg->animnum == ANIM_SNIPING_GETDOWN) {
		if (curframe >= modelGetAnimEndFrame(model)) {
			chr->act_attack.animcfg = var80067d70;

			modelSetAnimation(model, chr->act_attack.animcfg->animnum, chr->act_attack.flip,
					chr->act_attack.animcfg->unk10, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);
		}
	}

	if (!chr->aibot && chr->act_attack.reaim != 0) {
		if (chr->act_attack.reaim == 1) {
			float startframe;

			if (chr->act_attack.animcfg->unk24 >= 0) {
				startframe = chr->act_attack.animcfg->unk24;
			} else {
				startframe = chr->act_attack.animcfg->unk1c;
			}

			modelSetAnimation(model, modelGetAnimNum(model), model->anim->flip, startframe, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);

			if (chr->act_attack.animcfg->unk14 >= 0) {
				modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk14);
			}

			chr->act_attack.reaim = 2;
			chrResetAimEndProperties(chr);
			return;
		}

		if (chr->act_attack.reaim == 2) {
			if (curframe >= modelGetAnimEndFrame(model)) {
				chr->act_attack.flags &= ~ATTACKFLAG_DONTTURN;

				if (chr->act_attack.standing) {
					chrAttackStand(chr, chr->act_attack.flags, chr->act_attack.entityid);
				} else {
					chrAttackKneel(chr, chr->act_attack.flags, chr->act_attack.entityid);
				}
			}
			return;
		}
	}

	if (!chr->aibot && (chr->act_attack.flags & ATTACKFLAG_AIMONLY)) {
		if (chr->act_attack.flags & ATTACKFLAG_DONTTURN) {
			if (!chrCalcAim(chr, chr->act_attack.animcfg, chr->act_attack.firegun[HAND_LEFT], chr->act_attack.firegun[HAND_RIGHT], 0.2f)) {
				chr->act_attack.reaim = 1;
			}
			return;
		}

		if (curframe >= modelGetAnimEndFrame(model)) {
			chr->act_attack.flags |= ATTACKFLAG_DONTTURN;
			chr->act_attack.turning = TURNSTATE_OFF;
			return;
		}
	}

	if (!chr->aibot
			&& chr->act_attack.dorecoil == 0
			&& chr->act_attack.animcfg->unk24 > 0
			&& curframe <= chr->act_attack.animcfg->unk24
			&& curframe >= modelGetAnimEndFrame(model)) {
		modelSetAnimation(model, modelGetAnimNum(model), model->anim->flip,
				chr->act_attack.animcfg->unk24, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);

		if (chr->act_attack.dooneburst) {
			if (chr->act_attack.animcfg->unk14 >= 0) {
				modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk14);
			}
		} else {
			modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk1c);
		}
	}

	chrTickFire(chr);
}

void chrTickAttackRoll(struct chrdata *chr)
{
	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrAttackRollChooseAnimation(chr);

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	if (chr->act_attack.onehanded) {
		struct model *model = chr->model;
		float curframe = modelGetCurAnimFrame(model);

		if (chr->act_attack.animcfg == &g_RollAttackAnims[4]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[5]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[6]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[7]) {
			if (curframe >= chr->act_attack.animcfg->unk14) {
				struct attackanimconfig *newanimcfg = chr->act_attack.animcfg + 4;
				bool flip = model->anim->flip;
				float sp34 = 16;

				if (chr->act_attack.firegun[HAND_LEFT] && chr->act_attack.firegun[HAND_RIGHT]) {
					if (rngRandom() % 2 == 0) {
						newanimcfg += 4;
					} else {
						newanimcfg += 8;
					}
				}

				if (newanimcfg == &g_RollAttackAnims[8]) {
					sp34 = 24;
				} else if (newanimcfg == &g_RollAttackAnims[9]) {
					sp34 = 24;
				} else if (newanimcfg == &g_RollAttackAnims[10]) {
					sp34 = 32;
				} else if (newanimcfg == &g_RollAttackAnims[11]) {
					sp34 = 44;
				} else if (newanimcfg == &g_RollAttackAnims[12]) {
					sp34 = 24;
				} else if (newanimcfg == &g_RollAttackAnims[13]) {
					sp34 = 34;
				} else if (newanimcfg == &g_RollAttackAnims[14]) {
					sp34 = 32;
				} else if (newanimcfg == &g_RollAttackAnims[15]) {
					sp34 = 44;
				} else if (newanimcfg == &g_RollAttackAnims[16]) {
					sp34 = 24;
				} else if (newanimcfg == &g_RollAttackAnims[17]) {
					sp34 = 34;
				} else if (newanimcfg == &g_RollAttackAnims[18]) {
					sp34 = 32;
				} else if (newanimcfg == &g_RollAttackAnims[19]) {
					sp34 = 44;
				}

				chr->act_attack.turning = TURNSTATE_OFF;
				chr->act_attack.animcfg = newanimcfg;
				chr->sleep = 0;

				modelSetAnimation(model, newanimcfg->animnum, flip, newanimcfg->unk10,
						chrGetRangedSpeed(chr, 0.5f, 0.8f), sp34);

				if (chr->act_attack.dorecoil) {
					if (newanimcfg->unk24 >= 0.0f) {
						modelSetAnimEndFrame(model, newanimcfg->unk24);
					} else {
						modelSetAnimEndFrame(model, newanimcfg->unk1c);
					}
				} else {
					if (newanimcfg->unk20 >= 0.0f) {
						modelSetAnimEndFrame(model, newanimcfg->unk20);
					} else {
						if (newanimcfg->unk14 >= 0.0f) {
							modelSetAnimEndFrame(model, newanimcfg->unk14);
						}
					}
				}

				if (chr->act_attack.animcfg->unk0c != 0.0f) {
					union modelrwdata *rwdata = modelGetNodeRwData(model, model->definition->rootnode);
					rwdata->chrinfo.unk5c = sp34;
					rwdata->chrinfo.unk58 = -chr->act_attack.animcfg->unk0c / sp34;

					if (flip) {
						rwdata->chrinfo.unk58 = -rwdata->chrinfo.unk58;
					}
				}
			}
		} else if (chr->act_attack.animcfg == &g_RollAttackAnims[8]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[9]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[10]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[11]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[12]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[13]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[14]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[15]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[16]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[17]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[18]
				|| chr->act_attack.animcfg == &g_RollAttackAnims[19]) {
			if (!chr->act_attack.dorecoil
					&& chr->act_attack.animcfg->unk24 > 0
					&& curframe <= chr->act_attack.animcfg->unk24
					&& curframe >= modelGetAnimEndFrame(model)) {
				modelSetAnimation(model, modelGetAnimNum(model), model->anim->flip,
						chr->act_attack.animcfg->unk24, chrGetRangedSpeed(chr, 0.5f, 0.8f), 16);

				if (chr->act_attack.dooneburst) {
					if (chr->act_attack.animcfg->unk14 >= 0) {
						modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk14);
					}
				} else {
					modelSetAnimEndFrame(model, chr->act_attack.animcfg->unk1c);
				}
			}
		}
	}

	chrTickFire(chr);
}

void propUnsetDangerous(struct prop *prop)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_DangerousProps); i++) {
		if (g_DangerousProps[i] == prop) {
			g_DangerousProps[i] = NULL;
			return;
		}
	}
}

void propSetDangerous(struct prop *prop)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_DangerousProps); i++) {
		if (g_DangerousProps[i] == NULL) {
			g_DangerousProps[i] = prop;
			return;
		}
	}
}

void chrTickThrowGrenade(struct chrdata *chr)
{
	struct model *model;
	float frame;
	uint32_t hand;
	struct prop *weaponprop;
	struct defaultobj *obj;
	struct weaponobj *weapon;
	float frame2;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrThrowGrenadeChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	model = chr->model;
	frame = modelGetCurAnimFrame(model);
	hand = model->anim->flip ? HAND_LEFT : HAND_RIGHT;
	weaponprop = chrGetHeldProp(chr, hand);

	// Decide at which frame the grenade should become visible in the chr's hand
	if ((frame >= 20 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_STANDING)
			|| (frame >= 1 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_NOPIN)
			|| (frame >= 1 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_CROUCHING)) {
		obj = weaponprop->obj;
		obj->hidden &= ~OBJHFLAG_GONE;
	}

	// Decide at which frame the grenade leaves the chr's hand
	if ((frame >= 119 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_STANDING)
			|| (frame >= 57 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_NOPIN)
			|| (frame >= 58 && weaponprop && modelGetAnimNum(model) == ANIM_THROWGRENADE_CROUCHING)) {
		weapon = weaponprop->weapon;
		objSetDropped(weaponprop, DROPTYPE_THROWGRENADE);
		chr->hidden |= CHRHFLAG_DROPPINGITEM;
		weapon->timer240 = TICKS(240);
	}

	frame2 = modelGetCurAnimFrame(model);

	if (frame2 >= modelGetAnimEndFrame(model)) {
		chrStop(chr);
	} else {
		if ((frame >= 87 && frame <= 110 && modelGetAnimNum(model) == ANIM_THROWGRENADE_STANDING)
				|| (frame >= 5 && frame <= 45 && modelGetAnimNum(model) == ANIM_THROWGRENADE_NOPIN)
				|| ((frame >= 20 && frame <= 45 && modelGetAnimNum(model) == ANIM_THROWGRENADE_CROUCHING))) {
			float value = chrGetRangedSpeed(chr, 1, 3.2);
			chrTurn(chr, 1, 110, value, 0);
		}
	}
}

bool chrDetectDangerousObject(struct chrdata *chr, uint8_t flags)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_DangerousProps); i++) {
		struct prop *prop = g_DangerousProps[i];
		bool pass = false;

		if (prop) {
			if ((flags & 1) && prop->weapon &&
					prop->weapon->weaponnum == WEAPON_GRENADE &&
					prop->weapon->timer240 < TICKS(480)) {
				pass = true;
			}

			if ((flags & 2) && prop->type == PROPTYPE_EXPLOSION) {
				pass = true;
			}

			if (pass && chrGetSquaredDistanceToCoord(chr, &prop->pos) < 1600) {
				chr->runfrompos.x = g_DangerousProps[i]->pos.x;
				chr->runfrompos.y = g_DangerousProps[i]->pos.y;
				chr->runfrompos.z = g_DangerousProps[i]->pos.z;

				if (chr->aibot) {
					chr->aibot->flags |= BOTFLAG_AVOIDINGDANGEROUSPROP;
					chr->aibot->dangerouspropnum = i;
				}

				return true;
			}
		}
	}

	if (chr->aibot) {
		chr->aibot->flags &= ~BOTFLAG_AVOIDINGDANGEROUSPROP;
		chr->aibot->dangerouspropnum = -1;
	}

	return false;
}

void chrTickBondDie(struct chrdata *chr)
{
	// empty
}

bool func0f043f2c(struct chrdata *chr, struct coord *runpos, uint32_t arg2, float *turnspeed)
{
	struct prop *prop = chr->prop;
	struct model *model = chr->model;
	float accel;
	float maxspeed;
	float angle1;
	float finalangle;
	float angle2;
	float angle3;
	float xdiff = runpos->f[0] - prop->pos.f[0];
	float zdiff = runpos->f[2] - prop->pos.f[2];
	bool result;

	result = false;
	angle1 = atan2f(xdiff, zdiff);
	finalangle = chrGetRotY(chr);
	angle2 = angle1 - finalangle;

	if (finalangle > angle1) {
		angle2 += M_TAU;
	}

	if (chr->aibot) {
		if (chr->blurdrugamount > 0) {
			angle1 += chr->blurdrugamount * 0.00031410926021636f * sinf((g_Vars.lvframe60 % TICKS(1200)) * 0.0052351541817188f);

			if (angle1 >= M_TAU) {
				angle1 -= M_TAU;
			}

			angle1 += M_TAU;
		}

		finalangle = angle1;
		*turnspeed = 0;
		result = true;
	} else {
		angle3 = angle2;

		if (angle2 > M_PI) {
			angle3 = M_TAU - angle3;
		}

		if ((arg2 % 4) == 2) {
			maxspeed = 0.29915165901184f;
			accel = 0.014957583509386f;
		} else if ((arg2 % 4) == 1) {
			if (angle3 < 0.3926365673542f) {
				maxspeed = 0.019631829112768f;
			} else if (angle3 < 1.2564370632172f) {
				maxspeed = 0.098159141838551f;
			} else {
				maxspeed = 0.1963182836771f;
			}

			accel = 0.014957583509386f;
		} else {
			if (angle3 < 0.3926365673542f) {
				maxspeed = 0.0098159145563841f;
			} else if (angle3 < 1.2564370632172f) {
				maxspeed = 0.049079570919275f;
			} else {
				maxspeed = 0.12564370036125f;
			}

			accel = 0.0098159145563841f;
		}

		maxspeed *= model->anim->playspeed;
		accel *= model->anim->playspeed;

		applyRotation(&finalangle, angle1, turnspeed, accel, accel + accel, maxspeed);

		if (fabsf(finalangle - angle1) < 0.01f) {
			*turnspeed = 0;
			result = true;
		}
	}

	chrSetRotY(chr, finalangle);

	return result;
}

void chrTickAttackWalk(struct chrdata *chr)
{
	struct model *model = chr->model;
	struct prop *prop = chr->prop;
	struct prop *targetprop = chrGetTargetProp(chr);
	int i;
	float xdiff;
	float zdiff;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrAttackWalkChooseAnimation(chr);

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	chr->act_attackwalk.frame60count += g_Vars.lvupdate60;
	chr->lastwalk60 = g_Vars.lvframe60;

	// If stuck or time exceeded, stop
	if (chr->invalidmove == 1
			|| chr->lastmoveok60 < g_Vars.lvframe60 - TICKS(60)
			|| chr->act_attackwalk.frame60count > chr->act_attackwalk.frame60max) {
		if (modelGetCurAnimFrame(model) > modelGetNumAnimFrames(model) * 0.5f) {
			modelSetAnimSpeedAuto(model, 0, 16);
		} else {
			modelSetAnimSpeedAuto(model, modelGetNumAnimFrames(model) * 0.5f, 16);
		}

		chrRecordLastSeeTargetTime(chr);
		chrStop(chr);
		return;
	}

	// If within 3 metres of the player, stop
	xdiff = targetprop->pos.x - prop->pos.x;
	zdiff = targetprop->pos.z - prop->pos.z;

	if (xdiff < 300 && xdiff > -300 && zdiff < 300 && zdiff > -300) {
		chrRecordLastSeeTargetTime(chr);
		chrStop(chr);
		return;
	}

	if (func0f043f2c(chr, &targetprop->pos, 0, &chr->act_attackwalk.turnspeed)) {
		chr->act_attackwalk.facedtarget = true;
	}

	if (chr->act_attackwalk.frame60count > TICKS(20)) {
		chrCalcAim(chr, chr->act_attackwalk.animcfg,
				chr->act_attackwalk.firegun[HAND_LEFT],
				chr->act_attackwalk.firegun[HAND_RIGHT], 1);
	} else {
		chrResetAimEndProperties(chr);
	}

	if (chr->act_attackwalk.facedtarget && chr->act_attackwalk.frame60count > TICKS(30)) {
		for (i = 0; i < 2; i++) {
			if (chr->act_attackwalk.firegun[i]) {
				if (!chr->act_attackwalk.everytick[i]) {
					chrSetHandFiring(chr, i, true);
				} else if (chr->act_attackwalk.frame60count > chr->act_attackwalk.nextshot60
						&& (i == chr->act_attackwalk.nextgun || chr->act_attackwalk.everytick[chr->act_attackwalk.nextgun] == 0)) {
					chr->act_attackwalk.nextshot60 = chr->act_attackwalk.frame60count;

					if (chr->act_attackwalk.everytick[1 - i]) {
						if (chr->act_attackwalk.singleshot[i]) {
							chr->act_attackwalk.nextshot60 += TICKS(90);
						} else {
							chr->act_attackwalk.nextshot60 += TICKS(20);
						}
					} else {
						if (chr->act_attackwalk.singleshot[i]) {
							chr->act_attackwalk.nextshot60 += TICKS(180);
						} else {
							chr->act_attackwalk.nextshot60 += TICKS(40);
						}
					}

					chr->act_attackwalk.nextgun = 1 - chr->act_attackwalk.nextgun;

					chrSetHandFiring(chr, i, true);
				} else {
					chrSetHandFiring(chr, i, false);
				}
			} else {
				chrSetHandFiring(chr, i, false);
			}
		}
	} else {
		chrSetHandFiring(chr, HAND_LEFT, false);
		chrSetHandFiring(chr, HAND_RIGHT, false);
	}
}

/**
 * This function might be misnamed. It was named by isolating it and testing it
 * with different inputs to see what it returns, but I couldn't determine how
 * the range affects the latter part of the function.
 */
bool posIsMovingTowardsPosOrStoppedInRange(struct coord *prevpos, struct coord *moveddelta, struct coord *targetpos, float range)
{
	struct coord prevdist;
	float tmp;

	prevdist.x = targetpos->x - prevpos->x;
	prevdist.z = targetpos->z - prevpos->z;

	if (moveddelta->f[0] == 0 && moveddelta->f[2] == 0) {
		return prevdist.f[0] * prevdist.f[0] + prevdist.f[2] * prevdist.f[2] <= range * range;
	}

	tmp = moveddelta->f[0] * prevdist.f[0] + moveddelta->f[2] * prevdist.f[2];

	if (tmp > 0) {
		float sqmoveddist = moveddelta->f[0] * moveddelta->f[0] + moveddelta->f[2] * moveddelta->f[2];
		float sqprevdist = prevdist.f[0] * prevdist.f[0] + prevdist.f[2] * prevdist.f[2];

		if ((sqprevdist - range * range) * sqmoveddist <= tmp * tmp) {
			return true;
		}

		return false;
	}

	return false;
}

/**
 * Return true if:
 * - either prevpos or curpos is within the given range of targetpos, and
 * - the prev -> cur delta is moving towards the targetpos or is stopped inside
 *   the range.
 *
 * This is a lateral check, meaning the Y value is not considered.
 */
bool posIsArrivingLaterallyAtPos(struct coord *prevpos, struct coord *curpos, struct coord *targetpos, float range)
{
	struct coord moveddelta;

	if (prevpos->x <= targetpos->x - range && curpos->x <= targetpos->x - range) {
		return false;
	}

	if (prevpos->x >= targetpos->x + range && curpos->x >= targetpos->x + range) {
		return false;
	}

	if (prevpos->z <= targetpos->z - range && curpos->z <= targetpos->z - range) {
		return false;
	}

	if (prevpos->z >= targetpos->z + range && curpos->z >= targetpos->z + range) {
		return false;
	}

	moveddelta.x = curpos->x - prevpos->x;
	moveddelta.y = 0;
	moveddelta.z = curpos->z - prevpos->z;

	return posIsMovingTowardsPosOrStoppedInRange(prevpos, &moveddelta, targetpos, range);
}

/**
 * Return true if:
 * - either prevpos or curpos is within the given range of targetpos,
 * - the prev -> cur delta is moving towards the targetpos or is stopped inside
 *   the range, and
 * - either prevpos or curpos is within 150cm vertically of targetpos.
 */
bool posIsArrivingAtPos(struct coord *prevpos, struct coord *curpos, struct coord *targetpos, float range)
{
	if (prevpos->y <= targetpos->y - 150 && curpos->y <= targetpos->y - 150) {
		return false;
	}

	if (prevpos->y >= targetpos->y + 150 && curpos->y >= targetpos->y + 150) {
		return false;
	}

	return posIsArrivingLaterallyAtPos(prevpos, curpos, targetpos, range);
}

void chrTickRunPos(struct chrdata *chr)
{
	struct prop *prop = chr->prop;
	struct model *model = chr->model;
	uint32_t race = CHRRACE(chr);
	float fVar6;
	float zero;
	float fVar7;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrRunPosChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	chr->lastwalk60 = g_Vars.lvframe60;

	if (chr->invalidmove == 1
			|| g_Vars.lvframe60 - TICKS(60) > chr->lastmoveok60
			|| posIsArrivingLaterallyAtPos(&chr->prevpos, &prop->pos, &chr->act_runpos.pos, chr->act_runpos.neardist)) {
		if (race == RACE_HUMAN) {
			modelGetAnimNum(model);
		}

		zero = 0;
		fVar7 = modelGetCurAnimFrame(model);
		fVar6 = fVar7 - zero;

		if (fVar7 < 0) {
			fVar6 += modelGetNumAnimFrames(model);
		}

		if (modelGetNumAnimFrames(model) * 0.5f < fVar6) {
			zero = 0;
			modelSetAnimSpeedAuto(model, modelGetNumAnimFrames(model) - zero, 16);
		} else {
			zero = 0;
			fVar7 = modelGetNumAnimFrames(model) * 0.5f;
			fVar6 = fVar7 - zero;

			if (fVar7 < 0) {
				fVar6 += modelGetNumAnimFrames(model);
			}

			modelSetAnimSpeedAuto(model, fVar6, 16);
		}

		chrStop(chr);
		return;
	}

	func0f043f2c(chr, &chr->act_runpos.pos, 1, &chr->act_runpos.turnspeed);

	if (chr->act_runpos.eta60 > 0) {
		chr->act_runpos.eta60 -= g_Vars.lvupdate60;
	} else {
		fVar7 = 1;

		if (race == RACE_HUMAN) {
			if (modelGetAnimNum(model) == ANIM_RUNNING_ONEHANDGUN) {
				fVar7 = func0f02dff0(ANIM_RUNNING_ONEHANDGUN);
			} else {
				fVar7 = func0f02dff0(ANIM_RUNNING_TWOHANDGUN);
			}
		} else if (race == RACE_SKEDAR) {
			fVar7 = func0f02dff0(ANIM_SKEDAR_RUNNING);
		}

		chr->act_runpos.neardist += fVar7 * g_Vars.lvupdate60freal * modelGetAbsAnimSpeed(model);
	}
}

void func0f044b68(struct coord *arg0, struct coord *arg1, struct coord *arg2)
{
	struct coord sp0c;
	struct coord sp00;

	sp0c.x = arg1->x - arg0->x;
	sp0c.y = arg1->y - arg0->y;
	sp0c.z = arg1->z - arg0->z;

	sp00.x = -arg2->z;
	sp00.y = 0;
	sp00.z = arg2->x;

	if (sp00.f[0] * sp0c.f[0] + sp00.f[2] * sp0c.f[2] > 0) {
		// empty
	} else {
		sp0c.x = arg0->x;
		sp0c.y = arg0->y;
		sp0c.z = arg0->z;

		arg0->x = arg1->x;
		arg0->y = arg1->y;
		arg0->z = arg1->z;

		arg1->x = sp0c.x;
		arg1->y = sp0c.y;
		arg1->z = sp0c.z;
	}
}

/**
 * Check to see if the chr can see the next pos in their route.
 *
 * If an obstacle is found, set the leftpos and rightpos coordinates to the
 * left and right corners of the object from the chr's perspective and return
 * false.
 *
 * Return true if path ahead is clear.
 *
 * This is similar to chrNavCheckForObstacle. The difference between the two are
 * not yet understood.
 */
bool chrNavCanSeeNextPos(struct chrdata *chr, struct coord *chrpos, RoomNum *chrrooms, struct coord *aimpos, struct coord *leftpos, struct coord *rightpos, float negchrradius, float chrradius, int cdtypes, int arg9)
{
	struct coord spd4;
	float spd0;
	float spcc;
	float spc8;
	float spc4;
	float norm;
	bool spbc = false;
	bool spb8 = false;
	struct coord spac;
	struct coord spa0;
	struct coord sp94;
	struct coord sp88;
	bool result = false;
	float ymax;
	float ymin;
	float radius2;
	struct coord sp6c;
	struct coord sp60;
	RoomNum sp50[8];
	RoomNum sp40[8];
	struct prop *prop = chr->prop;

	chrGetBbox(prop, &radius2, &ymax, &ymin);

	spd4.x = aimpos->x - chrpos->x;
	spd4.y = 0.0f;
	spd4.z = aimpos->z - chrpos->z;

	if (spd4.f[0] == 0.0f && spd4.f[2] == 0.0f) {
		return true;
	}

	norm = 1.0f / sqrtf(spd4.f[0] * spd4.f[0] + spd4.f[2] * spd4.f[2]);
	spd4.x *= norm;
	spd4.z *= norm;

	spd0 = spd4.x * chrradius * 0.95f;
	spcc = spd4.z * chrradius * 0.95f;
	spc8 = spd4.x * chrradius * 1.2f;
	spc4 = spd4.z * chrradius * 1.2f;

	chrSetPerimEnabled(chr, false);

	sp6c.x = chrpos->x + spcc;
	sp6c.y = chrpos->y;
	sp6c.z = chrpos->z - spd0;
	sp60.x = (spd4.x * negchrradius) + (aimpos->x + spc4);
	sp60.y = aimpos->y;
	sp60.z = (spd4.z * negchrradius) + (aimpos->z - spc8);

	if (cdExamCylMove07(chrpos, chrrooms, &sp6c, sp50, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION
			|| cdExamCylMove03(&sp6c, sp50, &sp60, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION) {
		spbc = true;
		cdGetEdge(&spac, &spa0);
		func0f044b68(&spac, &spa0, &spd4);
	}

	sp6c.x = chrpos->x - spcc;
	sp6c.y = chrpos->y;
	sp6c.z = chrpos->z + spd0;

	sp60.x = (spd4.x * negchrradius) + (aimpos->x - spc4);
	sp60.y = aimpos->y;
	sp60.z = (spd4.z * negchrradius) + (aimpos->z + spc8);

	if (cdExamCylMove07(chrpos, chrrooms, &sp6c, sp50, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION
			|| cdExamCylMove03(&sp6c, chrrooms, &sp60, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION) {
		spb8 = true;
		cdGetEdge(&sp94, &sp88);
		func0f044b68(&sp94, &sp88, &spd4);
	}

	if (spbc && spb8) {
		func0f044b68(&spac, &sp94, &spd4);
		func0f044b68(&spa0, &sp88, &spd4);

		leftpos->x = spac.x;
		leftpos->y = spac.y;
		leftpos->z = spac.z;

		rightpos->x = sp88.x;
		rightpos->y = sp88.y;
		rightpos->z = sp88.z;
	} else if (spbc) {
		leftpos->x = spac.x;
		leftpos->y = spac.y;
		leftpos->z = spac.z;

		rightpos->x = spa0.x;
		rightpos->y = spa0.y;
		rightpos->z = spa0.z;
	} else if (spb8) {
		leftpos->x = sp94.x;
		leftpos->y = sp94.y;
		leftpos->z = sp94.z;

		rightpos->x = sp88.x;
		rightpos->y = sp88.y;
		rightpos->z = sp88.z;
	} else if (cdExamCylMove07(chrpos, chrrooms, aimpos, sp40, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION
			&& (!arg9 || cdExamCylMove01(chrpos, aimpos, chrradius, sp40, cdtypes, CHECKVERTICAL_YES, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION)) {
		result = true;
	} else {
		cdGetEdge(leftpos, rightpos);
		func0f044b68(leftpos, rightpos, &spd4);
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

/**
 * Check if the path ahead contains an obstacle that the chr should route
 * around.
 *
 * If an obstacle is found, set the leftpos and rightpos coordinates to the
 * left and right corners of the object from the chr's perspective and return
 * false.
 *
 * Return true if path ahead is clear.
 *
 * This is similar to chrNavCanSeeNextPos. The only difference is this one uses
 * the value1 and value2 variables.
 */
bool chrNavCheckForObstacle(struct chrdata *chr, struct coord *chrpos, RoomNum *chrrooms, struct coord *aimpos, struct coord *leftpos, struct coord *rightpos, float negchrradius, float chrradius, int cdtypes, bool hasobstacle)
{
	struct coord spd4;
	float spd0;
	float spcc;
	float spc8;
	float spc4;
	float norm;
	bool spbc = false;
	bool spb8 = false;
	struct coord spac;
	struct coord spa0;
	struct coord sp94;
	struct coord sp88;
	bool result = false;
	float value1;
	float value2;
	float ymax;
	float ymin;
	float radius2;
	struct coord sp6c;
	struct coord sp60;
	RoomNum sp50[8];
	RoomNum sp40[8];
	struct prop *prop = chr->prop;

	chrGetBbox(prop, &radius2, &ymax, &ymin);

	spd4.x = aimpos->x - chrpos->x;
	spd4.y = 0.0f;
	spd4.z = aimpos->z - chrpos->z;

	if (spd4.f[0] == 0.0f && spd4.f[2] == 0.0f) {
		return true;
	}

	norm = 1.0f / sqrtf(spd4.f[0] * spd4.f[0] + spd4.f[2] * spd4.f[2]);
	spd4.x *= norm;
	spd4.z *= norm;

	spd0 = spd4.x * chrradius * 0.95f;
	spcc = spd4.z * chrradius * 0.95f;
	spc8 = spd4.x * chrradius * 1.2f;
	spc4 = spd4.z * chrradius * 1.2f;

	chrSetPerimEnabled(chr, false);

	sp6c.x = chrpos->x + spcc;
	sp6c.y = chrpos->y;
	sp6c.z = chrpos->z - spd0;
	sp60.x = (spd4.x * negchrradius) + (aimpos->x + spc4);
	sp60.y = aimpos->y;
	sp60.z = (spd4.z * negchrradius) + (aimpos->z - spc8);

	if (cdExamCylMove07(chrpos, chrrooms, &sp6c, sp50, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION
			|| cdExamCylMove03(&sp6c, sp50, &sp60, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION) {
		spbc = true;
		cdGetEdge(&spac, &spa0);
		func0f044b68(&spac, &spa0, &spd4);
		value1 = cd00024e40();
	}

	sp6c.x = chrpos->x - spcc;
	sp6c.y = chrpos->y;
	sp6c.z = chrpos->z + spd0;

	sp60.x = (spd4.x * negchrradius) + (aimpos->x - spc4);
	sp60.y = aimpos->y;
	sp60.z = (spd4.z * negchrradius) + (aimpos->z + spc8);

	if (cdExamCylMove07(chrpos, chrrooms, &sp6c, sp50, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION
			|| cdExamCylMove03(&sp6c, chrrooms, &sp60, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) == CDRESULT_COLLISION) {
		spb8 = true;
		cdGetEdge(&sp94, &sp88);
		func0f044b68(&sp94, &sp88, &spd4);
		value2 = cd00024e40();
	}

	if (spbc && spb8) {
		if (value1 < value2) {
			leftpos->x = spac.x;
			leftpos->y = spac.y;
			leftpos->z = spac.z;

			rightpos->x = spa0.x;
			rightpos->y = spa0.y;
			rightpos->z = spa0.z;
		} else {
			leftpos->x = sp94.x;
			leftpos->y = sp94.y;
			leftpos->z = sp94.z;

			rightpos->x = sp88.x;
			rightpos->y = sp88.y;
			rightpos->z = sp88.z;
		}
	} else if (spbc) {
		leftpos->x = spac.x;
		leftpos->y = spac.y;
		leftpos->z = spac.z;

		rightpos->x = spa0.x;
		rightpos->y = spa0.y;
		rightpos->z = spa0.z;
	} else if (spb8) {
		leftpos->x = sp94.x;
		leftpos->y = sp94.y;
		leftpos->z = sp94.z;

		rightpos->x = sp88.x;
		rightpos->y = sp88.y;
		rightpos->z = sp88.z;
	} else if (cdExamCylMove07(chrpos, chrrooms, aimpos, sp40, cdtypes, 1, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION
			&& (!hasobstacle || cdExamCylMove01(chrpos, aimpos, chrradius, sp40, cdtypes, CHECKVERTICAL_YES, ymax - prop->pos.y, ymin - prop->pos.y) != CDRESULT_COLLISION)) {
		result = true;
	} else {
		cdGetEdge(leftpos, rightpos);
		func0f044b68(leftpos, rightpos, &spd4);
	}

	chrSetPerimEnabled(chr, true);

	return result;
}

bool chrNavTryObstacle(struct chrdata *chr, struct coord *arg1, bool arg2, struct coord *arg3, float radius, bool arg5, struct coord *nextpos, struct waydata *waydata, float arg8, int cdtypes, int arg10)
{
	struct prop *prop = chr->prop;
	struct coord sp68;
	struct coord sp5c;
	float norm;
	float angle;
	struct coord sp48;
	struct coord *sp44;
	struct coord *sp40;

	if (arg2) {
		sp44 = arg1;
		sp40 = arg3;
	} else {
		sp44 = arg3;
		sp40 = arg1;
	}

	sp68.x = arg1->x - prop->pos.x;
	sp68.y = 0.0f;
	sp68.z = arg1->z - prop->pos.z;

	if (sp68.f[0] != 0.0f || sp68.f[2] != 0.0f) {
		norm = sqrtf(sp68.f[0] * sp68.f[0] + sp68.f[2] * sp68.f[2]);

		if (norm > 0.0f) {
			norm = 1.0f / norm;
			sp68.x *= radius * norm;
			sp68.z *= radius * norm;
		} else {
			norm = 1.0f;
			sp68.z = radius * norm;
		}
	} else {
		norm = 1.0f;
		sp68.z = radius * norm;
	}

	if (radius * norm > 1.0f) {
		angle = 0.7852731347084f;
	} else {
		angle = acosf(radius * norm);
	}

	if (!arg2 && angle != 0.0f) {
		angle = M_TAU - angle;
	}

	sp48.x = -cosf(angle) * sp68.f[0] + sinf(angle) * sp68.f[2];
	sp48.y = 0.0f;
	sp48.z = -sinf(angle) * sp68.f[0] - cosf(angle) * sp68.f[2];

	sp5c.x = arg1->x + sp48.f[0];
	sp5c.y = arg1->y;
	sp5c.z = arg1->z + sp48.f[2];

	if (chrNavCanSeeNextPos(chr, &prop->pos, prop->rooms, &sp5c, sp44, sp40, arg8, chr->radius, cdtypes, 1)) {
		if (!arg5 || func0f03645c(chr, &prop->pos, prop->rooms, &sp5c, nextpos, cdtypes)) {
			if (arg10) {
				waydata->gotaimposobj = true;
				waydata->aimposobj.x = sp5c.x;
				waydata->aimposobj.y = sp5c.y;
				waydata->aimposobj.z = sp5c.z;
			} else {
				waydata->gotaimpos = true;
				waydata->aimpos.x = sp5c.x;
				waydata->aimpos.y = sp5c.y;
				waydata->aimpos.z = sp5c.z;
			}

			return true;
		}
	}

	return false;
}

/**
 * Check for doors in front of the chr and open them if possible.
 *
 * I'm guessing the coord argument is a position in front of the chr, and the
 * collision check is looking for doors between the chr and that point.
 *
 * - Multiplayer simulants can open any doors
 * - Solo chrs cannot open doors with OBJFLAG2_AICANNOTUSE
 * - In ntsc-beta, solo chrs can bring down lasers even if they have that flag
 *
 * The chr must be within 200cm of the door unless it's a laser.
 */
struct prop *chrOpenDoor(struct chrdata *chr, struct coord *rangepos)
{
	struct prop *doorprop = NULL;

	if (cdExamCylMove03(&chr->prop->pos, chr->prop->rooms, rangepos,
				CDTYPE_BG | CDTYPE_CLOSEDDOORS | CDTYPE_AJARDOORS,
				1, 0, 0) == CDRESULT_COLLISION) {
		doorprop = cdGetObstacleProp();
	}

	if (doorprop) {
		struct doorobj *door = doorprop->door;

		if (chr->aibot || (door->base.flags2 & OBJFLAG2_AICANNOTUSE) == 0)
		{
			float xdiff = doorprop->pos.x - chr->prop->pos.x;
			float zdiff = doorprop->pos.z - chr->prop->pos.z;

			if (xdiff * xdiff + zdiff * zdiff < 200 * 200 || (door->doorflags & DOORFLAG_DAMAGEONCONTACT)) {
				chrGoPosClearRestartTtl(chr);
				doorsChooseSwingDirection(chr->prop, doorprop->door);

				if (!doorCallLift(doorprop, false)) {
					doorsRequestMode(doorprop->door, DOORMODE_OPENING);
				}
			} else {
				doorprop = NULL;
			}
		} else {
			doorprop = NULL;
		}
	}

	return doorprop;
}

/**
 * Tick a chr's navigation process when they are using the "main" method of
 * navigating.
 *
 * Main refers to the chr being on screen or close to on screen, where all
 * collision checks are enabled. This is opposed to the magic method, where
 * they are off screen and the collision checks are skipped.
 *
 * The chr will generally move towards the nextpos, which is the position of the
 * next pad/corner in the route, and open doors along the way. However they must
 * also check for obstacles ahead of them such as other chrs or doors which have
 * opened into their path. If one is found, the chr routes around the obstacle
 * and then to the next pos again.
 */
void chrNavTickMain(struct chrdata *chr, struct coord *nextpos, struct waydata *waydata, bool arg3)
{
	struct prop *prop = chr->prop;
	struct coord sp100;
	struct coord spf4;
	int i;
	int cdtypes;

	// By default, do collision checks for everything except doors that AI can
	// open, but if they're blocking a door then include all doors
	cdtypes = (CDTYPE_ALL & ~CDTYPE_DOORS) | CDTYPE_DOORSLOCKEDTOAI;

	if (chr->hidden & CHRHFLAG_BLOCKINGDOOR) {
		cdtypes = CDTYPE_ALL;
	}

	for (i = 0; i < 1; i++) {
		if (0.0f);

		if (waydata->mode == WAYMODE_INIT || waydata->mode == WAYMODE_RETRY) {
			sp100.x = nextpos->x;
			sp100.y = nextpos->y;
			sp100.z = nextpos->z;

			// Check to see if the chr can see the next pad. This is almost
			// always true, but if the chr has tried to avoid an object they
			// may have gone behind a wall and can't see the pad any more.
			if (chrNavCanSeeNextPos(chr, &prop->pos, prop->rooms, &sp100, &waydata->obstacleleft, &waydata->obstacleright, -chr->radius, chr->radius, CDTYPE_PATHBLOCKER | CDTYPE_BG, arg3)) {
				// Can see the next pad
				waydata->gotaimpos = true;
				waydata->aimpos.x = sp100.x;
				waydata->aimpos.y = sp100.y;
				waydata->aimpos.z = sp100.z;
				waydata->mode = WAYMODE_HAVEAIMPOS;
			} else if (waydata->mode == WAYMODE_INIT) {
				waydata->mode = WAYMODE_LOST1;
				waydata->iter = 0;
			} else if (waydata->mode == WAYMODE_RETRY) {
				waydata->mode = WAYMODE_LOST2;
				waydata->iter = 0;
			}
		} else if (waydata->mode == WAYMODE_LOST1) {
			// Chr has tried to avoid an obstacle, and in the process can no
			// longer see the next pad. The chr will try to get back on the
			// route by navigating to either side of the obstacle they were
			// trying to avoid.
			float wantclearance = chr->radius * 1.26f;

			if (chrNavTryObstacle(chr, &waydata->obstacleleft, true, &spf4, wantclearance, true, nextpos, waydata, 0, CDTYPE_PATHBLOCKER | CDTYPE_BG, 0)) {
				// Will go to left side
				waydata->mode = WAYMODE_HAVEAIMPOS;
			} else if (chrNavTryObstacle(chr, &waydata->obstacleright, false, &spf4, wantclearance, true, nextpos, waydata, 0, CDTYPE_PATHBLOCKER | CDTYPE_BG, 0)) {
				// Will go to right side
				waydata->mode = WAYMODE_HAVEAIMPOS;
			} else {
				// Can't see the obstacle either!
				// Remain in LOST1 for 5 iterations to see if line of sight
				// comes back. If not, retry the next pad again.
				waydata->iter++;

				if (waydata->iter > 5) {
					waydata->mode = WAYMODE_RETRY;
				}
			}
		} else if (waydata->mode == WAYMODE_LOST2) {
			// Chr has gone through LOST1 without seeing the obstacle, then
			// tried to find the next pad but can't see that either. Try
			// navigating to the obstacle again, but with different arguments.
			// This is a more desparate attempt at returning to the path.
			float wantclearance = chr->radius * 1.26f;

			if (chrNavTryObstacle(chr, &waydata->obstacleleft, true, &spf4, wantclearance, false, NULL, waydata, 0, CDTYPE_PATHBLOCKER | CDTYPE_BG, 0)) {
				// Will go to left side
				waydata->mode = WAYMODE_HAVEAIMPOS;
			} else if (chrNavTryObstacle(chr, &waydata->obstacleright, false, &spf4, wantclearance, false, NULL, waydata, 0, CDTYPE_PATHBLOCKER | CDTYPE_BG, 0)) {
				// Will go to right side
				waydata->mode = WAYMODE_HAVEAIMPOS;
			} else {
				// Still can't see the obstacle using the desparate check.
				// Remain in LOST2 for 5 iterations to see if line of sight
				// comes back.
				waydata->iter++;

				if (waydata->iter > 5) {
					// Clear any references to the obstacle we were trying to
					// avoid and attempt to restart the process. I don't think
					// this is likely to work, but there's nothing else that can
					// be done.
					waydata->gotaimposobj = waydata->gotaimpos = false;
					waydata->aimposobj.x = waydata->aimpos.x = nextpos->x;
					waydata->aimposobj.y = waydata->aimpos.y = nextpos->y;
					waydata->aimposobj.z = waydata->aimpos.z = nextpos->z;
					waydata->mode = WAYMODE_INIT;
				}
			}
		} else if (waydata->mode == WAYMODE_HAVEAIMPOS) {
			// Chr is happily walking towards a position (aim pos).
			// The position could be the next pad, or it could be an arbitrary
			// position next to an obstacle that has to be routed around.
			// Either way, check that there's no new obstacles between the chr
			// and their aim pos.
			bool hasobstacle = true;

			if (!arg3
					&& nextpos->x == waydata->aimpos.x
					&& nextpos->y == waydata->aimpos.y
					&& nextpos->z == waydata->aimpos.z) {
				hasobstacle = false;
			}

			if (chrNavCheckForObstacle(chr, &prop->pos, prop->rooms, &waydata->aimpos, &waydata->obstacleleft, &waydata->obstacleright, -chr->radius, chr->radius, cdtypes, hasobstacle)) {
				// No obstacle ahead
				waydata->gotaimposobj = true;
				waydata->mode = WAYMODE_INIT;
				waydata->aimposobj.x = waydata->aimpos.x;
				waydata->aimposobj.y = waydata->aimpos.y;
				waydata->aimposobj.z = waydata->aimpos.z;
			} else {
				// Obstacle ahead
				waydata->mode = WAYMODE_NEWOBSTACLE;
				waydata->iter = 0;
			}
		} else if (waydata->mode == WAYMODE_NEWOBSTACLE) {
			// Chr has just noticed an obstacle ahead of them which they need to
			// route around. The obstacleleft and obstacleright coordinates are
			// already populated and they indicate the left and right corners of
			// the object. This part of the code decides which side to use and
			// calculates some turning angles.
			float f20;
			float spd0;
			float spcc;
			float f24 = chr->radius * 1.26f;

			f20 = atan2f(waydata->aimpos.x - prop->pos.x, waydata->aimpos.z - prop->pos.z);
			spd0 = f20 - atan2f(waydata->obstacleleft.x - prop->pos.x, waydata->obstacleleft.z - prop->pos.z);
			spcc = f20 - atan2f(waydata->obstacleright.x - prop->pos.x, waydata->obstacleright.z - prop->pos.z);

			if (spd0 < 0) {
				spd0 += M_TAU;
			}

			if (spd0 >= M_PI) {
				spd0 -= M_TAU;
			}

			if (spd0 < 0) {
				spd0 = -spd0;
			}

			if (spcc < 0) {
				spcc += M_TAU;
			}

			if (spcc >= M_PI) {
				spcc -= M_TAU;
			}

			if (spcc < 0) {
				spcc = -spcc;
			}

			if (spd0 < spcc) {
				float spc0;
				float spbc;
				float f22 = f24 * 1.1f;

				if (chrNavTryObstacle(chr, &waydata->obstacleleft, true, &spf4, f24, false, NULL, waydata, f22, cdtypes, 1)) {
					waydata->mode = WAYMODE_INIT;
					break;
				}

				spc0 = f20 - atan2f(waydata->obstacleleft.x - prop->pos.x, waydata->obstacleleft.z - prop->pos.z);
				spbc = f20 - atan2f(spf4.x - prop->pos.x, spf4.z - prop->pos.z);

				if (spc0 < 0) {
					spc0 += M_TAU;
				}

				if (spc0 >= M_PI) {
					spc0 -= M_TAU;
				}

				if (spc0 < 0) {
					spc0 = -spc0;
				}

				if (spbc < 0) {
					spbc += M_TAU;
				}

				if (spbc >= M_PI) {
					spbc -= M_TAU;
				}

				if (spbc < 0) {
					spbc = -spbc;
				}

				if (spbc < spc0) {
					if (chrNavTryObstacle(chr, &spf4, false, &spf4, f24, false, NULL, waydata, f22, cdtypes, 1)) {
						waydata->mode = WAYMODE_INIT;
						break;
					}
				}
			} else {
				float spb0;
				float spac;
				float f22 = f24 * 1.1f;

				if (chrNavTryObstacle(chr, &waydata->obstacleright, false, &spf4, f24, false, NULL, waydata, f22, cdtypes, 1)) {
					waydata->mode = WAYMODE_INIT;
					break;
				}

				spb0 = f20 - atan2f(waydata->obstacleright.x - prop->pos.x, waydata->obstacleright.z - prop->pos.z);
				spac = f20 - atan2f(spf4.x - prop->pos.x, spf4.z - prop->pos.z);

				if (spb0 < 0) {
					spb0 += M_TAU;
				}

				if (spb0 >= M_PI) {
					spb0 -= M_TAU;
				}

				if (spb0 < 0) {
					spb0 = -spb0;
				}

				if (spac < 0) {
					spac += M_TAU;
				}

				if (spac >= M_PI) {
					spac -= M_TAU;
				}

				if (spac < 0) {
					spac = -spac;
				}

				if (spac < spb0) {
					if (chrNavTryObstacle(chr, &spf4, true, &spf4, f24, false, NULL, waydata, f22, cdtypes, 1)) {
						waydata->mode = WAYMODE_INIT;
						break;
					}
				}
			}

			waydata->iter++;

			if (waydata->iter > 5) {
				waydata->gotaimposobj = false;
				waydata->mode = WAYMODE_INIT;
			}
		}
	}

	if (!waydata->gotaimposobj) {
		waydata->aimposobj.x = waydata->aimpos.x;
		waydata->aimposobj.y = waydata->aimpos.y;
		waydata->aimposobj.z = waydata->aimpos.z;
	}

	// Every 10 ticks, attempt to open any door in front of the chr
	if (waydata->age % 10 == 0) {
		struct prop *doorprop = chrOpenDoor(chr, &waydata->aimposobj);

		// Consider returning to stand animation while door is opening
		if (doorprop
				&& chr->aibot == NULL
				&& (chr->hidden & CHRHFLAG_BLOCKINGDOOR) == 0
				&& !chrGoPosIsWaiting(chr)) {
			chrChooseStandAnimation(chr, 16);
			chr->lastmoveok60 = g_Vars.lvframe60;
		}

		// Resume moving if there's no longer a door in the way
		// or if the chr is blocking the door
		if (!doorprop || (chr->hidden & CHRHFLAG_BLOCKINGDOOR)) {
			if (chr->aibot == NULL
					&& chrGoPosIsWaiting(chr)
					&& chr->liftaction != LIFTACTION_WAITINGFORLIFT
					&& chr->liftaction != LIFTACTION_WAITINGONLIFT) {
				if (chr->actiontype == ACT_PATROL) {
					chrPatrolChooseAnimation(chr);
				} else {
					chrGoPosChooseAnimation(chr);
				}
			}

			if (!doorprop) {
				chr->hidden &= ~CHRHFLAG_BLOCKINGDOOR;
			}
		}
	}

	// Handle turning and anim speed
	if (chr->actiontype == ACT_PATROL) {
		func0f043f2c(chr, &waydata->aimposobj, 0, &chr->act_patrol.turnspeed);
	} else {
		chr->unk32c_21 = chr->act_gopos.turnspeed != 0;

		func0f043f2c(chr, &waydata->aimposobj, chr->act_gopos.flags, &chr->act_gopos.turnspeed);

		if (chr->aibot == NULL && !chrGoPosIsWaiting(chr)) {
			if ((chr->act_gopos.flags & GOPOSMASK_SPEED) == GOPOSFLAG_RUN) {
				if (chr->act_gopos.turnspeed) {
					if (!chr->unk32c_21) {
						modelSetAnimSpeed(chr->model, 0.5f, 8);
					}
				} else {
					if (chr->unk32c_21) {
						if (chr->chrflags & CHRCFLAG_RUNFASTER) {
							modelSetAnimSpeed(chr->model, 0.65f, 32);
						} else {
							modelSetAnimSpeed(chr->model, 0.5f, 32);
						}
					}
				}
			} else if ((chr->act_gopos.flags & GOPOSMASK_SPEED) == GOPOSFLAG_JOG) {
				if (chr->act_gopos.turnspeed) {
					modelSetAnimSpeed(chr->model, 0.4f, 0);
				} else {
					modelSetAnimSpeed(chr->model, 0.5f, 0);
				}
			}
		}
	}
}

bool chrGoPosUpdateLiftAction(struct chrdata *chr, uint32_t curpadflags, bool arg2, bool arrivingatlift, int16_t curpadnum, int nextpadnum)
{
	bool advance = false;
	struct pad nextpad;
	uint32_t nextpadflags = 0;
	float nextground;
	float lifty;
	struct liftobj *lift;
	struct prop *liftprop = liftFindByPad(curpadnum);

	if (!liftprop) {
		return false;
	}

	lift = (struct liftobj *) liftprop->obj;

	lifty = liftGetY(lift);

	if (nextpadnum >= 0) {
		padUnpack(nextpadnum, PADFIELD_POS | PADFIELD_ROOM | PADFIELD_FLAGS, &nextpad);
		nextpadflags = nextpad.flags;
	}

	if (curpadflags & PADFLAG_AIWAITLIFT) {
		if (nextpadflags & PADFLAG_AIONLIFT) {
			if (arrivingatlift || chr->liftaction == LIFTACTION_WAITINGFORLIFT) {
				if (nextpadflags);

				// Begin entering lift if lift is under 40cm above this level
				advance = (lifty <= chr->manground + 40);

				// ...and (if solo mode) lift is over 1m under this level
				// (this logic allows MP simulants to drop down onto lifts)
				if (!g_Vars.normmplayerisrunning && advance) {
					advance = (lifty > chr->manground - 100);
				}

				// ...and if the lift has a door, is at least halfway open
				if (advance && lift->doors[lift->levelcur] && lift->doors[lift->levelcur]->frac < 0.5f) {
					advance = false;
				}
			}

			if (!advance) {
				if (arrivingatlift && chr->liftaction != LIFTACTION_WAITINGFORLIFT) {
					// Just arrived at lift
					chr->liftaction = LIFTACTION_WAITINGFORLIFT;

					chrChooseStandAnimation(chr, 16);

					if (nextpadnum >= 0) {
						// Call the lift
						chrOpenDoor(chr, &nextpad.pos);
					}
				}
			} else {
				// Enter lift
				chr->liftaction = LIFTACTION_NOTUSINGLIFT;

				if (chrGoPosIsWaiting(chr)) {
					if (chr->actiontype == ACT_PATROL) {
						chrPatrolChooseAnimation(chr);
					} else {
						chrGoPosChooseAnimation(chr);
					}
				}
			}
		} else {
			// On a wait lift pad, but the next pad in the route is not on the
			// lift, so the chr is running past the lift without using it.
			if (arrivingatlift) {
				advance = true;
				chr->liftaction = LIFTACTION_NOTUSINGLIFT;
			}
		}
	} else if (curpadflags & PADFLAG_AIONLIFT) {
		if (nextpadflags & PADFLAG_AIWAITLIFT) {
			// Waiting for door to close or lift to arrive at destination
			if (arg2 || chr->liftaction == LIFTACTION_WAITINGONLIFT) {
				// Continue waiting
				RoomNum rooms[] = {0, -1};

				rooms[0] = nextpad.room;

				nextground = cdFindFloorYColourTypeAtPos(&nextpad.pos, rooms, NULL, NULL);

				// Begin exiting lift if lift is 30cm under destination or higher
				advance = (lifty >= nextground - 30);

				// ...and (if solo) lift is under 1m above destination
				if (!g_Vars.normmplayerisrunning && advance) {
					advance = (lifty < nextground + 100);
				}

				// ...and if the lift has a door, is at least halfway open
				if (advance && lift->doors[lift->levelcur] && lift->doors[lift->levelcur]->frac < 0.5f) {
					advance = false;
				}
			}

			if (!advance) {
				if (arg2 && chr->liftaction != LIFTACTION_WAITINGONLIFT) {
					// Just arrived inside lift
					chr->liftaction = LIFTACTION_WAITINGONLIFT;
					chrChooseStandAnimation(chr, 16);
				}
			} else {
				// Start disembarking
				chr->liftaction = LIFTACTION_ONLIFT;

				if (chrGoPosIsWaiting(chr)) {
					if (chr->actiontype == ACT_PATROL) {
						chrPatrolChooseAnimation(chr);
					} else {
						chrGoPosChooseAnimation(chr);
					}
				}
			}
		} else {
			// Not our stop? Not sure why advance is true here. I guess the chr
			// can't go anywhere anyway because the next pad is above or below
			// them.
			if (arg2) {
				advance = true;
				chr->liftaction = LIFTACTION_ONLIFT;
			}
		}
	}

	return advance;
}

int chrIsUsingLift(struct chrdata *chr)
{
	return (chr->actiontype == ACT_GOPOS || chr->actiontype == ACT_PATROL) && chr->liftaction > 0;
}

int16_t chrGoPosGetNextPadNum(struct chrdata *chr)
{
	if (chr->act_gopos.waypoints[chr->act_gopos.curindex + 1]) {
		return chr->act_gopos.waypoints[chr->act_gopos.curindex + 1]->padnum;
	}

	return -1;
}

void chrTickGoPos(struct chrdata *chr)
{
	struct waypoint *waypoint;
	struct coord nextpos;
	RoomNum nextrooms[8];
	struct prop *prop = chr->prop;
	bool enteringmagic = false;
	struct pad pad;
	bool sp240 = true;
	struct coord curwppos;
	RoomNum curwprooms[8];
	uint32_t curwpflags;

	chr->act_gopos.flags &= ~(GOPOSFLAG_CROUCH | GOPOSFLAG_DUCK);

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrGoPosChooseAnimation(chr);

		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	chr->act_gopos.waydata.age++;

	// If stuck for 1 second
	if (chr->lastmoveok60 < g_Vars.lvframe60 - TICKS(60)) {
		if (chr->goposforce >= 0) {
			// Try and warp the chr past whatever obstacle is blocking them?
			struct coord sp196 = {0, 0, 0};

			chrDamageByMisc(chr, 1, &sp196, NULL, NULL);

			chr->lastmoveok60 = g_Vars.lvframe60;
			return;
		}

		// Goposforce was not set - restart the action to try and find a new route
		chrGoToRoomPos(chr, &chr->act_gopos.endpos, chr->act_gopos.endrooms, chr->act_gopos.flags);
	}

	chrGoPosConsiderRestart(chr);
	chrGoPosGetCurWaypointInfoWithFlags(chr, &curwppos, curwprooms, &curwpflags);

	// If magic mode ended over 3 seconds ago, not multiplayer, not in view of
	// eyespy, pad is nothing special and not in lift, then enter the magic move
	// mode.
	if (chr->act_gopos.waydata.mode != WAYMODE_MAGIC
			&& chr->act_gopos.waydata.lastvisible60 + TICKS(180) < g_Vars.lvframe60
			&& g_Vars.normmplayerisrunning == false
			&& chrIsRoomOffScreen(chr, &curwppos, curwprooms) // related to eyespy
			&& (curwpflags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT)) == 0
			&& chr->inlift == false) {
		enteringmagic = true;
		chrGoPosInitMagic(chr, &chr->act_gopos.waydata, &curwppos, &prop->pos);
	}

	if (g_NumChrsSeenPlayerRecently2 >= 9
			&& (chr->hidden & CHRHFLAG_BASICGUARD)
			&& (chr->flags & CHRFLAG0_ACTIVATEALARM) == 0) {
		chrStop(chr);
		return;
	}

	// If goposforce is set then decrease it on each tick. If it's reached -1
	// then stop the chr. I guess goposforce is not only used to warp past
	// obstacles, but is also used as a run countdown timer.
	if (chr->goposforce >= 0) {
		chr->goposforce -= g_Vars.lvupdate60;

		if (chr->goposforce < 0) {
			chrStop(chr);
			return;
		}
	}

	// Check if chr needs to exit magic mode
	if (chr->act_gopos.waydata.mode == WAYMODE_MAGIC) {
		if ((!enteringmagic && ((prop->flags & (PROPFLAG_ONANYSCREENPREVTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONTHISSCREENTHISTICK)) || !chrIsRoomOffScreen(chr, &curwppos, curwprooms)))
				|| (curwpflags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT))
				|| chr->inlift) {
			// Exiting magic mode
			chrGoPosInitExpensive(chr);
			chr->act_gopos.waydata.lastvisible60 = g_Vars.lvframe60;
			return;
		}

		chrNavTickMagic(chr, &chr->act_gopos.waydata, func0f0370a8(chr), &curwppos, curwprooms);
	} else {
		bool advance = false;
		bool arrivingxyz;
		bool arrivingxz;
		float sp180;
		float sp176;
		float sp172;
		float sp168;
		bool candosomething;
		float sp160;
		float sp156;
		struct waypoint *next;
		struct pad pad2;

		waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];

		if (waypoint) {
			padUnpack(waypoint->padnum, PADFIELD_FLAGS | PADFIELD_POS, &pad);

			arrivingxyz = posIsArrivingAtPos(&chr->prevpos, &prop->pos, &pad.pos, 30);
			arrivingxz = posIsArrivingLaterallyAtPos(&chr->prevpos, &prop->pos, &pad.pos, 30);

			if (pad.flags & PADFLAG_AICROUCH) {
				chr->act_gopos.flags |= GOPOSFLAG_CROUCH;
			} else if (pad.flags & PADFLAG_AIDUCK) {
				chr->act_gopos.flags |= GOPOSFLAG_DUCK;
			}

			if ((pad.flags & PADFLAG_AIWAITLIFT) || (pad.flags & PADFLAG_AIONLIFT)) {
				advance = chrGoPosUpdateLiftAction(chr, pad.flags, arrivingxz, arrivingxyz, waypoint->padnum, chrGoPosGetNextPadNum(chr));
			} else {
				if (arrivingxyz || (arrivingxz && (chr->inlift || (pad.flags & PADFLAG_AIIGNOREY)))) {
					advance = true;
				}
			}
		} else {
			// No more waypoints - chr is finished
			if (posIsArrivingAtPos(&chr->prevpos, &prop->pos, &chr->act_gopos.endpos, 30) ||
					(chr->inlift && posIsArrivingLaterallyAtPos(&chr->prevpos, &prop->pos, &chr->act_gopos.endpos, 30))) {
				if (chr->act_gopos.flags & GOPOSFLAG_FORPATHSTART) {
					chrTryStartPatrol(chr);
					return;
				}

				chrStop(chr);
				return;
			}
		}

		if (advance) {
			chrGoPosAdvanceWaypoint(chr);
		}

		// Every 10 ticks: Check something a couple of waypoints ahead
		// This might be checking if the chr has line of sight to that pad and can
		// walk straight to it.
		if (chr->act_gopos.waydata.age % 10 == 5 || (chr->act_gopos.flags & GOPOSFLAG_INIT)) {
			// Load waypoint that the chr is running to
			waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];

			if (waypoint) {
				padUnpack(waypoint->padnum, PADFIELD_FLAGS, &pad);

				if ((pad.flags & PADFLAG_AIWALKDIRECT) == 0) {
					// The waypoint the chr is running to doesn't have
					// PADFLAG_AIWALKDIRECT, so the chr is able to ignore it and run
					// towards the next one if it's in sight.

					// Load the next waypoint after the one the chr is running to
					waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex + 1];

					if (waypoint) {
						padUnpack(waypoint->padnum, PADFIELD_FLAGS, &pad);

						if ((pad.flags & PADFLAG_AIWALKDIRECT) == 0) {
							// And this one doesn't have PADFLAG_AIWALKDIRECT either,
							// so the chr can consider skipping this one too.
							waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex + 2];

							if (waypoint) {
								padUnpack(waypoint->padnum, PADFIELD_ROOM | PADFIELD_POS, &pad);

								nextpos.x = pad.pos.x;
								nextpos.y = pad.pos.y;
								nextpos.z = pad.pos.z;

								nextrooms[0] = pad.room;
								nextrooms[1] = -1;
							} else {
								nextpos.x = chr->act_gopos.endpos.x;
								nextpos.y = chr->act_gopos.endpos.y;
								nextpos.z = chr->act_gopos.endpos.z;

								roomsCopy(chr->act_gopos.endrooms, nextrooms);
							}

							// Some bbox related check
							if (func0f03654c(chr, &prop->pos, prop->rooms, &nextpos, nextrooms, NULL, chr->radius * 1.2f, CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
								chrGoPosAdvanceWaypoint(chr);
								chrGoPosAdvanceWaypoint(chr);
							}
						}
					}
				}
			}
		}

		if (chr->act_gopos.waydata.age % 10 == 0 || (chr->act_gopos.flags & GOPOSFLAG_INIT)) {
			waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];

			if (waypoint) {
				candosomething = (chr->act_gopos.flags & GOPOSFLAG_INIT) != 0;
				padUnpack(waypoint->padnum, PADFIELD_FLAGS | PADFIELD_POS, &pad);

				next = chr->act_gopos.waypoints[chr->act_gopos.curindex + 1];

				if (next) {
					padUnpack(next->padnum, PADFIELD_ROOM | PADFIELD_POS, &pad2);

					if ((pad.flags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT))
							&& (pad2.flags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT))) {
						candosomething = false;
					}
				}

				if ((pad.flags & PADFLAG_AIWALKDIRECT) == 0 || candosomething) {
					if (next) {
						nextpos.x = pad2.pos.x;
						nextpos.y = pad2.pos.y;
						nextpos.z = pad2.pos.z;

						nextrooms[0] = pad2.room;
						nextrooms[1] = -1;
					} else {
						nextpos.x = chr->act_gopos.endpos.x;
						nextpos.y = chr->act_gopos.endpos.y;
						nextpos.z = chr->act_gopos.endpos.z;

						roomsCopy(chr->act_gopos.endrooms, nextrooms);
					}

					// I suspect this is making the chr turn to face the next pad
					if ((pad.flags & PADFLAG_AIWALKDIRECT) && candosomething) {
						if (true) {
							sp180 = prop->pos.x - pad.pos.x;
							sp176 = prop->pos.z - pad.pos.z;
							sp172 = nextpos.x - pad.pos.x;
							sp168 = nextpos.z - pad.pos.z;
						}

						sp156 = sqrtf((sp180 * sp180 + sp176 * sp176) * (sp172 * sp172 + sp168 * sp168));

						if (sp156 > 0) {
							sp160 = acosf((sp180 * sp172 + sp176 * sp168) / sp156);

							// sp160 < DEG2RAD(45) || sp160 > DEG2RAD(315)
							if (sp160 < 0.7852731347084f || sp160 > 5.4969120025635f) {
								if (func0f03654c(chr, &prop->pos, prop->rooms, &nextpos, nextrooms, NULL, chr->radius * 1.2f, CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
									chrGoPosAdvanceWaypoint(chr);
								}
							}
						}
					} else {
						if (func0f03654c(chr, &prop->pos, prop->rooms, &nextpos, nextrooms, NULL, chr->radius * 1.2f, CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
							chrGoPosAdvanceWaypoint(chr);
						}
					}
				}
			}

			chr->act_gopos.flags &= ~GOPOSFLAG_INIT;
		}

		waypoint = chr->act_gopos.waypoints[chr->act_gopos.curindex];

		if (waypoint) {
			padUnpack(waypoint->padnum, PADFIELD_POS, &pad);

			nextpos.x = pad.pos.x;
			nextpos.y = pad.pos.y;
			nextpos.z = pad.pos.z;
		} else {
			nextpos.x = chr->act_gopos.endpos.x;
			nextpos.y = chr->act_gopos.endpos.y;
			nextpos.z = chr->act_gopos.endpos.z;

			if (chr->aibot && chr->myaction == MA_AIBOTGETITEM) {
				sp240 = false;
			}
		}

		chrNavTickMain(chr, &nextpos, &chr->act_gopos.waydata, sp240);
	}
}

void chrTickPatrol(struct chrdata *chr)
{
	struct prop *prop = chr->prop;
	bool enteringmagic;
	uint32_t flags = 0;
	struct coord sp58;
	RoomNum sp48[8];
	int arrivinglaterally;
	int arriving;
	bool advance;

	enteringmagic = 0;

	if (chr->hidden & CHRHFLAG_NEEDANIM) {
		if (modelIsAnimMerging(chr->model)) {
			return;
		}

		chrPatrolChooseAnimation(chr);
		chr->hidden &= ~CHRHFLAG_NEEDANIM;
	}

	chrPatrolGetCurWaypointInfoWithFlags(chr, &sp58, sp48, &flags);

	chr->act_patrol.waydata.age++;

	// Consider starting magic
	if (chr->act_patrol.waydata.mode != WAYMODE_MAGIC
			&& g_Vars.lvframe60 > chr->act_patrol.waydata.lastvisible60 + TICKS(180)
			&& !g_Vars.normmplayerisrunning
			&& chrIsRoomOffScreen(chr, &sp58, sp48)
			&& (flags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT)) == 0
			&& !chr->inlift) {
		enteringmagic = true;
		chrGoPosInitMagic(chr, &chr->act_patrol.waydata, &sp58, &prop->pos);
	}

	if (chr->act_patrol.waydata.mode == WAYMODE_MAGIC) {
		if ((!enteringmagic && ((prop->flags & (PROPFLAG_ONTHISSCREENTHISTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONANYSCREENPREVTICK)) || !chrIsRoomOffScreen(chr, &sp58, sp48)))
				|| (flags & (PADFLAG_AIWAITLIFT | PADFLAG_AIONLIFT))
				|| chr->inlift) {
			// Exit magic for lifts
			chr->act_patrol.waydata.lastvisible60 = g_Vars.lvframe60;
			func0f037580(chr);
		} else {
			// Continue magic
			chrNavTickMagic(chr, &chr->act_patrol.waydata, func0f0370a8(chr), &sp58, sp48);
		}

		footstepCheckMagic(chr);
	} else {
		arrivinglaterally = posIsArrivingLaterallyAtPos(&chr->prevpos, &prop->pos, &sp58, 30);
		arriving = posIsArrivingAtPos(&chr->prevpos, &prop->pos, &sp58, 30);
		advance = false;

		if ((flags & PADFLAG_AIWAITLIFT) || (flags & PADFLAG_AIONLIFT)) {
			advance = chrGoPosUpdateLiftAction(chr, flags, arrivinglaterally, arriving,
					chrPatrolCalculatePadNum(chr, 0),
					chrPatrolCalculatePadNum(chr, 1));
		} else if (arriving) {
			advance = true;
		}

		if (advance) {
			func0f0375b0(chr);
			chrPatrolGetCurWaypointInfo(chr, &sp58, sp48);
		}

		chrNavTickMain(chr, &sp58, &chr->act_patrol.waydata, true);
	}
}

bool chrTrySkJump(struct chrdata *chr, uint8_t arg1, uint8_t arg2, int arg3, uint8_t arg4)
{
	if (chr && chr->actiontype != ACT_SKJUMP
			&& chrIsReadyForOrders(chr)
			&& CHRRACE(chr) == RACE_SKEDAR) {
		return chrStartSkJump(chr, arg1, arg2, arg3, arg4);
	}

	return false;
}

bool chrStartSkJump(struct chrdata *chr, uint8_t arg1, uint8_t arg2, int arg3, uint8_t arg4)
{
	float radius;
	float ymax;
	float ymin;
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);
	bool iVar2;
	float distance = chrGetDistanceToCoord(chr, &target->pos);
	float diffs[2];
	float thing;
	int time60;

	if (distance < 200 || distance > 550 || !target) {
		return false;
	}

	chrGetBbox(prop, &radius, &ymax, &ymin);
	chrSetPerimEnabled(chr, false);
	propSetPerimEnabled(target, false);
	iVar2 = cdTestCylMove01(&prop->pos, prop->rooms, &target->pos,
			CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG,
			1, ymax - prop->pos.y, ymin - prop->pos.y);
	chrSetPerimEnabled(chr, true);
	propSetPerimEnabled(target, true);

	if (iVar2) {
		diffs[0] = target->pos.x - chr->prop->pos.x;
		diffs[1] = target->pos.z - chr->prop->pos.z;
		thing = sqrtf(diffs[0] * diffs[0] + diffs[1] * diffs[1]) * 2.5f / 21.0f;
		time60 = thing;

		if (time60 < TICKS(10)) {
			time60 = TICKS(10);
		}

		chr->act_skjump.vel[0] = diffs[0] / time60;
		chr->act_skjump.vel[1] = diffs[1] / time60;
		chr->act_skjump.roty = chrGetInverseTheta(chr) + chrGetAngleToPos(chr, &target->pos);
		chr->act_skjump.hit = false;
		chr->act_skjump.timer60 = time60;
		chr->act_skjump.total60 = time60;
		chr->act_skjump.ground = cdFindGroundAtCyl(&chr->prop->pos, chr->radius, chr->prop->rooms, NULL, NULL);
	} else {
		return false;
	}

	chr->actiontype = ACT_SKJUMP;
	chr->act_skjump.needsnewanim = true;
	chr->act_skjump.state = SKJUMPSTATE_TAKEOFF;

	return true;
}

void chrTickSkJump(struct chrdata *chr)
{
	if (g_Vars.lvupdate60 == 0) {
		return;
	}

	if (chr->act_skjump.needsnewanim) {
		chr->act_skjump.needsnewanim = false;

		switch (chr->act_skjump.state) {
		case SKJUMPSTATE_TAKEOFF:
			modelSetAnimation(chr->model, ANIM_SKEDAR_JUMPSTART, 0, 0, -1, 8);
			modelSetAnimSpeed(chr->model, 2.5, 0);
			break;
		case SKJUMPSTATE_AIRBORNE: {
				uint16_t sounds[] = {
					SFX_SKEDAR_ROAR_0532,
					SFX_SKEDAR_ROAR_0533,
					SFX_SKEDAR_ROAR_0534,
				};

				psCreate(NULL, chr->prop, sounds[rngRandom() % 3], -1,
						-1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
				modelSetAnimation(chr->model, ANIM_SKEDAR_JUMPAIR, 0, 0, -1, 16);
				modelSetAnimSpeed(chr->model, 1, 0);
			}
			break;
		}
	}

	{
		float fVar5;
		float fVar6;
		float fVar7;
		float frame;

		switch (chr->act_skjump.state) {
		case SKJUMPSTATE_TAKEOFF:
			fVar6 = chrGetInverseTheta(chr);
			fVar5 = modelTweenRotAxis(fVar6, chr->act_skjump.roty, 0.35);
			chrSetLookAngle(chr, fVar5);
			frame = modelGetCurAnimFrame(chr->model);

			if (frame >= modelGetAnimEndFrame(chr->model)) {
				chr->act_skjump.state++;
				chr->act_skjump.needsnewanim = true;
			}
			break;
		case SKJUMPSTATE_AIRBORNE:
			chr->act_skjump.pos.x = chr->act_skjump.vel[0] * g_Vars.lvupdate60 + chr->prop->pos.x;
			chr->act_skjump.pos.z = chr->act_skjump.vel[1] * g_Vars.lvupdate60 + chr->prop->pos.z;

			if (chr->act_skjump.total60 > 0) {
				fVar6 = 1.0f - chr->act_skjump.timer60 / (float)chr->act_skjump.total60;
				fVar7 = sinf(M_PI * fVar6);
				fVar7 = fVar7 * 160.0f + chr->act_skjump.ground;
			} else {
				fVar6 = 1;
				fVar7 = chr->act_skjump.ground;
			}

			chr->act_skjump.pos.y = fVar7 - chr->prop->pos.y;

			if (fVar6 < 0.5f && chr->act_skjump.pos.y < 0.0f) {
				chr->act_skjump.pos.y = 0;
			}

			if (!chr->act_skjump.hit && chrGetDistanceToTarget(chr) < 150.0f) {
				chrPunchInflictDamage(chr, 3, 150, false);
				chr->act_skjump.hit = true;
			}

			if (chr->act_skjump.timer60 > 0) {
				chr->act_skjump.timer60 -= g_Vars.lvupdate60;
			} else {
				chrTryStop(chr);
			}
			break;
		}
	}
}

void chraTick(struct chrdata *chr)
{
	uint32_t race = CHRRACE(chr);

	if (g_Vars.lvupdate240 < 1) {
		return;
	}

	if (chr->actiontype == ACT_INIT) {
		chr->chrflags |= CHRCFLAG_FORCETOGROUND;
		func0f02e9a0(chr, 0);
		chr->sleep = 0;
	}

	if (race == RACE_DRCAROLL) {
		g_DrCarollDyingTimer += g_Vars.lvupdate60;
	}

	chr->soundtimer += g_Vars.lvupdate60;
	chr->talktimer += g_Vars.lvupdate60;

	if (chr->aibot) {
		chr->aibot->unk030 += g_Vars.lvupdate60;
	}

	if (chr->hidden & CHRHFLAG_TIMER_RUNNING) {
		chr->timer60 += g_Vars.lvupdate60;
	}

	chr->sleep -= g_Vars.lvupdate60;

	if (chr->sleep < 0
			|| (chr->chrflags & CHRCFLAG_NEVERSLEEP)
			|| chr->alertness >= 65
			|| (chr->aibot && (chr->actiontype == ACT_DIE || chr->actiontype == ACT_DEAD))) {
		uint8_t pass = race == RACE_HUMAN || race == RACE_SKEDAR;
		chr->sleep = 0;

		chraiExecute(chr, PROPTYPE_CHR);

		// Consider setting shootingatmelist
		if (chr->prop) {
			if (chr->aimtesttimer60 < 1) {
				chr->aimtesttimer60 = TICKS(30);

				if (chr->aishootingatmelist >= 0
						&& ailistFindById(chr->aishootingatmelist) != chr->ailist
						&& chrIsTargetAimingAtMe(chr)) {
					chr->chrflags |= CHRCFLAG_CONSIDER_DODGE;
				}
			} else {
				chr->aimtesttimer60 -= g_Vars.lvupdate60;
			}
		}

		// Consider setting darkroomlist
		if (chr->prop
				&& chr->aidarkroomlist >= 0
				&& roomGetSettledLocalBrightness(chr->prop->rooms[0]) < 25
				&& ailistFindById(chr->aidarkroomlist) != chr->ailist) {
			chr->darkroomthing = true;
		}

		// Consider setting playerdeadlist
		if (chr->prop && chr->aiplayerdeadlist >= 0 && g_Vars.currentplayer->isdead) {
			uint32_t prevplayernum = g_Vars.currentplayernum;
			int i;
			int playercount = PLAYERCOUNT();
			bool alldead = true;

			if (playercount >= 2) {
				for (i = 0; i < playercount && alldead; i++) {
					if (i != prevplayernum) {
						setCurrentPlayerNum(i);

						if (g_Vars.currentplayer->isdead == false) {
							alldead = false;
						}
					}
				}

				setCurrentPlayerNum(prevplayernum);
			}

			if (alldead && ailistFindById(chr->aiplayerdeadlist) != chr->ailist) {
				chr->playerdeadthing = true;
			}
		}

		if (race == RACE_ROBOT) {
			robotSetMuzzleFlash(chr, 0, false);
			robotSetMuzzleFlash(chr, 1, false);
		}

		if (chr->prop) {
			if (g_Vars.in_cutscene) {
				switch (chr->actiontype) {
				case ACT_ANIM:   chrTickAnim(chr);   break;
				case ACT_PATROL: chrTickPatrol(chr); pass = false; break;
				}
			} else {
				switch (chr->actiontype) {
				case ACT_STAND:           chrTickStand(chr);           break;
				case ACT_KNEEL:           chrTickKneel(chr);           break;
				case ACT_ANIM:            chrTickAnim(chr);            break;
				case ACT_DIE:             chrTickDie(chr);             break;
				case ACT_ARGH:            chrTickArgh(chr);            break;
				case ACT_PREARGH:         chrTickPreArgh(chr);         break;
				case ACT_SIDESTEP:        chrTickSidestep(chr);        break;
				case ACT_JUMPOUT:         chrTickJumpOut(chr);         break;
				case ACT_DEAD:            chrTickDead(chr);            break;
				case ACT_ATTACK:          chrTickAttack(chr);          break;
				case ACT_ATTACKWALK:      chrTickAttackWalk(chr);      break;
				case ACT_ATTACKROLL:      chrTickAttackRoll(chr);      break;
				case ACT_RUNPOS:          chrTickRunPos(chr);          break;
				case ACT_PATROL:          chrTickPatrol(chr);          pass = false; break;
				case ACT_GOPOS:           chrTickGoPos(chr);           break;
				case ACT_SURRENDER:       chrTickSurrender(chr);       break;
				case ACT_TEST:            chrTickTest(chr);            break;
				case ACT_SURPRISED:       chrTickSurprised(chr);       break;
				case ACT_STARTALARM:      chrTickStartAlarm(chr);      break;
				case ACT_THROWGRENADE:    chrTickThrowGrenade(chr);    break;
				case ACT_BONDDIE:         chrTickBondDie(chr);         break;
				case ACT_DRUGGEDCOMINGUP: chrTickDruggedComingUp(chr); break;
				case ACT_DRUGGEDDROP:     chrTickDruggedDrop(chr);     break;
				case ACT_DRUGGEDKO:       chrTickDruggedKo(chr);       break;
				case ACT_ATTACKAMOUNT:    chrTickAttackAmount(chr);    break;
				case ACT_ROBOTATTACK:     chrTickRobotAttack(chr);     break;
				case ACT_SKJUMP:          chrTickSkJump(chr);          break;
				}
			}
		}

		chr->hidden &= ~CHRHFLAG_IS_HEARING_TARGET;
		chr->hidden2 &= ~CHRH2FLAG_CONSIDERPROXIES;

		if (pass) {
		}
	} else {
		footstepCheckMagic(chr);
	}
}

void cutsceneStart(uint32_t ailistid)
{
	struct prop *prop;

	g_CutsceneFrameOverrun240 = 0;
	g_CutsceneSkipRequested = false;
	g_CutsceneCurTotalFrame60f = 0;

	prop = g_Vars.activeprops;

	while (prop) {
		prop->lastupdateframe = 0xffff;
		prop->propupdate240 = 0;
		prop->propupdate60err = 2;

		prop = prop->next;
	}

	g_BgChrs[g_NumBgChrs - 1].ailist = ailistFindById(ailistid);
	g_BgChrs[g_NumBgChrs - 1].aioffset = 0;
	g_BgChrs[g_NumBgChrs - 1].aireturnlist = -1;
}

/**
 * Execute background tasks.
 *
 * - Calculate number of engaged chrs
 * - Handle switching to new cutscenes when playing all
 * - Run BG scripts
 * - Choose which corpses to fade
 */
void chraTickBg(void)
{
	int i;
	int numchrs = chrsGetNumSlots(); // Number of players + number of chars in the setup + 200
	int numaliveonscreen;
	int numdeadonscreen;
	int onscreenlen;
	int offscreenlen;
	int spawnslen;
	struct chrdata *onscreen[100]; // Increase original by 20x
	struct chrdata *offscreen[100]; // Increase original by 20x
	struct chrdata *spawns[200]; // Increase original by 20x
	int writeindex;
	int maxdeadonscreen;

	static int mosteveralive = 0;

	numaliveonscreen = 0;

	spawnslen = 0;
	numdeadonscreen = 0;

	// Count the number of chrs who are engaged with the player.
	// When these numbers are high, chrs can ignore gopos commands.
	g_NumChrsWithPlayerTarget = 0;
	g_NumChrsSeenPlayer = 0;
	g_NumChrsSeenPlayerRecently = 0;
	g_NumChrsSeenPlayerRecently2 = 0;

	if (g_Vars.normmplayerisrunning == false) {
		for (i = 0; i < numchrs; i++) {
			struct chrdata *chr = &g_ChrSlots[i];

			if (chr->model && chr->prop && !chrIsDead(chr)) {
				struct prop *targetprop = chrGetTargetProp(chr);

				if (targetprop && (targetprop->type == PROPTYPE_CHR || targetprop->type == PROPTYPE_PLAYER)) {
					if ((targetprop->type == PROPTYPE_PLAYER
								&& !(g_Vars.antiplayernum >= 0 && g_Vars.anti && g_Vars.anti->prop == targetprop)
								&& chrCompareTeams(chr, targetprop->chr, COMPARE_ENEMIES))
							|| CHRRACE(targetprop->chr) == RACE_EYESPY) {
						int time60;
						int lastsee;
						int lastvis;

						g_NumChrsWithPlayerTarget++;

						lastsee = chr->lastseetarget60;
						lastvis = chr->lastvisibletarget60;
						time60 = lastsee > lastvis ? lastsee : lastvis;

						if (time60) {
							g_NumChrsSeenPlayer++;

							if (g_Vars.lvframe60 - time60 < TICKS(240)) {
								g_NumChrsSeenPlayerRecently++;
								g_NumChrsSeenPlayerRecently2++;
							}
						}
					}
				}
			}
		}
	}

	// Handle switching to a new cutscene when using "Play All" from the menu
	if (g_Vars.autocutnum >= 0) {
		cutsceneStart(g_Vars.autocutnum + 0xc00);
		g_Vars.autocutnum = -1;
		g_Vars.autocutplaying = true;
	}

	// Run BG scripts
	for (i = 0; i < g_NumBgChrs; i++) {
		if (!g_Vars.autocutplaying || (g_BgChrs[i].hidden2 & CHRH2FLAG_TICKDURINGAUTOCUT)) {
			chraTick(&g_BgChrs[i]);
		}
	}

	// Calculate alive/dead counters. For *spawned* chrs that have died,
	// allow 10 corpses and start fading if there's more.
	{
		int numalive = 0;

		for (i = 0; i < numchrs; i++) {
			struct chrdata *chr = &g_ChrSlots[i];

			if (chr->model && chr->prop) {
				if (chr->actiontype != ACT_DEAD && chr->actiontype != ACT_DRUGGEDKO) {
					numalive++;
				}

				if (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) {
					if (chr->actiontype != ACT_DEAD && chr->actiontype != ACT_DRUGGEDKO) {
						numaliveonscreen++;
					} else if (chr->actiontype == ACT_DRUGGEDKO) {
						if ((chr->chrflags & CHRCFLAG_KEEPCORPSEKO) == 0) {
							numdeadonscreen++;
						}
					} else {
						numdeadonscreen++;
					}
				}

				if (chr->actiontype == ACT_DEAD
						|| (chr->actiontype == ACT_DRUGGEDKO && (chr->chrflags & CHRCFLAG_KEEPCORPSEKO) == 0)) {
					if (chr->hidden2 & CHRH2FLAG_SPAWNED) {
						spawns[spawnslen] = chr;
						spawnslen++;

						if (spawnslen >= 100) { // Increase spawnslen by factor of 10
							writeindex = rngRandom() % spawnslen;
							chrFadeCorpse(spawns[writeindex]);
							spawns[writeindex] = spawns[spawnslen - 1];
							spawnslen--;
						}
					}
				}
			}
		}

		if (numalive > mosteveralive) {
			mosteveralive = numalive;
		}
	}

	// Calculate how many corpses are allowed on screen. Ideally no more than
	// 5 chrs (alive or dead) to keep the number of polygons low.
	maxdeadonscreen = 100 - numaliveonscreen; // Increase by 20x

	// If the game is lagging, take the number of 60ths over 6 since the
	// previous frame and reduce the limit by that amount. So if it's been, say,
	// 8 60ths since the last frame then the max corpses will be reduced by 2.
	// This is why corpses are more likely to fade on screen when you change
	// weapons - the lag spike from changing guns triggers a lower corpse limit.
	/*if (g_Vars.lvupdate60 > 6) {
		maxdeadonscreen = maxdeadonscreen - g_Vars.lvupdate60 + 6;
	}*/

	if (maxdeadonscreen < 0) {
		maxdeadonscreen = 0;
	}
	
	// Decide which corpses to fade.
	// NTSC Beta implements its engagement counter here.
	if (numdeadonscreen) {
		numdeadonscreen = 0;
		onscreenlen = 0;
		offscreenlen = 0;

		for (i = 0; i < numchrs; i++) {
			struct chrdata *chr = &g_ChrSlots[i];

			if (chr->model) {
				if (chr->actiontype == ACT_DEAD
						|| (chr->actiontype == ACT_DRUGGEDKO && chr->prop && (chr->chrflags & CHRCFLAG_KEEPCORPSEKO) == 0)) {
					if (chr->prop->flags & PROPFLAG_ONANYSCREENPREVTICK) {
						// On-screen
						if (chr->act_dead.fadetimer60 < 0 && !chr->act_dead.fadenow) {
							numdeadonscreen++;

							// If there's too many corpses on screen, start fading.
							if (numdeadonscreen > maxdeadonscreen || chr->aibot) {
								chrFadeCorpse(chr);
								numdeadonscreen--;
							} else if (!chr->act_dead.fadewheninvis) {
								// If there are 2 or more corpses on screen,
								// start marking them to be removed once off screen
								onscreen[onscreenlen] = chr;
								onscreenlen++;

								if (onscreenlen >= 30) { // Increase by factor of 10
									writeindex = rngRandom() % onscreenlen;
									chrFadeCorpseWhenOffScreen(onscreen[writeindex]);
									onscreen[writeindex] = onscreen[onscreenlen - 1];
									onscreenlen--;
								}
							}
						}
					} else {
						// Off-screen
						if (!chr->act_dead.fadewheninvis) {
							offscreen[offscreenlen] = chr;
							offscreenlen++;

							// Allow up to 5 corpses off-screen
							if (offscreenlen >=  50) { // Increase by factor of 10
								writeindex = rngRandom() % offscreenlen;

								if (offscreen[writeindex]->actiontype != ACT_DEAD) {
									chrBeginDead(offscreen[writeindex]);
								}

								chrFadeCorpseWhenOffScreen(offscreen[writeindex]);
								offscreen[writeindex] = offscreen[offscreenlen - 1];
								offscreenlen--;
							}
						}
					}
				}
			}
		}
	}
}

bool chrSawTargetRecently(struct chrdata *chr)
{
	if (chr->lastseetarget60 > 0 && g_Vars.lvframe60 - chr->lastseetarget60 < TICKS(600)) {
		return true;
	}

	return false;
}

bool chrHeardTargetRecently(struct chrdata *chr)
{
	if (chr->lastheartarget60 > 0 && g_Vars.lvframe60 - chr->lastheartarget60 < TICKS(600)) {
		return true;
	}

	return false;
}

float chrGetAngleToPos(struct chrdata *chr, struct coord *pos)
{
	float chranglerad; // Direction the character is pointing
	float propanglerad; // Direction from the character to the given position
	float result;
	struct prop *prop;

	if (chr->prop->type == PROPTYPE_PLAYER) {
		uint32_t playernum = playermgrGetPlayerNumByProp(chr->prop);
		chranglerad = (360 - g_Vars.players[playernum]->vv_theta) * (M_TAU / 360);
	} else {
		chranglerad = chrGetInverseTheta(chr);
	}

	prop = chr->prop;
	propanglerad = atan2f(pos->x - prop->pos.x, pos->z - prop->pos.z);
	result = propanglerad - chranglerad;

	if (propanglerad < chranglerad) { // Make the result between 0 and 2*pi
		result += M_TAU;
	}

	return result; // Return the angle in radians
}

float chrGetAngleToTarget(struct chrdata *chr)
{
	struct prop *prop = chrGetTargetProp(chr);
	return chrGetAngleToPos(chr, &prop->pos);
}

void chrGetAttackEntityPos(struct chrdata *chr, uint32_t attackflags, int entityid, struct coord *pos, RoomNum *rooms)
{
	struct prop *targetprop;
	struct chrdata *targetchr;
	struct pad pad;

	if (attackflags & ATTACKFLAG_AIMATCHR) {
		// Aiming at a chr by chrnum
		targetchr = chrFindById(chr, entityid);

		if (!targetchr || !targetchr->prop) {
			targetchr = chr;
		}

		pos->x = targetchr->prop->pos.x;
		pos->y = targetchr->prop->pos.y;
		pos->z = targetchr->prop->pos.z;

		if (targetchr) {
#ifdef PLATFORM_N64
			chr = targetprop->chr; // bug? targetprop is never set before this
#else
			chr = targetchr;
#endif
		} else {
			chr = NULL;
		}

		roomsCopy(targetchr->prop->rooms, rooms);
	} else if (attackflags & ATTACKFLAG_AIMATPAD) {
		// Aiming at a pad by padnum
		int padnum = chrResolvePadId(chr, entityid);
		padUnpack(padnum, PADFIELD_POS | PADFIELD_ROOM, &pad);

		pos->x = pad.pos.x;
		pos->y = pad.pos.y;
		pos->z = pad.pos.z;

		rooms[0] = pad.room;
		rooms[1] = -1;
	} else {
		// Aiming at the chr's preconfigured target
		targetprop = chrGetTargetProp(chr);

		pos->x = targetprop->pos.x;
		pos->y = targetprop->pos.y;
		pos->z = targetprop->pos.z;

		if (targetprop->type == PROPTYPE_CHR && targetprop->chr) {
			chr = targetprop->chr;
		} else {
			chr = NULL;
		}

		roomsCopy(targetprop->rooms, rooms);
	}
}

float chrGetAngleFromTargetsFov(struct chrdata *chr)
{
	float targetfacingangle = 0;
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);
	float xdiff;
	float zdiff;
	float angletotarget;
	float result;

	if (prop && target) {
		xdiff = prop->pos.f[0] - target->pos.f[0];
		zdiff = prop->pos.f[2] - target->pos.f[2];
		angletotarget = atan2f(xdiff, zdiff);

		if (target->type == PROPTYPE_PLAYER) {
			int playernum = playermgrGetPlayerNumByProp(target);
			targetfacingangle = (360.0f - g_Vars.players[playernum]->vv_theta) * M_TAU / 360.0f;
		} else if (target->type == PROPTYPE_CHR) {
			targetfacingangle = chrGetInverseTheta(target->chr);
		}

		result = angletotarget - targetfacingangle;

		if (angletotarget < targetfacingangle) {
			result += M_TAU;
		}
	}

	return result;
}

float chrGetVerticalAngleToTarget(struct chrdata *chr)
{
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);
	float result = 0;

	if (prop && target) {
		float ydiff;
		float xdiff;
		float zdiff;

		xdiff = prop->pos.x - target->pos.x;
		ydiff = prop->pos.y - target->pos.y;
		zdiff = prop->pos.z - target->pos.z;

		result = atan2f(ydiff, sqrtf(xdiff * xdiff + zdiff * zdiff));

		if (result < 0) {
			result += M_TAU;
		}
	}

	return result;
}

bool chrIsInTargetsFovX(struct chrdata *chr, uint8_t fov360)
{
	float angle = chrGetAngleFromTargetsFov(chr);

	if ((angle < fov360 * 0.024539785459638f && angle < M_PI)
			|| (angle > M_TAU - fov360 * 0.024539785459638f && angle > M_PI)) {
		return true;
	}

	return false;
}

bool chrIsVerticalAngleToTargetWithin(struct chrdata *chr, uint8_t fov360)
{
	float val = chrGetVerticalAngleToTarget(chr);

	if ((val < fov360 * 0.024539785459638f && val < M_PI)
			|| (val > M_TAU - fov360 * 0.024539785459638f && val > M_PI)) {
		return true;
	}

	return false;
}

float chrGetAngleToTarget2(struct chrdata *chr, uint8_t reverse)
{
	float result;

	struct prop *chrprop = chr->prop;
	struct prop *targetprop = chrGetTargetProp(chr);

	float xdiff = chrprop->pos.x - targetprop->pos.x;
	float zdiff = chrprop->pos.z - targetprop->pos.z;

	float angle1 = atan2f(-xdiff, -zdiff);
	float angle2 = chrGetInverseTheta(chr) + M_PI * (int)reverse;

	result = angle1 - angle2;

	if (angle1 < angle2) {
		result += M_TAU;
	}

	return result;
}

bool chrIsTargetInFov(struct chrdata *chr, uint8_t arg1, uint8_t reverse)
{
	float angle = chrGetAngleToTarget2(chr, reverse);

	if ((angle < arg1 * 0.024539785459638f && angle < M_PI)
			|| (angle > M_TAU - arg1 * 0.024539785459638f && angle > M_PI)) {
		return true;
	}

	return false;
}

bool chrIsLookingAtPos(struct chrdata *chr, struct coord *pos, uint8_t arg2)
{
	float angle = chrGetAngleToPos(chr, pos);

	if ((angle < arg2 * 0.024539785459638f && angle < M_PI) ||
			(M_TAU - arg2 * 0.024539785459638f < angle && M_PI < angle)) {
		return true;
	}

	return false;
}

float chrGetDistanceToTarget(struct chrdata *chr)
{
	struct prop *prop = chrGetTargetProp(chr);
	return propGetDistanceToProp(chr->prop, prop);
}

float chrGetDistanceToCurrentPlayer(struct chrdata *chr)
{
	return propGetDistanceToProp(chr->prop, g_Vars.currentplayer->prop);
}

float propGetDistanceToProp(struct prop *a, struct prop *b)
{
	float xdiff = a->pos.x - b->pos.x;
	float ydiff = a->pos.y - b->pos.y;
	float zdiff = a->pos.z - b->pos.z;

	return sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
}

float propGetLateralDistanceToProp(struct prop *a, struct prop *b)
{
	float xdiff = a->pos.x - b->pos.x;
	float zdiff = a->pos.z - b->pos.z;

	return sqrtf(xdiff * xdiff + zdiff * zdiff);
}

float chrGetDistanceToPad(struct chrdata *chr, int pad_id)
{
	struct prop *prop = chr->prop;
	float xdiff, ydiff, zdiff;
	float distance = 0;
	struct pad pad;
	pad_id = chrResolvePadId(chr, pad_id);

	if (pad_id >= 0)
	{
		padUnpack(pad_id, PADFIELD_POS, &pad);
		xdiff = pad.pos.x - prop->pos.x;
		ydiff = pad.pos.y - prop->pos.y;
		zdiff = pad.pos.z - prop->pos.z;
		distance = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	}

	return distance;
}

float chrGetSameFloorDistanceToPad(struct chrdata *chr, int pad_id)
{
	struct prop *prop = chr->prop;
	float xdiff, ydiff, zdiff, ydiff_absolute;
	struct pad pad;
	float ret;

	pad_id = chrResolvePadId(chr, pad_id);
	padUnpack(pad_id, PADFIELD_POS, &pad);
	xdiff = pad.pos.x - prop->pos.x;
	ydiff = pad.pos.y - prop->pos.y;
	zdiff = pad.pos.z - prop->pos.z;

	if (ydiff > 0) {
		ydiff_absolute = ydiff;
	} else {
		ydiff_absolute = -ydiff;
	}

	if (ydiff_absolute < 150) {
		ret = sqrtf(xdiff * xdiff + zdiff * zdiff);
	} else {
		ret = 100000000;
	}

	return ret;
}

float chrGetDistanceToCoord(struct chrdata *chr, struct coord *pos)
{
	float xdiff = pos->x - chr->prop->pos.x;
	float ydiff = pos->y - chr->prop->pos.y;
	float zdiff = pos->z - chr->prop->pos.z;

	return sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
}

float chrGetLateralDistanceToCoord(struct chrdata *chr, struct coord *pos)
{
	float xdiff = pos->x - chr->prop->pos.x;
	float zdiff = pos->z - chr->prop->pos.z;

	return sqrtf(xdiff * xdiff + zdiff * zdiff);
}

float chrGetLateralDistanceToPad(struct chrdata *chr, int pad_id)
{
	struct prop *prop = chr->prop;
	float xdiff, zdiff;
	struct pad pad;

	float distance = 0;

	pad_id = chrResolvePadId(chr, pad_id);

	if (pad_id >= 0) {
		padUnpack(pad_id, PADFIELD_POS, &pad);
		xdiff = pad.pos.x - prop->pos.x;
		zdiff = pad.pos.z - prop->pos.z;
		distance = sqrtf(xdiff * xdiff + zdiff * zdiff);
	}

	return distance;
}

float chrGetSquaredDistanceToCoord(struct chrdata *chr, struct coord *pos)
{
	float xdiff = pos->x - chr->prop->pos.x;
	float ydiff = pos->y - chr->prop->pos.y;
	float zdiff = pos->z - chr->prop->pos.z;

	return xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;
}

float coordGetSquaredDistanceToCoord(struct coord *a, struct coord *b)
{
	float xdiff = a->x - b->x;
	float ydiff = a->y - b->y;
	float zdiff = a->z - b->z;

	return xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;
}

int chrGetPadRoom(struct chrdata *chr, int pad_id)
{
	int ret = -1;
	int pad_id_backup = pad_id;
	struct pad pad;

	if (pad_id >= 10000) {
		int resolved_pad_id = chrResolvePadId(chr, pad_id - 10000);

		if (resolved_pad_id >= 0)
		{
			padUnpack(resolved_pad_id, PADFIELD_ROOM, &pad);
			ret = pad.room;
		}
	} else {
		ret = pad_id;
	}

	return ret;
}

int chrResolvePadId(struct chrdata *chr, int pad_id)
{
	if (pad_id == 9000) {
		pad_id = chr->padpreset1;
	}

	return pad_id;
}

/**
 * For all chrs, clear their target and p1p2 values if set to the given player.
 *
 * This function is called when the given player has died. It causes all guards
 * to switch their focus to the remaining coop player.
 */
void chrsClearRefsToPlayer(int playernum)
{
	int otherplayernum;
	int playerpropnum;
	int i;

	if (g_Vars.coopplayernum >= 0) {
		if (playernum == g_Vars.bondplayernum) {
			otherplayernum = g_Vars.coopplayernum;
			playerpropnum = g_Vars.bond->prop - g_Vars.props;
		} else {
			otherplayernum = g_Vars.bondplayernum;
			playerpropnum = g_Vars.coop->prop - g_Vars.props;
		}

		for (i = 0; i < chrsGetNumSlots(); i++) {
			if (g_ChrSlots[i].p1p2 == playernum) {
				g_ChrSlots[i].p1p2 = otherplayernum;
			}

			if (g_ChrSlots[i].target == playerpropnum) {
				g_ChrSlots[i].target = -1;
			}
		}

		for (i = 0; i < g_NumBgChrs; i++) {
			if (g_BgChrs[i].p1p2 == playernum) {
				g_BgChrs[i].p1p2 = otherplayernum;
			}

			if (g_BgChrs[i].target == playerpropnum) {
				g_BgChrs[i].target = -1;
			}
		}
	}
}

int chrResolveId(struct chrdata *ref, int id)
{
	if (ref) {
		switch (id) {
		case CHR_SEESHOT:
			id = ref->chrseeshot;
			break;
		case CHR_SEEDIE:
			id = ref->chrseedie;
			break;
		case CHR_PRESET:
			id = ref->chrpreset1;
			break;
		case CHR_SELF:
			id = ref->chrnum;
			break;
		case CHR_CLONE:
			id = ref->chrdup;
			break;
		case CHR_BOND:
			if (g_Vars.bond && g_Vars.bond->prop && g_Vars.bond->prop->chr) {
				id = g_Vars.bond->prop->chr->chrnum;
			}
			break;
		case CHR_COOP:
			if (g_Vars.coop && g_Vars.coop->prop && g_Vars.coop->prop->chr) {
				id = g_Vars.coop->prop->chr->chrnum;
			}
			break;
		case CHR_ANTI:
			if (g_Vars.anti && g_Vars.anti->prop && g_Vars.anti->prop->chr) {
				id = g_Vars.anti->prop->chr->chrnum;
			}
			break;
		case CHR_P1P2:
			{
				uint32_t index = g_Vars.coopplayernum >= 0 ? ref->p1p2 : g_Vars.bondplayernum;
				struct player *player = g_Vars.players[index];
				if (player && player->prop && player->prop->chr) {
					id = player->prop->chr->chrnum;
				}
			}
			break;
		case CHR_P1P2_OPPOSITE:
			if (g_Vars.coopplayernum >= 0) {
				struct player *player = g_Vars.players[1 - ref->p1p2];
				if (player && player->prop && player->prop->chr) {
					id = player->prop->chr->chrnum;
				}
			}
			break;
		case CHR_TARGET:
			{
				struct prop *target = chrGetTargetProp(ref);
				if ((target->type == PROPTYPE_CHR || target->type == PROPTYPE_PLAYER) && target->chr) {
					id = target->chr->chrnum;
				}
			}
			break;
		}
	} else { // ref is NULL
		switch (id) {
		case CHR_BOND:
			if (g_Vars.bond && g_Vars.bond->prop && g_Vars.bond->prop->chr) {
				id = g_Vars.bond->prop->chr->chrnum;
			}
			break;
		case CHR_COOP:
			if (g_Vars.coop && g_Vars.coop->prop && g_Vars.coop->prop->chr) {
				id = g_Vars.coop->prop->chr->chrnum;
			}
			break;
		case CHR_ANTI:
			if (g_Vars.anti && g_Vars.anti->prop && g_Vars.anti->prop->chr) {
				id = g_Vars.anti->prop->chr->chrnum;
			}
			break;
		case CHR_P1P2:
			{
				struct player *player = g_Vars.players[g_Vars.bondplayernum];
				if (player && player->prop && player->prop->chr) {
					id = player->prop->chr->chrnum;
				}
			}
			break;
		case CHR_P1P2_OPPOSITE:
			if (g_Vars.coopplayernum >= 0) {
				struct player *player = g_Vars.players[g_Vars.coopplayernum];
				if (player && player->prop && player->prop->chr) {
					id = player->prop->chr->chrnum;
				}
			}
			break;
		}
	}

	return id;
}

struct chrdata *chrFindById(struct chrdata *basechr, int chrnum)
{
	struct chrdata *chr;
	int lower;
	int upper;
	int i;

	chrnum = chrResolveId(basechr, chrnum);
	chr = chrFindByLiteralId(chrnum);

	if (chr) {
		return chr;
	}

	lower = 0;
	upper = g_NumBgChrs;

	while (upper >= lower) {
		i = (lower + upper) / 2;

		if (chrnum == g_BgChrnums[i]) {
			return &g_BgChrs[i];
		}

		if (chrnum < g_BgChrnums[i]) {
			upper = i - 1;
		} else {
			lower = i + 1;
		}
	}

	return NULL;
}

int propGetIndexByChrId(struct chrdata *basechr, int chrnum)
{
	int index;

	if (chrnum == CHR_BOND || chrnum == CHR_BOND) {
		index = g_Vars.bond->prop - g_Vars.props;
	} else {
		struct chrdata *chr = chrFindById(basechr, chrnum);

		if (chr && chr->prop) {
			index = chr->prop - g_Vars.props;
		} else {
			index = -1;
		}
	}

	return index;
}

float chrGetDistanceToChr(struct chrdata *chr1, int chr2num)
{
	struct prop *prop1 = chr1->prop;
	struct chrdata *chr2 = chrFindById(chr1, chr2num);
	float distance = 0;

	if (chr2 && chr2->prop) {
		float xdiff = chr2->prop->pos.x - prop1->pos.x;
		float ydiff = chr2->prop->pos.y - prop1->pos.y;
		float zdiff = chr2->prop->pos.z - prop1->pos.z;
		distance = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	}

	return distance;
}

float chrGetDistanceFromTargetToPad(struct chrdata *chr, int pad_id)
{
	struct prop *prop = chrGetTargetProp(chr);
	float xdiff, ydiff, zdiff;
	struct pad pad;
	float distance = 0;
	pad_id = chrResolvePadId(chr, pad_id);

	if (pad_id >= 0)
	{
		padUnpack(pad_id, PADFIELD_POS, &pad);
		xdiff = pad.pos.x - prop->pos.x;
		ydiff = pad.pos.y - prop->pos.y;
		zdiff = pad.pos.z - prop->pos.z;
		distance = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	}

	return distance;
}

void chrSetFlags(struct chrdata *chr, uint32_t flags, uint8_t bank)
{
	if (bank == 0) {
		chr->flags |= flags;
	} else {
		chr->flags2 |= flags;
	}
}

void chrUnsetFlags(struct chrdata *chr, uint32_t flags, uint8_t bank)
{
	if (bank == 0) {
		chr->flags &= ~flags;
	} else {
		chr->flags2 &= ~flags;
	}
}

bool chrHasFlag(struct chrdata *chr, uint32_t flag, uint8_t bank)
{
	if (bank == 0) {
		return (chr->flags & flag) != 0;
	} else {
		return (chr->flags2 & flag) != 0;
	}
}

void chrSetFlagsById(struct chrdata *ref, uint32_t chrnum, uint32_t flags, uint32_t bank)
{
	struct chrdata *chr = chrFindById(ref, chrnum);

	if (chr) {
		chrSetFlags(chr, flags, bank);
	}
}

void chrUnsetFlagsById(struct chrdata *ref, uint32_t chrnum, uint32_t flags, uint32_t bank)
{
	struct chrdata *chr = chrFindById(ref, chrnum);

	if (chr) {
		chrUnsetFlags(chr, flags, bank);
	}
}

bool chrHasFlagById(struct chrdata *ref, uint32_t chrnum, uint32_t flag, uint32_t bank)
{
	struct chrdata *chr = chrFindById(ref, chrnum);

	if (chr) {
		return chrHasFlag(chr, flag, bank);
	}

	return false;
}

void chrSetStageFlag(struct chrdata *chr, uint32_t flag)
{
	g_StageFlags |= flag;
}

void chrUnsetStageFlag(struct chrdata *chr, uint32_t flag)
{
	g_StageFlags = g_StageFlags & ~flag;
}

bool chrHasStageFlag(struct chrdata *chr, uint32_t flag)
{
	return (g_StageFlags & flag) != 0;
}

bool chrIsHearingTarget(struct chrdata *chr)
{
	return (chr->hidden & CHRHFLAG_IS_HEARING_TARGET) != 0;
}

void chrRestartTimer(struct chrdata *chr)
{
	chr->timer60 = 0;
	chr->hidden |= CHRHFLAG_TIMER_RUNNING;
}

float chrGetTimer(struct chrdata *chr)
{
	return chr->timer60 * FRAMEDURATION;
}

bool chrIsTargetAimingAtMe(struct chrdata *chr)
{
	struct prop *target = chrGetTargetProp(chr);

	if (target) {
		if (target->type == PROPTYPE_CHR) {
			if (!chrHasLosToTarget(chr)) {
				return false;
			}

			return chrIsInTargetsFovX(chr, 20);
		}

		if (target->type == PROPTYPE_PLAYER) {
			if (g_Vars.bondvisible &&
					(cdTestLos05(&target->pos, target->rooms, &chr->prop->pos, chr->prop->rooms,
									  CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG,
									  GEOFLAG_BLOCK_SIGHT))) {
				struct model *model = chr->model;
				struct coord sp68;
				struct coord sp56;
				struct coord sp44;
				float somefloat = modelGetEffectiveScale(model) * 0.8f;

				bgun0f0a0c08(&sp68, &sp56);
				modelGetRootPosition(model, &sp44);
				mtx4TransformVecInPlace(camGetPlayerWorldToScreenMtx(), &sp44);

				if (utilsIsPointInCone(&sp68, &sp56, &sp44, somefloat)) {
					return true;
				}
			}
		}
	}

	return false;
}

bool chrResetNearMiss(struct chrdata *chr)
{
	bool has_flag = (chr->chrflags & CHRCFLAG_NEAR_MISS) != 0;
	chr->chrflags &= ~CHRCFLAG_NEAR_MISS;

	return has_flag;
}

int chrGetNumArghs(struct chrdata *chr)
{
	return chr->numarghs;
}

int chrGetNumCloseArghs(struct chrdata *chr)
{
	return chr->numclosearghs;
}

bool chrSawInjury(struct chrdata *chr, uint8_t arg1)
{
	bool saw_injury = chr->chrseeshot >= 0;

	if (saw_injury && arg1 == 0) {
		chr->chrseeshot = -1;
	} else if (saw_injury && arg1 == 1) {
		struct chrdata *victim = chrFindById(chr, chr->chrseeshot);

		if (victim && !chrCompareTeams(chr, victim, COMPARE_FRIENDS)) {
			saw_injury = false;
		}
	} else {
		chr->chrseeshot = -1;
	}

	return saw_injury;
}

bool chrSawDeath(struct chrdata *chr, uint8_t arg1)
{
	bool saw_death = chr->chrseedie >= 0;

	// The commented line below was likely originally there but removed before
	// the final version. Compare with chrSawInjury above.
	if (saw_death && arg1 == 0) {
		//chr->chrseedie = -1;
	} else if (saw_death && arg1 == 1) {
		struct chrdata *victim = chrFindById(chr, chr->chrseedie);

		if (victim && !chrCompareTeams(chr, victim, COMPARE_FRIENDS)) {
			saw_death = false;
			chr->chrseedie = -1;
		}
	} else {
		chr->chrseedie = -1;
	}

	return saw_death;
}

void decrementByte(uint8_t *dst, uint8_t amount)
{
	if (*dst < amount) {
		*dst = 0;
		return;
	}

	*dst -= amount;
}

void incrementByte(uint8_t *dst, uint8_t amount)
{
	if (0xff - amount < *dst) {
		*dst = 0xff;
		return;
	}

	*dst += amount;
}

bool chrCanHearAlarm(struct chrdata *chr)
{
	return alarmIsActive();
}

bool waypointIsWithin90DegreesOfPosAngle(struct waypoint *waypoint, struct coord *pos, float angle)
{
	float diffangle;
	struct pad pad;

	padUnpack(waypoint->padnum, PADFIELD_POS, &pad);

	diffangle = angle - atan2f(pad.pos.x - pos->x, pad.pos.z - pos->z);

	if (diffangle < 0) {
		diffangle += M_TAU;
	}

	if (diffangle < 1.5705462694168f || diffangle > 4.7116389274597f) {
		return true;
	}

	return false;
}

/**
 * Attempt to find a waypoint near pos which is in a particular quadrant to pos,
 * then return its padnum.
 *
 * For example, pos is typically the player's position, angle is the direction
 * the player is facing, and quadrant is which quadrant (front/back/left/right)
 * that is desired relative to the player's position and angle.
 *
 * The function starts by finding the closest waypoint to the pos. If it's not
 * in the quadrant then its neighouring waypoints are checked too. If none of
 * those are in the quadrant then no further checks are made and the function
 * returns -1.
 */
int chrFindWaypointWithinPosQuadrant(struct coord *pos, RoomNum *rooms, float angle, uint8_t quadrant)
{
	struct waypoint *waypoint = waypointFindClosestToPos(pos, rooms);
	int neighbournum;
	int i;

	if (waypoint) {
		switch (quadrant) {
		case QUADRANT_BACK:
			angle += M_PI;
			break;
		case QUADRANT_SIDE1:
			angle += 1.5705462694168f;
			break;
		case QUADRANT_SIDE2:
			angle += 4.7116389274597f;
			break;
		case QUADRANT_FRONT:
			break;
		}

		if (angle >= M_TAU) {
			angle -= M_TAU;
		}

		if (waypointIsWithin90DegreesOfPosAngle(waypoint, pos, angle)) {
			return waypoint->padnum;
		}

		for (i = 0; (neighbournum = waypoint->neighbours[i]) >= 0; i++) {
			if ((neighbournum & 0x8000) == 0) {
				neighbournum &= 0x3fff;

				if (waypointIsWithin90DegreesOfPosAngle(&g_StageSetup.waypoints[neighbournum], pos, angle)) {
					return g_StageSetup.waypoints[neighbournum].padnum;
				}
			}

		}
	}

	return -1;
}

bool func0f04a4ec(struct chrdata *chr, uint8_t quadrant)
{
	if (quadrant == QUADRANT_TOWARDSTARGET || quadrant == QUADRANT_AWAYFROMTARGET) {
		struct prop *prop = chr->prop;
		struct prop *target = chrGetTargetProp(chr);

		struct waypoint *chrwp = waypointFindClosestToPos(&prop->pos, prop->rooms);
		struct waypoint *tarwp = waypointFindClosestToPos(&target->pos, target->rooms);

		struct waypoint *waypoints[3];
		int numwaypoints;

		if (chrwp && tarwp) {
			if (quadrant == QUADRANT_TOWARDSTARGET) {
				navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
				numwaypoints = navFindRoute(chrwp, tarwp, waypoints, 3);
				navSetSeed(0, 0);

				if (numwaypoints >= 3) {
					chr->padpreset1 = waypoints[1]->padnum;
					return true;
				}
			} else {
				navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
				chrwp = navChooseRetreatPoint(chrwp, tarwp);
				navSetSeed(0, 0);

				if (chrwp) {
					chr->padpreset1 = chrwp->padnum;
					return true;
				}
			}
		}
	} else {
		int padnum = chrFindWaypointWithinPosQuadrant(&chr->prop->pos, chr->prop->rooms, chrGetInverseTheta(chr), quadrant);

		if (padnum >= 0) {
			chr->padpreset1 = padnum;
			return true;
		}
	}

	return false;
}

bool chrSetPadPresetToWaypointWithinTargetQuadrant(struct chrdata *chr, uint8_t quadrant)
{
	float angle;
	int padnum;
	struct prop *prop;

	if (quadrant == QUADRANT_TOWARDSTARGET || quadrant == QUADRANT_AWAYFROMTARGET) {
		return func0f04a4ec(chr, quadrant);
	}

	angle = 0;
	prop = chrGetTargetProp(chr);

	if (prop->type == PROPTYPE_PLAYER) {
		angle = (360.0f - g_Vars.players[playermgrGetPlayerNumByProp(prop)]->vv_theta) * M_TAU / 360.0f;
	} else if (prop->type == PROPTYPE_CHR) {
		angle = chrGetInverseTheta(prop->chr);
	}

	padnum = chrFindWaypointWithinPosQuadrant(&prop->pos, prop->rooms, angle, quadrant);

	if (padnum >= 0) {
		chr->padpreset1 = padnum;
		return true;
	}

	return false;
}

bool chrSetChrPresetToAnyChrNearSelf(struct chrdata *chr, float distance)
{
	return chrSetChrPresetToChrNearSelf(COMPARE_ANY, chr, distance);
}

bool chrSetChrPresetToChrNearSelf(uint8_t checktype, struct chrdata *chr, float distance)
{
	return chrSetChrPresetToChrNearPos(checktype, chr, distance, &chr->prop->pos, chr->prop->rooms);
}

bool chrSetChrPresetToChrNearPad(uint32_t checktype, struct chrdata *chr, float distance, int padnum)
{
	struct pad pad;
	RoomNum rooms[2];

	padnum = chrResolvePadId(chr, padnum);
	padUnpack(padnum, PADFIELD_POS | PADFIELD_ROOM, &pad);

	rooms[0] = pad.room;
	rooms[1] = -1;

	return chrSetChrPresetToChrNearPos(checktype, chr, distance, &pad.pos, rooms);
}

bool chrSetChrPresetToChrNearPos(uint8_t checktype, struct chrdata *chr, float distance, struct coord *pos, RoomNum *rooms)
{
	int i;
	int16_t *propnumptr;
	int16_t propnums[256];
	RoomNum allrooms[21];
	float xmin = pos->x - distance;
	float xmax = pos->x + distance;
	float ymin = pos->y - distance;
	float ymax = pos->y + distance;
	float zmin = pos->z - distance;
	float zmax = pos->z + distance;
	RoomNum neighbours[11];

	roomsCopy(rooms, allrooms);

	for (i = 0; rooms[i] != -1; i++) {
		bgRoomGetNeighbours(rooms[i], neighbours, 10);
		roomsAppend(neighbours, allrooms, 20);
	}

	roomGetProps(allrooms, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
			struct chrdata *loopchr = prop->chr;

			if (loopchr->chrnum != chr->chrnum
					&& !chrIsDead(loopchr)
					&& prop->pos.x >= xmin
					&& prop->pos.x <= xmax
					&& prop->pos.y >= ymin
					&& prop->pos.y <= ymax
					&& prop->pos.z >= zmin
					&& prop->pos.z <= zmax
					&& chrCompareTeams(loopchr, chr, checktype)) {
				chr->chrpreset1 = loopchr->chrnum;
				return true;
			}
		}

		*propnumptr++;
	}

	return false;
}

bool chrCompareTeams(struct chrdata *chr1, struct chrdata *chr2, uint8_t checktype)
{
	if (chr1 && chr1->prop) {
		if (checktype == COMPARE_ANY) {
			return true;
		}

		if (checktype == COMPARE_FRIENDS) { // Return true if chrs are friends
			if (g_Vars.normmplayerisrunning) {
				if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) && chr2->team == chr1->team) {
					return true;
				}
			} else {
				if (g_Vars.bond && g_Vars.bond->prop) {
					struct chrdata *playerchr = g_Vars.bond->prop->chr;

					// @bug fix: Only make Jon an ally if the stage is not Duel
					if(!g_Vars.stagenum == STAGE_DUEL)
					{
						// @bug: This makes Jon an ally in Duel
						if ((chr2 == playerchr && chr1->headnum == HEAD_JONATHAN) ||
								(chr1 == playerchr && chr2->headnum == HEAD_JONATHAN)) {
							return true;
						}
					}
				}

				if ((chr2->team & chr1->team) != 0) {
					return true;
				}
			}
		} else if (checktype == COMPARE_ENEMIES) { // Return true if chrs are enemies
			if (g_Vars.normmplayerisrunning) {
				if ((g_MpSetup.options & MPOPTION_TEAMSENABLED) == 0 || chr2->team != chr1->team) {
					return true;
				}
			} else {
				if (g_Vars.bond && g_Vars.bond->prop) {
					struct chrdata *playerchr = g_Vars.bond->prop->chr;

					// @bug fix: Only make Jon an ally if the stage is not Duel
					if(!g_Vars.stagenum == STAGE_DUEL)
					{
						// @bug: This makes Jon an ally in Duel
						if ((chr2 == playerchr && chr1->headnum == HEAD_JONATHAN) ||
								(chr1 == playerchr && chr2->headnum == HEAD_JONATHAN)) {
							return false;
						}
					}
				}

				if ((chr2->team & chr1->team) == 0) {
					return true;
				}
			}
		}

		return false;
	}

	return false;
}

void chrSetChrPreset(struct chrdata *chr, int chrpreset)
{
	chr->chrpreset1 = chrResolveId(chr, chrpreset);
}

void chrSetChrPresetByChrnum(struct chrdata *basechr, int chrnum, int chrpreset)
{
	struct chrdata *chr = chrFindById(basechr, chrnum);

	if (chr) {
		chr->chrpreset1 = chrResolveId(basechr, chrpreset);
	}
}

void chrSetPadPreset(struct chrdata *chr, int pad_id)
{
	chr->padpreset1 = chrResolvePadId(chr, pad_id);
}

void chrSetPadPresetByChrnum(struct chrdata *basechr, int chrnum, int pad_id)
{
	struct chrdata *chr = chrFindById(basechr, chrnum);

	if (chr) {
		chr->padpreset1 = chrResolvePadId(basechr, pad_id);
	}
}

/**
 * Check if chr has line of sight to their target. If not, find a route
 * to them then iterate the route backwards (from target to chr).
 * Find the first waypoint in view of both the chr and their target
 * and store it as the chr's pad preset.
 */
bool chrSetPadPresetToPadOnRouteToTarget(struct chrdata *chr)
{
	struct prop *target = chrGetTargetProp(chr);
	struct prop *prop = chr->prop;
	struct waypoint *fromwp;
	struct waypoint *towp;
	struct waypoint *waypoints[5];
	int numwaypoints;
	int i;
	struct pad pad;

	if (target->type != PROPTYPE_PLAYER || g_Vars.bondvisible) {
		if (cdTestLos04(&prop->pos, prop->rooms, &target->pos, CDTYPE_BG)) {
			return false;
		}

		fromwp = waypointFindClosestToPos(&prop->pos, prop->rooms);
		towp = waypointFindClosestToPos(&target->pos, target->rooms);

		if (fromwp && towp) {
			// Note from/to are swapped here, so the route is from target to chr
			navSetSeed(CHRNAVSEED(chr), CHRNAVSEED(chr));
			numwaypoints = navFindRoute(towp, fromwp, waypoints, 5);
			navSetSeed(0, 0);

			if (numwaypoints >= 3) {
				for (i = 0; waypoints[i] != NULL; i++) {
					struct waypoint *wp = waypoints[i];

					padUnpack(wp->padnum, PADFIELD_POS, &pad);

					if (cdTestLos04(&target->pos, target->rooms, &pad.pos, CDTYPE_BG)) {
						if (cdTestLos04(&prop->pos, prop->rooms, &pad.pos, CDTYPE_BG)) {
							chr->padpreset1 = wp->padnum;
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool chrIsPosOffScreen(struct coord *pos, RoomNum *rooms)
{
	bool offscreen = true;
	int i;

	if (envIsWithinFogRenderDistance(pos, 0)) {
		for (i = 0; rooms[i] != -1; i++) {
			if (bgRoomIsOnscreen(rooms[i])) {
				break;
			}
		}

		if (rooms[i] != -1) {
			// Room is visible by player
			offscreen = !camIsPosInScreenBox(pos, 200, bgGetRoomDrawSlot(rooms[i]));
		}
	}

	return offscreen;
}

/**
 * Test if a chr can be spawned into or nearby the given position, taking into
 * consideration collision checks.
 *
 * If the spawn can happen, the position and rooms are updated with the actual
 * position to be used and the function returns true.
 *
 * If the spawn cannot happen, the function return false.
 */
bool chrAdjustPosForSpawn(float chrradius, struct coord *pos, RoomNum *rooms, float angle, bool allowonscreen, bool force, bool onlysurrounding)
{
	struct coord testpos;
	int i;
	uint32_t types;
	RoomNum testrooms[8];
	float ymin;
	float ymax = 200;
	float curangle = angle;
	float ground;

	if (force) {
		types = CDTYPE_ALL & ~CDTYPE_BG;
		allowonscreen = true;
	} else {
		types = CDTYPE_ALL;
	}

	if (onlysurrounding) {
		// Skip testing the given pos, and just do the surrounding checks below.
		// Add 45 degrees to the angle here, but this isn't necessary.
		curangle += 0.7852731347084f;

		if (curangle >= M_TAU) {
			curangle -= M_TAU;
		}
	} else {
		// Check that the chr isn't being spawned out of bounds, and do a volume
		// test reaching 200cm above and below the chr's feet... unless there is
		// ground under the chr (highly likely), in which case reduce the volume
		// to be tested to the ground Y value. I'm not sure why this is useful,
		// because if the chr was being spawned on top of another chr or object
		// then the calculated ground value would be raised.
		ymin = -200;
		ground = cdFindGroundAtCyl(pos, chrradius, rooms, NULL, NULL);

		if (ground > -100000 && ground - pos->y < -200) {
			ymin = ground - pos->y;
		}

		if (cdTestVolume(pos, chrradius, rooms, types, CHECKVERTICAL_YES, ymax, ymin) != CDRESULT_COLLISION
				&& (allowonscreen || chrIsPosOffScreen(pos, rooms))) {
			return true;
		}
	}

	// On Defection some floating point precision issues result in P2 being placed in the way of P1's cutscene animation,
	// which makes P1 start stuck in P2 in coop, so the distance is increased to 80 to avoid that
	const float distance = (g_Vars.stagenum == STAGE_DEFECTION) ? 80.f : 60.f;
	for (i = 0; i < 8; i++) {
		testpos.x = pos->x + sinf(curangle) * distance;
		testpos.y = pos->y;
		testpos.z = pos->z + cosf(curangle) * distance;
		if ((onlysurrounding && cdTestCylMove04(pos, rooms, &testpos, testrooms, CDTYPE_ALL & ~CDTYPE_PLAYERS, 1, ymax, -200) != CDRESULT_COLLISION)
				|| (!onlysurrounding && cdTestLos11(pos, rooms, &testpos, testrooms, CDTYPE_BG))) {
			chrFindEnteredRooms(NULL, &testpos, testrooms);
			ground = cdFindGroundAtCyl(&testpos, chrradius, testrooms, 0, 0);
			ymin = -200;

			if (ground > -100000 && ground - pos->y < -200) {
				ymin = ground - pos->y;
			}

			if (cdTestVolume(&testpos, chrradius, testrooms, CDTYPE_ALL, CHECKVERTICAL_YES, ymax, ymin) != CDRESULT_COLLISION
					&& (allowonscreen || chrIsPosOffScreen(&testpos, testrooms))
					&& (!onlysurrounding || ground > -100000)) {
				pos->x = testpos.x;
				pos->y = testpos.y;
				pos->z = testpos.z;
				roomsCopy(testrooms, rooms);
				return true;
			}
		}

		curangle += 0.7852731347084f;

		if (curangle >= M_TAU) {
			curangle -= M_TAU;
		}
	}

	return false;
}

/**
 * Attempts to spawn a chr at the given coordinates.
 *
 * In low memory conditions, the function will iterate all existing chrs in
 * search of a corpse that can be reaped. If one is found then the reap will be
 * triggered, but the function will not attempt to spawn the chr until the next
 * time it's called.
 */
struct prop *chrSpawnAtCoord(int bodynum, int headnum, struct coord *pos, RoomNum *rooms, float angle, uint8_t *ailist, uint32_t spawnflags)
{
	struct prop *prop;
	struct coord pos2;
	RoomNum rooms2[8];

	if (chrsGetNumFree() > 1) {
		if (headnum < 0) {
			headnum = bodyChooseHead(bodynum);
		}

		pos2.x = pos->x;
		pos2.y = pos->y;
		pos2.z = pos->z;
		roomsCopy(rooms, rooms2);

		if (chrAdjustPosForSpawn(20, &pos2, rooms2, angle, (spawnflags & SPAWNFLAG_ALLOWONSCREEN) != 0, false, false))
		{
			struct model *model = bodyAllocateModel(bodynum, headnum, spawnflags);
			struct chrdata *chr;

			if (model) {
				prop = chrAllocate(model, &pos2, rooms2, angle, ailist);

				if (prop) {
					propActivateThisFrame(prop);
					propEnable(prop);

					chr = prop->chr;
					chr->headnum = headnum;
					chr->bodynum = bodynum;
					chr->race = bodyGetRace(chr->bodynum);
					chr->flags = 0;
					chr->flags2 = 0;
					chr->hidden2 |= CHRH2FLAG_SPAWNED;

					if (spawnflags & SPAWNFLAG_NOBLOOD) {
						chr->noblood = true;
					}

					return prop;
				}

				modelmgrFreeModel(model);
			}
		}
	}

	// Low memory - find a corpse to reap
	if (chrsGetNumFree() < 4) {
		struct chrdata *replacechr;
		int startindex;
		int index;

		replacechr = NULL;
		startindex = rngRandom() % g_NumChrSlots;
		index = startindex;

		do {
			if (g_ChrSlots[index].chrnum >= 0 && g_ChrSlots[index].model && g_ChrSlots[index].prop) {
				if (g_ChrSlots[index].actiontype == ACT_DEAD
						|| (g_ChrSlots[index].actiontype == ACT_DRUGGEDKO && (g_ChrSlots[index].chrflags & CHRCFLAG_KEEPCORPSEKO) == 0)
						) {
					// If we've found a chr that's ready to be reaped, great.
					// Bail out of the loop.
					if (g_ChrSlots[index].act_dead.invistimer60 >= TICKS(120)) {
						replacechr = &g_ChrSlots[index];
						break;
					}

					// Otherwise, this chr is dead/KO'ed and can be reaped as a
					// last resort, so store them and keep looping in search of
					// a better chr.
					if (replacechr == NULL) {
						replacechr = &g_ChrSlots[index];
					}
				}
			}

			index = (index + 1) % g_NumChrSlots;
		} while (index != startindex);

		if (replacechr) {
			replacechr->act_dead.fadewheninvis = true;
			replacechr->act_dead.fadenow = true;
		}
	}

	return NULL;
}

struct prop *chrSpawnAtPad(struct chrdata *basechr, int body, int head, int pad_id, uint8_t *ailist, uint32_t spawnflags)
{
	int resolved_pad_id = chrResolvePadId(basechr, pad_id);
	struct pad pad;
	RoomNum room[2];
	float fvalue;
	padUnpack(resolved_pad_id, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_ROOM, &pad);
	fvalue = atan2f(pad.look.x, pad.look.z);
	room[0] = pad.room;
	room[1] = -1;

	return chrSpawnAtCoord(body, head, &pad.pos, &room[0], fvalue, ailist, spawnflags);
}

struct prop *chrSpawnAtChr(struct chrdata *basechr, int body, int head, uint32_t chrnum, uint8_t *ailist, uint32_t spawnflags)
{
	struct chrdata *chr = chrFindById(basechr, chrnum);
	float fvalue;

	if (1) {
		fvalue = chrGetInverseTheta(chr);
	}

	return chrSpawnAtCoord(body, head, &chr->prop->pos, chr->prop->rooms, fvalue, ailist, spawnflags);
}

bool chrIsPropPresetBlockingSightToTarget(struct chrdata *chr)
{
	bool result = false;
	struct prop *prop = chr->prop;
	struct prop *target = chrGetTargetProp(chr);

	if (chr->proppreset1 >= 0) {
		chrSetPerimEnabled(chr, false);
		propSetPerimEnabled(target, false);

		if (!cdTestLos04(&prop->pos, prop->rooms, &target->pos,
					CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_PATHBLOCKER | CDTYPE_BG)) {
			struct prop *obstacle = cdGetObstacleProp();

			if (obstacle && obstacle->type == PROPTYPE_OBJ
					&& chr->proppreset1 == (int16_t)(obstacle - g_Vars.props)) {
				result = true;
			}
		}

		chrSetPerimEnabled(chr, true);
		propSetPerimEnabled(target, true);
	}

	return result;
}

bool chrMoveToPos(struct chrdata *chr, struct coord *pos, RoomNum *rooms, float angle, bool force)
{
	struct coord pos2;
	RoomNum rooms2[8];
	bool result = false;
	uint32_t nodetype;
	union modelrwdata *rwdata;
	struct player *player;
	float ground;

	pos2.x = pos->x;
	pos2.y = pos->y;
	pos2.z = pos->z;

	roomsCopy(rooms, rooms2);
	propSetPerimEnabled(chr->prop, false);

	if (chrAdjustPosForSpawn(chr->radius, &pos2, rooms2, angle, (chr->hidden & CHRHFLAG_WARPONSCREEN) != 0, force, (chr->hidden & CHRHFLAG_SPAWNONLYSURROUNDING) != 0))
	{
		ground = cdFindGroundInfoAtCyl(&pos2, chr->radius, rooms2, &chr->floorcol,
				&chr->floortype, NULL, &chr->floorroom, false, NULL);

		chr->ground = ground;
		chr->manground = ground;
		chr->sumground = ground * 10.0f;
		chr->prop->pos.x = pos2.x;
		chr->prop->pos.y = pos2.y;
		chr->prop->pos.z = pos2.z;

		propDeregisterRooms(chr->prop);
		roomsCopy(rooms2, chr->prop->rooms);
		chrUpdateRooms(chr);
		modelSetRootPosition(chr->model, &pos2);

		nodetype = chr->model->definition->rootnode->type;

		if ((nodetype & 0xff) == MODELNODETYPE_CHRINFO) {
			rwdata = modelGetNodeRwData(chr->model, chr->model->definition->rootnode);
			rwdata->chrinfo.ground = ground;
		}

		chr->chrflags |= CHRCFLAG_FORCETOGROUND;
		chrSetLookAngle(chr, angle);

		if (chr->prop->type == PROPTYPE_PLAYER) {
			player = g_Vars.players[playermgrGetPlayerNumByProp(chr->prop)];
			player->vv_manground = ground;
			player->vv_ground = ground;
			player->vv_theta = ((M_TAU - angle) * 360.0f) / M_TAU;
			player->vv_verta = 0;
			player->unk1c64 = 1;
		}

		result = true;
	}

	propSetPerimEnabled(chr->prop, true);

	return result;
}

bool chrCheckCoverOutOfSight(struct chrdata *chr, int covernum, bool soft)
{
	struct cover cover;
	struct prop *target;
	bool targetcanseecover;

	// @bug: Should be >= coverGetCount()
	if (covernum < 0 || covernum > coverGetCount() || !coverUnpack(covernum, &cover)) {
		return false;
	}

	target = chrGetTargetProp(chr);

	if (!target) {
		return false;
	}

	if (soft) {
		targetcanseecover = cdTestLos03(&target->pos, target->rooms, cover.pos,
				CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_BG,
				GEOFLAG_BLOCK_SIGHT);
	} else {
		targetcanseecover = cdIsNearlyInSight(&target->pos, target->rooms, cover.pos, 50.0f,
				CDTYPE_OBJS | CDTYPE_DOORS | CDTYPE_BG);
	}

	if (!targetcanseecover != false) {
		// Target can't see cover
		coverSetOutOfSight(covernum, true);
		return true;
	}

	coverSetOutOfSight(covernum, false);
	return false;
}

int chrAssignCoverByCriteria(struct chrdata *chr, uint16_t criteria, int refdist)
{
	RoomNum rooms[8];
	struct cover cover;
	struct covercandidate tmp;
	int oldcover;
	int i;
	struct prop *roomprop;
	int numcovers = coverGetCount();
	int numcandidates = 0;
	struct prop *target = chrGetTargetProp(chr);
	bool userandomdist = false;
	bool changed;
	float sqdist;
	float y = chr->prop->pos.y + 170;
	int currefdist = refdist;
	struct prop *gotoprop;

	if (criteria & COVERCRITERIA_DISTTOFETCHPROP) {
		if (!chr->aibot || !chr->aibot->gotoprop) {
			return -1;
		}

		gotoprop = chr->aibot->gotoprop;
	}

	if (chr == NULL) {
		return 0;
	}

	oldcover = chr->cover;
	refdist *= refdist;

	// Iterate all cover, filter them by criteria and store them in g_CoverCandidates
	for (i = 0; i < numcovers; i++) {
		if (coverUnpack(i, &cover)
				&& !coverIsSpecial(&cover)
				&& ((criteria & COVERCRITERIA_2000) == 0 || (cover.flags & COVERFLAG_OMNIDIRECTIONAL))
				&& ((criteria & COVERCRITERIA_1000) || (cover.flags & COVERFLAG_AIMDIFFROOM) == 0 || !roomArrayIntersects(cover.rooms, target->rooms))) {
			userandomdist = false;

			if ((criteria & COVERCRITERIA_0001) && (criteria & COVERCRITERIA_FURTHEREST)) {
				userandomdist = true;
				criteria &= ~(COVERCRITERIA_0001 | COVERCRITERIA_FURTHEREST);
			}

			if (((criteria & COVERCRITERIA_FORCENEWCOVER) == 0 || i != oldcover)
					&& ((criteria & COVERCRITERIA_2000) || !(coverIsInUse(i) || cover.pos->y > y))) {
				if (criteria & COVERCRITERIA_ROOMSFROMME) {
					roomprop = chr->prop;
				} else if (criteria & COVERCRITERIA_ROOMSFROMTARGET) {
					roomprop = target;
				} else if (criteria & COVERCRITERIA_DISTTOTARGET) {
					roomprop = target;
				} else {
					roomprop = chr->prop;
				}

				rooms[0] = roomprop->rooms[0];
				rooms[1] = -1;

				if (criteria & COVERCRITERIA_ALLOWNEIGHBOURINGROOMS) {
					bgRoomGetNeighbours(roomprop->rooms[0], &rooms[1], 6);
				} else if (criteria & COVERCRITERIA_ONLYNEIGHBOURINGROOMS) {
					bgRoomGetNeighbours(roomprop->rooms[0], &rooms[0], 7);
				}

				if (((criteria & COVERCRITERIA_0040) == 0 || !roomArrayIntersects(cover.rooms, rooms))
						&& ((criteria & COVERCRITERIA_0020) == 0 || roomArrayIntersects(cover.rooms, rooms))
						&& (rooms[1] == -1
							|| chr->oldrooms[0] == -1
							|| (criteria & COVERCRITERIA_0200) == 0
							|| !roomArrayIntersects(cover.rooms, chr->oldrooms))) {
					if (criteria & COVERCRITERIA_DISTTOME) {
						sqdist = chrGetSquaredDistanceToCoord(chr, cover.pos);
					} else if (criteria & COVERCRITERIA_DISTTOTARGET) {
						sqdist = coordGetSquaredDistanceToCoord(&target->pos, cover.pos);
					} else if (criteria & COVERCRITERIA_DISTTOFETCHPROP) {
						sqdist = coordGetSquaredDistanceToCoord(&gotoprop->pos, cover.pos);
					} else if (userandomdist) {
						sqdist = rngRandom() % 0xf000;
					} else {
						sqdist = 0;
						currefdist = 0;
					}

					if (!currefdist
							|| (currefdist < 0 && sqdist > refdist)
							|| (currefdist > 0 && sqdist < refdist)) {
						g_CoverCandidates[numcandidates].sqdist = sqdist;
						g_CoverCandidates[numcandidates].covernum = i;
						numcandidates++;
					}
				}
			}
		}
	}

	// Sort candidates by distance ascending, or descending if using COVERCRITERIA_FURTHEREST
	if (numcandidates >= 2) {
		do {
			changed = false;

			for (i = 0; i < numcandidates - 1; i++) {
				if ((((criteria & COVERCRITERIA_0001) || (criteria & COVERCRITERIA_DISTTOFETCHPROP) || userandomdist) && (g_CoverCandidates[i].sqdist > g_CoverCandidates[i + 1].sqdist))
						|| ((criteria & COVERCRITERIA_FURTHEREST) && g_CoverCandidates[i].sqdist < g_CoverCandidates[i + 1].sqdist)) {
					changed = true;

					tmp = g_CoverCandidates[i];
					g_CoverCandidates[i] = g_CoverCandidates[i + 1];
					g_CoverCandidates[i + 1] = tmp;
				}
			}
		} while (changed);
	}

	// Assign the first out of sight cover
	for (i = 0; i < numcandidates; i++) {
		if (1 && chrCheckCoverOutOfSight(chr, g_CoverCandidates[i].covernum, criteria & COVERCRITERIA_ALLOWSOFT)) {
			chr->cover = g_CoverCandidates[i].covernum;

			if (oldcover != -1) {
				coverSetInUse(oldcover, false);
			}

			coverSetInUse(chr->cover, true);

			return g_CoverCandidates[i].covernum;
		}
	}

	return -1;
}

/**
 * Find cover in the opposite direction (?) of the chr's runfrompos and assign
 * it to the chr. The distance from the runfrompos to the cover must be between
 * mindist and maxdist along the X/Z plane. The chr will not choose cover more
 * than 170cm higher than their current elevation, but strangely there is no
 * lower Y limit.
 *
 * Testing with this function logic results in the following, where:
 * R = run from pos
 * C = chr's pos
 * A = available cover
 * . = unavailable cover
 *
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A R A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . .
 *  A A A A A A A A A A A A A A . . . . . A
 *  A A A A A A A A A A A A A A . . . A A A
 *  A A A A A A A A A A A A A A . A A A A A
 *  A A A A A A A A A A A A A C A A A A A A
 *  . . . . . . . . . . . A A A A A A A A A
 *  . . . . . . . . . A A A A A A A A A A A
 *
 * The block of available cover around the runfrompos might be a bug, but if
 * this function is called with mindist = chr - runfrompos or higher then it
 * will work as expected.
 *
 * Preference is given to cover which is the "most opposite", meaning ones
 * which are directly behind the chr from the perspective of runfrompos.
 */
int chrAssignCoverAwayFromDanger(struct chrdata *chr, int mindist, int maxdist)
{
	int i;
	float vecfromdanger[2];
	float vectocover[2];
	float y;
	float ymax;
	float bestsqdist;
	float sqdist;
	int numcovers;
	int prevcover;
	int newcover;
	float coversqdistfrompos;
	struct cover cover;

	ymax = chr->prop->pos.y + 170;
	y = 0;
	bestsqdist = 0;
	newcover = -1;

	numcovers = coverGetCount();
	prevcover = chr->cover;

	mindist *= mindist;
	maxdist *= maxdist;

	if (mindist);
	if (maxdist);

	vecfromdanger[0] = chr->prop->pos.x - chr->runfrompos.x;
	vecfromdanger[1] = chr->prop->pos.z - chr->runfrompos.z;

	utilsNormalizeF(&vecfromdanger[0], &y, &vecfromdanger[1]);

	for (i = 0; i < numcovers; i++) {
		if (coverUnpack(i, &cover) && !coverIsInUse(i) && !(cover.pos->y > ymax) && !coverIsSpecial(&cover)) {
			coversqdistfrompos = coordGetSquaredDistanceToCoord(&chr->runfrompos, cover.pos);

			if (!(coversqdistfrompos < mindist) && !(coversqdistfrompos > maxdist)) {
				vectocover[0] = cover.pos->x - chr->prop->pos.x;
				vectocover[1] = cover.pos->z - chr->prop->pos.z;

				utilsNormalizeF(&vectocover[0], &y, &vectocover[1]);

				sqdist = vecfromdanger[0] * vectocover[0] + vecfromdanger[1] * vectocover[1];

				if (!(sqdist < 0) && sqdist > bestsqdist) {
					bestsqdist = sqdist;
					newcover = i;
					if (1);
				}
			}
		}

		if (numcovers);
	}

	chr->cover = newcover;

	if (newcover != -1) {
		if (prevcover != -1) {
			coverSetInUse(prevcover, false);
		}

		coverSetInUse(chr->cover, true);
	}

	return newcover;
}

int16_t chrGoToCover(struct chrdata *chr, uint8_t speed)
{
	struct cover cover;

	if (!chr) {
		return 0;
	}

	if (chrIsReadyForOrders(chr) && chr->cover != -1 && coverUnpack(chr->cover, &cover)) {
		chrGoToRoomPos(chr, cover.pos, &cover.rooms[0], speed);
		return chr->cover;
	}

	return -1;
}

bool chrRunFromPos(struct chrdata *chr, uint32_t goposflags, float rundist, struct coord *frompos)
{
	float curdistfrompos;
	struct coord delta;
	RoomNum rooms[8];

	if (chrIsReadyForOrders(chr)) {
		delta.x = chr->prop->pos.x - frompos->x;
		delta.y = chr->prop->pos.y;
		delta.z = chr->prop->pos.z - frompos->z;

		// @bug: This check should be &&. The runfrompos will fail if the
		// frompos is on the same X or Z axis as the chr, which is unlikely
		// because it's a floating point number.
		//if (delta.f[0] == 0.0f || delta.f[2] == 0.0f) {
		if (delta.f[0] == 0.0f && delta.f[2] == 0.0f) { // Ben's comment: fixing this
			return false;
		}

		curdistfrompos = sqrtf(delta.f[0] * delta.f[0] + delta.f[2] * delta.f[2]);
		delta.x *= rundist / curdistfrompos;
		delta.z *= rundist / curdistfrompos;

		chrSetPerimEnabled(chr, false);

		if (cdExamLos08(&chr->prop->pos, chr->prop->rooms, &delta, CDTYPE_ALL, GEOFLAG_WALL) == CDRESULT_COLLISION) {
			cdGetPos(&delta);
		}

		chrSetPerimEnabled(chr, true);

		propUpdatePositionRoomsSimple(&chr->prop->pos, chr->prop->rooms, &delta, rooms);

		return chrGoToRoomPos(chr, &delta, rooms, goposflags);
	}

	return false;
}

void chrAddTargetToBdlist(struct chrdata *chr)
{
	if (chr->prop) {
		struct prop *target = chrGetTargetProp(chr);
		int i;

		if (target) {
			for (i = 0; i < g_Vars.lvupdate60; i++) {
				chr->bdlist[chr->bdstart] = target->pos.x - chr->prop->pos.x;
				chr->bdstart++;
				chr->bdstart %= 60;

				chr->bdlist[chr->bdstart] = target->pos.z - chr->prop->pos.z;
				chr->bdstart++;
				chr->bdstart %= 60;
			}
		}
	}
}

int chrGetDistanceLostToTargetInLastSecond(struct chrdata *chr)
{
	int *bdlist = &chr->bdlist[0];
	int index = chr->bdstart;

	int x1 = bdlist[(index + 1) % 60];
	int z1 = bdlist[index];
	int olddist = sqrtf(x1 * x1 + z1 * z1);

	int x2 = bdlist[(index + 59) % 60];
	int z2 = bdlist[(index + 58) % 60];
	int curdist = sqrtf(x2 * x2 + z2 * z2);

	return curdist - olddist;
}

bool chrIsTargetNearlyInSight(struct chrdata *chr, uint32_t distance)
{
	struct prop *target = chrGetTargetProp(chr);

	return cdIsNearlyInSight(&chr->prop->pos, chr->prop->rooms, &target->pos, distance, CDTYPE_BG);
}

bool chrIsNearlyInTargetsSight(struct chrdata *chr, uint32_t distance)
{
	struct prop *target = chrGetTargetProp(chr);

	return cdIsNearlyInSight(&target->pos, target->rooms, &chr->prop->pos, distance, CDTYPE_BG);
}

float func0f04c784(struct chrdata *chr)
{
	float targetfacingangle = 0;
	struct prop *target = chrGetTargetProp(chr);
	float angletotarget;
	float result;

	if (target->type == PROPTYPE_CHR) {
		targetfacingangle = chrGetInverseTheta(target->chr);
	} else if (target->type == PROPTYPE_PLAYER) {
		int playernum = playermgrGetPlayerNumByProp(target);
		targetfacingangle = g_Vars.players[playernum]->vv_theta;
	}

	angletotarget = atan2f(target->pos.z - chr->prop->pos.z, target->pos.x - chr->prop->pos.x);
	result = (angletotarget * 360 / M_TAU - targetfacingangle) + 90;

	if (result > 180) {
		result -= 360;
	}

	return result;
}

/*
 * Attempts to reposition the character by rotating around their target.
 * 
 * This function is often used when a bot needs to sidestep or maneuver relative
 * to its target without running into geometry. It rotates the bot around the
 * target's position, checks for collisions, and adjusts accordingly.
 *
 */
bool chr0f04c874(struct chrdata *chr, uint32_t angle360, struct coord *pos, uint8_t arg3, uint8_t arg4)
{
	struct prop *target = chrGetTargetProp(chr);
	float sqdist = 0;
	float chrFacingDeg = func0f04c784(chr); // Character's current facing angle
	float cosine;
	float sine;
	struct coord chrpos;
	float xdiff;
	float ydiff;
	float zdiff;
	float scale;
	bool retry;
	struct coord saved;
	int result;
	float ymax;
	float ymin;
	float radius;

	// Save the character's current position
	chrpos = chr->prop->pos;

	do {
		float angle360f = angle360;

		// Convert angle to signed range [-180, 180]
		if (angle360f > 180) {
			angle360f -= 360;
		}

		// Compute relative angle difference from character's facing
		angle360f -= chrFacingDeg;

		// Convert to radians
		angle360f *= 0.017450513318181f;

		// Calculate 2D rotation components
		cosine = cosf(angle360f);
		sine = sinf(angle360f);

		// Vector from target to character
		xdiff = chrpos.x - target->pos.x;
		zdiff = chrpos.z - target->pos.z;

		// Rotate position around target and store result in pos
		pos->x = target->pos.x + (xdiff * cosine - zdiff * sine);
		pos->y = chrpos.y;
		pos->z = target->pos.z + (xdiff * sine + zdiff * cosine);

		// Get bounding box of the character
		chrGetBbox(chr->prop, &radius, &ymax, &ymin);

		// Test if this new position causes a collision
		result = cdExamCylMove03(&chrpos, chr->prop->rooms, pos,
				CDTYPE_BG | CDTYPE_OBJS | CDTYPE_DOORS, 1,
				ymax - chrpos.f[1],
				ymin - chrpos.f[1]);

		if (result == CDRESULT_COLLISION) {
			float xdiff;
			float zdiff;
			float tmp;

			// If blocked, reduce movement distance and try again
			cdGetPos(pos); // Get actual adjusted position after collision response

			xdiff = pos->x - chrpos.x;
			zdiff = pos->z - chrpos.z;
			tmp = sqrtf(xdiff * xdiff + zdiff * zdiff);
			scale = (tmp - 50.0f) / tmp;

			if (scale < 0) {
				// Too close to proceed — stay in place
				*pos = chrpos;
			} else {
				// Reduce step
				pos->x = chrpos.x + xdiff * scale;
				pos->y = chrpos.y;
				pos->z = chrpos.z + zdiff * scale;
			}
		}

		// If retry is allowed, remember the current pos and try the opposite angle
		if (arg3) {
			xdiff = chrpos.x - pos->x;
			ydiff = chrpos.y - pos->y;
			zdiff = chrpos.z - pos->z;

			sqdist = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

			saved.x = pos->x;
			saved.y = pos->y;
			saved.z = pos->z;

			angle360 = 360 - angle360;
			retry = true;
			arg3 = 0;
		} else {
			retry = false;
		}
	} while (retry);

	// If two positions were tested, use the one that moved less
	if (sqdist != 0) {
		float sqdist2;
		xdiff = chrpos.x - pos->x;
		ydiff = chrpos.y - pos->y;
		zdiff = chrpos.z - pos->z;

		sqdist2 = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

		if (sqdist2 < sqdist) {
			pos->x = saved.f[0];
			pos->y = saved.f[1];
			pos->z = saved.f[2];
		}
	}

	chrGoToPos(chr, pos, arg4);

	return true;
}

/**
 * Chrs are assigned to teams, and they can be assigned to more than one.
 * The team assignments determine if a chr considers another chr to be friendly
 * or not.
 *
 * The chr->team value is a bitmask of which teams they belong to. There are 8
 * teams total.
 *
 * The team list is an array of 264 shorts. The first 7 are indexes into
 * the same list which mark the start of each team. Team 0 does not have
 * an entry in this list because it always starts at offset 7.
 *
 * Elements 7 onwards are chrnums. Each team is terminated with -2.
 */
void rebuildTeams(void)
{
	int numchrs = chrsGetNumSlots();
	int16_t index = 7;
	int team;
	int i;
	struct chrdata *chr;
	uint8_t teammasks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	for (team = 0; team < ARRAYCOUNT(teammasks); team++) {
		if (team != 0) {
			g_TeamList[team - 1] = index;
		}

		for (i = 0; i < numchrs; i++) {
			chr = &g_ChrSlots[i];

			if (chr->chrnum >= 0 && (chr->team & teammasks[team])) {
				g_TeamList[index] = chr->chrnum;
				index++;
			}
		}

		g_TeamList[index] = -2;
		index++;

		if (index >= (MAX_CHRSPERTEAM + 1) * MAX_TEAMS) {
			break;
		}
	}
}

/**
 * Chrs are partitioned into squadrons for AI scripting purposes, where their
 * squadron number is in the range 0-15.
 *
 * The squadron list is an array of 272 shorts. The first 15 are indexes into
 * the same list which mark the start of each squadron. Squadron 0 does not have
 * an entry in this list because it always starts at offset 15.
 *
 * Elements 15 onwards are chrnums. Each squadron is terminated with -2.
 */
void rebuildSquadrons(void)
{
	int numchrs = chrsGetNumSlots();
	int16_t index = 15;
	int squadron;
	int i;

	for (squadron = 0; squadron < MAX_SQUADRONS; squadron++) {
		if (squadron != 0) {
			g_SquadronList[squadron - 1] = index;
		}

		for (i = 0; i < numchrs; i++) {
			struct chrdata *chr = &g_ChrSlots[i];

			if (chr->chrnum >= 0 && chr->squadron == squadron) {
				if (chr->prop == NULL || chr->prop->type != PROPTYPE_PLAYER) {
					g_SquadronList[index] = chr->chrnum;
					index++;
				}
			}
		}

		g_SquadronList[index] = -2;
		index++;

		if (index >= (MAX_CHRSPERSQUADRON + 1) * MAX_SQUADRONS) {
			break;
		}
	}
}

int16_t *teamGetChrIds(int team_id)
{
	int i;
	uint8_t lookup[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	for (i = 0; i < MAX_TEAMS; i++) {
		if (lookup[i] == team_id) {
			team_id = i;
			break;
		}
	}

	if (team_id < 0 || team_id >= MAX_TEAMS) {
		return NULL;
	}

	if (team_id != 0) {
		return &g_TeamList[g_TeamList[team_id - 1]];
	}

	return &g_TeamList[MAX_TEAMS - 1];
}

int16_t *squadronGetChrIds(int squadron_id)
{
	if (squadron_id < 0 || squadron_id >= MAX_SQUADRONS) {
		return NULL;
	}

	if (squadron_id != 0) {
		return &g_SquadronList[g_SquadronList[squadron_id - 1]];
	}

	return &g_SquadronList[MAX_SQUADRONS - 1];
}

void audioMarkAsRecentlyPlayed(int16_t audioid)
{
	g_RecentQuipsPlayed[g_RecentQuipsIndex++] = audioid;

	if (g_RecentQuipsIndex >= ARRAYCOUNT(g_RecentQuipsPlayed)) {
		g_RecentQuipsIndex = 0;
	}
}

bool audioWasNotPlayedRecently(int16_t audioid)
{
	uint8_t i;

	for (i = 0; i < ARRAYCOUNT(g_RecentQuipsPlayed); i++) {
		if (g_RecentQuipsPlayed[i] == audioid) {
			return false;
		}
	}

	return true;
}

void chrToggleModelPart(struct chrdata *chr, int partnum)
{
	if (chr && chr->model && chr->model->definition) {
		struct modelnode *node = modelGetPart(chr->model->definition, partnum);
		union modelrwdata *rwdata = NULL;

		if (node) {
			rwdata = modelGetNodeRwData(chr->model, node);
		}

		if (rwdata) {
			bool visible = rwdata->toggle.visible;
			rwdata->toggle.visible = !visible;
		}
	}
}

/**
 * Make the chr do an avoiding animation, as if they are avoiding some object
 * being thrown at them.
 *
 * The chosen animation is based on the chr's relative angle to their target.
 *
 * Collision checks are done, and no animation will be done if there would be a
 * collision.
 */
void chrAvoid(struct chrdata *chr)
{
	// The first 4 items here are animation numbers
	// and the second 4 are their corresponding end frames.
	int anims[] = {
		ANIM_AVOID_0064, ANIM_AVOID_0065, ANIM_AVOID_0066, ANIM_AVOID_0067,
		48, 58, 35, 35
	};

	int animindex;
	struct prop *target = chrGetTargetProp(chr);
	float relangle = chrGetAngleToPos(chr, &target->pos) / M_TAU * 360;
	uint32_t chranimflags = 0;
	float ymax;
	float ymin;
	float radius;
	int cdresult = CDRESULT_NOCOLLISION;
	float xdiff;
	float zdiff;
	float halfchrradius;
	float chrangle = modelGetChrRotY(chr->model);
	RoomNum dstrooms[8];
	struct coord dstpos;

	// @bug: This shouldn't be here, and the perim is not enabled again
	// if the chr is not ready for orders.
	chrSetPerimEnabled(chr, false);

	if (chrIsReadyForOrders(chr)) {
		chrSetPerimEnabled(chr, false);

		if (relangle > 45 && relangle <= 135) {
			animindex = 3;
			chranimflags = CHRANIMFLAG_FLIP;
			chrangle += M_PI;
		} else if (relangle > 135 && relangle < 225) {
			animindex = 2;

			if ((rngRandom() % 255) >= 2) {
				cdresult = CDRESULT_COLLISION;
				chr->chrflags &= ~CHRCFLAG_AVOIDING;
			}
		} else if (relangle > 225 && relangle < 315) {
			animindex = 3;
			chrangle += M_PI;
		} else {
			animindex = (rngRandom() % 100 < 50) ? 0 : 1;

			if (animindex == 0) {
				chrangle -= 1.5707963705063f;
			} else {
				chrangle += 1.5707963705063f;
			}
		}

		if (cdresult != CDRESULT_COLLISION) {
			dstpos.x = chr->prop->pos.x - sinf(chrangle) * 100;
			dstpos.y = chr->prop->pos.y;
			dstpos.z = chr->prop->pos.z + cosf(chrangle) * 100;

			chrGetBbox(chr->prop, &radius, &ymax, &ymin);

			halfchrradius = radius * 0.5f;

			propUpdatePositionRoomsSimple(&chr->prop->pos, chr->prop->rooms, &dstpos, dstrooms);
			chrFindEnteredRooms(chr, &dstpos, dstrooms);

			xdiff = dstpos.x - chr->prop->pos.x;
			zdiff = dstpos.z - chr->prop->pos.z;

			if (xdiff > halfchrradius || zdiff > halfchrradius || xdiff < -halfchrradius || zdiff < -halfchrradius) {
				cdresult = cdExamCylMove05(&chr->prop->pos, chr->prop->rooms, &dstpos, dstrooms, CDTYPE_ALL, true, ymax - chr->prop->pos.y, ymin - chr->prop->pos.y);
			}

			if (cdresult == CDRESULT_ERROR) {
				chr->chrflags &= ~CHRCFLAG_AVOIDING;
			} else if (cdresult == CDRESULT_NOCOLLISION) {
				chrStartAnim(chr, anims[animindex], 0, anims[4 + animindex], chranimflags, 2, 0.6f);
			} else {
				chr->chrflags &= ~CHRCFLAG_AVOIDING;
			}
		}

		chrSetPerimEnabled(chr, true);
	}
}

/**
 * Checks if the chr is doing an avoiding animation.
 *
 * These animations are possibly unused. In each one, the chr jumps backwards or
 * to the side as if avoiding something, then looks at whatever it was that just
 * went past.
 */
bool chrIsAvoiding(struct chrdata *chr)
{
	int anim = modelGetAnimNum(chr->model);
	chr->chrflags &= CHRCFLAG_AVOIDING;

	// Possible @bug or just sloppy code: The flag check below can never pass
	// because that flag was just turned off above.
	if (anim == ANIM_AVOID_0064
			|| anim == ANIM_AVOID_0065
			|| anim == ANIM_AVOID_0066
			|| anim == ANIM_AVOID_0067
			|| (chr->chrflags & ~CHRCFLAG_AVOIDING)) {
		return true;
	}

	return false;
}

void chrDrCarollEmitSparks(struct chrdata *chr)
{
	if (chr && chr->prop) {
		psCreate(0, chr->prop, SFX_SHIELD_DAMAGE, -1, -1, 0, 0, PSTYPE_NONE, 0, -1, 0, -1, -1, -1, -1);
		sparksCreate(chr->prop->rooms[0], chr->prop, &chr->prop->pos, NULL, 0, SPARKTYPE_ELECTRICAL);
	}
}
