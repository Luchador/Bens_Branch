#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/bondgun.h"
#include "game/gunfx.h"
#include "game/weaponutils.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/debug.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "game/utils.h"
#include "bss.h"
#include "game/mtxutils.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

#define BOLTBEAMTICKMODE_MANUAL    0
#define BOLTBEAMTICKMODE_AUTOMATIC 1

struct casing g_Casings[20];
struct boltbeam g_BoltBeams[8];
struct lasersight g_LaserSights[MAX_PLAYERS];

void beamCreate(struct beam *beam, int weaponnum, struct coord *from, struct coord *to)
{
	float distance;

	beam->from.x = from->x;
	beam->from.y = from->y;
	beam->from.z = from->z;

	beam->dir.x = to->x - from->x;
	beam->dir.y = to->y - from->y;
	beam->dir.z = to->z - from->z;

	distance = sqrtf(beam->dir.x * beam->dir.x + beam->dir.y * beam->dir.y + beam->dir.z * beam->dir.z);

	if (distance > 0) {
		beam->dir.x *= 1.0f / distance;
		beam->dir.y *= 1.0f / distance;
		beam->dir.z *= 1.0f / distance;
	}

	if (distance > 10000) {
		distance = 10000;
	}

	beam->age = 0;
	beam->weaponnum = weaponnum;
	beam->maxdist = distance;

	if (distance < 500) {
		distance = 500;
	}

	if (weaponnum == -1) {
		beam->speed = 0;
		beam->mindist = distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = 0;
	} else if (weaponnum == WEAPON_LASER || weaponnum == WEAPON_WATCHLASER) {
		beam->speed = 0.25f * distance;
		beam->mindist = 0.6f * distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = (-0.1f - RANDOMFRAC() * 0.3f) * distance;
	} else if (weaponnum == -2) {
		beam->speed = 0;
		beam->mindist = distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		beam->dist = 0;
	} else {
		float tmp;
		beam->speed = 0.2f * distance;
		beam->mindist = 0.2f * distance;

		if (beam->mindist > 3000) {
			beam->mindist = 3000;
		}

		tmp = RANDOMFRAC();
		beam->dist = (tmp + tmp - 1) * beam->speed;
	}

	if (beam->dist >= beam->maxdist) {
		beam->age = -1;
	}
}

void beamCreateForHand(int handnum)
{
	struct player *player = g_Vars.currentplayer;
	struct hand *hand = player->hands + handnum;
	Mtx *mtx = camGetPlayerWorldToScreenMtx();
	float tmp;

	tmp = hand->hitpos.x * (*mtx)[0][2] + hand->hitpos.y * (*mtx)[1][2] + hand->hitpos.z * (*mtx)[2][2] + (*mtx)[3][2];
	tmp = -tmp;

	if (tmp < hand->muzzlez) {
		// empty
	} else {
		struct beam *beam;
		int weaponnum = bgunGetWeaponNum(handnum);

		if (hand->gset.weaponnum == WEAPON_LASER && hand->gset.weaponfunc == FUNC_SECONDARY) {
			weaponnum = -2;
		}

		beam = &hand->beam;
		beamCreate(beam, weaponnum, &hand->muzzlepos, &hand->hitpos);

		if (beam->weaponnum == WEAPON_MAULER) {
			beam->weaponnum = -3 - (int)player->hands[handnum].matmot1;
		}

		if (player->prop->chr && PLAYERCOUNT() >= 2) {
			struct chrdata *chr = player->prop->chr;
			struct coord disttolast;
			struct coord disttocur;
			float radians;

			if (chr->fireslots[handnum] == -1) {
				chr->fireslots[handnum] = bgunAllocateFireslot();
			}

			if (chr->fireslots[handnum] != -1) {
				disttolast.x = hand->hitpos.x - player->chrmuzzlelastpos[handnum].x;
				disttolast.y = hand->hitpos.y - player->chrmuzzlelastpos[handnum].y;
				disttolast.z = hand->hitpos.z - player->chrmuzzlelastpos[handnum].z;

				utilsNormalizeF(&disttolast.x, &disttolast.y, &disttolast.z);

				disttocur.x = hand->hitpos.x - hand->muzzlepos.x;
				disttocur.y = hand->hitpos.y - hand->muzzlepos.y;
				disttocur.z = hand->hitpos.z - hand->muzzlepos.z;

				utilsNormalizeF(&disttocur.x, &disttocur.y, &disttocur.z);

				radians = acosf(disttolast.x * disttocur.x + disttolast.y * disttocur.y + disttolast.z * disttocur.z);

				if (!(radians > 0.08725257f) || weaponnum == -2) {
					beamCreate(&g_Fireslots[chr->fireslots[handnum]].beam, weaponnum, &player->chrmuzzlelastpos[handnum], &hand->hitpos);

					if (g_Fireslots[chr->fireslots[handnum]].beam.weaponnum == WEAPON_MAULER) {
						g_Fireslots[chr->fireslots[handnum]].beam.weaponnum = -3 - (int)player->hands[handnum].matmot1;
					}
				}
			}
		}
	}
}

