#include <ultra64.h>
#include <stdint.h>
#include <string.h>
#include "constants.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/file.h"
#include "game/zbuf.h"
#include "game/gfxmemory.h"
#include "game/menu.h"
#include "game/mtxutils.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#include "video.h"
#include "platform.h"

Mtxf g_ActiveProjectionMtx;
Mtx *g_CameraPerspectiveMtxF;
uint16_t g_ViPerspScale;
uint8_t g_ViBackIndex;

struct rend_vidat g_ViDataArray[NUM_GFXTASKS] = {
	{
		0, 0, 0, 0,
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // x and y
		60,                                     // fovy
		(float) FBALLOC_WIDTH_LO / (float) FBALLOC_HEIGHT_LO, // aspect
		30,                                     // znear
		10000,                                  // zfar
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // bufx and bufy
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // viewx and viewy
		0, 0,                                   // viewleft and viewtop
		true,                                   // usezbuf
		0,
	}, {
		0, 0, 0, 0,
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // x and y
		60,                                     // fovy
		(float) FBALLOC_WIDTH_LO / (float) FBALLOC_HEIGHT_LO, // aspect
		30,                                     // znear
		10000,                                  // zfar
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // bufx and bufy
		FBALLOC_WIDTH_LO, FBALLOC_HEIGHT_LO,    // viewx and viewy
		0, 0,                                   // viewleft and viewtop
		true,                                   // usezbuf
		0,
	},
};

struct rend_vidat *g_ViBackData = &g_ViDataArray[0];
bool g_ViReconfigured = false;

/**
 * Configure the VI to display the legal screen.
 *
 * It's also used for the "check controllers" message
 * if controller 1 is not connected.
 */
void viConfigureForLegal(void)
{
	int i;

	for (i = 0; i < NUM_GFXTASKS; i++) {
		g_ViDataArray[i].x = videoGetWidth();
		g_ViDataArray[i].bufx = videoGetWidth();
		g_ViDataArray[i].viewx = videoGetWidth();

		g_ViDataArray[i].y = videoGetHeight();
		g_ViDataArray[i].bufy = videoGetHeight();
		g_ViDataArray[i].viewy = videoGetHeight();
	}

	g_ViBackData = &g_ViDataArray[0];
}

/**
 * Allocate the colour framebuffers for the given stage.
 *
 * Regardless of whether hi-res is being used or not, the buffers are allocated
 * for hi-res. This is because hi-res can be changed mid-stage, and the engine
 * cannot reallocate the frame buffer without clearing the stage's entire memory
 * pool.
 *
 * The same is probably true for wide and cinema modes.
 */
void viReset(int stagenum)
{
	int i;
	int fbsize;
	uint8_t *ptr;
	uint8_t *fb0;
	uint8_t *fb1;

	viSetMode();

	fbsize = FBALLOC_WIDTH_HI * FBALLOC_HEIGHT_HI * NUM_FRAMEBUFFERS;

	ptr = mempAlloc(fbsize * sizeof(uint16_t) + 0x40, MEMPOOL_STAGE);

#ifdef PLATFORM_64BIT
	ptr = (uint8_t*)(((uintptr_t)ptr + 0x3f) & 0xffffffffffffffc0);
#else
	ptr = (uint8_t *)(((uintptr_t) ptr + 0x3f) & 0xffffffc0);
#endif

	g_FrameBuffers[0] = (uint16_t *) ptr;
	g_FrameBuffers[1] = (uint16_t *) (fbsize + ptr);

	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	fb0 = (uint8_t *) g_FrameBuffers[0];
	fb1 = (uint8_t *) g_FrameBuffers[1];

	for (i = 0; i < fbsize; i++) {
		fb0[i] = 0;
		fb1[i] = 0;
	}

	g_ViReconfigured = true;
}

/**
 * If black is true, set the video output to black indefinitely.
 * If black is false, unblack once all the framebuffers have been cycled through.
 *
 * The g_ViUnblackTimer value only ticks down when it's 2 or less,
 * so passing true to this function makes it not tick.
 */
void viBlack(bool black)
{
	//black += NUM_FRAMEBUFFERS;
	//g_ViUnblackTimer = black;
}

