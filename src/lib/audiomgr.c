#include <ultra64.h>
#include <stdint.h>
#include "game/utils.h"
#include "lib/sched.h"
#include "naudio/n_synthInternals.h"
#include "constants.h"
#include "bss.h"
#include "lib/audiodma.h"
#include "lib/lib_2fc60.h"
#include "data.h"
#include "types.h"
#include "audio.h"
#include "game/debug.h"

uint64_t g_AmgrMaxFrameTime240; // Stores the maximum frame time seen so far in the current 240-frame window
uint64_t g_AmgrTotalFrameTime240; // Accumulates the difference between g_AmgrElapsedGameTime and g_AmgrElapsedGameTime2 once per frame
uint64_t g_AmgrElapsedGameTime;
uint64_t g_AmgrElapsedGameTime2;
AMAudioMgr g_AudioManager;
uint32_t g_AmgrFreqPerTick;
int var800918e8;

uint32_t g_AudioCmdListIndex = 0x00000000;
uint8_t g_AudioFrameDownsampleCounter = 1;

#define AUDIO_BUFFER_THRESHOLD 1100
#define AUDIO_FRAME_SAMPLES_LOW 184
#define AUDIO_FRAME_SAMPLES_HIGH 368

void amgrCreate(ALSynConfig *config)
{
	float freqpertick;
	int i;

	config->outputRate = 22020;
	config->dmaproc = admaNew;
	freqpertick = config->outputRate / 30.0f;
	g_AmgrFreqPerTick = (int)freqpertick;

	if ((float)g_AmgrFreqPerTick < freqpertick) {
		g_AmgrFreqPerTick++;
	}

	g_AmgrFreqPerTick = g_AmgrFreqPerTick / SAMPLES * SAMPLES + SAMPLES;
	g_AudioFrameDownsampleCounter = 0;

	for (i = 0; i < ARRAYCOUNT(g_AudioManager.ACMDList); i++) {
		g_AudioManager.ACMDList[i] = alHeapAlloc(&g_SndHeap, 1, 2000 * sizeof(Acmd));
	}

	for (i = 0; i < ARRAYCOUNT(g_AudioManager.audioInfo); i++) {
		g_AudioManager.audioInfo[i] = alHeapAlloc(&g_SndHeap, 1, sizeof(AudioInfo));
		g_AudioManager.audioInfo[i]->frameSamples = 0;
		g_AudioManager.audioInfo[i]->data = alHeapAlloc(&g_SndHeap, 1, 1024 * 3);
	}

#ifndef AVOID_UB // these will be used after this scope ends, triggering a big boom
	{
#endif
		int sp590[] = { 0x00000001, 0x000014a0, 0x00000000, 0x00001358, 0x00004d0c, 0x00000000, 0x000053ff, 0x00000000, 0x00000000, 0x00000000 };
		int sp568[] = { 0x00000001, 0x000001b8, 0x00000000, 0x00000068, 0x00004000, 0x00000000, 0x00007fff, 0x00001db0, 0x00001b58, 0x00000000 };
		int sp540[] = { 0x00000001, 0x000001b8, 0x00000000, 0x00000068, 0x00000000, 0x00005fff, 0x00007fff, 0x0000017c, 0x000001f4, 0x00000000 };
		int sp478[] = { 0x00000006, 0x00001868, 0x00000000, 0x00000160, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000188, 0x00000640, 0x0000235e, 0xffffd99a, 0x0000750c, 0x00000000, 0x00000000, 0x00000bca, 0x00000318, 0x000009f8, 0x00004000, 0xffffc000, 0x00006d78, 0x00000000, 0x00000000, 0x00001286, 0x00000c78, 0x000015d8, 0x0000521a, 0xffffc000, 0x0000724f, 0x00000000, 0x00000000, 0x00001650, 0x00000d28, 0x000012c0, 0x00002143, 0xffffe000, 0x00005de4, 0x00000000, 0x00000000, 0x00002286, 0x00000000, 0x00001720, 0x000032c8, 0xffffcd38, 0x00000000, 0x00000000, 0x00000000, 0x00004500 };
		int sp430[] = { 0x00000002, 0x000008b0, 0x00000600, 0x00000760, 0x00007142, 0x00000000, 0x00005bff, 0x00000000, 0x00000000, 0x00007bc9, 0x00000000, 0x00000528, 0x00005f27, 0xffffb288, 0x00007ef1, 0x00000000, 0x00000001, 0x000066bb };
		int sp3c8[] = { 0x00000003, 0x00000b40, 0x00000000, 0x00000160, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000188, 0x00000640, 0x0000235e, 0xffffd99a, 0x000016f2, 0x00000000, 0x00000000, 0x00000bca, 0x00000318, 0x000009f8, 0x00004000, 0xffffc000, 0x0000186b, 0x00000000, 0x00000000, 0x00001286 };
		int sp360[] = { 0x00000003, 0x00000b40, 0x00000000, 0x00000160, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000188, 0x00000640, 0x0000235e, 0xffffd99a, 0x000016f2, 0x00000000, 0x00000000, 0x00000bca, 0x00000318, 0x000009f8, 0x00004000, 0xffffc000, 0x0000186b, 0x00000000, 0x00000000, 0x00001286 };
		int sp2f8[] = { 0x00000003, 0x00000898, 0x00000000, 0x000004a0, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000001a0, 0x00000340, 0x00000ccc, 0xfffff334, 0x00003fff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000528, 0x00001388, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00005000 };
		int sp270[] = { 0x00000004, 0x00000898, 0x00000000, 0x000005a8, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000001e0, 0x000004a0, 0x00000ccc, 0xfffff334, 0x00003fff, 0x00000000, 0x00000000, 0x00000000, 0x000005a8, 0x000007d0, 0x00000ccc, 0xfffff334, 0x00003fff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000810, 0x00001f40, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00005000 };
		int sp248[] = { 0x00000001, 0x00001130, 0x00000000, 0x00000f60, 0x00002ee0, 0x00000000, 0x00007fff, 0x00000000, 0x00000000, 0x00000000 };
		int sp1c0[] = { 0x00000004, 0x00000e98, 0x000000c0, 0x00000188, 0x00002666, 0xffffd99a, 0x00003484, 0x00000000, 0x00000000, 0x00000000, 0x000001b8, 0x00000580, 0x00004000, 0xffffc000, 0x000019eb, 0x00000000, 0x00000000, 0x00000000, 0x00000a50, 0x00000b98, 0x00002000, 0xffffe000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000cb8, 0x00004650, 0xffffb9b0, 0x00000000, 0x0000017c, 0x0000000a, 0x00000000 };
		int sp198[] = { 0x00000001, 0x00000528, 0x00000000, 0x00000448, 0x00003334, 0x00000000, 0x00007335, 0x00000000, 0x00000000, 0x00000000 };
		int sp090[] = { 0x00000008, 0x00001b80, 0x00000000, 0x000000c0, 0x00002666, 0xffffd99a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000000c0, 0x00000188, 0x00002666, 0xffffd99a, 0x00002b84, 0x00000000, 0x00000000, 0x00000000, 0x00000370, 0x00000b00, 0x00004000, 0xffffc000, 0x000011eb, 0x00000000, 0x00000000, 0x00000000, 0x00000420, 0x00000840, 0x00002000, 0xffffe000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000dc0, 0x00001810, 0x00004000, 0xffffc000, 0x000011eb, 0x00000000, 0x00000000, 0x00000000, 0x00000e70, 0x000014a0, 0x00002000, 0xffffe000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000014a0, 0x00001738, 0x00002000, 0xffffe000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001970, 0x000032c8, 0xffffcd38, 0x00000000, 0x0000017c, 0x0000000a, 0x00000000 };
		int sp068[] = { 0x00000001, 0x00000a50, 0x00000000, 0x00000898, 0x00003334, 0x00000000, 0x00007335, 0x00000000, 0x00000000, 0x00000000 };
		int sp040[] = { 0x00000001, 0x00000148, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

		config->params[0] = (int *) (sp090);

		if (g_SndMaxFxBusses >= 2) {
			for (i = 1; i < g_SndMaxFxBusses; i++) {
				config->params[i] = (int *) (sp068);
			}
		}
#ifndef AVOID_UB
	}
#endif

	n_alInit(&g_AudioManager.g, config);
	func00030bfc(0, 60);
}

extern uint32_t g_AdmaCurFrame;

void amgrFrame(void)
{
	static AudioInfo *previnfo = NULL;
	static int frameWindowCounter = 0;

	g_AmgrElapsedGameTime = utilsGetCount();

	AudioInfo *info = g_AudioManager.audioInfo[g_AdmaCurFrame % 3];

	admaBeginFrame();

	const int somevalue = audioGetBytesBuffered() / 4;
	Acmd *datastart = g_AudioManager.ACMDList[g_AudioCmdListIndex];
	int16_t *outbuffer = (int16_t *) (uintptr_t)(info->data);

	if (previnfo) {
		audioSetNextBuffer(previnfo->data, previnfo->frameSamples * 4);
	}

	if (somevalue > AUDIO_BUFFER_THRESHOLD && g_AudioFrameDownsampleCounter == 0) {
		// If over 1100 samples are buffered, reduce CPU use by rendering a single frame
		info->frameSamples = AUDIO_FRAME_SAMPLES_LOW;
		g_AudioFrameDownsampleCounter = 2;
	} else {
		// have space in audio queue, render 2 naudio frames this frame
		info->frameSamples = AUDIO_FRAME_SAMPLES_HIGH;

		if (g_AudioFrameDownsampleCounter != 0) {
			g_AudioFrameDownsampleCounter--;
		}
	}

	Acmd *cmd = n_alAudioFrame(datastart, &var800918e8, outbuffer, info->frameSamples);

	g_AudioCmdListIndex ^= 1;

	previnfo = info;

	//frameWindowCounter++;
	frameWindowCounter = (frameWindowCounter + 1) % 240; // Avoid overflow

	g_AmgrElapsedGameTime2 = utilsGetCount();

	//if (frameWindowCounter % 240 == 0) {
	if (frameWindowCounter == 0) {
		g_AmgrTotalFrameTime240 = 0; g_AmgrMaxFrameTime240 = 0;
	} else {
		g_AmgrTotalFrameTime240 = (g_AmgrTotalFrameTime240 + g_AmgrElapsedGameTime2) - g_AmgrElapsedGameTime;
	}

	if (g_AmgrMaxFrameTime240 < g_AmgrElapsedGameTime2 - g_AmgrElapsedGameTime) {
		g_AmgrMaxFrameTime240 = g_AmgrElapsedGameTime2 - g_AmgrElapsedGameTime;
	}
}
