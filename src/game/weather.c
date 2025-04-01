#include <ultra64.h>
#include <math.h>
#include <stdint.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/dlights.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/gfxmemory.h"
#include "game/sparks.h"
#include "game/utils.h"
#include "game/weather.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/lv.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"

struct weatherdata *g_WeatherData = NULL;

struct weathercfg g_WeatherConfig[WEATHERCFG_MAX_STAGES] = {
	{
		STAGE_G5BUILDING,
		WEATHERFLAG_CUTSCENE_ONLY,
		20.f,
		-800.f, 800.f,
		0.f,
	},
	{
		STAGE_CHICAGO,
		0,
		20.f,
		-800.f, 800.f,
		0.f,
		{
			//Weatherproof rooms
			ROOM_PETE_0062,
			ROOM_PETE_0061,
			ROOM_PETE_000F,
			ROOM_PETE_0052,
			ROOM_PETE_0045,
			ROOM_PETE_0044,
			ROOM_PETE_0043,
			ROOM_PETE_003D,
			ROOM_PETE_000E,
			ROOM_PETE_000D,
			ROOM_PETE_000C,
			ROOM_PETE_000B,
			ROOM_PETE_000A,
			ROOM_PETE_0009,
			ROOM_PETE_0008,
			ROOM_PETE_0007,
			ROOM_PETE_0006,
			ROOM_PETE_0005,
			ROOM_PETE_0004,
			ROOM_PETE_0037,
			ROOM_PETE_005B,
			ROOM_PETE_005F,
			ROOM_PETE_0060,
			ROOM_PETE_005D,
			ROOM_PETE_001D,
			ROOM_PETE_001F,
			ROOM_PETE_0027,
			ROOM_PETE_0050,
			ROOM_PETE_002C,
			ROOM_PETE_002F,
			ROOM_PETE_0030,
			ROOM_PETE_0011,
			ROOM_PETE_0024,
			ROOM_PETE_0033,
			ROOM_PETE_0034,
			ROOM_PETE_0035,
			ROOM_PETE_0036,
			ROOM_PETE_003C,
			ROOM_PETE_0046,
			ROOM_PETE_0047,
			ROOM_PETE_0049,
			ROOM_PETE_0055,
			ROOM_PETE_0056,
			ROOM_PETE_005E,
			ROOM_PETE_0063,
			ROOM_PETE_0069,
			ROOM_PETE_0003,
			ROOM_PETE_0016,
			ROOM_PETE_004F,
			ROOM_PETE_004B,
			ROOM_PETE_003F,
			ROOM_PETE_0013,
			ROOM_PETE_0019,
			0
		}
	},
	{
		STAGE_AIRBASE,
		WEATHERFLAG_INCLUDE | WEATHERFLAG_FORCE_WINDDIR,
		5.f,
		-800.f, 800.f,
		-2000.f,
		{
			//Rooms that have snow
			ROOM_CAVE_0088,
			ROOM_CAVE_0091,
			ROOM_CAVE_0087,
			ROOM_CAVE_0090,
			ROOM_CAVE_0086,
			ROOM_CAVE_0080,
			ROOM_CAVE_0083,
			ROOM_CAVE_0084,
			ROOM_CAVE_0085,
			ROOM_CAVE_007F,
			ROOM_CAVE_007E,
			ROOM_CAVE_0082,
			ROOM_CAVE_008F,
			ROOM_CAVE_008E,
			ROOM_CAVE_007B,
			ROOM_CAVE_007C,
			ROOM_CAVE_007D,
			ROOM_CAVE_0081,
			ROOM_CAVE_006F,
			0
		},
		1.5707963705063f,
		0.f,
		-5.f
	},
	{
		//Weatherproof rooms
		STAGE_CRASHSITE,
		0,
		10.f,
		-500.f, 500.f,
		0.f,
		{
			ROOM_AZT_001F,
			ROOM_AZT_0020,
			ROOM_AZT_0021,
			ROOM_AZT_0022,
			ROOM_AZT_0023,
			ROOM_AZT_0024,
			ROOM_AZT_0051,
			ROOM_AZT_0052,
			ROOM_AZT_0053,
			ROOM_AZT_0054,
			ROOM_AZT_0055,
			ROOM_AZT_0056,
			ROOM_AZT_0057,
			ROOM_AZT_0058,
			ROOM_AZT_0059,
			ROOM_AZT_005A,
			ROOM_AZT_005B,
			ROOM_AZT_005C,
			ROOM_AZT_005D,
			ROOM_AZT_005E,
			ROOM_AZT_005F,
			ROOM_AZT_0060,
			ROOM_AZT_0061,
			ROOM_AZT_0062,
			ROOM_AZT_0063,
			ROOM_AZT_0064,
			ROOM_AZT_002D,
			ROOM_AZT_0040,
			ROOM_AZT_0041,
			ROOM_AZT_0042,
			ROOM_AZT_0043,
			ROOM_AZT_0044,
			ROOM_AZT_0045,
			ROOM_AZT_0046,
			ROOM_AZT_0047,
			ROOM_AZT_0048,
			ROOM_AZT_0049,
			ROOM_AZT_004A,
			ROOM_AZT_004B,
			ROOM_AZT_004C,
			ROOM_AZT_004D,
			ROOM_AZT_004E,
			ROOM_AZT_004F,
			ROOM_AZT_0050,
			0
		}
	},
};

const struct weathercfg g_DefaultWeatherConfig = {
	0,
	0,
	20.f,
	-800.f, 800.f,
	0.f
};

const struct weathercfg *g_CurWeatherConfig = &g_DefaultWeatherConfig;

Gfx *weatherRender(Gfx *gdl)
{
	struct weatherdata *weather;

	if (!g_WeatherData) {
		return gdl;
	}
	if (g_CurWeatherConfig->zmax && g_Vars.currentplayer->cam_pos.z < g_CurWeatherConfig->zmax) {
		return gdl;
	}

	if ((g_CurWeatherConfig->flags & WEATHERFLAG_CUTSCENE_ONLY) && g_Vars.tickmode != TICKMODE_CUTSCENE) {
		return gdl;
	}

	weather = g_WeatherData;

	gSPDisplayList(gdl++, &var800613a0);
	gSPDisplayList(gdl++, &var80061380);

	if (weather->type == WEATHERTYPE_SNOW) {
		texSelect(&gdl, &g_TexGeneralConfigs[1], 2, 1, 2, 1, NULL);

		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetColorDither(gdl++, G_CD_NOISE);
		gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetTextureLOD(gdl++, G_TL_TILE);
		gDPSetTextureConvert(gdl++, G_TC_FILT);
		gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
		gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
		gDPSetAlphaDither(gdl++, G_AD_NOISE);
	}

	switch (weather->type) {
	case WEATHERTYPE_RAIN:
		gdl = weatherRenderRain(gdl, weather, 0);
		break;
	case WEATHERTYPE_SNOW:
		gdl = weatherRenderSnow(gdl, weather, 0);
		break;
	}

	return gdl;
}

void weatherSetBoundaries(struct weatherparticledata *data, int index, float min, float max)
{
	((float *)(&data->boundarymin))[index] = min;
	((float *)(&data->boundarymax))[index] = max;
	((float *)(&data->boundaryrange))[index] = fabsf(min) + fabsf(max);
}

struct weatherparticledata *weatherAllocateParticles(void)
{
	struct weatherparticledata *data = mempAlloc(sizeof(struct weatherparticledata), MEMPOOL_STAGE);
	uint32_t i;

	data->unk3e80.x = 0;
	data->unk3e80.y = 0;
	data->unk3e80.z = 0;

	weatherSetBoundaries(data, 0, -800, 800);
	weatherSetBoundaries(data, 1, g_CurWeatherConfig->ymin, g_CurWeatherConfig->ymax);
	weatherSetBoundaries(data, 2, -800, 800);

	i = 0;

	while (i != ARRAYCOUNT(g_WeatherData->particledata[0]->unk3ec8)) {
		data->unk3ec8[i++] = 0;
	}

	for (i = 0; i != (int)ARRAYCOUNT(g_WeatherData->particledata[0]->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];
		particle->pos.x = RANDOMFRAC() * 1600 - 800;
		particle->pos.y = RANDOMFRAC() * 1600 - 800;
		particle->pos.z = RANDOMFRAC() * 1600 - 800;

		particle->inc.x = RANDOMFRAC() * 10 - 5;
		particle->inc.y = -10 - RANDOMFRAC() * 40;
		particle->inc.z = RANDOMFRAC() * 10 - 5;

		particle->horizspeed = RANDOMFRAC() + 0.7f;

		particle->active = false;
	}

