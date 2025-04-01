#include <ultra64.h>
#include <stdio.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/bondgun.h"
#include "game/weaponutils.h"
#include "game/tex.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/inv.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/texdecompress.h"
#include "game/tex.h"
#include "game/modeldef.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/model.h"
#include "data.h"
#include "types.h"

struct skeleton *g_Skeletons[] = {
	&g_SkelChr,
	&g_SkelClassicGun,
	&g_Skel06,
	&g_SkelBasic,
	&g_SkelCctv,
	&g_SkelWindowedDoor,
	&g_Skel11,
	&g_Skel12,
	&g_Skel13,
	&g_SkelTerminal,
	&g_SkelCiHub,
	&g_SkelAutogun,
	&g_Skel17,
	&g_Skel18,
	&g_Skel19,
	&g_Skel0B,
	&g_SkelCasing,
	&g_SkelChrGun,
	&g_Skel0C,
	&g_SkelJoypad,
	&g_SkelLift,
	&g_SkelSkedar,
	&g_SkelLogo,
	&g_SkelPdLogo,
	&g_SkelHoverbike,
	&g_SkelJumpship,
	&g_Skel20,
	&g_Skel21,
	&g_Skel22,
	&g_SkelLaptopGun,
	&g_SkelK7Avenger,
	&g_SkelChopper,
	&g_SkelFalcon2,
	&g_SkelKnife,
	&g_SkelDrCaroll,
	&g_SkelRope,
	&g_SkelCmp150,
	&g_SkelBanner,
	&g_SkelDragon,
	&g_SkelSuperDragon,
	&g_SkelRocket,
	&g_Skel4A,
	&g_SkelShotgun,
	&g_SkelFarsight,
	&g_Skel4D,
	&g_SkelReaper,
	&g_SkelDropship,
	&g_SkelMauler,
	&g_SkelDevastator,
	&g_SkelRobot,
	&g_SkelPistol,
	&g_SkelAr34,
	&g_SkelMagnum,
	&g_SkelSlayerRocket,
	&g_SkelCyclone,
	&g_SkelSniperRifle,
	&g_SkelTranquilizer,
	&g_SkelCrossbow,
	&g_SkelHudPiece,
	&g_SkelTimedProxyMine,
	&g_SkelPhoenix,
	&g_SkelCallisto,
	&g_SkelHand,
	&g_SkelRcp120,
	&g_SkelSkShuttle,
	&g_SkelLaser,
	&g_SkelMaianUfo,
	&g_SkelGrenade,
	&g_SkelCableCar,
	&g_SkelSubmarine,
	&g_SkelTarget,
	&g_SkelEcmMine,
	&g_SkelUplink,
	&g_SkelRareLogo,
	&g_SkelWireFence,
	&g_SkelRemoteMine,
	&g_SkelBB,
	NULL // terminate list for sure
};

void modeldef0f1a7560(struct modeldef *modeldef, uint16_t filenum, uint32_t arg2, struct modeldef *modeldef2, struct texpool *texpool, bool arg5)
{
	int allocsize;
	int loadedsize;
	int sp84;
	uint32_t s0;
	uint32_t s4;
	uintptr_t s5;
	struct modelnode *node;
	struct modelnode *prevnode;
	uintptr_t gdl;
	Vtx *vertices;

	allocsize = fileGetAllocationSize(filenum);
	loadedsize = fileGetLoadedSize(filenum);
	node = NULL;

	modelIterateDisplayLists(modeldef, &node, (Gfx **)&gdl);

	s5 = gdl;

	if (gdl) {
		int v1 = allocsize - (loadedsize - (uintptr_t)(((uintptr_t)modeldef + (UNSEGADDR(gdl) & 0xffffff)) - (uintptr_t)modeldef));
		sp84 = (uintptr_t)v1 + (uintptr_t)((uintptr_t)modeldef - ((uintptr_t)modeldef + (UNSEGADDR(gdl) & 0xffffff)));

		texCopyGdls((Gfx *)((uintptr_t)modeldef + (UNSEGADDR(gdl) & 0xffffff)),
				(Gfx *)(v1 + (uintptr_t)modeldef),
				loadedsize - (uintptr_t)(((uintptr_t)modeldef + (UNSEGADDR(gdl) & 0xffffff)) - (uintptr_t)modeldef));
		texLoadFromConfigs(modeldef->texconfigs, modeldef->numtexconfigs, texpool, (uintptr_t)modeldef2 - (uintptr_t)arg2);

		while (node) {
			prevnode = node;
			s0 = gdl;

			modelIterateDisplayLists(modeldef, &node, (Gfx **) &gdl);

			if (gdl) {
				s4 = UNSEGADDR(gdl) - UNSEGADDR(s0);
			} else {
				s4 = loadedsize + (uintptr_t)modeldef - (uintptr_t)modeldef - (UNSEGADDR(s0) & 0xffffff);
			}

			modelNodeReplaceGdl(modeldef, prevnode, (Gfx *) s0, (Gfx *) s5);

			if (prevnode->type == MODELNODETYPE_DL) {
				struct modelrodata_dl *rodata = &prevnode->rodata->dl;
				vertices = rodata->vertices;
			} else {
				vertices = NULL;
			}

			s5 += texLoadFromGdl((Gfx *)((uintptr_t)modeldef + (UNSEGADDR(s0) & 0xffffff) + sp84), s4, (Gfx *)((uintptr_t)modeldef + (UNSEGADDR(s5) & 0xffffff)), texpool, (uint8_t *) vertices);
		}

		fileSetSize(filenum, modeldef, (((uintptr_t)modeldef + (UNSEGADDR(s5) & 0xffffff)) - (uintptr_t)modeldef + 0xf) & ~0xf, arg5);
	}
}

void modelPromoteTypeToPointer(struct modeldef *modeldef)
{
	int i;

	if ((uint32_t)modeldef->skel < 0x10000) {
		for (i = 0; g_Skeletons[i] != NULL; i++) {
			if ((int16_t)modeldef->skel == g_Skeletons[i]->skel) {
				modeldef->skel = g_Skeletons[i];
				return;
			}
		}
	}
}

struct modeldef *modeldefLoad(uint16_t fileid, uint8_t *dst, int size, struct texpool *arg3)
{
	struct modeldef *modeldef;

	g_LoadType = LOADTYPE_MODEL;

	if (dst) {
		modeldef = fileLoadToAddr(fileid, FILELOADMETHOD_EXTRAMEM, dst, size);
	} else {
		modeldef = fileLoadToNew(fileid, FILELOADMETHOD_EXTRAMEM, LOADTYPE_MODEL);
	}

	modelPromoteTypeToPointer(modeldef);
	modelPromoteOffsetsToPointers(modeldef, 0x5000000, (uintptr_t) modeldef);
	modeldef0f1a7560(modeldef, fileid, 0x5000000, modeldef, arg3, dst == NULL);

	return modeldef;
}

struct modeldef *modeldefLoadToNew(uint16_t fileid)
{
	return modeldefLoad(fileid, NULL, 0, NULL);
}

struct modeldef *modeldefLoadToAddr(uint16_t fileid, uint8_t *dst, int size)
{
	return modeldefLoad(fileid, dst, size, NULL);
}
