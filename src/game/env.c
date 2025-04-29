#include <ultra64.h>
#include "constants.h"
#include "game/debug.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/bg.h"
#include "game/env.h"
#include "bss.h"
#include "gfx.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"

bool g_FogEnabled;
//bool g_EnvHasTransparency;
struct distfadesettings *g_EnvDistFadeSettingsPtr;
struct distfadesettings g_EnvDistFadeSettings;

struct shadesettings g_EnvShadeSettings;
float g_EnvFogMaxFrac;
float g_EnvFogMinFrac;
struct environment *g_EnvOrigEnvironment;

float g_EnvFogMax = MAXFLOAT;
float g_EnvFogMin = 0;
struct environment g_Env = {900, 1000};

#define RGB(col) col >> 16, (col >> 8) & 0xff, col & 0xff
#define NO_SUNS 0, NULL
#define SUNS(arr) ARRAYCOUNT(arr), arr

//                   lensflare,        colour,        x,       y,        z, texture size, orb size
struct sun suns_area51[] = { 0, RGB(0xffe080),  -200000,  310000,  1000000, 20, 48 };
struct sun suns_villa[]  = { 1, RGB(0xffffff), -1000000,  200000,    50000, 20, 48 };
struct sun suns_ci[]     = { 1, RGB(0xffffff),   400000,  600000, -1000000, 20, 48 };

struct sun suns_skedar[] = {
	{ 1, RGB(0xffffff),  -400000,  600000,  1000000, 20, 48 },
	{ 1, RGB(0xffffff),        0, 1000000,  1000000, 25, 60 },
	{ 1, RGB(0xffffff),   400000,  600000,  1000000, 15, 36 },
};

struct sun suns_crashsite[] = { 1, RGB(0xffd7f2),  1900000,  300000, -1400000, 22, 48 };
struct sun suns_airbase[]   = { 1, RGB(0xffd7f2), -1200000,  200000,   150000, 30, 60 };

struct environment g_Environments[] = {
	//                                   |- distfade --|  |- fog -|                                             |---------- clouds ----------------------|   |--------- water ---------|
	// stage                  near  far  opa%  xlu%  ref  min   max       sky colour                            e  colour          scale  type e  height   colour         scale      type
	{ -1,                     15, 10000,    0,    0,   0,   0,    0,      RGB(0x001040), NO_SUNS,               0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),     0,     0, 0.0f, 0.0f, 0.0f},
	{ STAGE_CRASHSITE,        15, 10000,    0,    0,   0, 994,   1000,    RGB(0x9b2d1e), SUNS(suns_crashsite),  1, RGB(0xfafa00),  1500,   0,  0,    0,    RGB(0x000000),     -5000, 0, 0.0f, 0.0f, 0.0f},
	{ STAGE_PELAGIC,          15, 15000, 3333, 4444, 600, 995,   1000,    RGB(0x2d3e60), NO_SUNS,               1, RGB(0xf0f0f0),  5000,   0,  0,    0,    RGB(0x14212b),     -2000, 0, 0.0f, 0.0f, 0.0f},
	{ STAGE_VILLA,            15, 20000,    0,    0,   0, 981,   1047,    RGB(0x3986bb), SUNS(suns_villa),      1, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x00ffff),     -1850, 1, 0.0f, 0.0f, 0.0f},
	{ STAGE_RESCUE,           15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MAIANSOS,         15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_ATTACKSHIP,       15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_INFILTRATION,     15, 12000,   0,  0,  0,       0,    0,      RGB(0x000000), SUNS(suns_area51),     0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_ESCAPE,           15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), SUNS(suns_area51),     0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_DEEPSEA,          15, 10000,   0,  0,  0,       0,    0,      RGB(0x050000), NO_SUNS,               0, RGB(0x9b9b9b),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_DEFENSE,          15, 10000,   0,  0,  0,       0,    0,      RGB(0x65b2ff), SUNS(suns_ci),         0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_CITRAINING,       15, 10000,   0,  0,  0,       0,    0,      RGB(0x65b2ff), SUNS(suns_ci),         0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_DUEL,             10, 10000,   0,  0,  0,       0,    0,      RGB(0x65b2ff), SUNS(suns_ci),         0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_DEFECTION,        10, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x3a1100),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MBR,              10, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x3a1100),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_EXTRACTION,       10, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x3a1100),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_SKEDARRUINS,      15, 10000,   0,  0,  0,       0,    0,      RGB(0x6565ff), SUNS(suns_skedar),     0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_WAR,              15, 10000,   0,  0,  0,       0,    0,      RGB(0x6565ff), SUNS(suns_skedar),     0, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),      0,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_SKEDAR,        15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               1, RGB(0x3a1100),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_CHICAGO,          10, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               1, RGB(0x50280a),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_G5BUILDING,       15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x50280a),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_AIRFORCEONE,      15, 20000,   0,  0,  0,       0,    0,      RGB(0x001040), NO_SUNS,               1, RGB(0xffffff),  5000,   0,  0,    1,    RGB(0xffffff),  -5000,    2, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_RAVINE,        15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               1, RGB(0x50280a),  5000,   0,  0,    0,    RGB(0x000000), -31000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_AIRBASE,          15, 20000,   0,  0,  0,       0,    0,      RGB(0x001040), SUNS(suns_airbase),    1, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_INVESTIGATION,    15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x1e1e1e),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_PIPES,         15, 10000,   0,  0,  0,       0,    0,      RGB(0x000008), NO_SUNS,               1, RGB(0x46c7ba),  4500,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_G5BUILDING,    15, 10000,   0,  0,  0,       0,    0,      RGB(0x000008), NO_SUNS,               1, RGB(0x5a90a5),  4500,   0,  0,    0,    RGB(0x000000), -20000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_TEMPLE,        15, 10000,   0,  0,  0,       0,    0,      RGB(0x001080), NO_SUNS,               1, RGB(0xffffff),  5000,   0,  0,    0,    RGB(0x00ffff),  -1850,    1, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_COMPLEX,       15, 10000,   0,  0,  0,       0,    0,      RGB(0x020000), NO_SUNS,               1, RGB(0x82aac8),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_BASE,          15, 20000,   0,  0,  0,       0,    0,      RGB(0x040000), NO_SUNS,               1, RGB(0x82b464),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_AREA52,        15, 10000,   0,  0,  0,       0,    0,      RGB(0x000008), NO_SUNS,               1, RGB(0x46c7ba),  4500,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_WAREHOUSE,     15, 10000,   0,  0,  0,       0,    0,      RGB(0x020000), NO_SUNS,               1, RGB(0x82aac8),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_CARPARK,       15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               1, RGB(0x64c886),  5500,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_RUINS,         15, 10000,   0,  0,  0,       0,    0,      RGB(0x030000), NO_SUNS,               1, RGB(0x82e6aa),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_SEWERS,        15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               1, RGB(0x646464),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_FELICITY,      10, 10000,   0,  0,  0,       0,    0,      RGB(0x040500), NO_SUNS,               1, RGB(0x64d282),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_FORTRESS,      15, 10000,   0,  0,  0,       0,    0,      RGB(0x000008), NO_SUNS,               1, RGB(0x5a90a5),  5500,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_VILLA,         15, 10000,   0,  0,  0,       0,    0,      RGB(0x8888dc), NO_SUNS,               1, RGB(0xffaa2a),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ STAGE_MP_GRID,          15, 10000,   0,  0,  0,       0,    0,      RGB(0x000000), NO_SUNS,               0, RGB(0x1e1e1e),  5000,   0,  0,    0,    RGB(0x000000),  -5000,    0, 0.0f, 0.0f, 0.0f},
	{ 0 }, // Need a 0 at the end so it doesn't crash
};