	return data;
}

void weatherRollLightning(struct weatherdata *weather)
{
	weather->unk94 = 0;
	weather->unk98 = (rngRandom() & 7) + 1;
	weather->unk9c = (rngRandom() & 7) + 1;
	weather->unka0 = (rngRandom() & 7) + 1;
	weather->unka4 = (rngRandom() & 0xf) + 10;
}

void func0f131678(int arg0)
{
	int i;

	for (i = 0; i < arg0 + 1; i++) {
		if ((g_WeatherData->unk58[i].unk08 > 0 && g_WeatherData->unk58[i].unk04 > 0.0f)
				|| (g_WeatherData->unk58[i].unk08 < 1 && g_WeatherData->unk58[i].unk00 > 0)) {
			g_WeatherData->unk58[i].unk04 = 0.0f;
			g_WeatherData->unk58[i].unk08 = 100;

			if (g_WeatherData->unk58[i].unk08 < 0) {
				g_WeatherData->unk58[i].unk08 = -g_WeatherData->unk58[i].unk08;
			}
		}
	}
}

void weatherSetIntensity(int intensity)
{
	int dotheloop = -1;
	int special = -1;
	int i;

	if (intensity == g_WeatherData->intensity) {
		return;
	}

	switch (g_WeatherData->type) {
	case WEATHERTYPE_RAIN:
		if (g_WeatherData->intensity == 0) {
			weatherRollLightning(g_WeatherData);
		}

		special = -1;
		dotheloop = 1;

		switch (intensity) {
		//case 0: no lightning
		case 0:
			g_WeatherData->numdesiredparticles = 0;
			g_WeatherData->rddesiredlength = 100;
			g_WeatherData->rdtransitiontime = 100;

			if (g_WeatherData->rdtransitiontime < 0) {
				g_WeatherData->rdtransitiontime = -g_WeatherData->rdtransitiontime;
			}

			dotheloop = -1;
			g_WeatherData->lightningchance = -1;
			break;
		case 1:
			g_WeatherData->raindropfallspeed = 15;
			g_WeatherData->numdesiredparticles = 200;
			g_WeatherData->rddesiredlength = 100;
			g_WeatherData->rdtransitiontime = 100;

			if (g_WeatherData->rdtransitiontime < 0) {
				g_WeatherData->rdtransitiontime = -g_WeatherData->rdtransitiontime;
			}

			special = 0;
			g_WeatherData->lightningchance = 0.001f;
			break;
		case 2:
			g_WeatherData->raindropfallspeed = 18;
			g_WeatherData->numdesiredparticles = 400;
			g_WeatherData->rddesiredlength = 150;
			g_WeatherData->rdtransitiontime = 100;

			if (g_WeatherData->rdtransitiontime < 0) {
				g_WeatherData->rdtransitiontime = -g_WeatherData->rdtransitiontime;
			}

			special = 1;
			g_WeatherData->lightningchance = 0.01f;
			break;
		case 3:
			g_WeatherData->raindropfallspeed = 30;
			g_WeatherData->numdesiredparticles = 500;
			g_WeatherData->rddesiredlength = 300;
			g_WeatherData->rdtransitiontime = 100;

			if (g_WeatherData->rdtransitiontime < 0) {
				g_WeatherData->rdtransitiontime = -g_WeatherData->rdtransitiontime;
			}

			special = 2;
			g_WeatherData->lightningchance = 0.03f;
			break;
		}
		break;
	case WEATHERTYPE_SNOW:
		switch (intensity) {
		case 0:
			g_WeatherData->numdesiredparticles = 0;
			break;
		case 1:
			g_WeatherData->numdesiredparticles = 500;
			break;
		}
		break;
	}

	if (dotheloop >= 0) {
		for (i = 0; i != 3; i++) {
			if (i != special) {
				g_WeatherData->unk58[i].unk04 = 0;
				g_WeatherData->unk58[i].unk08 = 100;

				if (g_WeatherData->unk58[i].unk08 < 0) {
					g_WeatherData->unk58[i].unk08 = -g_WeatherData->unk58[i].unk08;
				}
			}
		}
	}

	if (special >= 0) {
		g_WeatherData->unk58[special].unk04 = 1;
		g_WeatherData->unk58[special].unk08 = 100;

		if (g_WeatherData->unk58[special].unk08 < 0) {
			g_WeatherData->unk58[special].unk08 = -g_WeatherData->unk58[special].unk08;
		}
	}

	g_WeatherData->intensity = intensity;
}

uint32_t g_RainSpeedExtra;
uint32_t g_SnowSpeed;
uint32_t g_SnowSpeedExtra;

void weatherTickRain(struct weatherdata *weather)
{
	int lVar6 = 0;
	int relativetotal = 0; // eg. -10 if deleted 10 particles, +10 if created 10
	struct weatherparticledata *data;
	int i;
	int iVar10;
	float rand;
	int lvupdate;

	if (weather->sndtransitiontime > 0) {
		weather->sndcurrentvolume += (weather->snddesiredvolume - weather->sndcurrentvolume) / weather->sndtransitiontime;
	}

	weather->sndtransitiontime--;

	if (weather->unk58[0].unk08 > 0) {
		weather->unk58[0].unk00 += (weather->unk58[0].unk04 - weather->unk58[0].unk00) / weather->unk58[0].unk08; //when unk00 is 0 no rain sound plays
		weather->unk58[0].unk08--;
	}

	if (weather->unk58[1].unk08 > 0) {
		weather->unk58[1].unk00 += (weather->unk58[1].unk04 - weather->unk58[1].unk00) / weather->unk58[1].unk08;
		weather->unk58[1].unk08--;
	}

	if (weather->unk58[2].unk08 > 0) {
		weather->unk58[2].unk00 += (weather->unk58[2].unk04 - weather->unk58[2].unk00) / weather->unk58[2].unk08;
		weather->unk58[2].unk08--;
	}

	// Rain noise
	for (i = 0; i != 4; i++) {
		int sounds[] = {
			0x80b7,
			0x80b6,
			0x80b8,
			-1,
		};

		iVar10 = weather->unk58[i].unk00 * 32767.0f * weather->sndcurrentvolume;

		if (lvIsPaused()) {
			 iVar10 = 0;
		}

		if (iVar10 > 0) {
			if (weather->audiohandles[i] == 0 && sounds[i] >= 0) {
				weather->rainsfxindex = sounds[i];
				sndStart(var80095200, weather->rainsfxindex, &weather->audiohandles[i], -1,
						-1, -1, -1, -1);
			}

			if (weather->audiohandles[i] != 0) {
				if (sndGetState(weather->audiohandles[i]) != AL_STOPPED) {
					sndAdjust(&weather->audiohandles[i], 0, iVar10 * 3 / 4, -1,
							weather->rainsfxindex, 1, 1, -1, 1);
				}
			}
		} else {
			if (weather->audiohandles[i] != 0) {
				audioStop(weather->audiohandles[i]);
			}
		}
	}

	if (weather->rdtransitiontime > 0) {
		lvupdate = g_Vars.lvupdate60;

		if (weather->rdtransitiontime < lvupdate) {
			weather->rdcurrentlength = weather->rddesiredlength;
		} else {
			weather->rdcurrentlength += lvupdate * ((weather->rddesiredlength - weather->rdcurrentlength) / weather->rdtransitiontime);
			weather->rdcurrentlength = weather->rddesiredlength;
			weather->rdtransitiontime -= lvupdate;
		}
	}

	if (g_CurWeatherConfig->flags & WEATHERFLAG_FORCE_WINDDIR) {
		// force weather direction if requested
		weather->windanglerad = g_CurWeatherConfig->windanglerad;
		weather->windspeedz = g_CurWeatherConfig->windspeedz;
		weather->windspeedx = g_CurWeatherConfig->windspeedx;
	}
	else if (weather->windangletransitiontime > 0) {
		int lvupdate = g_Vars.lvupdate60;

		if (weather->windangletransitiontime < lvupdate) {
			weather->windanglerad = weather->newwindangle;
		} else {
			weather->windanglerad += lvupdate * ((weather->newwindangle - weather->windanglerad) / weather->windangletransitiontime);
			weather->windangletransitiontime -= lvupdate;
		}

		if (weather->windanglerad > M_TAU) {
			weather->windanglerad = 0;
		}

		weather->windspeedx = cosf(weather->windanglerad) * weather->windspeed;
		weather->windspeedz = sinf(weather->windanglerad) * weather->windspeed;
		lVar6 = 1;
	} else if (RANDOMFRAC() > 0.99f) {
		rand = RANDOMFRAC();

		weather->newwindangle = (rand + rand) * M_PI;
		weather->windangletransitiontime = (weather->newwindangle - weather->windanglerad) / 0.01f;

		if (weather->windangletransitiontime < 0) {
			weather->windangletransitiontime = -weather->windangletransitiontime;
		}
	}

	data = g_WeatherData->particledata[0];

	for (i = 0; i != ARRAYCOUNT(data->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];

		particle->pos.x += particle->inc.x * LVUPDATE60FREAL();
		particle->pos.y += particle->inc.y * LVUPDATE60FREAL();
		particle->pos.z += particle->inc.z * LVUPDATE60FREAL();

		if (particle->pos.y < data->boundarymin.y) {
			lVar6 = 2;

			// Reset particle
			particle->pos.x = data->boundarymin.x + RANDOMFRAC() * (fabsf(data->boundarymin.x) + fabsf(data->boundarymax.x));
			particle->pos.z = data->boundarymin.z + RANDOMFRAC() * (fabsf(data->boundarymin.z) + fabsf(data->boundarymax.z));

			particle->horizspeed = RANDOMFRAC() + 0.7f;

			particle->inc.y = -(RANDOMFRAC() * g_RainSpeedExtra + weather->raindropfallspeed);

			if (abs(relativetotal) < 2 && weather->numcurrentsnowflakes != weather->numdesiredparticles) {
				if (weather->numcurrentsnowflakes < weather->numdesiredparticles) {
					if ((particle->active & 3) == 0) {
						particle->active = true;
						relativetotal++;
						weather->numcurrentsnowflakes++;
					}
				} else {
					if (particle->active & 3) {
						particle->active = false;
						relativetotal--;
						weather->numcurrentsnowflakes--;
					}
				}
			}
		}

		if (lVar6 > 0) {
			particle->inc.x = weather->windspeedx * particle->horizspeed;
			particle->inc.z = weather->windspeedz * particle->horizspeed;
		}
	}

	if (weather->intensity == 0 && weather->numcurrentsnowflakes < 100) {
		func0f131678(3);
	}
}

