#include <stdlib.h>
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
#include "types.h"
#include "video.h"

uint32_t g_ZbufWidth;
uint32_t g_ZbufHeight;
uint16_t g_ArtifactsCfb0[384];
uint16_t g_ArtifactsCfb1[384];
uint16_t g_ArtifactsCfb2[384];

uint16_t *g_ZbufPtr1 = NULL;
static void *g_ZbufAllocRaw = NULL; // Keep track of original pointer for free()

void *zbufGetAllocation(void)
{
	return g_ZbufPtr1;
}

void zbufReset(int stagenum)
{
	// Free any existing allocation
	if (g_ZbufAllocRaw != NULL) {
		free(g_ZbufAllocRaw);
		g_ZbufAllocRaw = NULL;
		g_ZbufPtr1 = NULL;
	}

	// Allocate new z-buffer if not title stage
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
	g_ZbufWidth = viGetWidth();
	g_ZbufHeight = viGetHeight();

	// Allocate extra for alignment
	size_t size = g_ZbufWidth * g_ZbufHeight * sizeof(uint16_t) + 0x40;
	g_ZbufAllocRaw = malloc(size);

	if (g_ZbufAllocRaw != NULL) {
		// Align to 64-byte boundary
		g_ZbufPtr1 = (void *)(((uintptr_t)g_ZbufAllocRaw + 0x3f) & ~0x3f);
	} else {
		g_ZbufPtr1 = NULL;
	}
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

	gDPPipeSync(gdl++);

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
	uint16_t *mainZBuffer = g_ZbufPtr1;
	uint32_t artifactIndex = 0;
	uint16_t *artifactCfb; // Color framebuffer for artifacts
	uint16_t *pixelOutputPtr;
	uint16_t *image;
	int i;

	viGetBackBuffer();
	artifactCfb = zbufGetArtifactsCfb(g_SchedWriteArtifactsIndex);
	g_SchedSpecialArtifactIndexes[g_SchedWriteArtifactsIndex] = 1;

	gDPPipeSync(gdl++);
	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), (uintptr_t)(artifactCfb));
	gDPSetScissor(gdl++, 0, 0, videoGetWidth(), videoGetHeight());
	gDPSetCycleType(gdl++, G_CYC_COPY);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0080, 4, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 160, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTile(gdl++, G_IM_FMT_I, G_IM_SIZ_8b, 160, 0x0080, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, 15);
	gSPTexture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
	gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, 0xff);
	gDPSetPrimColor(gdl++, 0, 0, 0xff, 0xff, 0xff, 0xff);
	gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
	gDPSetCombineMode(gdl++, G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(gdl++, G_TF_POINT);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetAlphaDither(gdl++, G_AD_DISABLE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureDetail(gdl++, G_TD_CLAMP);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gSPClearGeometryMode(gdl++, G_ZBUFFER);
	gDPTileSync(gdl++);

	for (i = 0; i < MAX_ARTIFACTS; i++) {
		if (artifacts[i].type != ARTIFACTTYPE_FREE) {
			pixelOutputPtr = &artifactCfb[artifactIndex];
			image = &mainZBuffer[artifacts[i].screenPos.screenY * viGetWidth()];

			gDPPipeSync(gdl++);
			gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, videoGetWidth(), image);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, 5, 0, 0, viGetWidth() - 1, 0);
			gDPPipeSync(gdl++);

			gSPTextureRectangle(gdl++,
					artifactIndex << 2, 0,
					(artifactIndex + 3) << 2, 0,
					G_TX_RENDERTILE, (artifacts[i].screenPos.screenX * 32) + 16, 0x0010, 0x1000, 0);

			artifacts[i].screenPos.outputPixelPtr = pixelOutputPtr;
			artifactIndex++;
		}
	}

	gDPPipeSync(gdl++);
	gDPLoadSync(gdl++);
	gDPTileSync(gdl++);
	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetBufWidth(), (uintptr_t)(viGetBackBuffer()));
	gDPSetScissorFrac(gdl++, 0, 0, viGetWidth() * 4.0f, viGetHeight() * 4.0f);
	gSPSetGeometryMode(gdl++, G_ZBUFFER);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);
	gDPSetColorDither(gdl++, G_CD_BAYER);

	return gdl;
}