struct environment *envGetCurrent(void)
{
	return &g_Env;
}

float envGetFogMax(void)
{
	return g_EnvFogMax;
}

float envGetSquaredFogMax(void)
{
	return g_EnvFogMax * g_EnvFogMax;
}

void envTick(void)
{
	struct zrange zrange;
	float zfar;
	float znear;
	float fogAlphaSlope;
	float fogAlphaOffset;

	if (!g_FogEnabled) {
		return;
	}

	// Get the Z range from the current vi configuration and scale it to world units
	viGetZRange(&zrange);

	// Convert percentage-based fog range to fractional (0.0 - 1.0)
	g_EnvFogMinFrac = g_Env.fogmin * 0.001f;
	g_EnvFogMaxFrac = g_Env.fogmax * 0.001f;

	// Compute actual fog distances based on camera near/far plane
	g_EnvFogMax = (zrange.far - zrange.near) * g_EnvFogMaxFrac + zrange.f[0];
	g_EnvFogMin = (zrange.far - zrange.near) * g_EnvFogMinFrac + zrange.f[0];

	// Save these for later shading math
	g_EnvShadeSettings.znear = zrange.near;
	g_EnvShadeSettings.zfar = zrange.far;

	znear = zrange.near;
	zfar = zrange.far;

	// Compute fog alpha slope and offset for linear interpolation across depth
	fogAlphaSlope  = 128.0f / (g_EnvFogMaxFrac - g_EnvFogMinFrac);
	fogAlphaOffset = (0.5f - g_EnvFogMinFrac) * 256.0f / (g_EnvFogMaxFrac - g_EnvFogMinFrac);

	// Store the computed fog shading values
	g_EnvShadeSettings.alphafar = -fogAlphaSlope * zfar * (znear + 1.0f) / (zfar - znear) / 255.0f;
	g_EnvShadeSettings.alphanear = (fogAlphaSlope * (zfar + 1.0f) / (zfar - znear) + fogAlphaOffset) / 255.0f;
}


