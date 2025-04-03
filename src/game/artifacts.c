#include <ultra64.h>
#include "lib/sched.h"
#include "constants.h"
#include "game/camera.h"
#include "game/dlights.h"
#include "game/env.h"
#include "game/utils.h"
#include "game/tex.h"
#include "game/sky.h"
#include "game/artifacts.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/room.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"
#include "lib/collision.h"
#include "lib/lib_17ce0.h"
#include "game/player.h"
#include "game/prop.h"
#include "game/debug.h"

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

uint16_t func0f13c574(float arg0)
{
	uint32_t value = arg0 * 8.0f;
	uint32_t left;
	uint32_t right = value;

	if (value > 0x3f800) {
		right = value & 0x7ff;
		right &= 0x7ff;
		left = 7;
	} else if (value > 0x3f000) {
		right = value & 0x7ff;
		right &= 0x7ff;
		left = 6;
	} else if (value > 0x3e000) {
		right = (value >> 1) & 0x7ff;
		right &= 0x7ff;
		left = 5;
	} else if (value > 0x3c000) {
		right = (value >> 2) & 0x7ff;
		right &= 0x7ff;
		left = 4;
	} else if (value > 0x38000) {
		right = (value >> 3) & 0x7ff;
		right &= 0x7ff;
		left = 3;
	} else if (value > 0x30000) {
		right = (value >> 4) & 0x7ff;
		right &= 0x7ff;
		left = 2;
	} else if (value > 0x20000) {
		right = (value >> 5) & 0x7ff;
		right &= 0x7ff;
		left = 1;
	} else {
		right = (value >> 6) & 0x7ff;
		right &= 0x7ff;
		left = 0;
	}

	return left << 13 | (right << 2);
}

int func0f13c710(float arg0)
{
	if (arg0 > 0.0f) {
		if (arg0 > 2147483520.0f) {
			arg0 = 2147483520;
		}
	} else {
		if (arg0 < -2147483520) {
			arg0 = -2147483520;
		}
	}

	return arg0;
}

bool artifactTestLos(struct coord *spec, struct coord *roompos, int xi, int yi)
{
	int i = 0;

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
	cam0f0b4c3c(crosspos, &gundir2d, 1.f);
	mtx4RotateVec(camGetProjectionMtxF(), &gundir2d, &gundir3d);

	return shotTestLos(&gunpos2d, &gundir2d, &gunpos3d, &gundir3d, &endpos);
}

