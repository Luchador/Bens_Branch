#include <ultra64.h>
#include <string.h>
#include "constants.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "lib/rzip.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "data.h"
#include "gfx.h"
#include "textureconfig.h"
#include "types.h"
#include "video.h"

void texSetBitstring(uint8_t *bitstring)
{
	g_TexBitstring = bitstring;
	g_TexAccumValue = 0;
	g_TexAccumNumBits = 0;
}

int texReadBits(int wantnumbits)
{
	while (g_TexAccumNumBits < wantnumbits) {
		g_TexAccumValue = g_TexAccumValue << 8 | *g_TexBitstring;
		g_TexBitstring++;
		g_TexAccumNumBits += 8;
	}

	g_TexAccumNumBits -= wantnumbits;

	return (g_TexAccumValue >> g_TexAccumNumBits) & ((1 << wantnumbits) - 1);
}

extern uint8_t *g_TextureConfigSegment;
extern uint32_t g_TexBase;
extern struct textureconfig *g_TexRedLinesConfigs;
extern struct textureconfig *g_TexGroup11Configs;

extern uint8_t *_textureconfigSegmentRomStart;
extern uint8_t *_textureconfigSegmentStart;
extern uint8_t *_textureconfigSegmentEnd;

void texReset(void)
{
	int stage;
	int i;

	// HACK: define a big table of pointers we need to fill and sizes of the textureconfig data that goes there
	#define DEFINE_TCPTR(ptr, data) { (void **)&ptr, (const void *)data, sizeof(data), ARRAYCOUNT(data) }
	static const struct {
		void **dst;
		const void *src;
		uint32_t size;
		int count;
	} tcptrs[] = {
		DEFINE_TCPTR(g_TexGdl1, g_TcGdl1),
		DEFINE_TCPTR(g_ExplosionTexturePairs, g_TcExplosionTexturePairs),
		DEFINE_TCPTR(g_TexWallhitConfigs, g_TcWallhitConfigs),
		DEFINE_TCPTR(g_TexBeamConfigs, g_TcBeamConfigs),
		DEFINE_TCPTR(g_TexLaserConfigs, g_TcLaserConfigs),
		DEFINE_TCPTR(g_TexGroup03Configs, g_TcGroup03Configs),
		DEFINE_TCPTR(g_TexGeCrosshairConfigs, g_TcGeCrosshairConfigs),
		DEFINE_TCPTR(g_TexRedLinesConfigs, g_TcRedLineConfigs),
		DEFINE_TCPTR(g_TexShadowConfigs, g_TcShadowConfigs),
		DEFINE_TCPTR(g_TexShieldConfigs, g_TcShieldConfigs),
		DEFINE_TCPTR(g_TexShardConfigs, g_TcShardConfigs),
		DEFINE_TCPTR(g_TexScreenConfigs, g_TcScreenConfigs),
		DEFINE_TCPTR(g_TexSkyWaterConfigs, g_TcSkyWaterConfigs),
		DEFINE_TCPTR(g_TexGroup11Configs, g_TcGroup11Configs),
		DEFINE_TCPTR(g_TexLightGlareConfigs, g_TcLightGlareConfigs),
		DEFINE_TCPTR(g_TexSparkConfigs, g_TcSparkConfigs),
		DEFINE_TCPTR(g_TexGeneralConfigs, g_TcGeneralConfigs),
		DEFINE_TCPTR(g_TexRadarConfigs, g_TcRadarConfigs),
		DEFINE_TCPTR(g_TexStarsConfigs, g_TcStarsConfigs),
	};
	#undef DEFINE_TCPTR

	// calculate total length, should be 0xb50 on a 32-bit platform with an unmodified textureconfig.c
	uint32_t len = 0;
	for (int i = 0; i < ARRAYCOUNT(tcptrs); ++i) {
		len += tcptrs[i].size;
	}

	g_TextureConfigSegment = mempAlloc(len, MEMPOOL_STAGE);
	g_TexBase = 0; // unused

	// set up pointers and fill them in
	uint32_t tcofs = 0;
	for (int i = 0; i < ARRAYCOUNT(tcptrs); ++i) {
		*tcptrs[i].dst = g_TextureConfigSegment + tcofs;
		memcpy(*tcptrs[i].dst, tcptrs[i].src, tcptrs[i].size);
		tcofs += tcptrs[i].size;
	}

	// calculate tc count, skipping the gdls and explosion pairs
	g_TexNumConfigs = 0;
	for (int i = 4; i < ARRAYCOUNT(tcptrs); ++i) {
		g_TexNumConfigs += tcptrs[i].count;
	}

	// reset backend texture cache
	videoResetTextureCache();

	// get backend texture settings
	g_TexFilter2D = videoGetTextureFilter2D() ? G_TF_BILERP : G_TF_POINT;

	// notify blur code that the blur framebuffer is probably full of garbage
	g_BlurFbDirty = true;

	g_TexWords = mempAlloc(ALIGN16(g_TexNumConfigs * sizeof(uintptr_t)), MEMPOOL_STAGE);

	for (i = 0; i < g_TexNumConfigs; i++) {
		g_TexWords[i] = NULL;
	}

	for (i = 0; i < ARRAYCOUNT(g_TcExplosionTexturePairs); i++) {
		texLoad(&g_ExplosionTexturePairs[i].texturenum1, NULL);
		texLoad(&g_ExplosionTexturePairs[i].texturenum2, NULL);
	}

	texLoadFromDisplayList(g_TexGdl1, 0, 0);

	stage = mainGetStageNum();
}
