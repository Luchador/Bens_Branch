#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libaudio.h"

#include "preprocess/common.h"

struct n64_adpcm_waveinfo {
	uint32_t loop; // ptr to ALADPCMloop
	uint32_t book; // ptr to ALADPCMBook
};

struct n64_raw_waveinfo {
	uint32_t loop; // ptr to ALRawLoop
};

struct n64_wavetable {
	uint32_t base;
	int len;
	uint8_t type;
	uint8_t flags;
	union {
		struct n64_adpcm_waveinfo adpcmWave;
		struct n64_raw_waveinfo rawWave;
	} waveInfo;
};

struct n64_sound {
	uint32_t envelope; // ptr to ALEnvelope
	uint32_t keyMap; // ptr to ALKeyMap
	uint32_t wavetable; // ptr to struct n64_wavetable
	uint8_t samplePan;
	uint8_t sampleVolume;
	uint8_t flags;
};

struct n64_instrument {
	uint8_t volume;
	uint8_t pan;
	uint8_t priority;
	uint8_t flags;
	uint8_t tremType;
	uint8_t tremRate;
	uint8_t tremDepth;
	uint8_t tremDelay;
	uint8_t vibType;
	uint8_t vibRate;
	uint8_t vibDepth;
	uint8_t vibDelay;
	int16_t bendRange;
	int16_t soundCount;
	uint32_t soundArray[1]; // ptr to struct n64_sound
};

struct n64_bank {
	int16_t instCount;
	uint8_t flags;
	uint8_t pad;
	int sampleRate;
	uint32_t percussion; // ptr to struct n64_instrument
	uint32_t instArray[1]; // ptr to struct n64_instrument
};

struct n64_bankfile {
	int16_t revision;
	int16_t bankCount;
	uint32_t bankArray[1]; // ptr to struct n64_bank
};

// only proceeds to convert the next item if it's not already converted
#define AL_NEXT_ITEM(field, func) { \
	struct ptrmarker *marker = ptrFind(srcpos); \
	if (marker == NULL) { \
		field = (void *)(uintptr_t)(dstpos); \
		ptrAdd(srcpos, (uintptr_t)(field)); \
		dstpos = func(dst, dstpos, src, srcpos); \
	} else { \
		field = (void *)marker->ptr_host; } \
}

static uint32_t convertAudioEnvelope(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	ALEnvelope *n64_envelope = (ALEnvelope *) &src[srcpos];
	ALEnvelope *host_envelope = (ALEnvelope *) &dst[dstpos];

	dstpos += sizeof(ALEnvelope);

	host_envelope->attackTime = PD_BE32(n64_envelope->attackTime);
	host_envelope->decayTime = PD_BE32(n64_envelope->decayTime);
	host_envelope->releaseTime = PD_BE32(n64_envelope->releaseTime);
	host_envelope->attackVolume = n64_envelope->attackVolume;
	host_envelope->decayVolume = n64_envelope->decayVolume;

	return dstpos;
}

static uint32_t convertAudioKeyMap(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	ALKeyMap *n64_keymap = (ALKeyMap *) &src[srcpos];
	ALKeyMap *host_keymap = (ALKeyMap *) &dst[dstpos];

	dstpos += sizeof(ALKeyMap);

	host_keymap->velocityMin = n64_keymap->velocityMin;
	host_keymap->velocityMax = n64_keymap->velocityMax;
	host_keymap->keyMin = n64_keymap->keyMin;
	host_keymap->keyMax = n64_keymap->keyMax;
	host_keymap->keyBase = n64_keymap->keyBase;
	host_keymap->detune = n64_keymap->detune;

	return dstpos;
}

static uint32_t convertAudioAdpcmLoop(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	ALADPCMloop *n64_loop = (ALADPCMloop *) &src[srcpos];
	ALADPCMloop *host_loop = (ALADPCMloop *) &dst[dstpos];

	dstpos += sizeof(ALADPCMloop);

	host_loop->start = PD_BE32(n64_loop->start);
	host_loop->end = PD_BE32(n64_loop->end);
	host_loop->count = PD_BE32(n64_loop->count);

	for (int i = 0; i < ARRAYCOUNT(host_loop->state); i++) {
		host_loop->state[i] = PD_BE16(n64_loop->state[i]);
	}

	return dstpos;
}