void envApplyEnvironment(struct environment *env)
{
	if(env == NULL)
	{
		return;
	}

	viSetZRange(15.0f, 40000.0f); //TEMP: hard code z ranges

	g_Env.fogmin = env->fogmin;
	g_Env.fogmax = env->fogmax;

	// If fogmin and fogmax are 0, disable fog
	g_FogEnabled = (g_Env.fogmin != 0 || g_Env.fogmax != 0);

	g_Env.sky_r = env->sky_r;
	g_Env.sky_g = env->sky_g;
	g_Env.sky_b = env->sky_b;
	g_Env.skyredfrac = g_Env.sky_r / 255.0f;
	g_Env.skygreenfrac = g_Env.sky_g / 255.0f;
	g_Env.skybluefrac = g_Env.sky_b / 255.0f;

	g_Env.numsuns = env->numsuns;
	g_Env.suns = env->suns;

	g_Env.clouds_enabled = env->clouds_enabled;
	g_Env.clouds_scale = env->clouds_scale;
	g_Env.clouds_type = env->clouds_type;
	g_Env.clouds_r = env->clouds_r;
	g_Env.clouds_g = env->clouds_g;
	g_Env.clouds_b = env->clouds_b;

	g_Env.water_enabled = env->water_enabled;
	g_Env.water_scale = env->water_scale;
	g_Env.water_type = env->water_type;
	g_Env.water_r = env->water_r;
	g_Env.water_g = env->water_g;
	g_Env.water_b = env->water_b;
	g_Env.clouds_height = env->clouds_height;

	if (!env->opaperc) {
		g_EnvDistFadeSettingsPtr = NULL;
	} else {
		g_EnvDistFadeSettings.opaperc = env->opaperc;
		g_EnvDistFadeSettings.xluperc = env->xluperc;
		g_EnvDistFadeSettings.refdist = env->refdist;
		g_EnvDistFadeSettingsPtr = &g_EnvDistFadeSettings;
	}

	envTick();
}

void envChooseAndApply(int stagenum)
{
	struct environment *env1;

	g_EnvFogMax = MAXFLOAT;
	g_EnvFogMin = 0;

	// Try to find an env1
	for (env1 = &g_Environments[0]; env1->stage != 0; env1++) {
		if (env1->stage == stagenum) {
			g_EnvOrigEnvironment = env1;
			envApplyEnvironment(g_EnvOrigEnvironment);
			return;
		}
	}

	if (env1 == NULL) {
		env1 = &g_Environments[0];
		g_EnvOrigEnvironment = env1;
		envApplyEnvironment(g_EnvOrigEnvironment);
	}

	g_EnvOrigEnvironment = NULL;
}

Gfx *envStartFog(Gfx *gdl)
{
    if (!g_FogEnabled) return gdl;

	RGBA fogColor = {g_Env.sky_r, g_Env.sky_g, g_Env.sky_b, 255};
    gfx_Set_Fog_Color(gdl++, fogColor);
    gfx_Fog_Position(gdl++, g_Env.fogmin, g_Env.fogmax);
    gfx_Set_Geometry_Mode(gdl++, G_FOG);

    return gdl;
}

Gfx *envStopFog(Gfx *gdl)
{
	if (!g_FogEnabled) {
		return gdl;
	}

	gfx_Clear_Geometry_Mode(gdl++, G_FOG);

	return gdl;
}

bool envIsWithinFogRenderDistance(struct coord *pos, float tolerance)
{
	struct coord sp24;
	Mtx *mtx;
	struct coord *campos;
	float tmp;

	if (!g_FogEnabled) {
		return true;
	}

	campos = &g_Vars.currentplayer->cam_pos;

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		return true;
	}

	mtx = camGetPlayerWorldToScreenMtx();

	sp24.x = pos->x - campos->x;
	sp24.y = pos->y - campos->y;
	sp24.z = pos->z - campos->z;

	tmp = sp24.f[0] * (*mtx)[0][0] + sp24.f[1] * (*mtx)[0][1] + sp24.f[2] * (*mtx)[0][2];

	if (tmp > g_EnvFogMax + tolerance) {
		return false;
	}

	return true;
}

struct distfadesettings *envGetDistFadeSettings(void)
{
	return g_EnvDistFadeSettingsPtr;
}

int envGetObjShadeMode(struct prop *prop, float out[4])
{
	if (!g_FogEnabled) {
		return SHADEMODE_OPA;
	}

	if (prop->z < 0.0f) {
		return SHADEMODE_OPA;
	}

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		return SHADEMODE_OPA;
	}

	out[0] = g_Env.skyredfrac;
	out[1] = g_Env.skygreenfrac;
	out[2] = g_Env.skybluefrac;
	out[3] = g_EnvShadeSettings.alphanear + g_EnvShadeSettings.alphafar / prop->z;

	if (out[3] < 0.0f) {
		return SHADEMODE_OPA;
	}

	if (out[3] > 1.0f) {
		return SHADEMODE_XLU;
	}

	return SHADEMODE_FRAC;
}
