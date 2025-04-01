#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/menuutils.h"
#include "game/tex.h"
#include "game/stars.h"
#include "game/textutils.h"
#include "game/camera.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"


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
	int v0 = 0;
	int v1 = 0;
	struct coord spd4;
	struct coord spc8;
	int i = 0;
	float spc0 = 0.0f;
	float spbc = 0.0f;
	int count = 0;
	int spb0 = 0;
	float f0 = 0.0f;
	int tmp = 0;
	int tmp1 = 0;
	int tmp2 = 0;

	g_StarPositions = NULL;

	g_StarsBelowHorizon = false;
	g_StarGridSize = 3;

	if (g_Vars.stagenum == STAGE_DEFECTION || g_Vars.stagenum == STAGE_EXTRACTION) {
		g_StarCount = 200;
		g_StarGridSize = 2;
	} else if (g_Vars.stagenum == STAGE_ATTACKSHIP) {
		g_StarsBelowHorizon = true;
		g_StarCount = 1200;
	} else {
		g_StarCount = 200;
		g_StarGridSize = 2;
	}

	tmp = g_StarGridSize + 1;
	g_StarPositions = mempAlloc(ALIGN64(g_StarCount * 3U + tmp * 72 * tmp + 6 * g_StarGridSize * g_StarGridSize * 4U + 4), MEMPOOL_STAGE);

	if (g_StarPositions != NULL) {
		g_StarPosIndexes = (int *)(g_StarPositions + g_StarCount * 3);

		for (i = 0; i < (6 * g_StarGridSize * g_StarGridSize + 1); i++) {
			g_StarPosIndexes[i] = 0;
		}

		count = 6 * g_StarGridSize * g_StarGridSize + 1;
		g_StarData3 = (float *)(count * sizeof(float) + (uintptr_t)g_StarPosIndexes);

		stars0f135c70();

		for (i = 0; i < g_StarCount; i++) {
			spd4.f[0] = 2.0f * RANDOMFRAC() - 1.0f;
			spd4.f[1] = g_StarsBelowHorizon ? 2.0f * RANDOMFRAC() - 1.0f : RANDOMFRAC();
			spd4.f[2] = 2.0f * RANDOMFRAC() - 1.0f;

			utilsNormalizeF(&spd4.f[0], &spd4.f[1], &spd4.f[2]);

			f0 = (fabsf(spd4.f[0]) > fabsf(spd4.f[1])) ? (fabsf(spd4.f[0]) > fabsf(spd4.f[2]) ? fabsf(spd4.f[0]) : fabsf(spd4.f[2])) : (fabsf(spd4.f[1]) > fabsf(spd4.f[2]) ? fabsf(spd4.f[1]) : fabsf(spd4.f[2]));

			spc8.f[0] = spd4.f[0] / f0;
			spc8.f[1] = spd4.f[1] / f0;
			spc8.f[2] = spd4.f[2] / f0;

			tmp1 = g_StarGridSize * g_StarGridSize;

			if (spc8.f[0] == 1 || spc8.f[0] == -1) {
				spb0 = spc8.f[0] == -1 ? 0 : 1;
				spc0 = spc8.f[1];
				spbc = spc8.f[2];
			} else if (spc8.f[1] == 1 || spc8.f[1] == -1) {
				spb0 = spc8.f[1] == -1 ? 2 : 3;
				spc0 = spc8.f[2];
				spbc = spc8.f[0];
			} else if (spc8.f[2] == 1 || spc8.f[2] == -1) {
				spb0 = spc8.f[2] == -1 ? 4 : 5;
				spc0 = spc8.f[0];
				spbc = spc8.f[1];
			} else {
				// empty
			}

			v0 = (spc0 + 1) / 2 * g_StarGridSize;
			v1 = (spbc + 1) / 2 * g_StarGridSize;

			if (v0 == g_StarGridSize) {
				v0--;
			}

			if (v1 == g_StarGridSize) {
				v1--;
			}

			tmp2 = v0 + g_StarGridSize * v1;

			starInsert(spb0 * tmp1 + tmp2, &spd4);
		}
	}
}