static uint32_t convertAudioAdpcmBook(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	ALADPCMBook *n64_book = (ALADPCMBook *) &src[srcpos];
	ALADPCMBook *host_book = (ALADPCMBook *) &dst[dstpos];

	dstpos += sizeof(ALADPCMBook);

	host_book->order = PD_BE32(n64_book->order);
	host_book->npredictors = PD_BE32(n64_book->npredictors);

	for (int i = 0; i < ARRAYCOUNT(host_book->book); i++) {
		host_book->book[i] = PD_BE16(n64_book->book[i]);
	}

	return dstpos;
}

static uint32_t convertAudioRawLoop(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	ALRawLoop *n64_loop = (ALRawLoop *) &src[srcpos];
	ALRawLoop *host_loop = (ALRawLoop *) &dst[dstpos];

	dstpos += sizeof(ALRawLoop);

	host_loop->start = PD_BE32(n64_loop->start);
	host_loop->end = PD_BE32(n64_loop->end);
	host_loop->count = PD_BE32(n64_loop->count);

	return dstpos;
}

static uint32_t convertAudioWaveTable(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_wavetable *n64_wavetable = (struct n64_wavetable *) &src[srcpos];
	ALWaveTable *host_wavetable = (ALWaveTable *) &dst[dstpos];

	dstpos += sizeof(ALWaveTable);

	host_wavetable->base = (void *)(uintptr_t)PD_BE32(n64_wavetable->base);
	host_wavetable->len = PD_BE32(n64_wavetable->len);
	host_wavetable->type = n64_wavetable->type;
	host_wavetable->flags = n64_wavetable->flags;

	if (host_wavetable->type == AL_ADPCM_WAVE) {
		if (n64_wavetable->waveInfo.adpcmWave.loop) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.adpcmWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.loop, convertAudioAdpcmLoop);
		} else {
			host_wavetable->waveInfo.adpcmWave.loop = NULL;
		}

		if (n64_wavetable->waveInfo.adpcmWave.book) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.adpcmWave.book);
			AL_NEXT_ITEM(host_wavetable->waveInfo.adpcmWave.book, convertAudioAdpcmBook);
		} else {
			host_wavetable->waveInfo.adpcmWave.book = NULL;
		}
	} else if (host_wavetable->type == AL_RAW16_WAVE) {
		if (n64_wavetable->waveInfo.rawWave.loop) {
			srcpos = PD_BE32(n64_wavetable->waveInfo.rawWave.loop);
			AL_NEXT_ITEM(host_wavetable->waveInfo.rawWave.loop, convertAudioRawLoop);
		} else {
			host_wavetable->waveInfo.rawWave.loop = NULL;
		}
	}

	return dstpos;
}

static uint32_t convertAudioSound(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_sound *n64_sound = (struct n64_sound *) &src[srcpos];
	ALSound *host_sound = (ALSound *) &dst[dstpos];

	dstpos += sizeof(ALSound);

	if (n64_sound->envelope) {
		srcpos = PD_BE32(n64_sound->envelope);
		AL_NEXT_ITEM(host_sound->envelope, convertAudioEnvelope);
	} else {
		host_sound->envelope = NULL;
	}

	if (n64_sound->keyMap) {
		srcpos = PD_BE32(n64_sound->keyMap);
		AL_NEXT_ITEM(host_sound->keyMap, convertAudioKeyMap);
	} else {
		host_sound->keyMap = NULL;
	}

	if (n64_sound->wavetable) {
		srcpos = PD_BE32(n64_sound->wavetable);
		AL_NEXT_ITEM(host_sound->wavetable, convertAudioWaveTable);
	} else {
		host_sound->wavetable = NULL;
	}

	host_sound->samplePan = n64_sound->samplePan;
	host_sound->sampleVolume = n64_sound->sampleVolume;
	host_sound->flags = n64_sound->flags;

	return dstpos;
}

