#include <ultra64.h>
#include "constants.h"
#include "game/gfxreplace.h"
#include "bss.h"
#include "data.h"
#include "gfx.h"
#include "types.h"

/**
 * When loading rooms, the game can scan the room's displaylists find/replace
 * GBI commands.
 *
 * Each pair of elements in these groups are find/replace pairs.
 */

/**
 * Enable fog (opaque displaylist)
 */
Gfx g_GfxGroup00[] = {
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_SURF2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_TERR2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_TERR2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_DECAL2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_DECAL2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_DECAL2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_DECAL2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_SURF2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_OPA_SURF2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_OPA_SURF2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_OPA_TERR2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_OPA_TERR2),

	gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_CUSTOM_06),
	gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
	gsDPSetCombineMode(G_CC_CUSTOM_07, G_CC_CUSTOM_07),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEI2),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_CUSTOM_08),
	gsDPSetCombineMode(G_CC_MODULATEI, G_CC_MODULATEI),
	gsDPSetCombineMode(G_CC_CUSTOM_09, G_CC_CUSTOM_09),
	0,
};

/**
 * Enable fog (translucent displaylist)
 */
Gfx g_GfxGroup01[] = {
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_DECAL2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_DECAL2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_SURF2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2),
	gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
	gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),

	gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_CUSTOM_06),
	gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
	gsDPSetCombineMode(G_CC_CUSTOM_07, G_CC_CUSTOM_07),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEI2),
	gsDPSetCombineMode(G_CC_TRILERP, G_CC_CUSTOM_08),
	gsDPSetCombineMode(G_CC_MODULATEI, G_CC_MODULATEI),
	gsDPSetCombineMode(G_CC_CUSTOM_09, G_CC_CUSTOM_09),
	0,
};

void gfxReplaceGbiCommands(Gfx *startgdl, Gfx *endgdl, int type)
{
	static Gfx *groups[] = {
		g_GfxGroup00,
		g_GfxGroup01,
	};

	Gfx *gdl = startgdl;

	while ((endgdl && gdl < endgdl) || (!endgdl && (int8_t)gdl->bytes[GFX_W0_BYTE(0)] != G_ENDDL)) {
		Gfx *src = groups[type];

		while (src->words.w0 != 0) {
			if (src->words.w0 == gdl->words.w0 && src->words.w1 == gdl->words.w1) {
				*gdl = src[1];
			}

			src += 2;
		}

		gdl++;
	}
}

void gfxReplaceGbiCommandsRecursively(struct roomblock *block, int type)
{
#ifndef AVOID_UB
	// Sometimes block is NULL when this is called.
	// If UBSan is being used, this will crash in this instance.
	if (block->type == ROOMBLOCKTYPE_PARENT);
#endif

	while (true) {
		if (!block) {
			return;
		}

		switch (block->type) {
		case ROOMBLOCKTYPE_LEAF:
			gfxReplaceGbiCommands(block->gdl, NULL, type);
			block = block->next;
			break;
		case ROOMBLOCKTYPE_PARENT:
			gfxReplaceGbiCommandsRecursively(block->child, type);
			block = block->next;
			break;
		default:
			return;
		}
	}
}
