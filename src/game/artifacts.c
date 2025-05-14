#include <math.h>
#include "lib/sched.h"
#include "constants.h"
#include "game/artifacts.h"
#include "game/bg.h"
#include "game/camera.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/env.h"
#include "game/mtxutils.h"
#include "game/player.h"
#include "game/prop.h"
#include "game/room.h"
#include "game/sky.h"
#include "game/stagetable.h"
#include "game/tex.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "lib/collision.h"
#include "lib/lib_17ce0.h"

uint8_t *var800a41a0;

void artifactsClear(void)
{
	struct artifact *artifacts = schedGetWriteArtifacts();
	int i;

	for (i = 0; i < MAX_ARTIFACTS; i++) {
		artifacts[i].type = ARTIFACTTYPE_FREE;
	}
}

void artifactsTick(void)
{
	schedIncrementWriteArtifacts();
	schedIncrementFrontArtifacts();
}

bool artifactTestLos(struct coord *spec, struct coord *roompos, int xi, int yi)
{
	if (!g_Vars.currentplayer) {
		return false;
	}

	struct coord endpos;
	endpos.x = roompos->x + spec->x;
	endpos.y = roompos->y + spec->y;
	endpos.z = roompos->z + spec->z;

	struct coord gundir2d;
	struct coord gunpos2d = {{ 0.f, 0.f, 0.f }};
	struct coord gundir3d;
	struct coord gunpos3d = g_Vars.currentplayer->cam_pos;
	float crosspos[2] = { (float)xi, (float)yi };
	camProjectScreenToWorldDir(crosspos, &gundir2d, 1.f);
	mtx4RotateVec((Mtx*)camGetProjectionMtx(), &gundir2d, &gundir3d);

	return propTestArtifactLos(&gunpos2d, &gundir2d, &gunpos3d, &gundir3d, &endpos);
}