uint32_t g_RainSpeedExtra = 20;
uint32_t g_SnowSpeed = 15;
uint32_t g_SnowSpeedExtra = 10;

void weatherTickSnow(struct weatherdata *weather)
{
	int lVar7 = 0;
	int relativetotal = 0; // eg. -10 if deleted 10 particles, +10 if created 10
	float rand;
	int lvupdate;
	int i;
	struct weatherparticledata *data;

	if (g_CurWeatherConfig->flags & WEATHERFLAG_FORCE_WINDDIR) {
		// force weather direction if requested
		weather->windanglerad = g_CurWeatherConfig->windanglerad;
		weather->windspeedz = g_CurWeatherConfig->windspeedz;
		weather->windspeedx = g_CurWeatherConfig->windspeedx;
	}

	else if (weather->windangletransitiontime > 0) {
		int lvupdate = g_Vars.lvupdate60;

		if (weather->windangletransitiontime < lvupdate) {
			weather->windanglerad = weather->newwindangle;
		} else {
			weather->windanglerad += lvupdate * ((weather->newwindangle - weather->windanglerad) / weather->windangletransitiontime);
			weather->windangletransitiontime -= lvupdate;
		}

		if (weather->windanglerad > M_TAU) {
			weather->windanglerad = 0;
		}

		weather->windspeedx = cosf(weather->windanglerad) * weather->windspeed;
		weather->windspeedz = sinf(weather->windanglerad) * weather->windspeed;
		lVar7 = 1;
	} else if (RANDOMFRAC() > 0.99f) {
		rand = RANDOMFRAC();

		weather->newwindangle = (rand + rand) * M_PI;
		weather->windangletransitiontime = (weather->newwindangle - weather->windanglerad) / 0.01f;

		if (weather->windangletransitiontime < 0) {
			weather->windangletransitiontime = -weather->windangletransitiontime;
		}
	}

	data = g_WeatherData->particledata[0];

	// 0
	data->unk3ec8[0] += 0.04f * LVUPDATE60FREAL();

	if (data->unk3ec8[0] < 0) {
		data->unk3ec8[0] += M_TAU;
	}

	if (data->unk3ec8[0] > M_TAU) {
		data->unk3ec8[0] -= M_TAU;
	}

	// 1
	data->unk3ec8[1] += -0.03f * LVUPDATE60FREAL();

	if (data->unk3ec8[1] < 0) {
		data->unk3ec8[1] += M_TAU;
	}

	if (data->unk3ec8[1] > M_TAU) {
		data->unk3ec8[1] -= M_TAU;
	}

	// 2
	data->unk3ec8[2] += 0.04f * LVUPDATE60FREAL();

	if (data->unk3ec8[2] < 0) {
		data->unk3ec8[2] += M_TAU;
	}

	if (data->unk3ec8[2] > M_TAU) {
		data->unk3ec8[2] -= M_TAU;
	}

	// 3
	data->unk3ec8[3] += 0.03f * LVUPDATE60FREAL();

	if (data->unk3ec8[3] < 0) {
		data->unk3ec8[3] += M_TAU;
	}

	if (data->unk3ec8[3] > M_TAU) {
		data->unk3ec8[3] -= M_TAU;
	}

	// 4
	data->unk3ec8[4] += 0.02f * LVUPDATE60FREAL();

	if (data->unk3ec8[4] < 0) {
		data->unk3ec8[4] += M_TAU;
	}

	if (data->unk3ec8[4] > M_TAU) {
		data->unk3ec8[4] -= M_TAU;
	}

	// 5
	data->unk3ec8[5] += 0.01f * LVUPDATE60FREAL();

	if (data->unk3ec8[5] < 0) {
		data->unk3ec8[5] += M_TAU;
	}

	if (data->unk3ec8[5] > M_TAU) {
		data->unk3ec8[5] -= M_TAU;
	}

	// 6
	data->unk3ec8[6] += -0.01f * LVUPDATE60FREAL();

	if (data->unk3ec8[6] < 0) {
		data->unk3ec8[6] += M_TAU;
	}

	if (data->unk3ec8[6] > M_TAU) {
		data->unk3ec8[6] -= M_TAU;
	}

	// 7
	data->unk3ec8[7] += -0.02f * LVUPDATE60FREAL();

	if (data->unk3ec8[7] < 0) {
		data->unk3ec8[7] += M_TAU;
	}

	if (data->unk3ec8[7] > M_TAU) {
		data->unk3ec8[7] -= M_TAU;
	}

	for (i = 0; i < ARRAYCOUNT(data->particles); i++) {
		struct weatherparticle *particle = &data->particles[i];

		particle->pos.x += particle->inc.x * LVUPDATE60FREAL();
		particle->pos.y += particle->inc.y * LVUPDATE60FREAL();
		particle->pos.z += particle->inc.z * LVUPDATE60FREAL();

		if (particle->pos.y < data->boundarymin.y) {
			lVar7 = 2;

			particle->pos.x = data->boundarymin.f[0] + RANDOMFRAC() * (fabsf(data->boundarymin.f[0]) + fabsf(data->boundarymax.f[0]));
			particle->pos.z = data->boundarymin.f[2] + RANDOMFRAC() * (fabsf(data->boundarymin.f[2]) + fabsf(data->boundarymax.f[2]));

			particle->horizspeed = RANDOMFRAC() + 0.7f;

			particle->inc.y = -(g_SnowSpeed / 10.0f) - (RANDOMFRAC() * g_SnowSpeedExtra) / 10.0f;
			particle->inc.x = weather->windspeedx * particle->horizspeed;
			particle->inc.z = weather->windspeedz * particle->horizspeed;

			if (abs(relativetotal) < 20 && weather->numcurrentsnowflakes != weather->numdesiredparticles) {
				if (weather->numcurrentsnowflakes < weather->numdesiredparticles) {
					if ((particle->active & 3) == 0) {
						particle->active = true;
						relativetotal++;
						weather->numcurrentsnowflakes++;
					}
				} else if (particle->active & 3) {
					particle->active = false;
					relativetotal--;
					weather->numcurrentsnowflakes--;
				}
			}
		}

		if (lVar7 > 0) {
			particle->inc.x = weather->windspeedx * particle->horizspeed;
			particle->inc.z = weather->windspeedz * particle->horizspeed;
		}
	}
}

void weatherConfigureRain(uint32_t intensity)
{
	if (g_WeatherData) {
		g_WeatherData->type = WEATHERTYPE_RAIN;
		weatherSetIntensity(intensity);
	}
}

