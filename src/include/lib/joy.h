#ifndef _IN_LIB_JOY_H
#define _IN_LIB_JOY_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void joyLockCyclicPolling(void);
void joyUnlockCyclicPolling(void);
void joySetPfsPollEnabled(bool enabled);
bool joyIsPfsPollEnabled(void);
void joySetPfsPollInterval(int value);
void joySetDefaultPfsPollInterval(void);
int joyShiftPfsStates(void);
void joyRecordPfsState(int8_t pfsstate);
void joyPollPfs(int force);
void joyInit(void);
void joyDisableTemporarily(void);
void joyReset(void);
void joyUpdateConnectionStatus(void);
unsigned int joyGetConnectedControllers(void);
void joyProcessPakState(void);
void joyDebugJoy(void);
void joyReadData(void);
void joySetAllowTitleInput(bool value); // Determines if the player can press a button to skip part of the intro or has to wait
int joyGetNumSamples(void);
int joyGetRStickXOnSample(int samplenum, int8_t contpadnum);
int joyGetRStickYOnSample(int samplenum, int8_t contpadnum);
int joyGetStickXOnSample(int samplenum, int8_t contpadnum);
int joyGetStickYOnSample(int samplenum, int8_t contpadnum);
int joyGetStickYOnSampleIndex(int samplenum, int8_t contpadnum);
int joyGetRStickYOnSampleIndex(int samplenum, int8_t contpadnum);
unsigned int joyGetButtonsOnSample(int samplenum, int8_t contpadnum, unsigned int mask);
unsigned int joyGetButtonsPressedOnSample(int samplenum, int8_t contpadnum, unsigned int mask);
int joyCountButtonsOnSpecificSamples(unsigned int *checksamples, int8_t contpadnum, unsigned int mask);
int8_t joyGetStickX(int8_t contpadnum);
int8_t joyGetStickY(int8_t contpadnum);
int8_t joyGetRStickX(int8_t contpadnum);
int8_t joyGetRStickY(int8_t contpadnum);
unsigned int joyGetButtons(int8_t contpadnum, unsigned int mask);
unsigned int joyGetButtonsPressedThisFrame(int8_t contpadnum, unsigned int mask);
bool joyIsCyclicPollingEnabled(void);
void joyDisableCyclicPolling(void);
void joyEnableCyclicPolling(void);
void joyDestroy(void);
void joyGetContpadNumsForPlayer(int8_t playernum, int *pad1, int *pad2);
void joyStopRumble(int8_t device, bool disablepolling);
int joyGetPakState(int8_t device);
int joyGetPakState2(int8_t device);
void joysTickRumble(void);

#endif