void artifactsCalculateGlaresForRoom(int roomnum)
{
	int i;
	int j;
	int k;
	int l;
	float f0;
	int numlights;
	float viewwidth;
	float viewheight;
	float viewleft;
	float viewtop;
	uint8_t *s1;
	float x;
	float y;
	float f16;
	float f20;
	int xi;
	int yi;
	float sp190;
	float brightnessfrac;
	float thisfrac;
	float tmp;
	float tmp2;
	float tmp3;
	float sp178;
	Mtxf sp138;
	Mtxf spf8;
	struct coord spec;
	float spdc[4];
	struct coord origin;
	struct coord spc4;
	struct light *roomlights;
	int index;
	struct artifact *artifacts = schedGetWriteArtifacts();
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	struct artifact *artifact;

	if (g_Rooms[roomnum].gfxdata != NULL && g_Rooms[roomnum].loaded240) {
		numlights = g_Rooms[roomnum].gfxdata->numlights;

		if (numlights != 0) {
			roomlights = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].gfxdata->lightsindex * 0x22];
			s1 = &var800a41a0[g_Rooms[roomnum].gfxdata->lightsindex * 3];

			roomPopulateMtx(&sp138, roomnum);
			mtx00015f88(bgGetScaleBg2Gfx(), &sp138);
			mtx4MultMtx4(camGetMtxF006c(), &sp138, &spf8);

			viewwidth = viGetViewWidth();
			viewheight = viGetViewHeight();
			viewleft = viGetViewLeft();
			viewtop = viGetViewTop();

			for (i = 0; i < numlights; i++) {
				origin.x = 0.0f;
				origin.y = 0.0f;
				origin.z = 0.0f;

				for (j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
					origin.x += roomlights[i].bbox[j].x;
					origin.y += roomlights[i].bbox[j].y;
					origin.z += roomlights[i].bbox[j].z;
				}

				origin.x /= 4.0f;
				origin.y /= 4.0f;
				origin.z /= 4.0f;

				for (j = 0; j != 3; j++) {
					spc4.f[j] = origin.f[j] - (campos->f[j] - g_BgRooms[roomnum].pos.f[j]);
				}

				s1[i * 3 + 1] = 0;
				s1[i * 3 + 2] = 0;

				tmp = roomlights[i].dirx * roomlights[i].dirx + roomlights[i].diry * roomlights[i].diry + roomlights[i].dirz * roomlights[i].dirz;
				f16 = spc4.f[0] * spc4.f[0] + spc4.f[1] * spc4.f[1] + spc4.f[2] * spc4.f[2];

				if (tmp > 0.0001f && f16 > 0.0001f) {
					sp190 = -((roomlights[i].dirx * spc4.f[0] + roomlights[i].diry * spc4.f[1] + roomlights[i].dirz * spc4.f[2]) / sqrtf(tmp * f16));

					if (sp190 > 0.4f) {
						sp190 = 0.4f;
					}

					sp190 *= 2.5f;
				} else {
					sp190 = 0.0f;
				}

				if (sp190 > 0.0f) {
					for (l = 3; l >= 0; l--) {
						spdc[l] = origin.f[0] * spf8.m[0][l] + origin.f[1] * spf8.m[1][l] + origin.f[2] * spf8.m[2][l] + spf8.m[3][l];

						if (l == 3 && spdc[l] <= 0.0f) {
							break;
						}
					}

					if (spdc[3] > 0.0001f) {
						f20 = 1.0f / spdc[3];
						x = func0f13c710(viewleft + (1.0f + spdc[0] * f20) * (viewwidth * 0.5f));
						y = func0f13c710(viewtop + (1.0f - spdc[1] * f20) * (viewheight * 0.5f));
						f0 = (spdc[2] * f20 * 511.0f + 511.0f) * 32.0f;

						if (f0 < 32576.0f) {
							brightnessfrac = 1.0f;
							tmp2 = (brightnessfrac - 1.00f);

							if (x <= 10.0f + viewleft) {
								brightnessfrac = 0.0f;
							} else if (y <= 30.0f + viewtop) {
								brightnessfrac = 0.0f;
							} else if (x >= -10.0f + viewleft + viewwidth) {
								brightnessfrac = 0.0f;
							} else if (y >= -30.0f + viewtop + viewheight) {
								brightnessfrac = 0.0f;
							}

							sp178 = 1.0f - 2.0f * tmp2;

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

							tmp3 = 32300.0f - f0;

							if (tmp3 < 0.0f) {
								tmp3 = 0.0f;
							}

							if (tmp3 > 1300.0f) {
								tmp3 = 1300.0f;
							}

							tmp3 *= 1.0f / 1300.0f;

							if (2.0f * tmp2 > 1.0f) {
								sp178 = 0.0f;
							}

							s1[i * 3 + 1] = sp190 * 255.0f * sp178;
							s1[i * 3 + 2] = brightnessfrac * tmp3 * sp190 * 64.0f * 1;
						}
					}
				}

				if (s1[i * 3 + 1] > 0) {
					for (j = 0; j < ARRAYCOUNT(roomlights[i].bbox); j++) {
						spec.x = origin.x + (roomlights[i].bbox[j].x - origin.x) * 0.6f;
						spec.y = origin.y + (roomlights[i].bbox[j].y - origin.y) * 0.6f;
						spec.z = origin.z + (roomlights[i].bbox[j].z - origin.z) * 0.6f;

						for (k = 3; k >= 0; k--) {
							spdc[k] = spec.f[0] * spf8.m[0][k] + spec.f[1] * spf8.m[1][k] + spec.f[2] * spf8.m[2][k] + spf8.m[3][k];

							if (k == 3 && spdc[k] <= 0.0f) {
								break;
							}
						}

						if (spdc[3] > 0.0f) {
							f20 = 1.0f / spdc[3];

							if (f20 > 9999.0f) {
								f20 = 9999.0f;
							}

							if (f20 < -9999.0f) {
								f20 = -9999.0f;
							}

							xi = func0f13c710(viewleft + (1.0f + spdc[0] * f20) * (viewwidth * 0.5f));
							yi = func0f13c710(viewtop + (1.0f - spdc[1] * f20) * (viewheight * 0.5f));
							f0 = (spdc[2] * f20 * 511.0f + 511.0f) * 32.0f;

							if (g_ZbufPtr1
									&& xi >= (int)viewleft
									&& xi < (int)(viewleft + viewwidth)
									&& yi >= (int)viewtop
									&& yi < (int)(viewtop + viewheight)
									&& f0 < 32576.0f) {
								index = envGetCurrent()->numsuns;
								index *= 8;
								artifact = artifacts;
								artifact += index;

								while (artifact->type != ARTIFACTTYPE_FREE) {
									index++;
									artifact++;
								}

								if (index < MAX_ARTIFACTS) {
									artifact->unk02 = artifactTestLos(&spec, &g_BgRooms[roomnum].pos, xi, yi);
									artifact->unk04 = func0f13c574(f0) >> 2;
									artifact->unk08 = &g_ZbufPtr1[viGetWidth() * yi + xi];
									artifact->light = &roomlights[i];
									artifact->type = ARTIFACTTYPE_GLARE;
									artifact->unk0c.u16_2 = xi;
									artifact->unk0c.u16_1 = yi;
								}
							}
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

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
	gDPSetTexturePersp(gdl++, G_TP_NONE);

	return gdl;
}

Gfx *artifactsUnconfigureForGlares(Gfx *gdl)
{
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	return gdl;
}

Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, int roomnum)
{
	int i;
	int j;
	int lightindex;
	struct artifact *artifacts;
	uint16_t min;
	uint16_t max;
	float lightop_cur_frac;
	int t2;
	struct light *light;
	uint8_t *s3;
	int k;
	int count;
	uint16_t t4;
	float add;
	int l;
	float brightness; // The closer you get to an artifact, the higher this becomes.
	int avg;
	float f0;
	int v1;
	int r;
	int g;
	int b;
	uint8_t colour[4];
	int16_t lightroompos[3];
	struct coord lightworldpos;
	struct coord lightscreenpos;
	float spdc[2];
	float spd4[2];
	float f24;
	bool extra;
	float f26;

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
				s3 = &var800a41a0[lightindex * 3];
				t2 = 0;
				min = 0xffff;
				max = 0;

				for (k = i; k < i + count; k++) {
					if (artifacts[k].unk04 > max) {
						max = artifacts[k].unk04;
					}

					if (artifacts[k].unk04 < min) {
						min = artifacts[k].unk04;
					}
				}

				avg = (max - min) >> 1;

				if (avg < 25) {
					avg = 25;
				}

				for (k = i; k < i + count; k++) {
					t2 += artifacts[k].unk02;

					artifacts[k].type = ARTIFACTTYPE_FREE;
				}

				s3[0] = func0f13d3c4(s3[0], t2 * 2);

				if (t2 > 0) {
					brightness = viGetFovY() * 0.017453292f;
					add = cosf(brightness) / sinf(brightness) * 14.6f;

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
						extra = false;
					} else {
						extra = true;
					}

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						s3[2] *= (int) (lightop_cur_frac * 7.0f);
					}

					f0 = s3[2] * (1.0f / 255.0f);

					skySetOverexposure((int) ((float)f0 * r), (int) ((float)f0 * g), (int) ((float)f0 * b));

					for (l = 0; l < 3; l++) {
						lightroompos[l] = (light->bbox[0].s[l] + light->bbox[1].s[l] + light->bbox[2].s[l] + light->bbox[3].s[l]) / 4;
						lightworldpos.f[l] = lightroompos[l] + g_BgRooms[roomnum].pos.f[l];
						lightscreenpos.f[l] = lightworldpos.f[l] - g_Vars.currentplayer->cam_pos.f[l];
					}

					mtx4RotateVecInPlace(camGetWorldToScreenMtxf(), &lightscreenpos);

					cam0f0b4d04(&lightscreenpos, spdc);

					brightness *= 27500.0f / (-lightscreenpos.z < 1.0f ? 1.0f : -lightscreenpos.z);

					if (light->brightnessmult != 0) {
						brightness *= light->brightnessmult * (1.0f / 32.0f);
					}

					brightness *= s3[1] * (1.0f / 255.0f);

					if (USINGDEVICE(DEVICE_NIGHTVISION)) {
						brightness *= 14.0f * lightop_cur_frac;
					}

					brightness += add;
					brightness *= 2.0f * roomGetSettledLocalBrightnessFrac(roomnum);

					if (brightness > 750.0f) {
						brightness = 750.0f;
					}

					f24 = stageGetCurrent()->light_width * brightness * 0.01f;
					f26 = stageGetCurrent()->light_height * brightness * 0.01f;

					f24 *= viGetViewWidth() * (1.0f / 240.0f) / camGetPerspAspect();
					f26 *= viGetViewHeight() * (1.0f / 240.0f);

					if (brightness > 3.0f) {
						float alpha = (light->colour & 0xf) * 17;

						colour[0] = r;
						colour[1] = g;
						colour[2] = b;

						alpha *= stageGetCurrent()->light_alpha / 255.0f;
						alpha *= (s3[1] / 255.0f);
						alpha *= (s3[0] / 8.0f);

						if (USINGDEVICE(DEVICE_NIGHTVISION)) {
							alpha *= lightop_cur_frac * 7.0f;
						}

						if (alpha > 255.0f) {
							alpha = 255.0f;
						}

						colour[3] = alpha;

						gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], colour[3]);

						spd4[0] = f24;
						spd4[1] = f26;

						textureCalcScreenCoords(&gdl, spdc, spd4, 64, 64, false, false, false, 1);

						// Make artifacts slightly brighter when true
						if (extra) {
							colour[0] = 0xff;
							colour[1] = 0xff;
							colour[2] = 0xff;
							colour[3] = stageGetCurrent()->light_alpha;
							colour[3] = s3[0] * colour[3] / 8;

							gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], colour[3]);

							spd4[0] = f24 * 0.4f;
							spd4[1] = f26 * 0.4f;

							textureCalcScreenCoords(&gdl, spdc, spd4, 64, 64, false, false, false, 1);
						}
					}
				}

				s3[1] = 0;
				s3[2] = 0;
			}

			// This is incrementing i past all the artifacts for this particular
			// light, then subtracting 1 because the for loop will add 1.
			i = i + count - 1;
		}
	}

	return gdl;
}
