#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/menuutils.h"
#include "game/player.h"
#include "game/tex.h"
#include "game/stars.h"
#include "game/textutils.h"
#include "game/camera.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "video.h"


int g_StarCount;
int8_t *g_StarPositions = NULL;
float *g_StarData3;
int g_StarGridSize;
int *g_StarPosIndexes;

bool g_StarsBelowHorizon = false;

struct textureconfig *g_StarConfig;

void stars0f135c70(void)
{
	struct coord coord;
	float mult;
	int i;
	int j;
	int k;
	float tmp = g_StarGridSize * 0.5f;

	for (i = 0; i < 6; i++) {
		for (j = 0; j <= g_StarGridSize; j++) {
			for (k = 0; k <= g_StarGridSize; k++) {
				int index = ((i * (g_StarGridSize + 1) * (g_StarGridSize + 1)) + k + (j * (g_StarGridSize + 1))) * 3;

				switch (i) {
				case 0:
				case 1:
					coord.x = (i == 0 ? -1.0f : 1.0f);
					coord.y = k / tmp - 1;
					coord.z = j / tmp - 1;
					break;
				case 2:
				case 3:
					coord.y = (i == 2 ? -1.0f : 1.0f);
					coord.x = j / tmp - 1;
					coord.z = k / tmp - 1;
					break;
				case 4:
				case 5:
					coord.z = (i == 4 ? -1.0f : 1.0f);
					coord.x = k / tmp - 1;
					coord.y = j / tmp - 1;
					break;
				}

				mult = 1.0f / sqrtf(coord.f[0] * coord.f[0] + coord.f[1] * coord.f[1] + coord.f[2] * coord.f[2]);

				g_StarData3[index + 0] = coord.x * mult;
				g_StarData3[index + 1] = coord.y * mult;
				g_StarData3[index + 2] = coord.z * mult;
			}
		}
	}
}

/**
 * Insert a star position *after* the given index.
 */
void starInsert(int index, struct coord *arg1)
{
	int i;

	// Shuffle g_StarPositions forward after the insertion point
	for (i = g_StarPosIndexes[g_StarGridSize * 6 * g_StarGridSize] - 1; i >= g_StarPosIndexes[index + 1]; i--) {
		g_StarPositions[i * 3 + 3] = g_StarPositions[i * 3 + 0];
		g_StarPositions[i * 3 + 4] = g_StarPositions[i * 3 + 1];
		g_StarPositions[i * 3 + 5] = g_StarPositions[i * 3 + 2];
	}

	// Write new data
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 0] = arg1->x * 127;
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 1] = arg1->y * 127;
	g_StarPositions[g_StarPosIndexes[index + 1] * 3 + 2] = arg1->z * 127;

	// Increment indexes after the insertion point
	for (i = index + 1; i <= g_StarGridSize * 6 * g_StarGridSize; i++) {
		g_StarPosIndexes[i]++;
	}
}