Gfx *beamRenderGeneric(Gfx *gdl, struct textureconfig *texconfig,
		float arg2, struct coord *headpos, uint32_t headcolour,
		float arg5, struct coord *tailpos, uint32_t tailcolour)
{
	struct coord spe4;
	float length;
	VtxF *vertices;
	struct coord spd0;
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	Mtx *spc8;
	Col *colours = gfxAllocateColours(2);
	Mtx sp84;
	Mtx *worldtoscreenmtx = camGetPlayerWorldToScreenMtx();
	struct coord sp74 = {0, 0, 0};
	float mult;
	struct coord sp5c;

	spe4.x = tailpos->x - headpos->x;
	spe4.y = tailpos->y - headpos->y;
	spe4.z = tailpos->z - headpos->z;

	length = sqrtf(spe4.x * spe4.x + spe4.y * spe4.y + spe4.z * spe4.z);

	if (length < 0.00001f && length > -0.00001f) {
		return gdl;
	}

	spe4.x /= length;
	spe4.y /= length;
	spe4.z /= length;

	mtx4TransformVec(camGetPlayerWorldToScreenMtx(), headpos, &sp5c);

	if (sp5c.x * arg2 > 10000.0f || sp5c.x * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.y * arg2 > 10000.0f || sp5c.y * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.z * arg2 > 10000.0f || sp5c.z * arg2 < -10000.0f) {
		return gdl;
	}

	mtx4TransformVec(camGetPlayerWorldToScreenMtx(), tailpos, &sp5c);

	if (sp5c.x * arg2 > 10000.0f || sp5c.x * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.y * arg2 > 10000.0f || sp5c.y * arg2 < -10000.0f) {
		return gdl;
	}

	if (sp5c.z * arg2 > 10000.0f || sp5c.z * arg2 < -10000.0f) {
		return gdl;
	}

	colours[0].word = PD_BE32(headcolour);
	colours[1].word = PD_BE32(tailcolour);

	spd0.x = (spe4.y * (campos->z - (headpos->z + length * spe4.z))) - (spe4.z * (campos->y - (headpos->y + length * spe4.y)));
	spd0.y = (spe4.z * (campos->x - (headpos->x + length * spe4.x))) - (spe4.x * (campos->z - (headpos->z + length * spe4.z)));
	spd0.z = (spe4.x * (campos->y - (headpos->y + length * spe4.y))) - (spe4.y * (campos->x - (headpos->x + length * spe4.x)));

	if (spd0.x != 0.0f || spd0.y != 0.0f || spd0.z != 0.0f) {
		utilsNormalizeF(&spd0.x, &spd0.y, &spd0.z);
	} else {
		spd0.x = 0.0f;
		spd0.y = 1.0f;
		spd0.z = 0.0f;
	}

	vertices = gfxAllocateVerticesF(4);
	spc8 = gfxAllocateMatrix();

	mtx4LoadTranslation(headpos, &sp84);

	mtxScaleRotationPart(1.0f / arg2, &sp84);
	mtxApplyAffineTransformInPlace(worldtoscreenmtx, &sp84);
	mtx4Copy(&sp84, spc8);

	mult = arg5 * arg2;

	sp74.x = spe4.x * (length * arg2);
	sp74.y = spe4.y * (length * arg2);
	sp74.z = spe4.z * (length * arg2);

	vertices[0].x = spd0.x * mult;
	vertices[0].y = spd0.y * mult;
	vertices[0].z = spd0.z * mult;
	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[0].colour = 0;

	vertices[1].x = -spd0.x * mult;
	vertices[1].y = -spd0.y * mult;
	vertices[1].z = -spd0.z * mult;
	vertices[1].s = texconfig->width * 32; // TODOF
	vertices[1].t = 0;
	vertices[1].colour = 0;

	vertices[2].x = sp74.x - spd0.x * mult;
	vertices[2].y = sp74.y - spd0.y * mult;
	vertices[2].z = sp74.z - spd0.z * mult;
	vertices[2].s = texconfig->width * 32;// TODOF
	vertices[2].t = texconfig->height * 32;// TODOF
	vertices[2].colour = 4;

	vertices[3].x = sp74.x + spd0.x * mult;
	vertices[3].y = sp74.y + spd0.y * mult;
	vertices[3].z = sp74.z + spd0.z * mult;
	vertices[3].s = 0;
	vertices[3].t = texconfig->height * 32;// TODOF
	vertices[3].colour = 4;

	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BACK);
	gfx_Matrix(gdl++, spc8, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gfx_Color(gdl++, colours, 2);

	texSelect(&gdl, texconfig, 4, 1, 2, true, NULL);

	gfx_VertexF(gdl++, vertices, 4, 0);
	gfx_Tri2(gdl++, 0, 1, 2, 2, 3, 0);

	return gdl;
}

