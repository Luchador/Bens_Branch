#ifndef _IN_LIB_LIB_317F0_H
#define _IN_LIB_LIB_317F0_H
#include "data.h"
#include "types.h"

void func00033090(struct sndstate *handle);
void func00033100(struct sndstate *state);
uint16_t sndpCountStates(int16_t *numfreeptr, int16_t *numallocedptr);
void sndpSetAddRefCallback(void *fn);
struct sndstate *func00033390(int arg0, ALSound *sound);
void sndpSetRemoveRefCallback(void *fn);
int sndGetState(struct sndstate *handle);
struct sndstate *func00033820(int arg0, int16_t soundnum, uint16_t vol, ALPan pan, float pitch, uint8_t fxmix, uint8_t fxbus, struct sndstate **handleptr);
void audioStop(struct sndstate *handle);
void func00033bc0(struct sndstate *handle);
void func00033db0(void);
void func00033dd8(void);
void audioPostEvent(struct sndstate *handle, int16_t type, int data);
uint16_t func00033ec4(uint8_t index);
struct sndstate *sndpGetHeadState(void);
ALMicroTime sndpGetCurTime(void);
void func00033f44(uint8_t index, uint16_t volume);

#endif