void starsReset(void)
{
	int gridX = 0;
	int gridY = 0;
	struct coord starDirection;
	struct coord majorAxisUnit;
	int i = 0;
	float majorCoord1 = 0.0f;
	float majorCoord2 = 0.0f;
	int indexCount = 0;
	int faceIndex = 0;
	float maxAbs = 0.0f;
	int gridCellCount = 0;
	int faceCellIndex = 0;

	g_StarPositions = NULL;

	g_StarsBelowHorizon = false;
	g_StarGridSize = 3;

	if (g_Vars.stagenum == STAGE_ATTACKSHIP) {
		g_StarsBelowHorizon = true;
		g_StarCount = 1200;
	} else {
		g_StarCount = 200;
		g_StarGridSize = 2;
	}

	int gridPlusOne = g_StarGridSize + 1;
	g_StarPositions = mempAlloc(
		ALIGN64((g_StarCount * 3) + (gridPlusOne * 72 * gridPlusOne) + (g_StarGridSize * g_StarGridSize * 24) + 4),
		MEMPOOL_STAGE
	);

	if (g_StarPositions != NULL) {
		g_StarPosIndexes = (int *)(g_StarPositions + g_StarCount * 3);

		for (i = 0; i < (6 * g_StarGridSize * g_StarGridSize + 1); i++) {
			g_StarPosIndexes[i] = 0;
		}

		indexCount = 6 * g_StarGridSize * g_StarGridSize + 1;
		g_StarData3 = (float *)(indexCount * sizeof(float) + (uintptr_t)g_StarPosIndexes);

		stars0f135c70();

		for (i = 0; i < g_StarCount; i++) {
			starDirection.x = 2.0f * RANDOMFRAC() - 1.0f;
			starDirection.y = g_StarsBelowHorizon ? 2.0f * RANDOMFRAC() - 1.0f : RANDOMFRAC();
			starDirection.z = 2.0f * RANDOMFRAC() - 1.0f;

			utilsNormalizeF(&starDirection.x, &starDirection.y, &starDirection.z);

			maxAbs = (fabsf(starDirection.x) > fabsf(starDirection.y))
				? (fabsf(starDirection.x) > fabsf(starDirection.z) ? fabsf(starDirection.x) : fabsf(starDirection.z))
				: (fabsf(starDirection.y) > fabsf(starDirection.z) ? fabsf(starDirection.y) : fabsf(starDirection.z));

			majorAxisUnit.x = starDirection.x / maxAbs;
			majorAxisUnit.y = starDirection.y / maxAbs;
			majorAxisUnit.z = starDirection.z / maxAbs;

			gridCellCount = g_StarGridSize * g_StarGridSize;

			if (majorAxisUnit.x == 1 || majorAxisUnit.x == -1) {
				faceIndex = majorAxisUnit.x == -1 ? 0 : 1;
				majorCoord1 = majorAxisUnit.y;
				majorCoord2 = majorAxisUnit.z;
			} else if (majorAxisUnit.y == 1 || majorAxisUnit.y == -1) {
				faceIndex = majorAxisUnit.y == -1 ? 2 : 3;
				majorCoord1 = majorAxisUnit.z;
				majorCoord2 = majorAxisUnit.x;
			} else if (majorAxisUnit.z == 1 || majorAxisUnit.z == -1) {
				faceIndex = majorAxisUnit.z == -1 ? 4 : 5;
				majorCoord1 = majorAxisUnit.x;
				majorCoord2 = majorAxisUnit.y;
			}

			gridX = (majorCoord1 + 1) / 2 * g_StarGridSize;
			gridY = (majorCoord2 + 1) / 2 * g_StarGridSize;

			if (gridX == g_StarGridSize) gridX--;
			if (gridY == g_StarGridSize) gridY--;

			faceCellIndex = faceIndex * gridCellCount + gridX + g_StarGridSize * gridY;

			starInsert(faceCellIndex, &starDirection);
		}
	}
}