Gfx *beamRender(Gfx *gdl, struct beam *beam, bool arg2, uint8_t arg3)
{
	Mtx *sp188;
	Mtx sp148;

	if (arg3 < 5 && beam->age >= 0) {
		Col *colours = gfxAllocateColours(1);
		struct coord sp138;
		struct coord *campos = &g_Vars.currentplayer->cam_pos;
		float sp130;
		float sp12c = beam->mindist;
		VtxF *vertices;
		float sp124 = beam->dist;
		struct coord sp118;
		struct coord sp10c;
		struct coord sp100 = {0, 0, 0};
		struct coord spf4 = {0, 0, 0};
		float spf0 = 1.4142f;
		struct textureconfig *texconfig = &g_TexBeamConfigs[arg3];
		int i;
		Mtx *worldtoscreenmtx = camGetPlayerWorldToScreenMtx();
		int j;
		int spd8;
		struct coord spcc;
		float tmp;
		float spc0[2];
		float spb8[2];
		float f14;
		float f16;
		float f18;
		float spa8;
		float spa4;

		switch (beam->weaponnum) {
		case WEAPON_CYCLONE:
			texconfig = &g_TexBeamConfigs[1];
			break;
		case WEAPON_TRANQUILIZER:
			texconfig = &g_TexBeamConfigs[3];
			break;
		case WEAPON_MAULER:
		case WEAPON_PHOENIX:
		case WEAPON_CALLISTO:
		case WEAPON_REAPER:
		case WEAPON_FARSIGHT:
			texconfig = &g_TexBeamConfigs[4];
			break;
		}

		if (beam->weaponnum == -1 || beam->weaponnum == WEAPON_CYCLONE) {
			colours[0].word = PD_BE32(0xffffff7f);
		} else {
			colours[0].word = 0xffffffff;
		}

		if (beam->weaponnum == WEAPON_LASER) {
			// Laser primary
			sp130 = 50.0f;
			texconfig = &g_TexLaserConfigs[0];
		} else if (beam->weaponnum == -2) {
			// Laser secondary
			sp130 = 10.0f;
			texconfig = &g_TexLaserConfigs[0];

			colours[0].a = 150 + (rngRandom() % 50);

			if ((rngRandom() % 5) == 0) {
				colours[0].r = colours[0].g = 255 - (rngRandom() % 100);
			}
		} else {
			sp130 = 30.0f;
		}

		if (beam->weaponnum <= -3) {
			// Mauler
			sp130 = sp130 * ((beam->weaponnum + 3) * 2.0f + 1.0f);
			texconfig = &g_TexBeamConfigs[4];
		}

		sp138.x = beam->from.x;
		sp138.y = beam->from.y;
		sp138.z = beam->from.z;

		if (sp124 > 0.0f) {
			sp138.x += sp124 * beam->dir.x;
			sp138.y += sp124 * beam->dir.y;
			sp138.z += sp124 * beam->dir.z;
		} else {
			sp12c += sp124;
			sp124 = 0.0f;
		}

		if (sp124 + sp12c > beam->maxdist) {
			sp12c = beam->maxdist - sp124;
		}

		sp10c.x = (beam->dir.y * (campos->z - (sp138.z + sp12c * beam->dir.z))) - (beam->dir.z * (campos->y - (sp138.y + sp12c * beam->dir.y)));
		sp10c.y = (beam->dir.z * (campos->x - (sp138.x + sp12c * beam->dir.x))) - (beam->dir.x * (campos->z - (sp138.z + sp12c * beam->dir.z)));
		sp10c.z = (beam->dir.x * (campos->y - (sp138.y + sp12c * beam->dir.y))) - (beam->dir.y * (campos->x - (sp138.x + sp12c * beam->dir.x)));

		if (sp10c.x != 0.0f || sp10c.y != 0.0f || sp10c.z != 0.0f) {
			utilsNormalizeF(&sp10c.x, &sp10c.y, &sp10c.z);

			sp10c.x *= sp130;
			sp10c.y *= sp130;
			sp10c.z *= sp130;
		} else {
			sp10c.x = 0.0f;
			sp10c.y = sp130;
			sp10c.z = 0.0f;
		}

		sp118.x = beam->dir.y * sp10c.z - beam->dir.z * sp10c.y;
		sp118.y = beam->dir.z * sp10c.x - beam->dir.x * sp10c.z;
		sp118.z = beam->dir.x * sp10c.y - beam->dir.y * sp10c.x;

		utilsNormalizeF(&sp118.x, &sp118.y, &sp118.z);

		sp118.x *= sp130;
		sp118.y *= sp130;
		sp118.z *= sp130;

		if (beam->weaponnum == WEAPON_LASER) {
			vertices = gfxAllocateVerticesF(8);
		} else {
			vertices = gfxAllocateVerticesF(4);
		}

		sp188 = gfxAllocateMatrix();

		if (sp12c > 0.0f
				&& sp138.x > -32000.0f && sp138.x < 32000.0f
				&& sp138.y > -32000.0f && sp138.y < 32000.0f
				&& sp138.z > -32000.0f && sp138.z < 32000.0f) {
			spd8 = true;
			mtx4LoadTranslation(&sp138, &sp148);
			mtxScaleRotationPart(0.1f, &sp148);
			mtxApplyAffineTransformInPlace(worldtoscreenmtx, &sp148);

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (sp148[i][j] < -32000.0f || sp148[i][j] > 32000.0f) {
						spd8 = false;
						break;
					}
				}
			}

			if (spd8) {
				mtx4Copy(&sp148, sp188);

				if (beam->weaponnum == -2 && PLAYERCOUNT() == 1) {
					spcc.x = sp138.x + beam->dir.x * sp12c;
					spcc.y = sp138.y + beam->dir.y * sp12c;
					spcc.z = sp138.z + beam->dir.z * sp12c;

					mtx4TransformVecInPlace(worldtoscreenmtx, &spcc);

					spb8[0] = spb8[1] = sp130 / 10;
					tmp = -spcc.z;

					camScaleViewToScreen(spb8, tmp, spc0);

					if (spc0[0] < 2) {
						spcc.x *= spc0[0] * 0.5f;
						spcc.y *= spc0[0] * 0.5f;
						spcc.z *= spc0[0] * 0.5f;
					}

					mtx4TransformVecInPlace(camGetProjectionMtx(), &spcc);

					spcc.x -= sp138.x;
					spcc.y -= sp138.y;
					spcc.z -= sp138.z;

					sp100.x = spcc.x * 10;
					sp100.y = spcc.y * 10;
					sp100.z = spcc.z * 10;
				} else {
					sp100.x = beam->dir.x * (sp12c * 10);
					sp100.y = beam->dir.y * (sp12c * 10);
					sp100.z = beam->dir.z * (sp12c * 10);
				}

				if (sp100.x > -30000.0f && sp100.x < 30000.0f
						&& sp100.y > -30000.0f && sp100.y < 30000.0f
						&& sp100.z > -30000.0f && sp100.z < 30000.0f) {
					vertices[0].x = sp10c.x;
					vertices[0].y = sp10c.y;
					vertices[0].z = sp10c.z;
					vertices[0].s = texconfig->width * 32.0f; // TODOF
					vertices[0].t = 0;
					vertices[0].colour = 0;

					vertices[1].x = -sp10c.x;
					vertices[1].y = -sp10c.y;
					vertices[1].z = -sp10c.z;
					vertices[1].s = 0;
					vertices[1].t = 0;
					vertices[1].colour = 0;

					vertices[2].x = sp100.x + sp10c.x * 0.9f;
					vertices[2].y = sp100.y + sp10c.y * 0.9f;
					vertices[2].z = sp100.z + sp10c.z * 0.9f;
					vertices[2].s = texconfig->width * 32; // TODOF
					vertices[2].t = texconfig->height * 32; // TODOF
					vertices[2].colour = 0;

					vertices[3].x = sp100.x - sp10c.x * 0.9f;
					vertices[3].y = sp100.y - sp10c.y * 0.9f;
					vertices[3].z = sp100.z - sp10c.z * 0.9f;
					vertices[3].s = 0;
					vertices[3].t = texconfig->height * 32; // TODOF
					vertices[3].colour = 0;

					if (beam->weaponnum == WEAPON_LASER) {
						f14 = campos->x - sp138.x;
						f16 = campos->y - sp138.y;
						f18 = campos->z - sp138.z;

						spa8 = f14 * f14 + f16 * f16 + f18 * f18;

						f14 = campos->x - (sp138.x + beam->dir.x * sp12c);
						f16 = campos->y - (sp138.y + beam->dir.y * sp12c);
						f18 = campos->z - (sp138.z + beam->dir.z * sp12c);

						spa4 = f14 * f14 + f16 * f16 + f18 * f18;

						if (spa4 < spa8) {
							spf4.x = sp100.x;
							spf4.y = sp100.y;
							spf4.z = sp100.z;
							spf0 *= 0.9f;
						}

						vertices[4].x = spf4.x + sp118.x * spf0;
						vertices[4].y = spf4.y + sp118.y * spf0;
						vertices[4].z = spf4.z + sp118.z * spf0;
						vertices[4].s = g_TexGroup03Configs[0].width * 32; // TODOF
						vertices[4].t = g_TexGroup03Configs[0].height * 32; // TODOF
						vertices[4].colour = 0;

						vertices[5].x = spf4.x - sp118.x * spf0;
						vertices[5].y = spf4.y - sp118.y * spf0;
						vertices[5].z = spf4.z - sp118.z * spf0;
						vertices[5].s = 0;
						vertices[5].t = 0;
						vertices[5].colour = 0;

						vertices[6].x = spf4.x + sp10c.x * spf0;
						vertices[6].y = spf4.y + sp10c.y * spf0;
						vertices[6].z = spf4.z + sp10c.z * spf0;
						vertices[6].s = 0;
						vertices[6].t = g_TexGroup03Configs[0].height * 32; // TODOF
						vertices[6].colour = 0;

						vertices[7].x = spf4.x - sp10c.x * spf0;
						vertices[7].y = spf4.y - sp10c.y * spf0;
						vertices[7].z = spf4.z - sp10c.z * spf0;
						vertices[7].s = g_TexGroup03Configs[0].width * 32; // TODOF
						vertices[7].t = 0;
						vertices[7].colour = 0;
					}

					gfx_Clear_Geometry_Mode(gdl++, G_CULL_BACK);
					gfx_Matrix(gdl++, sp188, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
					gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
					gfx_Set_Render_Mode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
					gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
					gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
					gfx_Set_Combine_LERP(gdl++,
						G_CCMUX_ENVIRONMENT, G_CCMUX_SHADE, G_CCMUX_TEXEL0, G_CCMUX_SHADE,     // Color cycle 0
						G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0,                   // Alpha cycle 0
						G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0,                            // Color cycle 1 (unused)
						G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0);                           // Alpha cycle 1 (unused)
					gfx_Color(gdl++, colours, 1);

					if (beam->weaponnum == WEAPON_LASER) {
						texSelect(&gdl, &g_TexGroup03Configs[0], 4, arg2, 2, true, NULL);

						gfx_VertexF(gdl++, vertices, 8, 0);
						gfx_Tri2(gdl++, 4, 5, 6, 4, 5, 7);

						texSelect(&gdl, texconfig, 4, arg2, 2, true, NULL);

						gfx_Tri2(gdl++, 0, 2, 3, 0, 3, 1);
					} else {
						texSelect(&gdl, texconfig, 4, arg2, 2, true, NULL);

						gfx_VertexF(gdl++, vertices, 4, 0);
						gfx_Tri2(gdl++, 0, 2, 3, 0, 3, 1);
					}
				}
			}
		}
	}

	return gdl;
}