void artifactsCalculateGlaresForRoom(int roomnum)
{
	float lightDepth;
	float x;
	float y;
	float invW;
	float dist;
	int xi;
	int yi;
	float directionalDot;
	float brightnessfrac;
	float thisfrac;
	float clampDiff;
	float depthFalloff;
	float directionalScale;
	Mtx sp138;
	Mtx spf8;
	struct coord spec;
	float screenPos[4];
	int index;
	struct artifact *artifacts = schedGetWriteArtifacts();
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	struct artifact *artifact;

	if (!g_Rooms[roomnum].gfxdata || !g_Rooms[roomnum].loaded240) {
		return;
	}

	int numlights = g_Rooms[roomnum].gfxdata->numlights;

	if (numlights == 0) {
		return;
	}

	struct light *roomlights = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].gfxdata->lightsindex * 0x22];
	uint8_t *lightGlares = &var800a41a0[g_Rooms[roomnum].gfxdata->lightsindex * 3];

	roomPopulateMtx(&sp138, roomnum);
	mtx4MultMtx4(g_Vars.currentplayer->artifactMtx, &sp138, &spf8);

	float viewwidth = viGetViewWidth();
	float viewheight = viGetViewHeight();
	float viewleft = viGetViewLeft();
	float viewtop = viGetViewTop();

	for (int i = 0; i < numlights; i++) {
		struct coord lightOrigin = {0};
		for (int j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
			lightOrigin.x += roomlights[i].bbox[j].x;
			lightOrigin.y += roomlights[i].bbox[j].y;
			lightOrigin.z += roomlights[i].bbox[j].z;
		}
		lightOrigin.x /= 4.0f;
		lightOrigin.y /= 4.0f;
		lightOrigin.z /= 4.0f;

		struct coord lightToCam;
		for (int j = 0; j != 3; j++) {
			lightToCam.f[j] = lightOrigin.f[j] - (campos->f[j] - g_BgRooms[roomnum].pos.f[j]);
		}

		lightGlares[i * 3 + 1] = 0;
		lightGlares[i * 3 + 2] = 0;

		float lightDirLengthSq = roomlights[i].dirx * roomlights[i].dirx + roomlights[i].diry * roomlights[i].diry + roomlights[i].dirz * roomlights[i].dirz;
		float camToLightLengthSq = lightToCam.f[0] * lightToCam.f[0] + lightToCam.f[1] * lightToCam.f[1] + lightToCam.f[2] * lightToCam.f[2];

		

		if (lightDirLengthSq > 0.0001f && camToLightLengthSq > 0.0001f) {
			directionalDot = -((roomlights[i].dirx * lightToCam.f[0] + roomlights[i].diry * lightToCam.f[1] + roomlights[i].dirz * lightToCam.f[2]) / sqrtf(lightDirLengthSq * camToLightLengthSq));

			if (directionalDot > 0.4f) {
				directionalDot = 0.4f;
			}

			directionalDot *= 2.5f;
		} else {
			directionalDot = 0.0f;
		}

		if (directionalDot > 0.0f) {
			for (int l = 3; l >= 0; l--) {
				screenPos[l] = lightOrigin.f[0] * spf8[0][l] + lightOrigin.f[1] * spf8[1][l] + lightOrigin.f[2] * spf8[2][l] + spf8[3][l];

				if (l == 3 && screenPos[l] <= 0.0f) {
					break;
				}
			}

			if (screenPos[3] > 0.0001f) {
				invW = 1.0f / screenPos[3];
				x = utilsClampF(viewleft + (1.0f + screenPos[0] * invW) * (viewwidth * 0.5f), -2147483520.0f, 2147483520.0f);
				y = utilsClampF(viewtop + (1.0f - screenPos[1] * invW) * (viewheight * 0.5f), -2147483520.0f, 2147483520.0f);
				lightDepth = (screenPos[2] * invW * 511.0f + 511.0f) * 32.0f;

				if (lightDepth < 32576.0f * 2) {
					brightnessfrac = 1.0f;
					clampDiff = (brightnessfrac - 1.00f);

					if (x <= 10.0f + viewleft) {
						brightnessfrac = 0.0f;
					} else if (y <= 30.0f + viewtop) {
						brightnessfrac = 0.0f;
					} else if (x >= -10.0f + viewleft + viewwidth) {
						brightnessfrac = 0.0f;
					} else if (y >= -30.0f + viewtop + viewheight) {
						brightnessfrac = 0.0f;
					}

					directionalScale = 1.0f - 2.0f * clampDiff;

					if (brightnessfrac != 0.0f) {
						brightnessfrac = 1.0f;

						if (x < viewleft + 90.0f) {
							thisfrac = (x - (10.0f + viewleft)) / 80.0f;

							if (thisfrac < brightnessfrac) {
								brightnessfrac = thisfrac;
							}
						}

						if (y < viewtop + 100.0f) {
							thisfrac = (y - (viewtop + 30.0f)) / 70.0f;

							if (thisfrac < brightnessfrac) {
								brightnessfrac = thisfrac;
							}
						}

						if (x > viewleft + viewwidth - 90.0f) {
							thisfrac = (viewleft + viewwidth - 10.0f - x) / 80.0f;

							if (thisfrac < brightnessfrac) {
								brightnessfrac = thisfrac;
							}
						}

						if (y > viewtop + viewheight - 100.0f) {
							thisfrac = (viewtop + viewheight - 30.0f - y) / 70.0f;

							if (thisfrac < brightnessfrac) {
								brightnessfrac = thisfrac;
							}
						}
					}

					depthFalloff = 32300.0f * 2 - lightDepth;

					if (depthFalloff < 0.0f) {
						depthFalloff = 0.0f;
					}

					if (depthFalloff > 2600.0f) {
						depthFalloff = 2600.0f;
					}

					depthFalloff *= 1.0f / 2600.0f;

					if (2.0f * clampDiff > 1.0f) {
						directionalScale = 0.0f;
					}

					lightGlares[i * 3 + 1] = directionalDot * 255.0f * directionalScale;
					lightGlares[i * 3 + 2] = brightnessfrac * depthFalloff * directionalDot * 64.0f * 1;
				}
			}
		}

		if (lightGlares[i * 3 + 1] > 0) {
			for (int j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
				spec.x = lightOrigin.x + (roomlights[i].bbox[j].x - lightOrigin.x) * 0.6f;
				spec.y = lightOrigin.y + (roomlights[i].bbox[j].y - lightOrigin.y) * 0.6f;
				spec.z = lightOrigin.z + (roomlights[i].bbox[j].z - lightOrigin.z) * 0.6f;

				for (int k = 3; k >= 0; k--) {
					screenPos[k] = spec.f[0] * spf8[0][k] + spec.f[1] * spf8[1][k] + spec.f[2] * spf8[2][k] + spf8[3][k];

					if (k == 3 && screenPos[k] <= 0.0f) {
						break;
					}
				}

				if (screenPos[3] > 0.0f) {
					invW = 1.0f / screenPos[3];

					if (invW > 9999.0f) {
						invW = 9999.0f;
					}

					if (invW < -9999.0f) {
						invW = -9999.0f;
					}

					xi = utilsClampF(viewleft + (1.0f + screenPos[0] * invW) * (viewwidth * 0.5f), -2147483520.0f, 2147483520.0f);
					yi = utilsClampF(viewtop + (1.0f - screenPos[1] * invW) * (viewheight * 0.5f), -2147483520.0f, 2147483520.0f);
					lightDepth = (screenPos[2] * invW * 511.0f + 511.0f) * 32.0f;

					if (g_ZbufPtr1
							&& xi >= (int)viewleft
							&& xi < (int)(viewleft + viewwidth)
							&& yi >= (int)viewtop
							&& yi < (int)(viewtop + viewheight)) {
						index = envGetCurrent()->numsuns;
						index *= 8;
						artifact = artifacts;
						artifact += index;

						while (artifact->type != ARTIFACTTYPE_FREE) {
							index++;
							artifact++;
						}

						if (index < MAX_ARTIFACTS) {
							artifact->losCheckResult = artifactTestLos(&spec, &g_BgRooms[roomnum].pos, xi, yi);
							artifact->zbufferDepth = lightDepth;
							artifact->zbufferPixelPtr = &g_ZbufPtr1[viGetWidth() * yi + xi];
							artifact->light = &roomlights[i];
							artifact->type = ARTIFACTTYPE_GLARE;
							artifact->screenPos.screenX = xi;
							artifact->screenPos.screenY = yi;
							artifact->dist = sqrt(camToLightLengthSq);
						}
					}
				}
			}
		}
	}
}