void weatherConfigureSnow(uint32_t intensity)
{
	if (g_WeatherData) {
		g_WeatherData->type = WEATHERTYPE_SNOW;
		weatherSetIntensity(intensity);
	}
}

bool weatherIsRoomWeatherProof(int room)
{
	if (room >= 0 && room < g_Vars.roomcount) {
		// check room's extra_flags
		if (g_Rooms[room].extra_flags & ROOMFLAG_EX_WEATHERPROOF) {
			// this room has no weather
			return true;
		}
	}

	return false;
}

Gfx *weatherRenderRain(Gfx *gdl, struct weatherdata *weather, int arg2)
{
	int numtestrooms;
	int p;
	int i;
	int timings1[10];
	struct weatherparticledata *particledata;
	Mtxf *mtx;
	struct weatherparticle *particle;
	int timings2[8];
	int numsparksavailable;
	int testrooms[50];
	float f0;
	struct coord spca8;
	struct coord spc9c;
	struct coord spc90;
	int brightness;
	int soundnum;
	float scale;
	int badrooms[50];
	struct coord badbbmin[50];
	struct coord badbbmax[50];
	int numbadrooms;
	int bboxes[50][6];

	static uint32_t rainwidth = 1;
	static uint32_t raincol1 = 0xaaaaaa1f;
	static uint32_t raincol2 = 0x11111844;
	static uint32_t rainout = 50;
	static uint32_t cddiv = 2500;
	static uint32_t wetclip = 1;
	static uint32_t bounder = 1;
	static uint32_t trypitch = 22000;

	numsparksavailable = 1;
	numtestrooms = 0;
	numbadrooms = 0;

	if (g_Vars.lvupdate240 <= 0) {
		numsparksavailable = 0;
	}

	utilsGetCount();

	for (i = 0; i < ARRAYCOUNT(timings1); i++) {
		timings1[i] = 0;
	}

	texSelect(&gdl, &g_TexGeneralConfigs[1], 2, 1, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0);

	{
		struct coord campos;
		struct coord sp224;
		int numneighbours;
		float sp218[2];
		float sp214;
		Vtx *vertices;
		int n;
		bool ok;
		Mtxf worldtoscreenmtx;
		struct coord positions[4];
		int numtris;
		float cddiv2;
		float rainout2;
		float f2;
		float frac;
		int volume;
		int t;
		int j;
		float pitch;
		Col *colours;
		float tmp;
		struct coord distcamtobbmax;
		struct coord distcamtobbmin;
#ifdef AVOID_UB
		RoomNum neighbours[21];
#else
		RoomNum neighbours[20];
#endif

		particledata = weather->particledata[arg2];
		numtris = 0;

		mtx4LoadIdentity(&worldtoscreenmtx);
		mtx00015be0(camGetWorldToScreenMtxf(), &worldtoscreenmtx);

		worldtoscreenmtx.m[3][0] = 0.0f;
		worldtoscreenmtx.m[3][1] = 0.0f;
		worldtoscreenmtx.m[3][2] = 0.0f;

		mtx = gfxAllocateMatrix();

		mtxF2L(&worldtoscreenmtx, mtx);

		gSPMatrix(gdl++, (uintptr_t)(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

		campos.f[0] = g_Vars.currentplayer->cam_pos.f[0];
		campos.f[1] = g_Vars.currentplayer->cam_pos.f[1];
		campos.f[2] = g_Vars.currentplayer->cam_pos.f[2];

		sp224.f[0] = campos.f[0] - particledata->unk3e80.f[0];
		sp224.f[1] = campos.f[1] - particledata->unk3e80.f[1];
		sp224.f[2] = campos.f[2] - particledata->unk3e80.f[2];

		if (fabsf(sp224.f[0]) > fabsf(particledata->boundarymin.f[0]) + fabsf(particledata->boundarymax.f[0])
				|| fabsf(sp224.f[1]) > fabsf(particledata->boundarymin.f[1]) + fabsf(particledata->boundarymax.f[1])
				|| fabsf(sp224.f[2]) > fabsf(particledata->boundarymin.f[2]) + fabsf(particledata->boundarymax.f[2])) {
			sp224.f[0] = particledata->boundaryrange.f[0] / 2.0f;
			sp224.f[1] = particledata->boundaryrange.f[1] / 2.0f;
			sp224.f[2] = particledata->boundaryrange.f[2] / 2.0f;
		}

		for (p = 0; p < ARRAYCOUNT(particledata->particles); p++) {
			particle = &particledata->particles[p];

			// x
			f0 = particle->pos.f[0] - particledata->boundarymin.f[0] - sp224.f[0];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[0];
			}

			if (f0 > particledata->boundaryrange.f[0]) {
				f0 -= particledata->boundaryrange.f[0];
			}

			particle->pos.f[0] = particledata->boundarymin.f[0] + f0;

			// y
			f0 = particle->pos.f[1] - particledata->boundarymin.f[1] - sp224.f[1];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[1];
			}

			if (f0 > particledata->boundaryrange.f[1]) {
				f0 -= particledata->boundaryrange.f[1];
			}

			particle->pos.f[1] = particledata->boundarymin.f[1] + f0;

			// z
			f0 = particle->pos.f[2] - particledata->boundarymin.f[2] - sp224.f[2];

			if (f0 < 0.0f) {
				f0 += particledata->boundaryrange.f[2];
			}

			if (f0 > particledata->boundaryrange.f[2]) {
				f0 -= particledata->boundaryrange.f[2];
			}

			particle->pos.f[2] = particledata->boundarymin.f[2] + f0;
		}

		particledata->unk3e80.f[0] = campos.f[0];
		particledata->unk3e80.f[1] = campos.f[1];
		particledata->unk3e80.f[2] = campos.f[2];

		// Increase or decrease rain volume depending on whether the camera
		// is in a weatherproof room or not.
		if (weatherIsRoomWeatherProof(g_Vars.currentplayer->cam_room)) {
			if (weather->sndcurrentvolume > 0.99f) {
				weather->snddesiredvolume = 0.65f;
				weather->sndtransitiontime = 9;
			}
		} else {
			if (weather->sndcurrentvolume < 0.66f) {
				weather->snddesiredvolume = 1.0f;
				weather->sndtransitiontime = 7;
			}
		}

		// Update thunder and lightning
		if (g_Vars.lvupdate240 > 0) {
			g_SkyLightningActive = false;

			if (weather->unk94 < 0) {
				if (RANDOMFRAC() < weather->lightningchance) {
					weatherRollLightning(weather);
				}
			} else {
				if (weather->unk98 - 1 == weather->unk94
						|| weather->unk9c - 1 == weather->unk94
						|| weather->unka0 - 1 == weather->unk94) {
					g_SkyLightningActive = true;
				}

				if (weather->unk98 == weather->unk94
						|| weather->unk9c == weather->unk94
						|| weather->unka0 == weather->unk94) {
					brightness = 150;

					if (weather->unk9c == weather->unk94) {
						brightness = 200;
					}

					for (i = 1; i < g_Vars.roomcount; i++) {
						if (!weatherIsRoomWeatherProof(i)) {
							roomSetFlashBrightness(i, brightness);
						}
					}
				}

				if (weather->unka4 == weather->unk94) {
					soundnum = SFX_80BA_THUNDER;
					pitch = 0.4f + RANDOMFRAC() * 1.5f;
					frac = RANDOMFRAC();

					if (frac <= 0.2f && frac > .1f) {
						soundnum = SFX_80BB_THUNDER;
					}

					if (frac <= 0.3f && frac > 0.20000001788139f) {
						soundnum = SFX_80BC_THUNDER;
					}

					if (frac <= 0.4f && frac > .3f) {
						soundnum = SFX_80BD_THUNDER;
					}

					if (frac <= 0.5f && frac > .4f) {
						soundnum = SFX_80BE_THUNDER;
					}

					if (frac <= 0.6f && frac > .5f) {
						soundnum = SFX_80BF_LIGHTNING;
					}

					if (frac <= 0.7f && frac > 0.59999996423721f) {
						soundnum = SFX_80C0_LIGHTNING;
					}

					if (frac <= 0.8f && frac > .7f) {
						soundnum = SFX_80C1_LIGHTNING;
					}

					if (frac <= 0.9f && frac > 0.79999995231628f) {
						soundnum = SFX_80C2_LIGHTNING;
					}

					if (frac <= 1.0f && frac > .9f) {
						soundnum = SFX_80C3_LIGHTNING;
					}

					if (weather->audiohandles[3] == NULL) {
						weather->rainsfxindex = soundnum;
						sndStart(var80095200, soundnum, &weather->audiohandles[3], -1, -1, -1, -1, -1);
						weather->unk58[3].unk00 = 1;

						if (weather->audiohandles[3] != NULL) {
							volume = weather->sndcurrentvolume;

							if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
								volume /= 2;
							}

							sndAdjust(&weather->audiohandles[3], 0, volume, -1, weather->rainsfxindex, 1.00f, 1, -1, 1);
							audioPostEvent(weather->audiohandles[3], AL_SNDP_PITCH_EVT, *(int *)&pitch);
						}
					}
				}

				weather->unk94++;

				if (weather->unk94 > 150) {
					weather->unk94 = -1;
				}
			}
		}

		if (wetclip) {
			distcamtobbmin.f[0] = particledata->boundarymin.f[0] + g_Vars.currentplayer->cam_pos.f[0];
			distcamtobbmax.f[0] = particledata->boundarymax.f[0] + g_Vars.currentplayer->cam_pos.f[0];
			distcamtobbmin.f[1] = particledata->boundarymin.f[1] + g_Vars.currentplayer->cam_pos.f[1];
			distcamtobbmax.f[1] = particledata->boundarymax.f[1] + g_Vars.currentplayer->cam_pos.f[1];
			distcamtobbmin.f[2] = particledata->boundarymin.f[2] + g_Vars.currentplayer->cam_pos.f[2];
			distcamtobbmax.f[2] = particledata->boundarymax.f[2] + g_Vars.currentplayer->cam_pos.f[2];

			if (numtestrooms < ARRAYCOUNT(testrooms)) {
				testrooms[numtestrooms] = g_Vars.currentplayer->cam_room;
				numtestrooms++;
			}

			for (scale = 1.0f, t = 0; t < numtestrooms; t++) {
				numneighbours = bgRoomGetNeighbours(testrooms[t], neighbours, 20);

				for (n = 0; n < numneighbours; n++) {
					if (g_Rooms[neighbours[n]].flags & ROOMFLAG_ONSCREEN) {
						ok = true;

						for (j = 0; j < numtestrooms; j++) {
							if (testrooms[j] == neighbours[n]) {
								ok = false;
							}
						}

						if (ok) {
							if (distcamtobbmax.f[0] < g_Rooms[neighbours[n]].bbmin[0] || distcamtobbmin.f[0] > g_Rooms[neighbours[n]].bbmax[0]) {
								ok = false;
							}

							if (distcamtobbmax.f[1] < g_Rooms[neighbours[n]].bbmin[1] || distcamtobbmin.f[1] > g_Rooms[neighbours[n]].bbmax[1]) {
								ok = false;
							}

							if (distcamtobbmax.f[2] < g_Rooms[neighbours[n]].bbmin[2] || distcamtobbmin.f[2] > g_Rooms[neighbours[n]].bbmax[2]) {
								ok = false;
							}
						}

						if (ok && numtestrooms < ARRAYCOUNT(testrooms)) {
							testrooms[numtestrooms] = neighbours[n];
							numtestrooms++;
						} else {
							// empty
						}
					}
				}
			}

			for (t = 0; t < numtestrooms; t++) {
				if (weatherIsRoomWeatherProof(testrooms[t])) {
					// @bug: Overflowing badbbmin and badbbmax if badrooms is full.
					// (These writes should be inside the if statement).
					badbbmin[numbadrooms].f[0] = g_Rooms[testrooms[t]].bbmin[0] / scale;
					badbbmin[numbadrooms].f[1] = g_Rooms[testrooms[t]].bbmin[1] / scale;
					badbbmin[numbadrooms].f[2] = g_Rooms[testrooms[t]].bbmin[2] / scale;

					badbbmax[numbadrooms].f[0] = g_Rooms[testrooms[t]].bbmax[0] / scale;
					badbbmax[numbadrooms].f[1] = g_Rooms[testrooms[t]].bbmax[1] / scale;
					badbbmax[numbadrooms].f[2] = g_Rooms[testrooms[t]].bbmax[2] / scale;

					if (numbadrooms < ARRAYCOUNT(badrooms)) {
						badrooms[numbadrooms] = testrooms[t];
						numbadrooms++;
					}
				}
			}

			for (i = 0; i < numbadrooms; i++) {
				bboxes[i][0] = g_Rooms[badrooms[i]].bbmin[0];
				bboxes[i][1] = g_Rooms[badrooms[i]].bbmin[1];
				bboxes[i][2] = g_Rooms[badrooms[i]].bbmin[2];
				bboxes[i][3] = g_Rooms[badrooms[i]].bbmax[0];
				bboxes[i][4] = g_Rooms[badrooms[i]].bbmax[1];
				bboxes[i][5] = g_Rooms[badrooms[i]].bbmax[2];
			}
		}

		colours = gfxAllocateColours(2);
		colours[0].word = PD_BE32(raincol1);
		colours[1].word = PD_BE32(raincol2);

		gSPColor(gdl++, (uintptr_t)(colours), 2);

		timings2[0] = utilsGetCount();

		for (p = 0; p < ARRAYCOUNT(particledata->particles); p++) {
			struct coord sp108;
			struct coord spfc;
			struct weatherparticle *particle2 = &particledata->particles[p];
			int vtxindex = numtris * 3;
			struct coord spe4;
			bool draw = true;
			struct coord spd4;

			if (particle2->active & 3) {
				timings2[7] = utilsGetCount();

				sp108.f[0] = particle2->pos.f[0] + particledata->unk3e80.f[0];
				sp108.f[1] = particle2->pos.f[1] + particledata->unk3e80.f[1];
				sp108.f[2] = particle2->pos.f[2] + particledata->unk3e80.f[2];

				if (cam0f0b5b9c(&sp108, 150)) {
					timings1[7] = timings1[7] + utilsGetCount() - timings2[7];

					sp218[0] = particle2->pos.f[0];
					sp218[1] = particle2->pos.f[2];

					sp214 = sqrtf(sp218[0] * sp218[0] + sp218[1] * sp218[1]);

					if (sp214 < 0.00001f) {
						// empty
					} else {
						if (numtris == 0) {
							vertices = gfxAllocateVertices(12);
						}

						sp218[0] /= sp214;
						sp218[1] /= sp214;

						for (i = 0; i < 4; i++) {
							vertices[i + vtxindex].s = 0;
							vertices[i + vtxindex].t = 0;

							positions[i].f[0] = particle2->pos.f[0];
							positions[i].f[1] = particle2->pos.f[1];
							positions[i].f[2] = particle2->pos.f[2];
						}

						timings2[1] = utilsGetCount();
						timings2[2] = utilsGetCount();

						if (wetclip && numbadrooms > 0) {
							spca8.f[0] = spc90.f[0] = (particle2->pos.f[0] + particledata->unk3e80.f[0]) * scale;
							spca8.f[1] = spc90.f[1] = (particle2->pos.f[1] + particledata->unk3e80.f[1]) * scale;
							spca8.f[2] = spc90.f[2] = (particle2->pos.f[2] + particledata->unk3e80.f[2]) * scale;

							spc9c.f[0] = ((particle2->pos.f[0] - (weather->windspeedx * (rainout / 10.0f))) + particledata->unk3e80.f[0]) * scale;
							spc9c.f[1] = ((particle2->pos.f[1] + weather->rdcurrentlength) + particledata->unk3e80.f[1]) * scale;
							spc9c.f[2] = ((particle2->pos.f[2] - (weather->windspeedz * (rainout / 10.0f))) + particledata->unk3e80.f[2]) * scale;

							spfc.f[0] = spc9c.f[0] - spc90.f[0];
							spfc.f[1] = spc9c.f[1] - spc90.f[1];
							spfc.f[2] = spc9c.f[2] - spc90.f[2];

							if (spca8.f[0] < spc9c.f[0]) {
								tmp = spc9c.f[0];
								spc9c.f[0] = spca8.f[0];
								spca8.f[0] = tmp;
							}

							if (spca8.f[1] < spc9c.f[1]) {
								tmp = spca8.f[1];
								spca8.f[1] = spc9c.f[1];
								spc9c.f[1] = tmp;
							}

							if (spca8.f[2] < spc9c.f[2]) {
								tmp = spca8.f[2];
								spca8.f[2] = spc9c.f[2];
								spc9c.f[2] = tmp;
							}

							timings2[3] = utilsGetCount();

							for (i = 0; i < numbadrooms; i++) {
								if (spc9c.f[0] <= g_Rooms[badrooms[i]].bbmax[0]
										&& spca8.f[0] >= g_Rooms[badrooms[i]].bbmin[0]
										&& spc9c.f[2] <= g_Rooms[badrooms[i]].bbmax[2]
										&& spca8.f[2] >= g_Rooms[badrooms[i]].bbmin[2]
										&& spc9c.f[1] <= g_Rooms[badrooms[i]].bbmax[1]
										&& spca8.f[1] >= g_Rooms[badrooms[i]].bbmin[1]
										&& bounder
										&& bgTestLineIntersectsIntBbox(&spc90, &spfc, &bboxes[i][0], &bboxes[i][3])) {
									draw = false;
								}
							}

							timings1[3] = timings1[3] + utilsGetCount() - timings2[3];
						}

						timings1[2] = timings1[2] + utilsGetCount() - timings2[2];

						if (draw) {
							timings2[4] = utilsGetCount();

							cddiv2 = cddiv / 10.0f;
							rainout2 = rainout / 10.0f;
							f2 = rainwidth * (1.0f + sp214 / cddiv2);

							positions[0].f[0] += -sp218[1] * -f2;
							positions[0].f[2] += sp218[0] * -f2;

							positions[1].f[0] += -sp218[1] * f2;
							positions[1].f[2] += sp218[0] * f2;

							positions[3].f[0] -= weather->windspeedx * rainout2 + -sp218[1] * f2;
							positions[3].f[1] += weather->rdcurrentlength;
							positions[3].f[2] -= weather->windspeedz * rainout2 + sp218[0] * f2;

							positions[2].f[0] -= weather->windspeedx * rainout2 + -sp218[1] * -f2;
							positions[2].f[1] += weather->rdcurrentlength;
							positions[2].f[2] -= weather->windspeedz * rainout2 + sp218[0] * -f2;

							// @bug: Writing to offset 3 overflows the vertices allocation.
							// The vertices array has 12 elements and is iterated 4 times,
							// incrementing vtxindex by 3 each time. This is harmless though,
							// as it writes into unallocated space unless the displaylist is full.
							// And if it's full then you have bigger problems to worry about.
							// Writes also occur with the s and t values further below.
							vertices[vtxindex + 0].colour = 0;
							vertices[vtxindex + 1].colour = 0;
							vertices[vtxindex + 2].colour = 4;
							vertices[vtxindex + 3].colour = 4;

							if (numsparksavailable > 0) {
								spe4.f[0] = (particle2->pos.f[0] + particledata->unk3e80.f[0]) * scale;
								spe4.f[1] = (particle2->pos.f[1] + particledata->unk3e80.f[1]) * scale;
								spe4.f[2] = (particle2->pos.f[2] + particledata->unk3e80.f[2]) * scale;

								for (i = 0; i < numtestrooms; i++) {
									if (spe4.f[0] <= g_Rooms[testrooms[i]].bbmax[0]
											&& spe4.f[0] >= g_Rooms[testrooms[i]].bbmin[0]
											&& spe4.f[2] <= g_Rooms[testrooms[i]].bbmax[2]
											&& spe4.f[2] >= g_Rooms[testrooms[i]].bbmin[2]
											&& spe4.f[1] <= g_Rooms[testrooms[i]].bbmax[1]
											&& spe4.f[1] >= g_Rooms[testrooms[i]].bbmin[1]
											&& spe4.f[1] + scale * (particle2->inc.f[1] * 1) < g_Rooms[testrooms[i]].bbmin[1]) {
										spd4.f[0] = particle2->pos.f[0] + particledata->unk3e80.f[0];
										spd4.f[1] = g_Rooms[testrooms[i]].bbmin[1] / scale;
										spd4.f[2] = particle2->pos.f[2] + particledata->unk3e80.f[2];

										sparksCreate(testrooms[i], NULL, &spd4, &particle2->inc, NULL, SPARKTYPE_SHALLOWWATER);

										numsparksavailable--;
									}
								}
							}

							timings1[4] = timings1[4] + utilsGetCount() - timings2[4];
							timings2[5] = utilsGetCount();

							vertices[vtxindex + 0].t = 256;
							vertices[vtxindex + 1].s = 256;
							vertices[vtxindex + 1].t = 256;
							vertices[vtxindex + 2].s = 256;
							vertices[vtxindex + 3].t = 0;
							vertices[vtxindex + 3].s = 0;
							vertices[vtxindex + 2].t = 0;
							vertices[vtxindex + 0].s = 0;

							timings1[5] = timings1[5] + utilsGetCount() - timings2[5];
							timings2[6] = utilsGetCount();

							vertices[vtxindex + 0].x = positions[0].f[0];
							vertices[vtxindex + 0].y = positions[0].f[1];
							vertices[vtxindex + 0].z = positions[0].f[2];

							vertices[vtxindex + 1].x = positions[1].f[0];
							vertices[vtxindex + 1].y = positions[1].f[1];
							vertices[vtxindex + 1].z = positions[1].f[2];

							vertices[vtxindex + 2].x = positions[2].f[0];
							vertices[vtxindex + 2].y = positions[2].f[1];
							vertices[vtxindex + 2].z = positions[2].f[2];

							if (numtris == 3) {
								gSPVertex(gdl++, (uintptr_t)(vertices), 12, 0);
								gSPTri4(gdl++, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
								numtris = 0;
							} else {
								numtris++;
							}

							timings1[6] = timings1[6] + utilsGetCount() - timings2[6];
							timings1[1] = timings1[1] + utilsGetCount() - timings2[1];
						}
					}

					if (numtris);
				}
			}
		}

		if (numtris > 0) {
			gSPVertex(gdl++, (uintptr_t)(vertices), 12, 0);

			if (numtris == 1) {
				gSPTri1(gdl++, 0, 1, 2);
			}

			if (numtris == 2) {
				gSPTri2(gdl++, 0, 1, 2, 3, 4, 5);
			}

			if (numtris == 3) {
				gSPTri3(gdl++, 0, 1, 2, 3, 4, 5, 6, 7, 8);
			}
		}
	}

	return gdl;
}