void beamTick(struct beam *beam)
{
	if (beam->age >= 0) {
		if (beam->weaponnum == -2) {
			beam->age++;

			if (beam->age > 1) {
				beam->age = -1;
			}
		} else {
			if (g_Vars.lvupdate240 <= 8) {
				// Not lagging
				beam->dist += beam->speed * g_Vars.lvupdate60f;
			} else {
				// Lagging
				beam->dist += beam->speed * (2 + RANDOMFRAC() * 0.5f);
			}

			if (beam->dist >= beam->maxdist) {
				beam->age = -1;
			}
		}
	}
}

bool g_CasingsActive = false;

struct casing *casingCreate(struct modeldef *modeldef, Mtx *mtx)
{
	float rot[3][3];
	struct casing *casing = g_Casings;
	struct casing *end = g_Casings + ARRAYCOUNT(g_Casings);

	while (casing < end && casing->modeldef != NULL) {
		casing++;
	}

	if (casing < end) {
		casing->modeldef = modeldef;
		casing->pos.x = (*mtx)[3][0];
		casing->pos.y = (*mtx)[3][1];
		casing->pos.z = (*mtx)[3][2];

		mtx4ToMtx3(mtx, rot);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				casing->rot[i][j] = rot[i][j] * 4096.0f;
			}
		}

		g_CasingsActive = true;

		return casing;
	}

	return NULL;
}

