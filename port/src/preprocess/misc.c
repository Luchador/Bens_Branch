#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <PR/gbi.h>

#include "data.h"
#include "bss.h"
#include "game/setuputils.h"
#include "game/texdecompress.h"
#include "mod.h"

#include "preprocess/common.h"

uint8_t *preprocessAnimations(uint8_t* data, uint32_t size, uint32_t* outSize)
{
	// set the anim table pointers as well
	extern uint8_t *_animationsTableRomStart;
	extern uint8_t *_animationsTableRomEnd;

	// the animation table is at the end of the segment
	uint32_t *animtbl = (void *)(data + size - 0x38a0);
	_animationsTableRomStart = (uint8_t *)animtbl;
	_animationsTableRomEnd = data + size;

	PD_SWAP_VAL(*animtbl);
	const uint32_t count = *animtbl++;

	struct animtableentry *anim = (struct animtableentry *)animtbl;
	for (uint32_t i = 0; i < count; ++i, ++anim) {
		PD_SWAP_VAL(anim->numframes);
		PD_SWAP_VAL(anim->bytesperframe);
		PD_SWAP_VAL(anim->headerlen);
		PD_SWAP_VAL(anim->data);
		// if an external replacement exists, replace the table entry and mark the offset
		if (modAnimationLoadDescriptor(i, anim) > 0) {
			anim->data = 0xffffffff;
		}
	}

	return NULL;
}

uint8_t *preprocessMpConfigs(uint8_t* data, uint32_t size, uint32_t* outSize)
{
	const uint32_t count = size / sizeof(struct mpconfig);
	struct mpconfig *cfg = (struct mpconfig *)data;
	for (uint32_t i = 0; i < count; ++i, ++cfg) {
		PD_SWAP_VAL(cfg->setup.options);
		PD_SWAP_VAL(cfg->setup.teamscorelimit);
		PD_SWAP_VAL(cfg->setup.chrslots);
		// TODO: are these required or are they always 0?
		PD_SWAP_VAL(cfg->setup.fileguid.deviceserial);
		PD_SWAP_VAL(cfg->setup.fileguid.fileid);
		for (int j = 0; j < ARRAYCOUNT(cfg->setup.weapons); ++j) {
			if (cfg->setup.weapons[j] >= 0x25) {
				cfg->setup.weapons[j] += (MPWEAPON_SHIELD - MPWEAPON_PP9I);
			}
		}
	}

	return NULL;
}

uint8_t *preprocessTexturesList(uint8_t* data, uint32_t size, uint32_t* outSize)
{
	struct texture *tex = (struct texture *)data;
	const uint32_t count = size / sizeof(*tex);
	for (uint32_t i = 0; i < count; ++i, ++tex) {
		// TODO: it sure looks like none of the fields except soundsurfacetype, surfacetype and dataoffset are set
		// just swap the last 3 bytes of the first word...
		const uint32_t dofs = (uint32_t)tex->dataoffset << 8;
		tex->dataoffset = PD_BE32(dofs);
		// ...and the surface types in the first byte
		const uint8_t tmp = tex->soundsurfacetype;
		tex->soundsurfacetype = tex->surfacetype;
		tex->surfacetype = tmp;
	}

	return NULL;
}
