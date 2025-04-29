#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "lib/sched.h"
#include "game/player.h"
#include "game/zbuf.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "video.h"

uint32_t g_ZbufWidth;
uint32_t g_ZbufHeight;
uint16_t g_ArtifactsCfb0[0x180]; // 0x180 = 384
uint16_t g_ArtifactsCfb1[0x180];
uint16_t g_ArtifactsCfb2[0x180];

uint16_t *g_ZbufPtr1 = NULL;

void *zbufGetAllocation(void)
{
	return g_ZbufPtr1;
}

void zbufReset(int stagenum)
{
	g_ZbufPtr1 = NULL;

	if (stagenum != STAGE_TITLE) {
		zbufAllocate();
	}
}

/**
 * In 4MB 2-player, the viewports are displayed with a vertical split and are
 * only half a screen height (the top 25% and bottom 25% of the screen are
 * black), so a half-height z-buffer is allocated.
 *
 * In 8MB, the full hi-res buffer is allocated. This makes sense for solo
 * missions because the player can switch to hi-res mid game. For normal
 * multiplayer this is wasteful but there's plenty of memory. For coop and anti
 * this is also wasteful, and memory is tight. They could have saved 137.5 KB.
 *
 * The allocation sizes need to enforce a minimum because the allocation is also
 * used by lighting initialisation code.
 */
void zbufAllocate(void)
{
	g_ZbufWidth = MAX(640, FBALLOC_WIDTH_HI);

	if (g_Vars.normmplayerisrunning && PLAYERCOUNT() >= 2) {
		g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_HI);
	} else {
		g_ZbufHeight = MAX(220, FBALLOC_HEIGHT_HI);
	}

	g_ZbufPtr1 = mempAlloc(g_ZbufWidth * g_ZbufHeight * sizeof(uint16_t) + 0x40, MEMPOOL_STAGE);
	g_ZbufPtr1 = (void *) (((uintptr_t) g_ZbufPtr1 + 0x3f) & ~0x3f);
}

/**
 * In 8MB multiplayer, players on the bottom half of the screen have their
 * z-buffer shifted backwards by half a screen. This is safe because it's using
 * a scissor on the viewport.
 *
 * This allows the z-buffer allocation to be half a screen instead of a full
 * screen, however zbufAllocate allocates the full hi-res screen for 8MB,
 * so this benefit is not realised. The shifting code is likely from GE.
 */
Gfx *zbufConfigureRdp(Gfx *gdl)
{
	uint32_t subamount;

	if (g_Vars.normmplayerisrunning
			&& (g_Vars.currentplayernum >= 2 || (PLAYERCOUNT() == 2 && g_Vars.currentplayernum == 1))) {
		subamount = playerGetFbWidth() * playerGetFbHeight();

		if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			subamount = 0;
		}
	} else {
		subamount = 0;
	}

	return gdl;
}

/**
 * Clear the current player's portion of the z-buffer.
 */
Gfx *zbufClear(Gfx *gdl)
{
	gfx_No_Param(gdl++, G_CLEAR_DEPTH_EXT);

	return gdl;
}

uint16_t *zbufGetArtifactsCfb(int index)
{
	uint16_t *addr;

	if (index == 0) {
		addr = g_ArtifactsCfb0;
	}

	if (index == 1) {
		addr = g_ArtifactsCfb1;
	}

	if (index == 2) {
		addr = g_ArtifactsCfb2;
	}

	addr = (uint16_t *) (((uintptr_t) addr + 0x3f) & ~0x3f);

	return addr;
}

Gfx *zbufDrawArtifactsOffscreen(Gfx *gdl)
{
	struct artifact *artifacts = schedGetWriteArtifacts();
	uint16_t *sp4c = g_ZbufPtr1;
	uint32_t s4 = 0;
	uint16_t *sp44;
	uint16_t *s2;
	uint16_t *image;

	viGetBackBuffer();
	sp44 = zbufGetArtifactsCfb(g_SchedWriteArtifactsIndex);
	g_SchedSpecialArtifactIndexes[g_SchedWriteArtifactsIndex] = 1;

	gfx_Set_Color_Image(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), (uintptr_t)(sp44));
	gfx_Set_Scissor(gdl++, 0, 0, videoGetWidth(), videoGetHeight());
	gfx_Set_Cycle_Type(gdl++, G_CYC_COPY);
	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0080, 4, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 160, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Tile(gdl++, G_IM_FMT_I, G_IM_SIZ_8b, 160, 0x0080, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, 15);
	gfx_Texture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
	RGBA envColor = {255, 255, 255, 255};
	gfx_Set_Env_Color(gdl++, envColor);
	RGBA primColor = {255, 255, 255, 255};
	gfx_Set_Prim_Color(gdl++, primColor);
	gfx_Set_Render_Mode(gdl++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
	gfx_Set_Texture_Filter(gdl++, G_TF_POINT);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Clear_Geometry_Mode(gdl++, G_ZBUFFER);

	for (int i = 0; i < MAX_ARTIFACTS; i++) {
		if (artifacts[i].type != ARTIFACTTYPE_FREE) {
			s2 = &sp44[s4];
			image = &sp4c[artifacts[i].screenPos.screenY * viGetWidth()];

			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, videoGetWidth(), image);
			gfx_Load_Block(gdl++, 5, 0, 0, viGetWidth() - 1, 0);

			gdl += gfx_Texture_Rectangle(gdl,
					s4 << 2, 0,
					(s4 + 3) << 2, 0,
					G_TX_RENDERTILE, (artifacts[i].screenPos.screenX * 32) + 16, 0x0010, 0x1000, 0);

			artifacts[i].screenPos.outputPixelPtr = s2;
			s4++;
		}
	}

	gfx_Set_Color_Image(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), (uintptr_t)(viGetBackBuffer()));
	gfx_Set_Scissor(gdl++, 0, 0, viGetWidth(), viGetHeight());
	gfx_Set_Geometry_Mode(gdl++, G_ZBUFFER);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

	return gdl;
}