void casingCreateForHand(int handnum, float ground, Mtx *mtx)
{
	float oldyspeed;
	struct casing *casing = NULL;
	struct player *player = g_Vars.currentplayer;
	Mtx spec;
	float spc8[3][3];
	int weaponnum = bgunGetWeaponNum(handnum);
	int casingtype = -1;
	struct weaponfunc *func = gsetGetWeaponFunction2(&player->hands[handnum].gset);
	struct weapon *weapondef = weaponFindById(player->gunctrl.weaponnum);
	struct weaponfunc_shoot *shootfunc = NULL;
	struct modeldef *modeldef;

	if ((func->type & 0xff) == INVENTORYFUNCTYPE_SHOOT) {
		shootfunc = (struct weaponfunc_shoot *)func;
	}

	if (func->ammoindex < 0) {
		return;
	}

	if (weapondef != NULL && shootfunc != NULL) {
		if (weapondef->ammos[func->ammoindex] != NULL) {
			casingtype = weapondef->ammos[func->ammoindex]->casingeject;
		}
	}

	if (casingtype < 0) {
		return;
	}

	mtx4Copy(mtx, &spec);

	modeldef = bgunGetCartModeldef();

	if (modeldef != NULL) {
		casing = casingCreate(modeldef, &spec);
	}

	if (casing != NULL) {
		struct coord spa4 = {0, 0, 0};
		Mtx sp64;
		uint32_t magic = 0x15aca6;
		uint32_t sp5c;
		uint32_t sp4c;
		float newyspeed;
		float f0;

		casing->ground = ground;

		if (weaponnum == WEAPON_PP9I || weaponnum == WEAPON_CC13
				|| weaponnum == WEAPON_FALCON2 || weaponnum == WEAPON_MAGSEC4) {
			casing->speed.x = -(RANDOMFRAC() * 0.5333333f * 0.0625f + 0.5333333f);
			casing->speed.y = RANDOMFRAC() * 2.5f * 0.0625f + 2.5f;
			casing->speed.z = 0.0f;

			mtx4RotateVecInPlace(mtx, &casing->speed);

			spa4.x = 2.0f * RANDOMFRAC() * M_TAU * 0.0625f - 0.39263657f;
			spa4.y = 2.0f * RANDOMFRAC() * M_TAU * 0.0625f - 0.39263657f;
			spa4.z = 2.0f * RANDOMFRAC() * M_TAU * 0.0625f - 0.39263657f;

			mtx4LoadRotation(&spa4, &sp64);
			mtx4ToMtx3(&sp64, spc8);

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					casing->rotspeed[i][j] = spc8[i][j] * 4096.0f;
				}
			}

			sp5c = ((int)((rngRandom() >> 24) * magic) >> 10) + magic;
			f0 = (rngRandom() % sp5c) / 781250;

			newyspeed = casing->speed.y - f0 * 0.2777778f;

			casing->pos.y += f0 * (casing->speed.y + newyspeed) * 0.5f;
			casing->pos.x += f0 * casing->speed.x;
			casing->pos.z += f0 * casing->speed.z;

			casing->speed.y = newyspeed;

			if (g_Vars.lvupdate240 > 0) {
				casing->speed.x += (player->hands[handnum].posmtx[3][0] - player->hands[handnum].prevmtx[3][0]) / g_Vars.lvupdate60freal;
				casing->speed.y += (player->hands[handnum].posmtx[3][1] - player->hands[handnum].prevmtx[3][1]) / g_Vars.lvupdate60freal;
				casing->speed.z += (player->hands[handnum].posmtx[3][2] - player->hands[handnum].prevmtx[3][2]) / g_Vars.lvupdate60freal;
			}
		} else {
			if (weaponnum == WEAPON_REAPER) {
				casing->speed.x = -(RANDOMFRAC() * 0.41666666f * 0.125f + 0.41666666f);
				casing->speed.y = RANDOMFRAC() * 3.3333333f * 0.125f + 3.3333333f;
			} else {
				casing->speed.x = -((RANDOMFRAC() * 1.4166666f * 0.125f) + 1.4166666f);
				casing->speed.y = RANDOMFRAC() * 1.6666666f * 0.125f + 1.6666666f;
			}

			casing->speed.z = 0.0f;

			if (weaponnum == WEAPON_DY357MAGNUM || weaponnum == WEAPON_DY357LX) {
				casing->speed.x = 0.0f;
				casing->speed.y = 0.0f;
				casing->speed.z = -1.0f;
			}

			mtx4RotateVecInPlace(mtx, &casing->speed);

			if (weaponnum == WEAPON_REAPER) {
				spa4.x = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;
				spa4.y = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;
				spa4.z = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;

				mtx4LoadRotation(&spa4, &sp64);
				mtx4RotateVecInPlace(&sp64, &casing->speed);
			}

			spa4.x = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;
			spa4.y = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;
			spa4.z = 2.0f * RANDOMFRAC() * M_TAU * 0.015625f - 0.09815914f;

			mtx4LoadRotation(&spa4, &sp64);
			mtx4ToMtx3(&sp64, spc8);

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					casing->rotspeed[i][j] = spc8[i][j] * 4096.0f;
				}
			}

			sp4c = ((int) ((rngRandom() >> 24) * magic) >> 10) + magic;
			f0 = (rngRandom() % sp4c) / 781250;

			newyspeed = casing->speed.y - f0 * 0.2777778f;

			casing->pos.y += f0 * (casing->speed.y + newyspeed) * 0.5f;
			casing->pos.x += f0 * casing->speed.x;
			casing->pos.z += f0 * casing->speed.z;

			casing->speed.y = newyspeed;

			if (g_Vars.lvupdate240 > 0) {
				casing->speed.x += (player->hands[handnum].posmtx[3][0] - player->hands[handnum].prevmtx[3][0]) / g_Vars.lvupdate60freal;
				casing->speed.y += (player->hands[handnum].posmtx[3][1] - player->hands[handnum].prevmtx[3][1]) / g_Vars.lvupdate60freal;
				casing->speed.z += (player->hands[handnum].posmtx[3][2] - player->hands[handnum].prevmtx[3][2]) / g_Vars.lvupdate60freal;
			}
		}
	}
}

void casingRender(struct casing *casing, Gfx **gdlptr)
{
	Gfx *gdl = *gdlptr;
	struct modeldef *modeldef = casing->modeldef;
	Mtx *matrices = gfxAllocate(modeldef->nummatrices * sizeof(Mtx));
	struct model model;
	struct modelrenderdata renderdata = { NULL, true, 3 };
	Mtx mtx;
	bool render = true;

	modelAllocateRwData(modeldef);
	modelInit(&model, modeldef, NULL, true);

	model.matrices = matrices;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mtx[i][j] = casing->rot[i][j] * (1.0f / 4096.0f);
		}
	}

	mtx[3][0] = 0.0f;
	mtx[3][1] = 0.0f;
	mtx[3][2] = 0.0f;

	mtx[0][3] = 0.0f;
	mtx[1][3] = 0.0f;
	mtx[2][3] = 0.0f;
	mtx[3][3] = 1.0f;

	mtxScaleRotationPart(0.10f, &mtx);
	mtx4SetTranslation(&casing->pos, &mtx);
	mtxApplyAffineTransform(camGetPlayerWorldToScreenMtx(), &mtx, (Mtx*)model.matrices);

	// Check if any coordinate is out of range
	for (int i = 0; i < 3; i++) {
		if (model.matrices[0][3][i] > 30000) {
			render = false;
		} else if (model.matrices[0][3][i] < -30000) {
			render = false;
		}
	}

	if (render) {
		renderdata.zbufferenabled = 1;
		renderdata.gdl = gdl;
		renderdata.unk10 = matrices;
		renderdata.unk30 = 4;
		renderdata.envcolour = g_Vars.currentplayer->gunshadecol[0] << 24
			| g_Vars.currentplayer->gunshadecol[1] << 16
			| g_Vars.currentplayer->gunshadecol[2] << 8
			| g_Vars.currentplayer->gunshadecol[3];

		modelRender(&renderdata, &model);

		*gdlptr = renderdata.gdl;
	}
}

void casingsRender(Gfx **gdlptr)
{
	if (g_CasingsActive) {
		struct casing *end = g_Casings + ARRAYCOUNT(g_Casings);
		struct casing *casing = g_Casings;

		while (casing < end) {
			if (casing->modeldef) {
				casingRender(casing, gdlptr);
			}

			casing++;
		}
	}
}

int boltbeamFindByProp(struct prop *prop)
{
	int result = -1;
	int i = 0;

	for (; i < 8 && result == -1; i++) {
		if (g_BoltBeams[i].unk00_prop == prop) {
			result = i;
		}
	}

	return result;
}

