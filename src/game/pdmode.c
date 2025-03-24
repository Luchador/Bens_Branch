#include <ultra64.h>
#include "constants.h"
#include "game/pdmode.h"
#include "game/bondgun.h"
#include "game/weaponutils.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/options.h"
#include "bss.h"
#include "data.h"
#include "types.h"

f32 pdmodeGetEnemyReactionSpeed(void)
{
	return 0;
}

f32 pdmodeGetEnemyHealth(void)
{
	if (g_MissionConfig.pdmode) {
		return g_MissionConfig.pdmodehealthf;
	}

	return 1;
}

f32 pdmodeGetEnemyDamage(void)
{
	if (g_MissionConfig.pdmode) {
		return g_MissionConfig.pdmodedamagef;
	}

	return 1;
}

f32 pdmodeGetEnemyAccuracy(void)
{
	if (g_MissionConfig.pdmode) {
		return g_MissionConfig.pdmodeaccuracyf;
	}

	return 1;
}