static uint32_t convertAudioInstrument(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_instrument *n64_instrument = (struct n64_instrument *) &src[srcpos];
	ALInstrument *host_instrument = (ALInstrument *) &dst[dstpos];
	const int16_t soundCount = PD_BE16(n64_instrument->soundCount);

	host_instrument->volume = n64_instrument->volume;
	host_instrument->pan = n64_instrument->pan;
	host_instrument->priority = n64_instrument->priority;
	host_instrument->flags = n64_instrument->flags;
	host_instrument->tremType = n64_instrument->tremType;
	host_instrument->tremRate = n64_instrument->tremRate;
	host_instrument->tremDepth = n64_instrument->tremDepth;
	host_instrument->tremDelay = n64_instrument->tremDelay;
	host_instrument->vibType = n64_instrument->vibType;
	host_instrument->vibRate = n64_instrument->vibRate;
	host_instrument->vibDepth = n64_instrument->vibDepth;
	host_instrument->vibDelay = n64_instrument->vibDelay;
	host_instrument->bendRange = PD_BE16(n64_instrument->bendRange);
	host_instrument->soundCount = (soundCount);

	dstpos = dstpos + sizeof(ALInstrument) + sizeof(uintptr_t) * (soundCount - 1);

	for (int i = 0; i < soundCount; i++) {
		srcpos = PD_BE32(n64_instrument->soundArray[i]);
		AL_NEXT_ITEM(host_instrument->soundArray[i], convertAudioSound);
	}

	return dstpos;
}

static uint32_t convertAudioBank(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_bank *n64_bank = (struct n64_bank *) &src[srcpos];
	ALBank *host_bank = (ALBank *) &dst[dstpos];
	const int16_t instCount = PD_BE16(n64_bank->instCount);

	host_bank->instCount = (instCount);
	host_bank->flags = n64_bank->flags;
	host_bank->pad = n64_bank->pad;
	host_bank->sampleRate = PD_BE32(n64_bank->sampleRate);

	dstpos = dstpos + sizeof(ALBank) + sizeof(uintptr_t) * (instCount - 1);

	if (n64_bank->percussion) {
		srcpos = PD_BE32(n64_bank->percussion);
		AL_NEXT_ITEM(host_bank->percussion, convertAudioInstrument);
	} else {
		host_bank->percussion = 0;
	}

	for (int i = 0; i < instCount; i++) {
		srcpos = PD_BE32(n64_bank->instArray[i]);
		AL_NEXT_ITEM(host_bank->instArray[i], convertAudioInstrument);
	}

	return dstpos;
}

static uint32_t convertAudioBankFile(uint8_t *dst, uint8_t *src)
{
	struct n64_bankfile *n64_bankfile = (struct n64_bankfile *)src;
	ALBankFile *host_bankfile = (ALBankFile *)dst;
	const int16_t bankCount = PD_BE16(n64_bankfile->bankCount);

	host_bankfile->revision = PD_BE16(n64_bankfile->revision);
	host_bankfile->bankCount = (bankCount);

	uint32_t dstpos = sizeof(ALBankFile) + sizeof(uintptr_t) * (bankCount - 1);

	for (int i = 0; i < bankCount; i++) {
		host_bankfile->bankArray[i] = (void *)(uintptr_t)(dstpos);
		uint32_t srcpos = PD_BE32(n64_bankfile->bankArray[i]);
		dstpos = convertAudioBank(dst, dstpos, src, srcpos);
	}

	return dstpos;
}

uint8_t *preprocessALBankFile(uint8_t *src, uint32_t size, uint32_t *outSize)
{
	ptrReset();

	const uint32_t dstlen = size * 3; // this should overshoot any possible bank size, but * 2 also works for vanilla banks
	uint8_t *dst = sysMemZeroAlloc(dstlen);

	uint32_t reallen = convertAudioBankFile(dst, src);
	if (reallen > dstlen || ALIGN16(reallen) > dstlen) {
		sysFatalError("overflow when trying to preprocess an ALBankFile, size %u dstlen %u reallen %u", size, dstlen, reallen);
	}

	reallen = ALIGN16(reallen);

	if (reallen < dstlen) {
		dst = sysMemRealloc(dst, reallen);
	}

	*outSize = reallen;

	return dst;
}


uint8_t *preprocessALCMidiHdr(uint8_t *data, uint32_t size, uint32_t *outSize)
{
	ALCMidiHdr *hdr = (ALCMidiHdr *)data;
	PD_SWAP_VAL(hdr->division);
	for (int i = 0; i < ARRAYCOUNT(hdr->trackOffset); ++i) {
		PD_SWAP_VAL(hdr->trackOffset[i]);
	}
	return NULL;
}

uint8_t *preprocessSequences(uint8_t* data, uint32_t size, uint32_t *outSize)
{
	struct seqtable *seq = (struct seqtable *)data;
	PD_SWAP_VAL(seq->count);

	for (int16_t i = 0; i < seq->count; ++i) {
		PD_SWAP_VAL(seq->entries[i].binlen);
		PD_SWAP_VAL(seq->entries[i].ziplen);
		PD_SWAP_VAL(seq->entries[i].romaddr);
	}

	return NULL;
}
