#include <ultra64.h>
#include <n_libaudio.h>
#include "constants.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/speaker.h"
#include "lib/lib_2fc60.h"
#include "lib/lib_317f0.h"
#include "lib/lib_39c80.h"
#include "lib/lib_3d280.h"
#include "lib/lib_3e3e0.h"
#include "data.h"
#include "types.h"

struct N_SpeakerType N_SpeakerType;
u8 var8009c344[2];
u8 var8009c346[2];
u8 var8009c348[4];

void speakersSetMode(u8 mode)
{
	s32 i;

	N_SpeakerType.surround = 0;
	N_SpeakerType.mono = 0;
	N_SpeakerType.headphone = 0;

	switch (mode) {
	case SPEAKERMODE_MONO:
		N_SpeakerType.mono = 1;
		break;
	case SPEAKERMODE_HEADPHONE:
		N_SpeakerType.headphone = 1;
		break;
	case SPEAKERMODE_SURROUND:
		N_SpeakerType.surround = 1;
		break;
	}

	for (i = 0; i < 2; i++) {
		speaker00034104(i, 0);
	}
}

void speaker00034104(s32 index, s32 arg1)
{
	if (arg1 == 0) {
		arg1 = var8009c348[index];
	}

	var8009c344[index] = 0;
	var8009c346[index] = 0;

	switch (arg1) {
	case 2:
		if (N_SpeakerType.surround) {
			var8009c346[index] = 1;
		}
		break;
	case 3:
		if (N_SpeakerType.surround) {
			var8009c344[index] = 1;
		}
		break;
	case 4:
		if (!N_SpeakerType.mono) {
			var8009c344[index] = 1;
		}
		break;
	case 5:
		if (!N_SpeakerType.mono) {
			var8009c344[index] = 1;
			var8009c346[index] = 1;
		}
		break;
	}

	var8009c348[index] = arg1;
}