int boltbeamCreate(struct prop *prop)
{
	int beamnum = boltbeamFindByProp((struct prop *) -1);

	if (beamnum >= 0) {
		g_BoltBeams[beamnum].tickmode = BOLTBEAMTICKMODE_MANUAL;
		g_BoltBeams[beamnum].unk00_prop = prop;
	}

	return beamnum;
}

void boltbeamFree(struct prop *prop)
{
	int beamnum = boltbeamFindByProp(prop);

	if (beamnum != -1) {
		g_BoltBeams[beamnum].unk00 = -1;
	}
}

void boltbeamSetHeadPos(int beamnum, struct coord *pos)
{
	g_BoltBeams[beamnum].headpos.x = pos->x;
	g_BoltBeams[beamnum].headpos.y = pos->y;
	g_BoltBeams[beamnum].headpos.z = pos->z;
}

void boltbeamSetTailPos(int beamnum, struct coord *pos)
{
	g_BoltBeams[beamnum].tailpos.x = pos->x;
	g_BoltBeams[beamnum].tailpos.y = pos->y;
	g_BoltBeams[beamnum].tailpos.z = pos->z;
}

void boltbeamIncrementHeadPos(int beamnum, float arg1, bool arg2)
{
	float dist;

#define DIFF(i) (g_BoltBeams[beamnum].tailpos.f[i] - g_BoltBeams[beamnum].headpos.f[i])
#define SQDIFF(i) (DIFF(i) * DIFF(i))

	dist = sqrtf(SQDIFF(0) + SQDIFF(1) + SQDIFF(2));

	if (dist > arg1 && !arg2) {
		float tmp[3];

		if (1);

		tmp[0] = (g_BoltBeams[beamnum].headpos.x - g_BoltBeams[beamnum].tailpos.x) / dist;
		tmp[1] = (g_BoltBeams[beamnum].headpos.y - g_BoltBeams[beamnum].tailpos.y) / dist;
		tmp[2] = (g_BoltBeams[beamnum].headpos.z - g_BoltBeams[beamnum].tailpos.z) / dist;

		g_BoltBeams[beamnum].headpos.x = g_BoltBeams[beamnum].tailpos.x + tmp[0] * arg1;
		g_BoltBeams[beamnum].headpos.y = g_BoltBeams[beamnum].tailpos.y + tmp[1] * arg1;
		g_BoltBeams[beamnum].headpos.z = g_BoltBeams[beamnum].tailpos.z + tmp[2] * arg1;
	}
}

void boltbeamSetAutomatic(int beamnum, float speed)
{
	g_BoltBeams[beamnum].tickmode = BOLTBEAMTICKMODE_AUTOMATIC;
	g_BoltBeams[beamnum].unk00 = 0;
	g_BoltBeams[beamnum].speed = speed;
}

Gfx *boltbeamsRender(Gfx *gdl)
{
	for (int i = 0; i < ARRAYCOUNT(g_BoltBeams); i++) {
		if (g_BoltBeams[i].unk00 != -1) {
			gdl = beamRenderGeneric(gdl, g_TexLaserConfigs, 1, &g_BoltBeams[i].headpos, 0xafafff00, 2, &g_BoltBeams[i].tailpos, 0xafafff7f);
		}
	}

	return gdl;
}

void boltbeamsTick(void)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_BoltBeams); i++) {
		if (g_BoltBeams[i].unk00 != -1 && g_BoltBeams[i].tickmode == BOLTBEAMTICKMODE_AUTOMATIC) {
			float length = sqrtf(
					(g_BoltBeams[i].tailpos.x - g_BoltBeams[i].headpos.x) * (g_BoltBeams[i].tailpos.x - g_BoltBeams[i].headpos.x) +
					(g_BoltBeams[i].tailpos.y - g_BoltBeams[i].headpos.y) * (g_BoltBeams[i].tailpos.y - g_BoltBeams[i].headpos.y) +
					(g_BoltBeams[i].tailpos.z - g_BoltBeams[i].headpos.z) * (g_BoltBeams[i].tailpos.z - g_BoltBeams[i].headpos.z));

			length -= g_BoltBeams[i].speed * LVUPDATE60FREAL() / 60.0f;

			if (length < 0) {
				g_BoltBeams[i].unk00 = -1;
			} else {
				boltbeamIncrementHeadPos(i, length, false);
			}
		}
	}
}

/**
 * Return true if a lasersight with the given ID exists, or false if not.
 *
 * Additionally, populate the index pointer with the index of the lasersight
 * if it exists, or any free slot if it doesn't.
 */
bool lasersightExists(int id, int *index)
{
	int fallback = -1;
	int exact = -1;
	int i = 0;

	for (; i < 4 && exact == -1; i++) {
		if (g_LaserSights[i].id == id) {
			exact = i;
		}

		if (g_LaserSights[i].id == -1) {
			fallback = i;
		}
	}

	if (exact == -1) {
		*index = fallback;
		return false;
	}

	*index = exact;
	return true;
}