// Offets the window during explosions to create a shaking effect
void viHandleShake(void)
{
	int offset;

	if (g_ViShakeTimer != 0) {
		g_ViShakeTimer--;

		if (g_ViShakeTimer == 0) {
			g_ViShakeIntensity = 0;
		}
	}

	offset = g_ViShakeDirection * g_ViShakeIntensity;
	g_ViShakeDirection = -g_ViShakeDirection;

	videoSetWindowOffset(0, offset);

	/*if(g_ViUnblackTimer) {
		videoClearScreen();
	}*/
}

void viUpdateMode(void)
{
	struct rend_vidat *prevdata;

	videoClearScreen();
	
	g_SchedViModesPending = true;

	prevdata = g_ViBackData;

	// Rotate to the next framebuffer index
	g_ViBackIndex = (g_ViBackIndex + 1) % NUM_FRAMEBUFFERS;

	g_ViBackData = &g_ViDataArray[g_ViBackIndex];

	memcpy(g_ViBackData, prevdata, sizeof(struct rend_vidat));

	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	if (g_ViReconfigured) {
		g_ViReconfigured = false;
		viBlack(false);
	}
}

void viShake(float intensity)
{
	if (intensity > 14) {
		intensity = 14;
	}

	if (intensity < 0) {
		intensity = 0;
	}

	g_ViShakeIntensity = intensity * g_ViShakeIntensityMult;
	g_ViShakeTimer = 20;
}

void viSetMode()
{
	g_ViBackData->mode = 1;

	g_ViBackData->x = g_ViBackData->bufx = videoGetNativeWidth();
	g_ViBackData->y = g_ViBackData->bufy = videoGetNativeHeight();
}

uint16_t *viGetBackBuffer(void)
{
	return g_ViBackData->fb;
}

Vp *viGetCurrentPlayerViewport(void)
{
	return &g_Vars.currentplayer->viewport[g_ViBackIndex];
}