Gfx *starsRender(Gfx *gdl)
{
	Mtxf mtx;
	float viewleft = viGetViewLeft();
	float viewright = viewleft + viGetViewWidth();
	float viewtop = viGetViewTop();
	float viewbottom = viewtop + viGetViewHeight();
	int i = 0;
	float sp154;
	struct coord sp148;
	float screenmidx = g_Vars.currentplayer->c_screenleft + g_Vars.currentplayer->c_halfwidth;
	float screenmidy = g_Vars.currentplayer->c_screentop + g_Vars.currentplayer->c_halfheight;
	int j;
	int k;
	int l;
	int tmp;
	uint32_t colours[4];

	if (g_StarPositions == NULL) {
		return gdl;
	}

	// Ben's comment: make stars twinkle. This code was in the original game, it was just missing the colours[i] assignment in the for loop. I also made the stars twinkle 5x faster.
	colours[0] = colourBlend(0xffffff7f, 0x7777777f, menuGetSinOscFrac(10) * 255);
	colours[1] = colourBlend(0x0000aa7f, 0x2222ff7f, menuGetSinOscFrac(20) * 255);
	colours[2] = colourBlend(0x0000ff7f, 0x5555ff7f, menuGetCosOscFrac(10) * 255);
	colours[3] = colourBlend(0xaaaaff7f, 0x7777ff7f, menuGetCosOscFrac(20) * 255);

	colours[i] = colourBlend(colours[i], colours[i] & 0xff, 0x5f);

	sp154 = cosf(0.017453199252486f * (90.0f - viGetFovY() / viGetAspect() * 0.5f));

	mtx4LoadIdentity(&mtx);
	mtxApplyAffineInPlace(camGetWorldToScreenMtxf(), &mtx);

	mtx.m[3][0] = 0.0f;
	mtx.m[3][1] = 0.0f;
	mtx.m[3][2] = 0.0f;

	mtxScaleTransform(262.9f, &mtx);

	mtx.m[0][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx.m[1][1] *= g_Vars.currentplayer->c_recipscaley;
	mtx.m[2][1] *= g_Vars.currentplayer->c_recipscaley;

	mtx.m[0][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx.m[1][0] *= g_Vars.currentplayer->c_recipscalex;
	mtx.m[2][0] *= g_Vars.currentplayer->c_recipscalex;

	sp148.f[0] = g_Vars.currentplayer->cam_look.f[0];
	sp148.f[1] = g_Vars.currentplayer->cam_look.f[1];
	sp148.f[2] = g_Vars.currentplayer->cam_look.f[2];

	gdl = textSetPrimColour(gdl, 0xffffffff);

	gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	//texSelect(&gdl, &g_TexStarsConfigs[0], 2, 1, 2, 1, NULL);

	for (i = 0; i < 6; i++) {
		if (g_StarsBelowHorizon || i != 2) {
			float f0;
			float f0_2;
			bool spd0[4][4];
			struct coord spc4;

			for (j = 0; j <= g_StarGridSize; j++) {
				for (k = 0; k <= g_StarGridSize; k++) {
					tmp = ((g_StarGridSize + 1) * i * (g_StarGridSize + 1) + k + j * (g_StarGridSize + 1)) * 3;
					f0 = sp148.f[0] * g_StarData3[tmp] + sp148.f[1] * g_StarData3[tmp + 1] + sp148.f[2] * g_StarData3[tmp + 2];

					if (f0 <= sp154) {
						spd0[k][j] = true;
					} else {
						spd0[k][j] = false;
					}
				}
			}

			for (j = 0; j < g_StarGridSize; j++) {
				for (k = 0; k < g_StarGridSize; k++) {
					if (spd0[k][j] == 0 || spd0[k + 1][j] == 0 || spd0[k][j + 1] == 0 || spd0[k + 1][j + 1] == 0) {
						int tmp = g_StarGridSize * g_StarGridSize * i + k + j * g_StarGridSize;
						int colourindex = 0;
						float screenpos[2];
						int drawpos[2];
						int nextgroupstart = g_StarPosIndexes[tmp];
						int groupsize = (g_StarPosIndexes[tmp + 1] - g_StarPosIndexes[tmp]) / 4 + 1;
						int8_t *pos = &g_StarPositions[g_StarPosIndexes[tmp] * 3];

						for (l = g_StarPosIndexes[tmp]; l < g_StarPosIndexes[tmp + 1]; l++) {
							if (nextgroupstart == l) {
								gDPSetPrimColorViaWord(gdl++, 0, 0, colours[colourindex]);

								colourindex++;
								nextgroupstart += groupsize;
							}

							spc4.f[0] = pos[0];
							spc4.f[1] = pos[1];
							spc4.f[2] = pos[2];
							pos += 3;

							f0_2 = 1.0f / (mtx.m[0][2] * spc4.f[0] + mtx.m[1][2] * spc4.f[1] + mtx.m[2][2] * spc4.f[2]);
							screenpos[1] = screenmidy + (mtx.m[0][1] * spc4.f[0] + mtx.m[1][1] * spc4.f[1] + mtx.m[2][1] * spc4.f[2]) * f0_2;

							if (screenpos[1] > viewtop && screenpos[1] < viewbottom) {
								screenpos[0] = screenmidx - (mtx.m[0][0] * spc4.f[0] + mtx.m[1][0] * spc4.f[1] + mtx.m[2][0] * spc4.f[2]) * f0_2;

								if (screenpos[0] > viewleft && screenpos[0] < viewright) {
									drawpos[0] = screenpos[0];
									drawpos[1] = screenpos[1];

									gDPFillRectangle(gdl++, drawpos[0], drawpos[1], drawpos[0] + 1, drawpos[1] + 1);
								}
							}
						}
					}
				}
			}
		}
	}

	gdl = textSetCCCustom02(gdl);

	return gdl;
}
