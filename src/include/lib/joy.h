#ifndef _IN_LIB_JOY_H
#define _IN_LIB_JOY_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void joyLockCyclicPolling(void);
void joyUnlockCyclicPolling(void);
void joySetPfsPollEnabled(bool enabled);
bool joyIsPfsPollEnabled(void);
void joySetPfsPollInterval(s32 value);
void joySetDefaultPfsPollInterval(void);
s32 joyShiftPfsStates(void);
void joyRecordPfsState(u8 pfsstate);
void joyPollPfs(s32 force);
void joySetPfsTemporarilyPlugged(s8 index);
void joyInit(void);
void joyDisableTemporarily(void);
void joyReset(void);
void joy00013e84(void);
u32 joyGetConnectedControllers(void);
void joy00014238(void);
void joyDebugJoy(void);
void joyReadData(void);
void joySetAllowTitleInput(bool value); // Determines if the player can press a button to skip part of the intro or has to wait
s32 joyGetNumSamples(void);
s32 joyGetRStickXOnSample(s32 samplenum, s8 contpadnum);
s32 joyGetRStickYOnSample(s32 samplenum, s8 contpadnum);
s32 joyGetStickXOnSample(s32 samplenum, s8 contpadnum);
s32 joyGetStickYOnSample(s32 samplenum, s8 contpadnum);
s32 joyGetStickYOnSampleIndex(s32 samplenum, s8 contpadnum);
s32 joyGetRStickYOnSampleIndex(s32 samplenum, s8 contpadnum);
u32 joyGetButtonsOnSample(s32 samplenum, s8 contpadnum, u32 mask);
u32 joyGetButtonsPressedOnSample(s32 samplenum, s8 contpadnum, u32 mask);
s32 joyCountButtonsOnSpecificSamples(u32 *arg0, s8 contpadnum, u32 mask);
s8 joyGetStickX(s8 contpadnum);
s8 joyGetStickY(s8 contpadnum);
s8 joyGetRStickX(s8 contpadnum);
s8 joyGetRStickY(s8 contpadnum);
u32 joyGetButtons(s8 contpadnum, u32 mask);
u32 joyGetButtonsPressedThisFrame(s8 contpadnum, u32 mask);
bool joyIsCyclicPollingEnabled(void);
void joyDisableCyclicPolling(void);
void joyEnableCyclicPolling(void);
void joyDestroy(void);
void joyGetContpadNumsForPlayer(s8 playernum, s32 *pad1, s32 *pad2);
void joyStopRumble(s8 device, bool disablepolling);
s32 joyGetPakState(s8 device);
s32 joyGetPakState2(s8 device);
void joysTickRumble(void);

#endif