Gfx *weatherRenderSnow(Gfx *gdl, struct weatherdata *weather, int arg2)
{
	struct weatherparticledata *particledata;
	struct weatherparticle *particle;
	int j;
	int k;
	int p;
	uint32_t sp137c[1];
	uint32_t sp1354[1];
	bool a0;
	bool s1;
	float f22 = 0.0f;
	int sp126c[50];
	int sp1268;
	float sp1168[8][4][2];
	struct coord sp115c;
	struct coord sp1150;
	float sp114c;
	float sp1148;
	float sp1144;
	int j2;
	int sp1078[50];
	struct coord spe20[50];
	struct coord spbc8[50];
	float sp264[50][12];
	float sp260;
	int s7;
	Col *colours;
	float f0;
	int numneighbours;
	float f20;
	float f2;
	float f0_3;
	struct coord sp234;
	struct coord sp228;
	float sp220;
	float sp21c;
	Mtxf *mtx;
	Vtx *vertices; // 214
	Vtx *vtxbatch;
	Mtxf sp1cc;
	struct coord sp19c[4];
	int sp198;
	float f24;
	int i; // 184
	struct coord sp178;
	struct coord sp16c;
#ifdef AVOID_UB
	RoomNum sp144[21]; // prevent bgRoomGetNeighbours from writing out of bounds
#else
	RoomNum sp144[20];
#endif
	struct coord sp124;
	struct coord sp118;
	float f26;
	float sp108;
	float f16;
	int numcolours = 16;
	float range = 150.0f;

	static uint32_t var8007f100 = 50;
	static uint32_t snowwidth = 5;
	static uint32_t snowheight = 10;
	static uint32_t snowcol1 = 0x8888aaff;
	static uint32_t snowcol2 = 0xffffff7f;

	s7 = 0;
	sp1268 = 0;

	texSelect(&gdl, &g_TexGeneralConfigs[0], 4, 0, 2, 1, NULL);

	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0,
			0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0);
	particledata = weather->particledata[arg2];

	sp198 = 0;

	mtx4LoadIdentity(&sp1cc);
	mtx00015be0(camGetWorldToScreenMtxf(), &sp1cc);

	sp1cc.m[3][0] = 0.0f;
	sp1cc.m[3][1] = 0.0f;
	sp1cc.m[3][2] = 0.0f;

	mtx = gfxAllocateMatrix();

	mtxF2L(&sp1cc, mtx);

	gSPMatrix(gdl++, (uintptr_t)(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	sp234.f[0] = g_Vars.currentplayer->cam_pos.f[0];
	sp234.f[1] = g_Vars.currentplayer->cam_pos.f[1];
	sp234.f[2] = g_Vars.currentplayer->cam_pos.f[2];

	sp228.f[0] = sp234.f[0] - particledata->unk3e80.f[0];
	sp228.f[1] = sp234.f[1] - particledata->unk3e80.f[1];
	sp228.f[2] = sp234.f[2] - particledata->unk3e80.f[2];

	if (fabsf(sp228.f[0]) > fabsf(particledata->boundarymin.f[0]) + fabsf(particledata->boundarymax.f[0])
			|| fabsf(sp228.f[1]) > fabsf(particledata->boundarymin.f[1]) + fabsf(particledata->boundarymax.f[1])
			|| fabsf(sp228.f[2]) > fabsf(particledata->boundarymin.f[2]) + fabsf(particledata->boundarymax.f[2])) {
		sp228.f[0] = particledata->boundaryrange.f[0] / 2.0f;
		sp228.f[1] = particledata->boundaryrange.f[1] / 2.0f;
		sp228.f[2] = particledata->boundaryrange.f[2] / 2.0f;
	}

	// 4ac8
	for (p = 0; p < 500; p++) {
		particle = &particledata->particles[p];

		// x
		f0 = particle->pos.f[0] - particledata->boundarymin.f[0] - sp228.f[0];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[0];
		}

		if (f0 > particledata->boundaryrange.f[0]) {
			f0 -= particledata->boundaryrange.f[0];
		}

		particle->pos.f[0] = particledata->boundarymin.f[0] + f0;

		// y
		f0 = particle->pos.f[1] - particledata->boundarymin.f[1] - sp228.f[1];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[1];
		}

		if (f0 > particledata->boundaryrange.f[1]) {
			f0 -= particledata->boundaryrange.f[1];
		}

		particle->pos.f[1] = particledata->boundarymin.f[1] + f0;

		// z
		f0 = particle->pos.f[2] - particledata->boundarymin.f[2] - sp228.f[2];

		if (f0 < 0.0f) {
			f0 += particledata->boundaryrange.f[2];
		}

		if (f0 > particledata->boundaryrange.f[2]) {
			f0 -= particledata->boundaryrange.f[2];
		}

		particle->pos.f[2] = particledata->boundarymin.f[2] + f0;
	}

	// 4bbc
	particledata->unk3e80.f[0] = sp234.f[0];
	particledata->unk3e80.f[1] = sp234.f[1];
	particledata->unk3e80.f[2] = sp234.f[2];

	sp16c.f[0] = particledata->boundarymin.f[0] + g_Vars.currentplayer->cam_pos.f[0];
	sp178.f[0] = particledata->boundarymax.f[0] + g_Vars.currentplayer->cam_pos.f[0];
	sp16c.f[1] = particledata->boundarymin.f[1] + g_Vars.currentplayer->cam_pos.f[1];
	sp178.f[1] = particledata->boundarymax.f[1] + g_Vars.currentplayer->cam_pos.f[1];
	sp16c.f[2] = particledata->boundarymin.f[2] + g_Vars.currentplayer->cam_pos.f[2];
	sp178.f[2] = particledata->boundarymax.f[2] + g_Vars.currentplayer->cam_pos.f[2];

	if (sp1268 < 50) {
		sp126c[sp1268] = g_Vars.currentplayer->cam_room;
		sp1268++;
	}

	// 4c54
	for (i = 0; i < sp1268; i++) {
		numneighbours = bgRoomGetNeighbours(sp126c[i], sp144, 20);

		for (j2 = 0; j2 < numneighbours; j2++) {
			a0 = true;

			if (g_Rooms[sp144[j2]].flags & ROOMFLAG_ONSCREEN) {
				for (k = 0; k < sp1268; k++) {
					if (sp126c[k] == sp144[j2]) {
						a0 = false;
					}
				}

				if (a0) {
					if (sp178.f[0] < g_Rooms[sp144[j2]].bbmin[0] || sp16c.f[0] > g_Rooms[sp144[j2]].bbmax[0]) {
						a0 = false;
					}

					if (sp178.f[1] < g_Rooms[sp144[j2]].bbmin[1] || sp16c.f[1] > g_Rooms[sp144[j2]].bbmax[1]) {
						a0 = false;
					}

					if (sp178.f[2] < g_Rooms[sp144[j2]].bbmin[2] || sp16c.f[2] > g_Rooms[sp144[j2]].bbmax[2]) {
						a0 = false;
					}
				}

				if (a0 && sp1268 < 50) {
					sp126c[sp1268] = sp144[j2];
					sp1268++;
				}
			}
		}
	}

	// 4dc0
	for (i = 0; i < sp1268; i++) {
		if (weatherIsRoomWeatherProof(sp126c[i])) {
			spe20[s7].f[0] = g_Rooms[sp126c[i]].bbmin[0] / 1;
			spe20[s7].f[1] = g_Rooms[sp126c[i]].bbmin[1] / 1;
			spe20[s7].f[2] = g_Rooms[sp126c[i]].bbmin[2] / 1;

			spbc8[s7].f[0] = g_Rooms[sp126c[i]].bbmax[0] / 1;
			spbc8[s7].f[1] = g_Rooms[sp126c[i]].bbmax[1] / 1;
			spbc8[s7].f[2] = g_Rooms[sp126c[i]].bbmax[2] / 1;

			if (s7 < 50) {
				sp1078[s7] = sp126c[i];
				s7++;
			}
		}

	}

	// 4ea4
	for (j = 0; j < s7; j++) {
		if (0);
		sp264[j][6] = sp264[j][0] = g_Rooms[sp1078[j]].bbmin[0] / 1 - snowwidth;
		sp264[j][9] = sp264[j][3] = g_Rooms[sp1078[j]].bbmax[0] / 1 + snowwidth;
		sp264[j][6] -= var8007f100;
		sp264[j][9] += var8007f100;

		sp264[j][7] = sp264[j][1] = g_Rooms[sp1078[j]].bbmin[1] / 1 - snowwidth;
		sp264[j][10] = sp264[j][4] = g_Rooms[sp1078[j]].bbmax[1] / 1 + snowwidth;
		sp264[j][7] -= var8007f100;
		sp264[j][10] += var8007f100;

		sp264[j][8] = sp264[j][2] = g_Rooms[sp1078[j]].bbmin[2] / 1 - snowwidth;
		sp264[j][11] = sp264[j][5] = g_Rooms[sp1078[j]].bbmax[2] / 1 + snowwidth;
		sp264[j][8] -= var8007f100;
		sp264[j][11] += var8007f100;
	}

	// 4ff0
	for (j = 0; j < 8; j++) {
		sp1168[j][0][0] = sinf(particledata->unk3ec8[j]);
		sp1168[j][0][1] = cosf(particledata->unk3ec8[j]);
		sp1168[j][1][0] = sinf(particledata->unk3ec8[j] + M_PI * 0.5f);
		sp1168[j][1][1] = cosf(particledata->unk3ec8[j] + M_PI * 0.5f);
		sp1168[j][2][0] = sinf(particledata->unk3ec8[j] + M_PI);
		sp1168[j][2][1] = cosf(particledata->unk3ec8[j] + M_PI);
		sp1168[j][3][0] = sinf(particledata->unk3ec8[j] + M_PI * 1.5f);
		sp1168[j][3][1] = cosf(particledata->unk3ec8[j] + M_PI * 1.5f);
	}

	// 514c
	colours = gfxAllocateColours(numcolours);

	for (j = 0; j < numcolours; j++) {
		uint32_t alpha = ((numcolours + 1) * 255 - j * 255) / (numcolours + 1);
		colours[j].word = PD_BE32((snowcol1 & 0xffffff00) | alpha);
	}

	gSPColor(gdl++, (uintptr_t)(colours), numcolours);

	// 51f8
	for (p = 0; p < 500; p++) {
		struct weatherparticle *particle = &particledata->particles[p];
		int tmp2;
		s1 = true;

		if (particle->active & 3) {
			sp1354[0] = utilsGetCount();

			sp124.f[0] = particle->pos.f[0] + particledata->unk3e80.f[0];
			sp124.f[1] = particle->pos.f[1] + particledata->unk3e80.f[1];
			sp124.f[2] = particle->pos.f[2] + particledata->unk3e80.f[2];

			if (cam0f0b5b9c(&sp124, 5)) {
				sp137c[0] = sp137c[0] + utilsGetCount() - sp1354[0];

				sp21c = particle->pos.f[0];
				sp220 = particle->pos.f[2];

				f20 = sqrtf(sp220 * sp220 + sp21c * sp21c);

				if (f20 < 0.00001f) {
					// empty
				} else {
					if (sp198 == 0) {
						vertices = gfxAllocateVertices(8);
					}

					sp260 = 0.0f;

					sp21c /= f20;
					sp220 /= f20;

					for (j = 0; j < 4; j++) {
						vertices[j + sp198 * 4].s = 0;
						vertices[j + sp198 * 4].t = 0;

						sp19c[j].f[0] = particle->pos.f[0];
						sp19c[j].f[1] = particle->pos.f[1];
						sp19c[j].f[2] = particle->pos.f[2];
					}

					// 5344
					if (s7 > 0) {
						sp118.f[0] = particle->pos.f[0] + particledata->unk3e80.f[0];
						sp118.f[1] = particle->pos.f[1] + particledata->unk3e80.f[1];
						sp118.f[2] = particle->pos.f[2] + particledata->unk3e80.f[2];

						for (j = 0; j < s7; j++) {
							// 5398
							if (s1
									&& sp264[j][6] < sp118.f[0]
									&& sp264[j][9] > sp118.f[0]
									&& sp264[j][7] < sp118.f[1]
									&& sp264[j][10] > sp118.f[1]
									&& sp264[j][8] < sp118.f[2]
									&& sp264[j][11] > sp118.f[2]) {
								if (sp264[j][0] < sp118.f[0]
										&& sp264[j][3] > sp118.f[0]
										&& sp264[j][1] < sp118.f[1]
										&& sp264[j][4] > sp118.f[1]
										&& sp264[j][2] < sp118.f[2]
										&& sp264[j][5] > sp118.f[2]) {
									s1 = false;
								} else {
									// 54a8
									f2 = 0.0f;

									// 54d8
									if (sp264[j][0] > sp118.f[0]) {
										f2 = sp118.f[0] - sp264[j][6];
									}

									// 54ec
									if (sp264[j][3] < sp118.f[0]) {
										f2 = sp118.f[0] - sp264[j][9];
									}

									// 5500
									f2 = fabsf(f2) / var8007f100;

									// 5524
									if (f2 > sp260) {
										sp260 = f2;
									}

									if (sp264[j][2] > sp118.f[2]) {
										f2 = sp118.f[2] - sp264[j][8];
									}

									if (sp264[j][5] < sp118.f[2]) {
										f2 = sp118.f[2] - sp264[j][11];
									}

									f2 = fabsf(f2) / var8007f100;

									if (f2 > sp260) {
										sp260 = f2;
									}
								}
							}
						}
					}

					// 559c
					if (s1) {
						int j;
						float val1;
						float val2;
						float val3;

						tmp2 = sp198 * 4;

						if (range);

						f0_3 = sqrtf(particle->pos.f[0] * particle->pos.f[0]
								+ particle->pos.f[1] * particle->pos.f[1]
								+ particle->pos.f[2] * particle->pos.f[2]);

						f24 = particle->pos.f[0] / f0_3;
						sp108 = particle->pos.f[1] / f0_3;
						f26 = particle->pos.f[2] / f0_3;

						// 55fc
						f0_3 = sqrtf(f24 * f24 + f26 * f26);

						val2 = f24 / f0_3;
						val1 = f26 / f0_3;
						val3 = -val2;

						sp114c = sp108 * val2;
						sp1148 = -f0_3;
						sp1144 = sp108 * val1;

						sp115c.f[0] = -sp220;
						sp115c.f[1] = 1.0f;
						sp115c.f[2] = sp21c;

						// 5720
						for (j = 0; j < 4; j++) {
							sp19c[j].f[0] += snowwidth * val1 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp114c * sp1168[(p >> 2) & 7][j][1];
							sp19c[j].f[1] += snowwidth *  f22 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp1148 * sp1168[(p >> 2) & 7][j][1];
							sp19c[j].f[2] += snowwidth * val3 * sp1168[(p >> 2) & 7][j][0] + snowwidth * sp1144 * sp1168[(p >> 2) & 7][j][1];
						}

						// 5784
						// x
						f16 = 0.0f;

						if (particle->pos.f[0] < particledata->boundarymin.f[0] + range) {
							f16 = particle->pos.f[0] - particledata->boundarymin.f[0] - range;
						}

						if (particle->pos.f[0] > particledata->boundarymax.f[0] - range) {
							f16 = particle->pos.f[0] - particledata->boundarymax.f[0] + range;
						}

						f16 = fabsf(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 5870
						// y
						f16 = 0.0f;

						if (particle->pos.f[1] < particledata->boundarymin.f[1] + range) {
							f16 = particle->pos.f[1] - particledata->boundarymin.f[1] - range;
						}

						if (particle->pos.f[1] > particledata->boundarymax.f[1] - range) {
							f16 = particle->pos.f[1] - particledata->boundarymax.f[1] + range;
						}

						f16 = fabsf(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 58f8
						// z
						f16 = 0.0f;

						if (particle->pos.f[2] < particledata->boundarymin.f[2] + range) {
							f16 = particle->pos.f[2] - particledata->boundarymin.f[2] - range;
						}

						if (particle->pos.f[2] > particledata->boundarymax.f[2] - range) {
							f16 = particle->pos.f[2] - particledata->boundarymax.f[2] + range;
						}

						f16 = fabsf(f16) / range;

						if (f16 > sp260) {
							sp260 = f16;
						}

						// 5978
						vertices[tmp2 + 0].colour = (int) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 1].colour = (int) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 2].colour = (int) (sp260 * 16.0f) * 4;
						vertices[tmp2 + 3].colour = (int) (sp260 * 16.0f) * 4;

						// Note: Goal writes all the S's first then T's. XBLA uses ST pairs.
						// And the rain function uses a different order too.
						{
							uint16_t x1;
							uint16_t y1;
							uint16_t x2;
							uint16_t y2;

							y2 += 0;
							x1 = ((p & 1) >> 0) * 8;
							y1 = ((p & 2) >> 1) * 8;
							x2 = x1 + 8;
							y2 = y1 + 8;

							vertices[tmp2 + 0].s = x1 * 32;
							vertices[tmp2 + 0].t = y2 * 32;
							vertices[tmp2 + 1].s = x2 * 32;
							vertices[tmp2 + 1].t = y2 * 32;
							vertices[tmp2 + 2].s = x2 * 32;
							vertices[tmp2 + 2].t = y1 * 32;
							vertices[tmp2 + 3].s = x1 * 32;
							vertices[tmp2 + 3].t = y1 * 32;

							// rain order:
							//vertices[tmp2 + 0].t = y2 * 32;
							//vertices[tmp2 + 1].s = x2 * 32;
							//vertices[tmp2 + 1].t = y2 * 32;
							//vertices[tmp2 + 2].s = x2 * 32;
							//vertices[tmp2 + 3].t = y1 * 32;
							//vertices[tmp2 + 3].s = x1 * 32;
							//vertices[tmp2 + 2].t = y1 * 32;
							//vertices[tmp2 + 0].s = x1 * 32;

							if (sp198 && sp198 && sp198);
						}

						vertices[tmp2 + 0].x = sp19c[0].f[0];
						vertices[tmp2 + 0].y = sp19c[0].f[1];
						vertices[tmp2 + 0].z = sp19c[0].f[2];

						vertices[tmp2 + 1].x = sp19c[1].f[0];
						vertices[tmp2 + 1].y = sp19c[1].f[1];
						vertices[tmp2 + 1].z = sp19c[1].f[2];

						vertices[tmp2 + 2].x = sp19c[2].f[0];
						vertices[tmp2 + 2].y = sp19c[2].f[1];
						vertices[tmp2 + 2].z = sp19c[2].f[2];

						vertices[tmp2 + 3].x = sp19c[3].f[0];
						vertices[tmp2 + 3].y = sp19c[3].f[1];
						vertices[tmp2 + 3].z = sp19c[3].f[2];

						if (sp198 == 1) {
							gSPVertex(gdl++, (uintptr_t)(vertices), 8, 0);
							gSPTri4(gdl++, 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4);
							sp198 = 0;
						} else {
							sp198 = 1;
						}
					}
				}
			}
		}
	}

	if (sp198 > 0) {
		gSPVertex(gdl++, (uintptr_t)(vertices), 8, 0);
		gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
	}

	return gdl;
}

void weatherStop(void)
{
	if (g_WeatherData) {
		if (g_WeatherData->audiohandles[0]) {
			audioStop(g_WeatherData->audiohandles[0]);
		}

		if (g_WeatherData->audiohandles[1]) {
			audioStop(g_WeatherData->audiohandles[1]);
		}

		if (g_WeatherData->audiohandles[2]) {
			audioStop(g_WeatherData->audiohandles[2]);
		}

		if (g_WeatherData->audiohandles[3]) {
			audioStop(g_WeatherData->audiohandles[3]);
		}

		g_WeatherData = NULL;
	}
}
