#pragma once

#include "types.h"
#include "platform.h"

struct mp3decfourbytes {
	uint8_t bytes[2];
	int8_t unk02;
	int8_t unk03;
#ifdef PLATFORM_64BIT
	uint32_t _pad_;
#endif
};

struct asistream_scalefac {
	/*0x3d08*/ uint32_t l[22];
	/*0x3d60*/ uint32_t unk3d60;
	/*0x3d64*/ uint32_t s[3][13];
};

struct asistream_4f64 {
	float unk00[18];
};

struct asistream {
	/*0x0000*/ int unk00;
	/*0x0004*/ int (*unk04)(int arg0, void *arg1, int arg2, int arg3);
	/*0x0008*/ int unk08;
	/*0x000c*/ int unk0c;
	/*0x0010*/ int unk10;
	/*0x0014*/ int unk14;
	/*0x0018*/ int unk18;
	/*0x2000*/ uint8_t unk1c[0x2000];
	/*0x201c*/ int unk201c;
	/*0x2020*/ int unk2020;
	/*0x2024*/ uint8_t buffer[0x40];
	/*0x2064*/ int count;
	/*0x2068*/ uint32_t unk2068;
	/*0x206c*/ uint32_t unk206c;
	/*0x2070*/ struct mp3thing unk2070[6];
	/*0x3ba0*/ int unk3ba0;
	/*0x3ba4*/ uint32_t version;
	/*0x3ba8*/ uint32_t layer;
	/*0x3bac*/ uint32_t crctype;
	/*0x3bb0*/ uint32_t bitrateindex;
	/*0x3bb4*/ uint32_t samplerateindex;
	/*0x3bb8*/ uint32_t haspadding;
	/*0x3bbc*/ uint32_t privatebit;
	/*0x3bc0*/ uint32_t channelmode;
	/*0x3bc4*/ uint32_t unk3bc4;
	/*0x3bc8*/ uint32_t unk3bc8;
	/*0x3bcc*/ uint32_t unk3bcc;
	/*0x3bd0*/ uint32_t unk3bd0;
	/*0x3bd4*/ bool doneinitial;
	/*0x3bd8*/ uint32_t initialversion;
	/*0x3bdc*/ uint32_t initiallayer;
	/*0x3be0*/ uint32_t initialcrctype;
	/*0x3be4*/ uint32_t initialsamplerateindex;
	/*0x3be8*/ uint32_t initialchannelmode;
	/*0x3bec*/ uint32_t unk3bec;
	/*0x3bf0*/ uint32_t unk3bf0;
	/*0x3bf4*/ uint32_t main_data_begin;
	/*0x3bf8*/ uint32_t scfsi[1][32];
	/*0x3c78*/ uint32_t part2_3_length[2][1];
	/*0x3c80*/ uint32_t big_value[2][1];
	/*0x3c88*/ int global_gain[2][1];
	/*0x3c90*/ uint32_t scalefac_compress[2][1];
	/*0x3c98*/ uint32_t window_switching[2][1];
	/*0x3ca0*/ uint32_t block_type[2][1];
	/*0x3ca8*/ uint32_t mixed_block_flag[2][1];
	/*0x3cb0*/ uint32_t table_select[2][1][3];
	/*0x3cc8*/ uint32_t subblock_gain[2][1][3];
	/*0x3ce0*/ uint32_t region0_count[2][1];
	/*0x3ce8*/ uint32_t region1_count[2][1];
	/*0x3cf0*/ uint32_t preflag[2][1];
	/*0x3cf8*/ uint32_t scalefac_scale[2][1];
	/*0x3d00*/ uint32_t count1table_select[2][1];
	struct asistream_scalefac scalefac[2][1];
	/*0x3ef8*/ uint32_t unk3ef8;
	/*0x3efc*/ uint32_t unk3efc[6];
	/*0x3f14*/ uint32_t unk3f14[26];
	/*0x3f7c*/ int unk3f7c;
	/*0x3f80*/ int unk3f80;
	/*0x3f84*/ int unk3f84;
	/*0x3f88*/ int unk3f88;
	/*0x3f8c*/ int numchannels;
	/*0x3f90*/ int numgranules;
	/*0x3f94*/ int16_t unk3f94[1][578];
	/*0x4418*/ uint8_t unk4418[1][578];
	/*0x465c*/ int unk465c[1];
	/*0x4660*/ int unk4660[1];
	/*0x4664*/ struct asistream_4f64 unk4664[1][32];
	/*0x4f64*/ struct asistream_4f64 unk4f64[2][32];
	/*0x6164*/ uint8_t unk6164[0x900];
	/*0x6a64*/ struct asistream_4f64 unk6a64[2][32];
	/*0x7c64*/ uint8_t unk7c64[0x810];
	/*0x8474*/ int unk8474;
	/*0x8478*/ bool (*unk8478)(struct asistream *stream);
	/*0x847c*/ bool (*unk847c)(struct asistream *stream);
};

uint32_t mp3mainInit(void);
struct asistream *mp3main00044460(int arg0, void *arg1, int arg2);
int mp3main0004453c(struct asistream *stream, struct mp3thing **arg1, int *arg2);