uint8_t func0f13d3c4(uint8_t arg0, uint8_t arg1)
{
	if (arg1 >= arg0 + 7) {
		return arg0 + 7;
	}

	if (arg1 <= arg0 - 7) {
		return arg0 - 7;
	}

	return arg1;
}

Gfx *artifactsConfigureForGlares(Gfx *gdl)
{
	struct stagetableentry *stage = stageGetCurrent();

	texSelect(&gdl, &g_TexLightGlareConfigs[stage->light_type], 4, 0, 2, 1, NULL);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	gfx_Set_Combine_LERP(gdl++,
			0, 0, 0, G_CCMUX_ENVIRONMENT, G_ACMUX_TEXEL0, 0, G_ACMUX_ENVIRONMENT, 0,
			0, 0, 0, G_CCMUX_ENVIRONMENT, G_ACMUX_TEXEL0, 0, G_ACMUX_ENVIRONMENT, 0);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);

	return gdl;
}

Gfx *artifactsUnconfigureForGlares(Gfx *gdl)
{
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

	return gdl;
}

Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, int roomnum)
{
	int i, j, k, l;
	int lightindex;
	struct artifact *artifacts;
	struct light *light;
	float minDepth, maxDepth;
	float lightop_cur_frac;
	int totalLosHits;
	uint8_t *lightStats;
	int count;
	float addGlow;
	float brightness; // The closer you get to an artifact, the higher this becomes.
	int v1;
	int r, g, b;
	uint8_t envColor[4];
	int16_t lightroompos[3];
	struct coord lightworldpos;
	struct coord lightscreenpos;
	float screenPos[2];
	float screenSize[2];
	float aspectScale;
	bool addWhiteOverlay;
	float screenScale;

	artifacts = schedGetFrontArtifacts();
	lightop_cur_frac = roomGetLightOpCurFrac(roomnum);

	if (g_Rooms[roomnum].gfxdata == NULL || g_Rooms[roomnum].loaded240 == 0) {
		return gdl;
	}

	for (i = envGetCurrent()->numsuns * 8; i < MAX_ARTIFACTS; i++) {
		struct light *light2 = artifacts[i].light;
		count = 0;

		for (j = i; j < MAX_ARTIFACTS && artifacts[j].type == ARTIFACTTYPE_GLARE && artifacts[j].light == light2; j++) {
			count++;
		}

		light = artifacts[i].light;

		if (count > 0) {
			if (roomnum == light->roomnum) {
				lightindex = ((uintptr_t)light - (uintptr_t)g_BgLightsFileData) / sizeof(struct light);
				lightStats = &var800a41a0[lightindex * 3];
				totalLosHits = 0;
				minDepth = 65536.0f;
				maxDepth = 0.0f;

				for (k = i; k < i + count; k++) {
					if (artifacts[k].zbufferDepth > maxDepth) {
						maxDepth = artifacts[k].zbufferDepth;
					}

					if (artifacts[k].zbufferDepth < minDepth) {
						minDepth = artifacts[k].zbufferDepth;
					}
				}

				for (k = i; k < i + count; k++) {
					totalLosHits += artifacts[k].losCheckResult;

					artifacts[k].type = ARTIFACTTYPE_FREE;
				}

				lightStats[0] = func0f13d3c4(lightStats[0], totalLosHits * 2);

				if (totalLosHits > 0) {
					brightness = viGetFovY() * 0.017453292f;
					addGlow = cosf(brightness) / sinf(brightness) * 14.6f;

					if (lightIsHealthy(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
						if (!lightIsOn(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
							continue;
						}

						brightness = 1.0f;
					} else if (lightTickBroken(roomnum, lightindex - g_Rooms[roomnum].gfxdata->lightsindex)) {
						// Decrease artifact brightness on broken lights to 40%
						brightness = 0.4f;
					} else {
						continue;
					}

					r = ((light->colour >> 12) & 0xf) * 17;
					g = ((light->colour >> 8) & 0xf) * 17;
					b = ((light->colour >> 4) & 0xf) * 17;

					if ((r == 0xff && g == 0xff && b == 0xff) || (r == 0xff && g + b < 35)) {
						addWhiteOverlay = false;
					} else {
						addWhiteOverlay = true;
					}

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						lightStats[2] *= (int) (lightop_cur_frac * 7.0f);
					}

					for (l = 0; l < 3; l++) {
						lightroompos[l] = (light->bbox[0].s[l] + light->bbox[1].s[l] + light->bbox[2].s[l] + light->bbox[3].s[l]) / 4;
						lightworldpos.f[l] = lightroompos[l] + g_BgRooms[roomnum].pos.f[l];
						lightscreenpos.f[l] = lightworldpos.f[l] - g_Vars.currentplayer->cam_pos.f[l];
					}

					mtx4RotateVecInPlace((Mtx*)camGetPlayerWorldToScreenMtx(), &lightscreenpos);

					camProjectViewToScreen(&lightscreenpos, screenPos);

					brightness *= 27500.0f / (-lightscreenpos.z < 1.0f ? 1.0f : -lightscreenpos.z);

					if (light->brightnessmult != 0) {
						brightness *= light->brightnessmult * (1.0f / 32.0f);
					}

					brightness *= lightStats[1] * (1.0f / 255.0f);

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						brightness *= 14.0f * lightop_cur_frac;
					}

					brightness += addGlow;
					brightness *= 2.0f * roomGetSettledLocalBrightnessFrac(roomnum);

					if (brightness > 750.0f) {
						brightness = 750.0f;
					}

					aspectScale = stageGetCurrent()->light_width * brightness * 0.01f;
					screenScale = stageGetCurrent()->light_height * brightness * 0.01f;

					aspectScale *= viGetViewWidth() * (1.0f / 240.0f) / camGetPerspAspect();
					screenScale *= viGetViewHeight() * (1.0f / 240.0f);

					if (brightness > 3.0f) {
						float alpha = (light->colour & 0xf) * 17;

						float dist = 1.0f - (artifacts[i].dist / 2500.0f);
						float overexposureAmount = lightStats[2] * (1.0f / 255.0f) * utilsClampF(dist, 0.0f, 1.0f);
						
						envColor[0] = r;
						envColor[1] = g;
						envColor[2] = b;

						alpha *= stageGetCurrent()->light_alpha / 255.0f;
						alpha *= (lightStats[1] / 255.0f);
						alpha *= (lightStats[0] / 8.0f);

						if (USINGDEVICE(DEVICE_NIGHTVISION)) {
							alpha *= lightop_cur_frac * 7.0f;
						}

						if (alpha > 255.0f) {
							alpha = 255.0f;
						}

						envColor[3] = alpha;

						// Only render overexposure if alpha is over 10.0
						if(alpha > 10.0f)
						{
							skySetOverexposure((int) ((float)overexposureAmount * r), (int) ((float)overexposureAmount * g), (int) ((float)overexposureAmount * b));
							RGBA color = {envColor[0], envColor[1], envColor[2], envColor[3]};
							gfx_Set_Env_Color(gdl++, color);
						}
						// Still render the light, but don't do overexposure
						else
						{
							RGBA color = {envColor[0], envColor[1], envColor[2], 255};
							gfx_Set_Env_Color(gdl++, color);
						}

						screenSize[0] = aspectScale;
						screenSize[1] = screenScale;

						utilsRenderScreenTexture(&gdl, screenPos, screenSize, 64, 64, false, false, true);

						// Make artifacts slightly brighter when true
						if (addWhiteOverlay) {
							envColor[0] = 255;
							envColor[1] = 255;
							envColor[2] = 255;
							envColor[3] = stageGetCurrent()->light_alpha;
							envColor[3] = lightStats[0] * envColor[3] / 8;

							RGBA color = {envColor[0], envColor[1], envColor[2], envColor[3]};
							gfx_Set_Env_Color(gdl++, color);

							screenSize[0] = aspectScale * 0.4f;
							screenSize[1] = screenScale * 0.4f;

							utilsRenderScreenTexture(&gdl, screenPos, screenSize, 64, 64, false, false, true);
						}
					}
				}

				lightStats[1] = 0;
				lightStats[2] = 0;
			}

			// This is incrementing i past all the artifacts for this particular
			// light, then subtracting 1 because the for loop will add 1.
			i = i + count - 1;
		}
	}

	return gdl;
}
