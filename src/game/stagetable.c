#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "data.h"
#include "types.h"

// When adding or removing items from this table you must also update the
// STAGEINDEX constants in constants.h.
// NOTE: extra fields have been appended to stagetableentry in the PC port
struct stagetableentry g_Stages[61] = {
	//       id,                  light type, lia, liw, lih, ?, bg,               tiles,              pads,              setup,           mpsetp,                                                   ?,   ?,   ?   alarm                   extragunmem
	/*0x00*/ STAGE_MAIANSOS,      2,          255, 100, 100, 0, FILE_BG_LUE_SEG,  FILE_BG_LUE_TILES,  FILE_BG_SEV_PADS,  FILE_USETUPSEV,  FILE_UMP_SETUPSEV,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x01*/ STAGE_WAR,           2,          255, 100, 100, 0, FILE_BG_SHO_SEG,  FILE_BG_SHO_TILES,  FILE_BG_STAT_PADS, FILE_USETUPSTAT, FILE_UMP_SETUPSTAT,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x02*/ STAGE_MP_RAVINE,     2,          255, 100, 100, 0, FILE_BG_AREC_SEG, FILE_BG_AREC_TILES, FILE_BG_AREC_PADS, FILE_USETUPAREC, FILE_UMP_SETUPAREC,                                     700, 800, 400,  SFX_ALARM_DEFAULT,      0,
	/*0x03*/ STAGE_ESCAPE,        2,          255, 100, 100, 0, FILE_BG_LUE_SEG,  FILE_BG_LUE_TILES,  FILE_BG_TRA_PADS,  FILE_USETUPTRA,  FILE_UMP_SETUPTRA,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x04*/ STAGE_CRASHSITE,     2,          255, 100, 100, 0, FILE_BG_AZT_SEG,  FILE_BG_AZT_TILES,  FILE_BG_AZT_PADS,  FILE_USETUPAZT,  FILE_UMP_SETUPAZT,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x05*/ STAGE_CHICAGO,       2,          255, 100, 100, 0, FILE_BG_PETE_SEG, FILE_BG_PETE_TILES, FILE_BG_PETE_PADS, FILE_USETUPPETE, FILE_UMP_SETUPPETE,                                     -1,  400, 0,    SFX_ALARM_CHICAGO,      25 * 1024,
	/*0x06*/ STAGE_G5BUILDING,    2,          255, 100, 100, 0, FILE_BG_DEPO_SEG, FILE_BG_DEPO_TILES, FILE_BG_DEPO_PADS, FILE_USETUPDEPO, FILE_UMP_SETUPDEPO,                                     -1,  400, 0,    SFX_ALARM_2,            0,
	/*0x07*/ STAGE_MP_COMPLEX,    2,          255, 100, 100, 0, FILE_BG_REF_SEG,  FILE_BG_REF_TILES,  FILE_BG_REF_PADS,  FILE_USETUPREF,  FILE_UMP_SETUPREF,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x08*/ STAGE_MP_G5BUILDING, 2,          255, 100, 100, 0, FILE_BG_CRYP_SEG, FILE_BG_CRYP_TILES, FILE_BG_CRYP_PADS, FILE_USETUPCRYP, FILE_UMP_SETUPCRYP,                                     700, 800, 400,  SFX_ALARM_DEFAULT,      0,
	/*0x09*/ STAGE_PELAGIC,       2,          255, 100, 100, 0, FILE_BG_DAM_SEG,  FILE_BG_DAM_TILES,  FILE_BG_DAM_PADS,  FILE_USETUPDAM,  FILE_UMP_SETUPDAM,                                      -1,  400, 0,    SFX_ALARM_2,            0,
	/*0x0a*/ STAGE_EXTRACTION,    2,          255, 100, 100, 0, FILE_BG_AME_SEG,  FILE_BG_AME_TILES,  FILE_BG_ARK_PADS,  FILE_USETUPARK,  FILE_UMP_SETUPARK,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x0b*/ STAGE_MP_TEMPLE,     2,          255, 100, 100, 0, FILE_BG_JUN_SEG,  FILE_BG_JUN_TILES,  FILE_BG_JUN_PADS,  FILE_USETUPJUN,  FILE_UMP_SETUPJUN,                                      700, 800, 400,  SFX_ALARM_DEFAULT,      0,
	/*0x0c*/ STAGE_CITRAINING,    2,          255, 100, 100, 0, FILE_BG_DISH_SEG, FILE_BG_DISH_TILES, FILE_BG_DISH_PADS, FILE_USETUPDISH, FILE_UMP_SETUPDISH,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x0d*/ STAGE_AIRBASE,       2,          255, 100, 100, 0, FILE_BG_CAVE_SEG, FILE_BG_CAVE_TILES, FILE_BG_CAVE_PADS, FILE_USETUPCAVE, FILE_UMP_SETUPCAVE,                                     -1,  400, 0,    SFX_ALARM_AIRBASE,      25 * 1024,
	/*0x0e*/ STAGE_MP_PIPES,      2,          255, 100, 100, 0, FILE_BG_CRAD_SEG, FILE_BG_CRAD_TILES, FILE_BG_CRAD_PADS, FILE_USETUPCRAD, FILE_UMP_SETUPCRAD,                                     700, 800, 400,  SFX_ALARM_DEFAULT,      0,
	/*0x0f*/ STAGE_SKEDARRUINS,   2,          255, 100, 100, 0, FILE_BG_SHO_SEG,  FILE_BG_SHO_TILES,  FILE_BG_SHO_PADS,  FILE_USETUPSHO,  FILE_UMP_SETUPSHO,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x10*/ STAGE_VILLA,         2,          255, 100, 100, 0, FILE_BG_ELD_SEG,  FILE_BG_ELD_TILES,  FILE_BG_ELD_PADS,  FILE_USETUPELD,  FILE_UMP_SETUPELD,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      25 * 1024,
	/*0x11*/ STAGE_DEFENSE,       2,          255, 100, 100, 0, FILE_BG_DISH_SEG, FILE_BG_DISH_TILES, FILE_BG_IMP_PADS,  FILE_USETUPIMP,  FILE_UMP_SETUPIMP,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x12*/ STAGE_INFILTRATION,  2,          255, 100, 100, 0, FILE_BG_LUE_SEG,  FILE_BG_LUE_TILES,  FILE_BG_LUE_PADS,  FILE_USETUPLUE,  FILE_UMP_SETUPLUE,                                      -1,  400, 0,    SFX_ALARM_INFILTRATION, 0,
	/*0x13*/ STAGE_DEFECTION,     2,          255, 100, 100, 0, FILE_BG_AME_SEG,  FILE_BG_AME_TILES,  FILE_BG_AME_PADS,  FILE_USETUPAME,  FILE_UMP_SETUPAME,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x14*/ STAGE_AIRFORCEONE,   2,          255, 100, 100, 0, FILE_BG_RIT_SEG,  FILE_BG_RIT_TILES,  FILE_BG_RIT_PADS,  FILE_USETUPRIT,  FILE_UMP_SETUPRIT,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      25 * 1024,
	/*0x15*/ STAGE_MP_SKEDAR,     2,          255, 100, 100, 0, FILE_BG_OAT_SEG,  FILE_BG_OAT_TILES,  FILE_BG_OAT_PADS,  FILE_USETUPOAT,  FILE_UMP_SETUPOAT,                                      700, 800, 400,  SFX_ALARM_DEFAULT,      0,
	/*0x16*/ STAGE_INVESTIGATION, 2,          255, 100, 100, 0, FILE_BG_EAR_SEG,  FILE_BG_EAR_TILES,  FILE_BG_EAR_PADS,  FILE_USETUPEAR,  FILE_UMP_SETUPEAR,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x17*/ STAGE_ATTACKSHIP,    2,          255, 100, 100, 0, FILE_BG_LEE_SEG,  FILE_BG_LEE_TILES,  FILE_BG_LEE_PADS,  FILE_USETUPLEE,  FILE_UMP_SETUPLEE,                                      -1,  400, 0,    SFX_ALARM_ATTACKSHIP,   25 * 1024,
	/*0x18*/ STAGE_RESCUE,        2,          255, 100, 100, 0, FILE_BG_LUE_SEG,  FILE_BG_LUE_TILES,  FILE_BG_LIP_PADS,  FILE_USETUPLIP,  FILE_UMP_SETUPLIP,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x19*/ STAGE_MBR,           2,          255, 100, 100, 0, FILE_BG_AME_SEG,  FILE_BG_AME_TILES,  FILE_BG_AME_PADS,  FILE_USETUPWAX,  FILE_UMP_SETUPWAX,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1a*/ STAGE_DEEPSEA,       8,          96,  80,  200, 0, FILE_BG_PAM_SEG,  FILE_BG_PAM_TILES,  FILE_BG_PAM_PADS,  FILE_USETUPPAM,  FILE_UMP_SETUPPAM,                                      300, 600, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1b*/ STAGE_DUEL,          2,          255, 100, 100, 0, FILE_BG_DISH_SEG, FILE_BG_DISH_TILES, FILE_BG_ATE_PADS,  FILE_USETUPATE,  FILE_UMP_SETUPATE,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1c*/ STAGE_MP_BASE,       2,          255, 100, 100, 0, FILE_BG_MP1_SEG,  FILE_BG_MP1_TILES,  FILE_BG_MP1_PADS,  FILE_USETUPMP1,  FILE_UMP_SETUPMP1,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1d*/ STAGE_MP_AREA52,     2,          255, 100, 100, 0, FILE_BG_MP3_SEG,  FILE_BG_MP3_TILES,  FILE_BG_MP3_PADS,  FILE_USETUPMP3,  FILE_UMP_SETUPMP3,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1e*/ STAGE_MP_WAREHOUSE,  2,          255, 100, 100, 0, FILE_BG_MP4_SEG,  FILE_BG_MP4_TILES,  FILE_BG_MP4_PADS,  FILE_USETUPMP4,  FILE_UMP_SETUPMP4,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x1f*/ STAGE_MP_CARPARK,    2,          255, 100, 100, 0, FILE_BG_MP5_SEG,  FILE_BG_MP5_TILES,  FILE_BG_MP5_PADS,  FILE_USETUPMP5,  FILE_UMP_SETUPMP5,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x20*/ STAGE_MP_RUINS,      2,          255, 100, 100, 0, FILE_BG_MP9_SEG,  FILE_BG_MP9_TILES,  FILE_BG_MP9_PADS,  FILE_USETUPMP9,  FILE_UMP_SETUPMP9,                                      -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x21*/ STAGE_MP_SEWERS,     2,          255, 100, 100, 0, FILE_BG_MP10_SEG, FILE_BG_MP10_TILES, FILE_BG_MP10_PADS, FILE_USETUPMP10, FILE_UMP_SETUPMP10,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x22*/ STAGE_MP_FELICITY,   2,          255, 100, 100, 0, FILE_BG_MP11_SEG, FILE_BG_MP11_TILES, FILE_BG_MP11_PADS, FILE_USETUPMP11, FILE_UMP_SETUPMP11,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x23*/ STAGE_MP_FORTRESS,   2,          255, 100, 100, 0, FILE_BG_MP12_SEG, FILE_BG_MP12_TILES, FILE_BG_MP12_PADS, FILE_USETUPMP12, FILE_UMP_SETUPMP12,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x24*/ STAGE_MP_VILLA,      2,          255, 100, 100, 0, FILE_BG_MP13_SEG, FILE_BG_MP13_TILES, FILE_BG_MP13_PADS, FILE_USETUPMP13, FILE_UMP_SETUPMP13,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
	/*0x25*/ STAGE_MP_GRID,       2,          255, 100, 100, 0, FILE_BG_MP15_SEG, FILE_BG_MP15_TILES, FILE_BG_MP15_PADS, FILE_USETUPMP15, FILE_UMP_SETUPMP15,                                     -1,  400, 0,    SFX_ALARM_DEFAULT,      0,
};

struct stagetableentry *stageGetCurrent(void)
{
	struct stagetableentry *stage = g_Stages;
	struct stagetableentry *end = (struct stagetableentry *)(uintptr_t)stage + ARRAYCOUNT(g_Stages);
	int stagenum = g_Vars.stagenum;

	while (stage < end) {
		if (stage->id == stagenum) {
			return stage;
		}

		stage++;
	}

	return NULL;
}

int stageGetIndex(int stagenum)
{
	struct stagetableentry *stage = g_Stages;
	struct stagetableentry *end = (struct stagetableentry *)(uintptr_t)stage + ARRAYCOUNT(g_Stages);
	int i = 0;

	while (stage < end) {
		if (stage->id == stagenum) {
			return i;
		}

		stage++;
		i++;
	}

	return -1;
}
