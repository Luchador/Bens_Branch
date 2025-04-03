#include <PR/ultratypes.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include "platform.h"
#include "config.h"
#include "audio.h"
#include "system.h"

static SDL_AudioDeviceID dev;
static const int16_t *nextBuf;
static uint32_t nextSize = 0;

static int bufferSize = 512;
static int queueLimit = 8192;

int audioInit(void)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		sysLogPrintf(LOG_ERROR, "SDL audio init error: %s", SDL_GetError());
		return -1;
	}

	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 22020; // TODO: this might cause trouble for some platforms
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = bufferSize;
	want.callback = NULL;

	nextBuf = NULL;

	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (dev == 0) {
		sysLogPrintf(LOG_ERROR, "SDL_OpenAudio error: %s", SDL_GetError());
		return -1;
	}

	SDL_PauseAudioDevice(dev, 0);

	return 0;
}

int audioGetBytesBuffered(void)
{
	return SDL_GetQueuedAudioSize(dev);
}

int audioGetSamplesBuffered(void)
{
	return audioGetBytesBuffered() / 4;
}

void audioSetNextBuffer(const int16_t *buf, uint32_t len)
{
	nextBuf = buf;
	nextSize = len;
}

void audioEndFrame(void)
{
	if (nextBuf && nextSize) {
		if (audioGetSamplesBuffered() < queueLimit) {
			SDL_QueueAudio(dev, nextBuf, nextSize);
		}
		nextBuf = NULL;
		nextSize = 0;
	}
}

PD_CONSTRUCTOR static void audioConfigInit(void)
{
	configRegisterInt("Audio.BufferSize", &bufferSize, 0, 1 * 1024 * 1024);
	configRegisterInt("Audio.QueueLimit", &queueLimit, 0, 1 * 1024 * 1024);
}