Gfx *starsRender(Gfx *gdl)
{
	Mtx mtx;
	float viewleft = playerGetViewportLeftReal();
	float viewright = viewleft + playerGetViewportWidthReal();
	float viewtop = playerGetViewportTopReal();
	float viewbottom = viewtop + playerGetViewportHeightReal();
	float fovCosThreshold;
	struct coord camLookVector;
	float screenmidx = playerGetViewportLeftReal() + g_Vars.currentplayer->c_halfwidth;
	float screenmidy = playerGetViewportTopReal() + g_Vars.currentplayer->c_halfheight;
	int j;
	int k;
	uint32_t colours[4];

	if (g_StarPositions == NULL) {
		return gdl;
	}

	// Ben's comment: make stars twinkle. This code was in the original game, it was just missing the colours[i] assignment in the for loop.
	colours[0] = colourBlend(0xffffff7f, 0x7777777f, menuGetSinOscFrac(10) * 255);
	colours[1] = colourBlend(0x0000aa7f, 0x2222ff7f, menuGetSinOscFrac(20) * 255);
	colours[2] = colourBlend(0x0000ff7f, 0x5555ff7f, menuGetCosOscFrac(10) * 255);
	colours[3] = colourBlend(0xaaaaff7f, 0x7777ff7f, menuGetCosOscFrac(20) * 255);

	colours[0] = colourBlend(colours[0], colours[0] & 0xff, 95);

	fovCosThreshold = cosf(0.017453199252486f * (90.0f - viGetFovY() / videoGetAspect() * 0.5f));

	mtxIdent(&mtx);
	mtxApplyAffineTransformInPlace(camGetPlayerWorldToScreenMtx(), &mtx);

	mtx[3][0] = 0.0f;
	mtx[3][1] = 0.0f;
	mtx[3][2] = 0.0f;

	mtxScale3x4(262.9f, &mtx);

	mtx[0][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx[1][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx[2][1] *= g_Vars.currentplayer->c_recipscaley;

	mtx[0][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx[1][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx[2][0] *= g_Vars.currentplayer->c_recipscalex;

	camLookVector.x = g_Vars.currentplayer->cam_look.x;
	camLookVector.y = g_Vars.currentplayer->cam_look.y;
	camLookVector.z = g_Vars.currentplayer->cam_look.z;

	gdl = textSetPrimColour(gdl, 0xffffffff);

	gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	//texSelect(&gdl, &g_TexStarsConfigs[0], 2, 1, 2, 1, NULL);

	for (int i = 0; i < 6; i++) {
		if (g_StarsBelowHorizon || i != 2) {
			float f0;
			float f0_2;
			bool starVisible[4][4];
			struct coord starPos;

			for (j = 0; j <= g_StarGridSize; j++) {
				for (k = 0; k <= g_StarGridSize; k++) {
					int tmp = ((g_StarGridSize + 1) * i * (g_StarGridSize + 1) + k + j * (g_StarGridSize + 1)) * 3;
					f0 = camLookVector.x * g_StarData3[tmp] + camLookVector.y * g_StarData3[tmp + 1] + camLookVector.z * g_StarData3[tmp + 2];

					if (f0 <= fovCosThreshold) {
						starVisible[k][j] = true;
					} else {
						starVisible[k][j] = false;
					}
				}
			}

			for (j = 0; j < g_StarGridSize; j++) {
				for (k = 0; k < g_StarGridSize; k++) {
					if (starVisible[k][j] == 0 || starVisible[k + 1][j] == 0 || starVisible[k][j + 1] == 0 || starVisible[k + 1][j + 1] == 0) {
						int tmp = g_StarGridSize * g_StarGridSize * i + k + j * g_StarGridSize;
						int colourindex = 0;
						float screenpos[2];
						float drawpos[2];
						int nextgroupstart = g_StarPosIndexes[tmp];
						int groupsize = (g_StarPosIndexes[tmp + 1] - g_StarPosIndexes[tmp]) / 4 + 1;
						int8_t *pos = &g_StarPositions[g_StarPosIndexes[tmp] * 3];

						for (int l = g_StarPosIndexes[tmp]; l < g_StarPosIndexes[tmp + 1]; l++) {
							if (nextgroupstart == l) {
								struct RGBA tmp = utilsUnpackColorRGBA(colours[colourindex]);
								gfx_Set_Prim_Color(gdl++, tmp);

								colourindex++;
								nextgroupstart += groupsize;
							}

							starPos.x = pos[0];
							starPos.y = pos[1];
							starPos.z = pos[2];
							pos += 3;

							f0_2 = 1.0f / (mtx[0][2] * starPos.x + mtx[1][2] * starPos.y + mtx[2][2] * starPos.z);
							screenpos[1] = screenmidy + (mtx[0][1] * starPos.x + mtx[1][1] * starPos.y + mtx[2][1] * starPos.z) * f0_2;

							if (screenpos[1] > viewtop && screenpos[1] < viewbottom) {
								screenpos[0] = screenmidx - (mtx[0][0] * starPos.x + mtx[1][0] * starPos.y + mtx[2][0] * starPos.z) * f0_2;

								if (screenpos[0] > viewleft && screenpos[0] < viewright) {
									drawpos[0] = screenpos[0];
									drawpos[1] = screenpos[1];

									gdl += gfx_Fill_Rectangle(gdl, drawpos[0], drawpos[1], drawpos[0] + 1, drawpos[1] + 1);
								}
							}
						}
					}
				}
			}
		}
	}

	gdl = textSetCCPrimColorTexAlpha(gdl);

	return gdl;
}
