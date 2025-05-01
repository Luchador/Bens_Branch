#include <ultra64.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "game/camera.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "game/menu.h"
#include "game/mtxutils.h"
#include "game/options.h"
#include "game/player.h"
#include "game/tex.h"
#include "game/utils.h"
#include "game/zbuf.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "video.h"
#include "platform.h"

Mtx g_ActiveProjectionMtx;
Mtx *g_CameraPerspectiveMtx;
uint8_t g_ViBackIndex;
static uint8_t *g_FbRawPtr = NULL;
static uint8_t *g_FbPtr = NULL;

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
	}
};

struct rend_vidat *g_ViBackData = &g_ViDataArray[0];
bool g_ViReconfigured = false;
int g_ViSlot = 0;

/**
 * Configure the VI to display the legal screen.
 *
 * It's also used for the "check controllers" message
 * if controller 1 is not connected.
 */
void viConfigureForLegal(void)
{
		g_ViDataArray[0].x = FBALLOC_WIDTH_LO;
		g_ViDataArray[0].bufx = FBALLOC_WIDTH_LO;
		g_ViDataArray[0].viewx = FBALLOC_WIDTH_LO;

		g_ViDataArray[0].y = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].bufy = FBALLOC_HEIGHT_LO;
		g_ViDataArray[0].viewy = FBALLOC_HEIGHT_LO;
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
	int fbsize;
	uint8_t *fb0;
	uint8_t *fb1;

	viSetMode();

	fbsize = FBALLOC_WIDTH_HI * FBALLOC_HEIGHT_HI * NUM_FRAMEBUFFERS;

	if (g_FbRawPtr) {
		free(g_FbRawPtr);  // Only free the *original* pointer
		g_FbRawPtr = NULL;
		g_FbPtr = NULL;
	}

	// Allocate raw memory and keep the original for later free
	g_FbRawPtr = (uint8_t*)malloc(fbsize * sizeof(uint16_t) + 0x40);
	g_FbPtr = (uint8_t*)(((uintptr_t)g_FbRawPtr + 0x3f) & ~0x3f);  // 64-byte aligned

	g_FrameBuffers[0] = (uint16_t *) g_FbPtr;
	g_FrameBuffers[1] = (uint16_t *) (g_FbPtr + fbsize);

	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	fb0 = (uint8_t *) g_FrameBuffers[0];
	fb1 = (uint8_t *) g_FrameBuffers[1];

	for (int i = 0; i < fbsize; i++) {
		fb0[i] = 0;
		fb1[i] = 0;
	}

	g_ViReconfigured = true;
}

// Offets the window during explosions to create a shaking effect
void viHandleShake(void)
{
	if (g_ViShakeTimer != 0) {
		g_ViShakeTimer--;

		if (g_ViShakeTimer == 0) {
			g_ViShakeIntensity = 0;
		}
	}

	int offset = g_ViShakeDirection * g_ViShakeIntensity;
	g_ViShakeDirection = -g_ViShakeDirection;

	videoSetWindowOffset(0, offset);
}

void viUpdateMode(void)
{
	struct rend_vidat *prevdata;

	switch (g_ViBackData->mode) {
	case VIMODE_NONE:
		videoClearScreen();
		break;
	case VIMODE_LO:
		break;
	}

	int slot = g_ViSlot;

	if (g_ViBackData->mode == VIMODE_LO) {
		g_SchedViModesPending[slot] = true;
	} else {
		g_SchedViModesPending[slot] = false;
	}

	slot = (slot + 1) % NUM_GFXTASKS;
	g_ViSlot = slot;

	prevdata = g_ViBackData;

	g_ViBackIndex = (g_ViBackIndex + 1) % NUM_FRAMEBUFFERS;

	g_ViBackData = g_ViDataArray + g_ViBackIndex;

	memcpy(g_ViBackData, prevdata, sizeof(struct rend_vidat));

	g_ViBackData->fb = g_FrameBuffers[g_ViBackIndex];

	if (g_ViReconfigured) {
		g_ViReconfigured = false;
	}
}

