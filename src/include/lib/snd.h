#ifndef _IN_LIB_SND_H
#define _IN_LIB_SND_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

bool sndIsFiltered(int audioid);
bool sndIsPlayingMp3(void);
uint16_t snd0000e9dc(void);
void sndSetSfxVolume(uint16_t volume);
void snd0000ea80(uint16_t volume);
void sndResetCurMp3(void);
void sndLoadSfxCtl(void);
void sndIncrementAges(void);
ALEnvelope *sndLoadEnvelope(uintptr_t offset, uint16_t index);
ALKeyMap *sndLoadKeymap(uintptr_t offset, uint16_t index);
ALADPCMBook* sndLoadAdpcmBook(uintptr_t offset, uint16_t index);
ALADPCMloop* sndLoadAdpcmLoop(uintptr_t offset, uint16_t index);
ALWaveTable* sndLoadWavetable(uintptr_t offset, uint16_t index);
void sndSetSoundMode(int mode);
ALSound *sndLoadSound(int16_t soundnum);
void seqInit(struct seqinstance *seq);
void sndInit(void);
bool sndIsMp3(int16_t soundnum);
bool sndStopMp3(int16_t arg0);
bool seqPlay(struct seqinstance *seq, int tracknum);
uint16_t seqGetVolume(struct seqinstance *seq);
void seqSetVolume(struct seqinstance *seq, uint16_t volume);
void snd0000fe20(void);
void snd0000fe50(void);
void sndTick(void);
bool sndIsDisabled(void);
void sndStartMp3ByFilenum(uint32_t filenum);
void sndAdjust(struct sndstate **handle, bool ismp3, int vol, int pan, int soundnum, float pitch, int fxbus, int fxmix, bool forcefxmix);
struct sndstate *snd00010718(struct sndstate **handle, int flags, int volume, int pan, int soundnum, float pitch, int fxbus, int fxmix, bool forcefxmix);
struct sndstate *sndStart(int arg0, int16_t sound, struct sndstate **handle, int volume, int pan, float pitch, int fxbus, int fxmix);
void sndStartMp3(int16_t soundnum, int volume, int pan, int responseflags);
void sndPlayNosedive(int seconds);
void sndStopNosedive(void);
void sndTickNosedive(void);
void sndPlayUfo(int seconds);
void sndStopUfo(void);
void sndTickUfo(void);

#endif