Gfx *lasersightRenderDot(Gfx *gdl)
{
	Mtx *mtx;
	float f0;
	float f20;
	struct player *player = g_Vars.currentplayer;
	Mtx sp1b0;
	struct coord campos;
	Mtx sp164;
	Mtx sp124;
	int i;

	static uint32_t sp1 = 800;
	static uint32_t sp2 = 7000 * 3;
	static uint32_t sp3 = 9000 * 3;
	static uint32_t spb = 24;
	static uint32_t spi = 6;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_ENVIRONMENT, G_CCMUX_SHADE, G_CCMUX_TEXEL0, G_CCMUX_SHADE,     // Color cycle 0
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0,                   // Alpha cycle 0
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0,                            // Color cycle 1 (unused)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0);                           // Alpha cycle 1 (unused)

	mtxIdent(&sp164);
	mtxApplyAffineTransformInPlace(camGetPlayerWorldToScreenMtx(), &sp164);
	mtxIdent(&sp124);
	mtxApplyAffineTransformInPlace(camGetProjectionMtx(), &sp124);

	sp124[3][0] = sp124[3][1] = sp124[3][2] = 0.0f;

	mtxIdent(&sp1b0);
	mtxApplyAffineTransformInPlace(camGetPlayerWorldToScreenMtx(), &sp1b0);

	campos.x = player->cam_pos.x;
	campos.y = player->cam_pos.y;
	campos.z = player->cam_pos.z;

	sp1b0[3][0] = 0.0f;
	sp1b0[3][1] = 0.0f;
	sp1b0[3][2] = 0.0f;

	mtxScale3x4(0.2f, &sp1b0);

	mtx = gfxAllocateMatrix();
	mtx4Copy(&sp1b0, mtx);

	gfx_Matrix(gdl++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	for (i = 0; i < ARRAYCOUNT(g_LaserSights); i++) {
		if (g_LaserSights[i].id != -1) {
			struct coord pos;
			struct coord rot;
			Col *colours;
			VtxF *vertices;

			pos.x = g_LaserSights[i].dotpos.x;
			pos.y = g_LaserSights[i].dotpos.y;
			pos.z = g_LaserSights[i].dotpos.z;

			rot.x = g_LaserSights[i].dotrot.x;
			rot.y = g_LaserSights[i].dotrot.y;
			rot.z = g_LaserSights[i].dotrot.z;

			colours = gfxAllocateColours(2);

			colours[0].word = PD_BE32(0xff00005f);
			colours[1].word = PD_BE32(0xff00000f);

			gfx_Color(gdl++, colours, 2);

			if (g_LaserSights[i].unk28 > 0.0f) {
				if (!(g_LaserSights[i].dotpos.x < 0.0000001f) || !(g_LaserSights[i].dotpos.x > -0.000001f)
						|| !(g_LaserSights[i].dotpos.y < 0.0000001f) || !(g_LaserSights[i].dotpos.y > -0.000001f)
						|| !(g_LaserSights[i].dotpos.z < 0.0000001f) || !(g_LaserSights[i].dotpos.z > -0.000001f)) {
					float spcc;
					float spc8;
					float spc4;
					float f22;
					float f24;
					float f26;
					float f28;
					float f30;
					float f2;
					float f00;

					f20 = spi;

					pos.x = (pos.x - campos.x) * 5.0f;
					pos.y = (pos.y - campos.y) * 5.0f;
					pos.z = (pos.z - campos.z) * 5.0f;

					f0 = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);

					spcc = sp1;

					if (f0 > spcc) {
						spc8 = sp2;

						if (f0 > spc8) {
							spc4 = sp3;

							if (f0 > spc4) {
								f20 = 0.1f;
							} else {
								float tmp1 = spb + f20;
								float tmp2 = (f0 - spc8) / (spc4 - spc8);
								f20 = tmp1 - tmp1 * tmp2;
							}
						} else {
							f20 = f20 + (f0 - spcc) * (spb / (spc8 - spcc));
						}
					}

					texSelect(&gdl, &g_TexGeneralConfigs[4], 4, 0, 2, true, NULL);

					if (rot.x == 0.0f && rot.z == 0.0f) {
						spcc = 0.0f;
						spc8 = 0.0f;
						spc4 = 1.0f;
						f22 = 1.0f;
						f24 = 0.0f;
						f00 = 0;
					} else {
						float f0 = sqrtf(rot.x * rot.x + rot.y * rot.y + rot.z * rot.z);

						f26 = rot.x / f0;
						f30 = rot.y / f0;
						f28 = rot.z / f0;

						f0 = sqrtf(f26 * f26 + f28 * f28);
						f2 = f26 / f0;

						f22 = f28 / f0;
						spcc = f30 * f2;
						spc8 = -f0;
						spc4 = f30 * f22;
						f24 = -f2;
						f00 = 0;
					}

					vertices = gfxAllocateVerticesF(4);

					vertices[3].colour = 0;
					vertices[2].colour = 0;
					vertices[1].colour = 0;
					vertices[0].colour = 0;

					vertices[0].s = 0;
					vertices[0].t = 0;
					vertices[1].s = 512; // TODOF
					vertices[1].t = 0;
					vertices[2].s = 512; // TODOF
					vertices[2].t = 512; // TODOF
					vertices[3].s = 0;
					vertices[3].t = 512; // TODOF

					vertices[0].x = pos.x + (-f20 * f22) + (f20 * spcc);
					vertices[0].y = pos.y + (-f20 * f00) + (f20 * spc8);
					vertices[0].z = pos.z + (-f20 * f24) + (f20 * spc4);

					vertices[1].x = pos.x + (f20 * f22) + (f20 * spcc);
					vertices[1].y = pos.y + (f20 * f00) + (f20 * spc8);
					vertices[1].z = pos.z + (f20 * f24) + (f20 * spc4);

					vertices[2].x = pos.x + (f20 * f22) + (-f20 * spcc);
					vertices[2].y = pos.y + (f20 * f00) + (-f20 * spc8);
					vertices[2].z = pos.z + (f20 * f24) + (-f20 * spc4);

					vertices[3].x = pos.x + (-f20 * f22) + (-f20 * spcc);
					vertices[3].y = pos.y + (-f20 * f00) + (-f20 * spc8);
					vertices[3].z = pos.z + (-f20 * f24) + (-f20 * spc4);

					gfx_VertexF(gdl++, vertices, 4, 0);

					gfx_Tri2(gdl++, 0, 1, 2, 2, 3, 0);
				}
			}
		}
	}

	return gdl;
}