void viShake(float intensity)
{
	utilsClampF(intensity, 0.0f, 14.0f);

	g_ViShakeIntensity = intensity * g_ViShakeIntensityMult;
	g_ViShakeTimer = 20;
}

void viSetMode()
{
	g_ViBackData->mode = 1;

	g_ViBackData->x = g_ViBackData->bufx = FBALLOC_WIDTH_LO;
	g_ViBackData->y = g_ViBackData->bufy = FBALLOC_HEIGHT_LO;
}

uint16_t *viGetBackBuffer(void)
{
	return g_ViBackData->fb;
}

Vp *viGetCurrentPlayerViewport(void)
{
	return &g_Vars.currentplayer->viewport[g_ViBackIndex];
}

/**
 * Sets up the projection and modelview matrices for a basic rendering pass.
 * The projection matrix is constructed using the VI backbuffer camera parameters.
 * The camera matrix is used without its translation component (origin-locked).
 * This is useful for rendering objects like Defection's Moon which doesn't change position.
 */
Gfx *viSetCamNoTranslation(Gfx *gdl)
{
	Mtx projF;
	Mtx viewNoTransF;
	Mtx projViewF;
	Mtx identityF;
	Mtx *projMtx;
	Mtx *modelviewMtx;

	// Create a perspective projection matrix
	mtxPerspective((Mtx*)&projF, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar * 2);

	// Copy the current camera matrix and zero its translation part
	mtx4Copy(camGetPlayerWorldToScreenMtx(), &viewNoTransF);
	viewNoTransF[3][0] = 0;
	viewNoTransF[3][1] = 0;
	viewNoTransF[3][2] = 0;

	// Multiply projection * view matrix (no translation)
	mtx4MultMtx4(&projF, &viewNoTransF, &projViewF);
	projMtx = gfxAllocateMatrix();
	memcpy(projMtx, projViewF, sizeof(*projMtx));

	// Load identity modelview matrix
	mtxIdent(&identityF);
	modelviewMtx = gfxAllocateMatrix();
	memcpy(modelviewMtx, &identityF, sizeof(*modelviewMtx));

	// Set projection matrix
	gfx_Matrix(gdl++, projMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	// Set modelview matrix
	gfx_Matrix(gdl++, modelviewMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *viSetNearAndFarPlanes(Gfx *gdl, float znear, float zfar)
{
	Mtx *mtx = gfxAllocateMatrix();

	mtxPerspective(mtx, g_ViBackData->fovy, g_ViBackData->aspect, znear, zfar);

	gfx_Matrix(gdl++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *vi0000ad5c(Gfx *gdl, Vp *vp)
{
	vp[g_ViBackIndex].vp.vscale[0] = g_ViBackData->viewx;
	vp[g_ViBackIndex].vp.vtrans[0] = g_ViBackData->viewx / 2+ g_ViBackData->viewleft;

	vp[g_ViBackIndex].vp.vscale[1] = g_ViBackData->viewy;
	vp[g_ViBackIndex].vp.vtrans[1] = g_ViBackData->viewy / 2 + g_ViBackData->viewtop;

	gfx_Viewport(gdl++, &vp[g_ViBackIndex]);

	g_CameraPerspectiveMtx = gfxAllocateMatrix();
	mtxPerspective(&g_ActiveProjectionMtx, g_ViBackData->fovy, g_ViBackData->aspect, g_ViBackData->znear, g_ViBackData->zfar);
	memcpy(g_CameraPerspectiveMtx, g_ActiveProjectionMtx, sizeof(*g_CameraPerspectiveMtx));

	gfx_Matrix(gdl++, g_CameraPerspectiveMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	camSetPerspectiveMtxL(g_CameraPerspectiveMtx);
	camSetSkyMtx(&g_ActiveProjectionMtx);

	return gdl;
}

Gfx *viSetFovAndAspect(Gfx *gdl, float fovy, float aspect)
{
	Mtx *mtx = gfxAllocateMatrix();

	mtxPerspective(mtx, fovy, aspect, g_ViBackData->znear, g_ViBackData->zfar);

	gfx_Matrix(gdl++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *vi0000b1d0(Gfx *gdl)
{
	gdl = vi0000ad5c(gdl, &g_Vars.currentplayer->viewport[0]);

	gfx_Set_Color_Image(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, g_ViBackData->bufx, (uintptr_t)(g_ViBackData->fb));

	return gdl;
}

Gfx *viPrepareZbuf(Gfx *gdl)
{
	if (g_ViBackData->usezbuf) {
		gfx_No_Param(gdl++, G_CLEAR_DEPTH_EXT);
	}

	return gdl;
}

Gfx *viFillBuffer(Gfx *gdl)
{
	gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);
	gfx_Fill_Rectangle(gdl++, 0, 0, g_ViBackData->bufx - 1, g_ViBackData->bufy - 1);

	return gdl;
}

Gfx *viRenderViewportEdges(Gfx *gdl)
{
	gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);
	gfx_Set_Scissor(gdl++, 0, 0, viGetWidth(), viGetHeight());
	RGBA fillColor = {0, 0, 0, 255};
	gfx_Set_Fill_Color(gdl++, fillColor);

	if (PLAYERCOUNT() == 1
			|| ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
				&& playerHasSharedViewport() && g_Vars.currentplayernum == 0))
	{
		// Single viewport
		if (viGetViewTop() > 0) {
			// Fill above
			gfx_Fill_Rectangle(gdl++, 0, 0, viGetWidth() - 1, viGetViewTop() - 1);
		}

		if (viGetViewTop() + viGetViewHeight() < viGetHeight()) {
			// Fill below
			gfx_Fill_Rectangle(gdl++,
					0, viGetViewTop() + viGetViewHeight(),
					viGetWidth() - 1, viGetHeight() - 1);
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
				gfx_Fill_Rectangle(gdl++, 0, 0, viGetWidth() - 1, g_Vars.players[topplayernum]->viewtop - 1);
			}

			if (g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight < viGetHeight()) {
				// Fill below all viewports - full width
				gfx_Fill_Rectangle(gdl++,
						0, g_Vars.players[bottomplayernum]->viewtop + g_Vars.players[bottomplayernum]->viewheight,
						viGetWidth() - 1, viGetHeight() - 1);
			}

			// Horizontal middle line
			gfx_Fill_Rectangle(gdl++,
					0, g_Vars.players[tmpplayernum]->viewtop - 1,
					viGetWidth() - 1, g_Vars.players[tmpplayernum]->viewtop - 1);

			if (PLAYERCOUNT() >= 3 ||
					(PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL))) {
				if (PLAYERCOUNT() == 2) {
					tmpplayernum = 0;
				}

				// Vertical middle line
				gfx_Fill_Rectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, 0,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth, viGetHeight() - 1);
			}

			if (PLAYERCOUNT() == 3) {
				// Blank square in P4 spot
				gfx_Fill_Rectangle(gdl++,
						g_Vars.players[tmpplayernum]->viewleft + g_Vars.players[tmpplayernum]->viewwidth + 1, g_Vars.players[tmpplayernum]->viewtop,
						viGetWidth() - 1, viGetHeight() - 1);
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

	camSetPerspective(g_ViBackData->fovy, g_ViBackData->aspect);
	camSetScale();
}

void viSetAspect(float aspect)
{
	g_ViBackData->aspect = aspect;

	camSetPerspective(g_ViBackData->fovy, g_ViBackData->aspect);
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
	camSetPerspective(g_ViBackData->fovy, g_ViBackData->aspect);
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

	camSetPerspective(g_ViBackData->fovy, g_ViBackData->aspect);
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
	RGBA fillColor = {r, g, b, 255};
	gfx_Set_Fill_Color(gdl++, fillColor);

	return gdl;
}

void viStop(void)
{
	if (g_FbRawPtr) {
		free(g_FbRawPtr);
		g_FbRawPtr = NULL;
		g_FbPtr = NULL;
	}
}