Gfx *viSetupSkyProjection(Gfx *gdl)
{
	Mtxf perspectiveMtxF;
	Mtxf viewMtxF;
	Mtxf combinedMtxF;
	Mtxf identityMtxF;
	Mtx *combinedMtxL;
	Mtx *identityMtxL;
	uint16_t perspNorm;

	// Create a perspective matrix with extended z-far for sky objects
	mtxPerspectiveF(
		perspectiveMtxF.m,
		g_ViBackData->fovy,
		g_ViBackData->aspect,
		g_ViBackData->znear,
		g_ViBackData->zfar + g_ViBackData->zfar,
		1
	);

	// Copy the current camera view matrix
	mtx4Copy(camGetWorldToScreenMtxf(), &viewMtxF);

	// Remove translation so sky objects remain static in view
	viewMtxF.m[3][0] = 0;
	viewMtxF.m[3][1] = 0;
	viewMtxF.m[3][2] = 0;

	// Combine perspective and adjusted view matrices
	mtx4MultMtx4(&perspectiveMtxF, &viewMtxF, &combinedMtxF);
	combinedMtxL = gfxAllocateMatrix();
	mtxF2L2(combinedMtxF.m, combinedMtxL);

	// Load identity for modelview (no transformation)
	mtx4LoadIdentity(&identityMtxF);
	identityMtxL = gfxAllocateMatrix();
	mtxF2L2(identityMtxF.m, identityMtxL);

	// Set the projection and modelview matrices
	gSPMatrix(gdl++, (uintptr_t)(combinedMtxL), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
	gSPMatrix(gdl++, (uintptr_t)(identityMtxL), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *viSetupProjectionWithZRange(Gfx *gdl, float znear, float zfar)
{
	Mtxf tmp;
	Mtx *mtx = gfxAllocateMatrix();

	mtxPerspectiveF(tmp.m, g_ViBackData->fovy, g_ViBackData->aspect, znear, zfar, 1);
	mtxF2L2(tmp.m, mtx);

	gSPMatrix(gdl++, (uintptr_t)(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *viSetupViewportAndPerspective(Gfx *gdl, Vp *vp)
{
	vp[g_ViBackIndex].vp.vscale[0] = g_ViBackData->viewx * 2;
	vp[g_ViBackIndex].vp.vtrans[0] = g_ViBackData->viewx * 2 + g_ViBackData->viewleft * 4;

	vp[g_ViBackIndex].vp.vscale[1] = g_ViBackData->viewy * 2;
	vp[g_ViBackIndex].vp.vtrans[1] = g_ViBackData->viewy * 2 + g_ViBackData->viewtop * 4;

	gSPViewport(gdl++, (uintptr_t)(&vp[g_ViBackIndex]));

	g_CameraPerspectiveMtxF = gfxAllocateMatrix();
	mtxPerspectiveF(g_ActiveProjectionMtx.m, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	mtxF2L2(g_ActiveProjectionMtx.m, g_CameraPerspectiveMtxF);

	gSPMatrix(gdl++, (uintptr_t)(g_CameraPerspectiveMtxF), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	camSetPerspectiveMtxL(g_CameraPerspectiveMtxF);
	camSetMtxF1754(&g_ActiveProjectionMtx);

	return gdl;
}

Gfx *viSetupFixedZPerspective(Gfx *gdl, Vp *vp)
{
	vp[g_ViBackIndex].vp.vscale[0] = g_ViBackData->viewx * 2;
	vp[g_ViBackIndex].vp.vtrans[0] = g_ViBackData->viewx * 2 + g_ViBackData->viewleft * 4;

	vp[g_ViBackIndex].vp.vscale[1] = g_ViBackData->viewy * 2;
	vp[g_ViBackIndex].vp.vtrans[1] = g_ViBackData->viewy * 2 + g_ViBackData->viewtop * 4;

	vp[g_ViBackIndex].vp.vscale[2] = 511;
	vp[g_ViBackIndex].vp.vtrans[2] = 511;

	vp[g_ViBackIndex].vp.vscale[3] = 0;
	vp[g_ViBackIndex].vp.vtrans[3] = 0;

	gSPViewport(gdl++, (uintptr_t)(&vp[g_ViBackIndex]));

	g_CameraPerspectiveMtxF = gfxAllocateMatrix();
	mtxPerspectiveF(g_ActiveProjectionMtx.m, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	mtxF2L2(g_ActiveProjectionMtx.m, g_CameraPerspectiveMtxF);

	gSPMatrix(gdl++, (uintptr_t)(g_CameraPerspectiveMtxF), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	camSetPerspectiveMtxL(g_CameraPerspectiveMtxF);
	camSetMtxF1754(&g_ActiveProjectionMtx);

	return gdl;
}

Gfx *viSetupWeaponProjection(Gfx *gdl, float fovy, float aspect)
{
	Mtxf tmp;
	Mtx *mtx = gfxAllocateMatrix();

	mtxPerspectiveF(tmp.m, fovy, aspect, g_ViBackData->znear, g_ViBackData->zfar, 1);
	mtxF2L2(tmp.m, mtx);

	gSPMatrix(gdl++, (uintptr_t)(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *viPrepareHudDraw(Gfx *gdl)
{
	gdl = viSetupViewportAndPerspective(gdl, &g_Vars.currentplayer->viewport[0]);

	gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, g_ViBackData->bufx, (uintptr_t)(g_ViBackData->fb));

	return gdl;
}

Gfx *viPrepareZbuf(Gfx *gdl)
{
	if (g_ViBackData->usezbuf) {
		gdl = zbufConfigureRdp(gdl);
		gdl = zbufClear(gdl);
	}

	return gdl;
}

Gfx *viFillBuffer(Gfx *gdl)
{
	gDPSetCycleType(gdl++, G_CYC_FILL);
	gDPFillRectangle(gdl++, 0, 0, g_ViBackData->bufx - 1, g_ViBackData->bufy - 1);
	gDPPipeSync(gdl++);

	return gdl;
}

Gfx *viRenderViewportEdges(Gfx *gdl)
{
	gDPSetCycleType(gdl++, G_CYC_FILL);
	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, viGetWidth(), viGetHeight());
	gDPSetFillColor(gdl++, GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1));

	if (PLAYERCOUNT() == 1
			|| ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
				&& playerHasSharedViewport() && g_Vars.currentplayernum == 0))
	{
		// Single viewport
		if (viGetViewTop() > 0) {
			// Fill above
			gDPFillRectangle(gdl++, 0, 0, viGetWidth() - 1, viGetViewTop() - 1);
			gDPPipeSync(gdl++);
		}

		if (viGetViewTop() + viGetViewHeight() < viGetHeight()) {
			// Fill below
			gDPFillRectangle(gdl++,
					0, viGetViewTop() + viGetViewHeight(),
					viGetWidth() - 1, viGetHeight() - 1);
			gDPPipeSync(gdl++);
		}
	} else {
		if (g_Vars.currentplayerindex == 0) {
			int topplayernum = 0;
			int bottomplayernum = 0;
			int tmpplayernum = 0;

			if (PLAYERCOUNT() == 2) {
				bottomplayernum = 1;
				tmpplayernum = 1;
			} else if (PLAYERCOUNT() >= 3) {
				bottomplayernum = 2;
				tmpplayernum = 2;
			}

			if (g_Vars.players[topplayernum]->viewtop > 0) {
				// Fill above all viewports - full width
				gDPFillRectangle(gdl++, 0, 0, viGetWidth() - 1, g_Vars.players[topplayernum]->viewtop - 1);
				gDPPipeSync(gdl++);
			}

			if (g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight < viGetHeight()) {
				// Fill below all viewports - full width
				gDPFillRectangle(gdl++,
						0, g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight,
						viGetWidth() - 1, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}

			// Horizontal middle line
			gDPFillRectangle(gdl++,
					0, g_Vars.players[tmpplayernum]->viewtop - 1,
					viGetWidth() - 1, g_Vars.players[tmpplayernum]->viewtop - 1);
			gDPPipeSync(gdl++);

			if (PLAYERCOUNT() >= 3 ||
					(PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL))) {
				if (PLAYERCOUNT() == 2) {
					tmpplayernum = 0;
				}

				// Vertical middle line
				gDPFillRectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, 0,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}

			if (PLAYERCOUNT() == 3) {
				// Blank square in P4 spot
				gDPFillRectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth + 1, g_Vars.players[tmpplayernum]->viewtop,
						viGetWidth() - 1, viGetHeight() - 1);
				gDPPipeSync(gdl++);
			}
		}
	}

	return gdl;
}

void viSetBufSize(int16_t width, int16_t height)
{
	g_ViBackData->bufx = width;
	g_ViBackData->bufy = height;
}

int16_t viGetBufWidth(void)
{
	return g_ViBackData->bufx;
}

int16_t viGetBufHeight(void)
{
	return g_ViBackData->bufy;
}

void viSetSize(int16_t width, int16_t height)
{
	g_ViBackData->x = width;
	g_ViBackData->y = height;
}

int16_t viGetWidth(void)
{
	return g_ViBackData->x;
}

int16_t viGetHeight(void)
{
	return g_ViBackData->y;
}

void viSetViewSize(int16_t width, int16_t height)
{
	g_ViBackData->viewx = width;
	g_ViBackData->viewy = height;

	camSetScreenSize(g_ViBackData->viewx, g_ViBackData->viewy);
	camSetScale();
}

int16_t viGetViewWidth(void)
{
	return g_ViBackData->viewx;
}

int16_t viGetViewHeight(void)
{
	return g_ViBackData->viewy;
}

void viSetViewPosition(int16_t left, int16_t top)
{
	g_ViBackData->viewleft = left;
	g_ViBackData->viewtop = top;

	camSetScreenPosition(g_ViBackData->viewleft, g_ViBackData->viewtop);
}

int16_t viGetViewLeft(void)
{
	return g_ViBackData->viewleft;
}

int16_t viGetViewTop(void)
{
	return g_ViBackData->viewtop;
}

void viSetUseZBuf(bool use)
{
	g_ViBackData->usezbuf = use;
}

void viSetFovY(float fovy)
{
	g_ViBackData->fovy = fovy;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

void viSetAspect(float aspect)
{
	g_ViBackData->aspect = aspect;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

float viGetAspect(void)
{
	return videoGetAspect();
}

void viSetFovAspectAndSize(float fovy, float aspect, int16_t width, int16_t height)
{
	g_ViBackData->fovy = fovy;
	g_ViBackData->aspect = aspect;
	g_ViBackData->viewx = width;
	g_ViBackData->viewy = height;

	camSetScreenSize(g_ViBackData->viewx, g_ViBackData->viewy);
	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

float viGetFovY(void)
{
	return g_ViBackData->fovy;
}

void viSetZRange(float near, float far)
{
	g_ViBackData->znear = near;
	g_ViBackData->zfar = far;

	camSetPerspective(g_ViBackData->znear, g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

void viGetZRange(struct zrange *zrange)
{
	zrange->near = g_ViBackData->znear;
	zrange->far = g_ViBackData->zfar;
}

// Used for setting sky background color
Gfx *viSetFillColour(Gfx *gdl, int r, int g, int b)
{
	gDPSetFillColor(gdl++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));

	return gdl;
}