Gfx *lasersightRenderBeam(Gfx *gdl)
{
	struct player *player = g_Vars.currentplayer;
	Mtx *mtx;
	int i;
	Mtx sp198;
	struct coord campos;
	Mtx sp14c;
	Mtx sp10c;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_ENVIRONMENT, G_CCMUX_SHADE, G_CCMUX_TEXEL0, G_CCMUX_SHADE,     // Color cycle 0
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0,                   // Alpha cycle 0
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0,                            // Color cycle 1 (unused)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0);                           // Alpha cycle 1 (unused)
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, &g_TexGeneralConfigs[3], 4, 0, 2, 1, NULL);
	mtxIdent(&sp14c);

	mtxApplyAffineTransformInPlace(camGetPlayerWorldToScreenMtx(), &sp14c);
	mtxIdent(&sp10c);
	mtxApplyAffineTransformInPlace(camGetProjectionMtx(), &sp10c);

	sp10c[3][1] = 0;
	sp10c[3][0] = 0;
	sp10c[3][2] = 0;

	mtxIdent(&sp198);
	mtxApplyAffineTransformInPlace(camGetPlayerWorldToScreenMtx(), &sp198);

	campos.x = player->cam_pos.x;
	campos.y = player->cam_pos.y;
	campos.z = player->cam_pos.z;

	sp198[3][0] = 0;
	sp198[3][1] = 0;
	sp198[3][2] = 0;

	mtxScale3x4(0.2f, &sp198);
	mtx = gfxAllocateMatrix();
	mtx4Copy(&sp198, mtx);

	gfx_Matrix(gdl++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	for (i = 0; i < ARRAYCOUNT(g_LaserSights); i++) {
		if (g_LaserSights[i].id != -1) {
			Col *colours;
			struct coord spcc;
			struct coord spc0;
			struct coord spb4;
			struct coord spa8;
			VtxF *vertices;
			struct coord sp98;

			sp98.x = g_LaserSights[i].beamnear.x;
			sp98.y = g_LaserSights[i].beamnear.y;
			sp98.z = g_LaserSights[i].beamnear.z;

			mtx4TransformVecInPlace(&sp14c, &sp98);

			spa8.x = sp98.x < 0.0f ? 1.0f : -1.0f;
			spa8.y = 2.0f;
			spa8.z = 0.0f;

			utilsNormalizeF(&spa8.x, &spa8.y, &spa8.z);

			mtx4RotateVecInPlace(&sp10c, &spa8);

			spcc.x = g_LaserSights[i].beamnear.x;
			spcc.y = g_LaserSights[i].beamnear.y;
			spcc.z = g_LaserSights[i].beamnear.z;

			spcc.x = (spcc.x - campos.x) * 5.0f;
			spcc.y = (spcc.y - campos.y) * 5.0f;
			spcc.z = (spcc.z - campos.z) * 5.0f;

			spc0.x = g_LaserSights[i].beamfar.x;
			spc0.y = g_LaserSights[i].beamfar.y;
			spc0.z = g_LaserSights[i].beamfar.z;

			spc0.x = (spc0.x - campos.x) * 5.0f;
			spc0.y = (spc0.y - campos.y) * 5.0f;
			spc0.z = (spc0.z - campos.z) * 5.0f;

			spb4.x = spc0.x - spcc.x;
			spb4.y = spc0.y - spcc.y;
			spb4.z = spc0.z - spcc.z;

			utilsNormalizeF(&spb4.x, &spb4.y, &spb4.z);

			colours = gfxAllocateColours(2);

			colours[0].word = PD_BE32(0xff00005f);
			colours[1].word = PD_BE32(0xff00000f);

			gfx_Color(gdl++, colours, 2);

			vertices = gfxAllocateVerticesF(6);

			vertices[0].colour = 0;
			vertices[1].colour = 0;
			vertices[2].colour = 0;
			vertices[3].colour = 0;
			vertices[4].colour = 4;
			vertices[5].colour = 4;

			vertices[0].s = 0;
			vertices[0].t = 0;
			vertices[1].s = 0;
			vertices[1].t = 256; // TODOF
			vertices[2].s = 32; // TODOF
			vertices[2].t = 0;
			vertices[3].s = 32; // TODOF
			vertices[3].t = 256; // TODOF
			vertices[4].s = 0;
			vertices[4].t = 0;
			vertices[5].s = 0;
			vertices[5].t = 256; // TODOF

			vertices[0].x = spcc.x - spa8.x * 15.0f;
			vertices[0].y = spcc.y - spa8.y * 15.0f;
			vertices[0].z = spcc.z - spa8.z * 15.0f;

			vertices[1].x = spcc.x + spa8.x * 15.0f;
			vertices[1].y = spcc.y + spa8.y * 15.0f;
			vertices[1].z = spcc.z + spa8.z * 15.0f;

			vertices[2].x = spcc.x + (200 * spb4.x) - (spa8.x * 15.0f);
			vertices[2].y = spcc.y + (200 * spb4.y) - (spa8.y * 15.0f);
			vertices[2].z = spcc.z + (200 * spb4.z) - (spa8.z * 15.0f);

			vertices[3].x = spcc.x + (200 * spb4.x) + (spa8.x * 15.0f);
			vertices[3].y = spcc.y + (200 * spb4.y) + (spa8.y * 15.0f);
			vertices[3].z = spcc.z + (200 * spb4.z) + (spa8.z * 15.0f);

			vertices[4].x = spcc.x + (400 * spb4.x) - (spa8.x * 15.0f);
			vertices[4].y = spcc.y + (400 * spb4.y) - (spa8.y * 15.0f);
			vertices[4].z = spcc.z + (400 * spb4.z) - (spa8.z * 15.0f);

			vertices[5].x = spcc.x + (400 * spb4.x) + (spa8.x * 15.0f);
			vertices[5].y = spcc.y + (400 * spb4.y) + (spa8.y * 15.0f);
			vertices[5].z = spcc.z + (400 * spb4.z) + (spa8.z * 15.0f);

			gfx_VertexF(gdl++, vertices, 6, 0);

			gfx_Tri4(gdl++, 0, 1, 2, 2, 3, 1, 2, 3, 5, 2, 5, 4);
		}
	}

	return gdl;
}

void lasersightSetBeam(int id, int arg1, struct coord *near, struct coord *far)
{
	int i;

	if (!lasersightExists(id, &i)) {
		if (i == -1) {
			return;
		}

		g_LaserSights[i].id = id;
	}

	g_LaserSights[i].beamnear.x = near->x;
	g_LaserSights[i].beamnear.y = near->y;
	g_LaserSights[i].beamnear.z = near->z;

	g_LaserSights[i].beamfar.x = far->x;
	g_LaserSights[i].beamfar.y = far->y;
	g_LaserSights[i].beamfar.z = far->z;

	g_LaserSights[i].unk44 = arg1;
	g_LaserSights[i].unk28 = 0;
}

void lasersightSetDot(int arg0, struct coord *pos, struct coord *rot)
{
	int i;

	if (lasersightExists(arg0, &i)) {
		g_LaserSights[i].unk28 += 1.0f;

		g_LaserSights[i].dotpos.x = pos->x;
		g_LaserSights[i].dotpos.y = pos->y;
		g_LaserSights[i].dotpos.z = pos->z;

		g_LaserSights[i].dotrot.x = rot->x;
		g_LaserSights[i].dotrot.y = rot->y;
		g_LaserSights[i].dotrot.z = rot->z;
	}
}

void lasersightFree(int arg0)
{
	int i;

	if (lasersightExists(arg0, &i)) {
		g_LaserSights[i].id = -1;
	}
}
