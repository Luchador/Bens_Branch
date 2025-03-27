#ifndef _IN_GAME_CHALLENGE_H
#define _IN_GAME_CHALLENGE_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

extern uint32_t g_MpChallengeIndex;
extern struct mpconfigfull *g_MpCurrentChallengeConfig;

void challengesInit(void);

void challengeDetermineUnlockedFeatures(void);
void challengePerformSanityChecks(void);
char *challengeGetNameBySlot(int slot);
bool challengeIsCompletedByAnyChrWithNumPlayersBySlot(int slot, int numplayers);
struct mpconfigfull *challengeLoadConfig(int confignum, uint8_t *buffer, int len);
int challengeForceUnlockFeature(int featurenum, uint8_t *array, int tail, int len);
int challengeForceUnlockSetupFeatures(struct mpsetup *mpsetup, uint8_t *array, int len);
void challengeForceUnlockConfigFeatures(struct mpconfig *config, uint8_t *array, int len, int challengeindex);
void challengeForceUnlockBotFeatures(void);
void challengeApply(void);
char *challengeGetCurrentDescription(void);
char *challengeGetConfigDescription(struct mpconfigfull *mpconfig);
bool challengeIsCompleteForEndscreen(void);
bool aiMpInitSimulants(void);
bool challengeIsAvailableToPlayer(int chrnum, int challengeindex);
bool challengeIsAvailableToAnyPlayer(int challengeindex);
int challengeGetNumAvailable(void);
char *challengeGetName(int challengeindex);
void challengeSetCurrentBySlot(int slotnum);
int challengeGetCurrent(void);
struct mpconfigfull *challengeLoad(int challengeindex, uint8_t *buffer, int len);
struct mpconfigfull *challengeLoadBySlot(int n, uint8_t *buffer, int len);
struct mpconfigfull *challengeLoadCurrent(uint8_t *buffer, int len);
void challengeRemoveForceUnlocks(void);
int challengeRemovePlayerLock(void);
void challengeLoadAndStoreCurrent(uint8_t *buffer, int len);
void challengeUnsetCurrent(void);
bool challengeIsLoaded(void);
int challengeGetAutoFocusedIndex(int mpchrnum);
char *challengeGetName2(int playernum, int challengeindex);
bool challengeIsCompletedByPlayerWithNumPlayers2(int mpchrnum, int index, int numplayers);
bool challengeIsCompletedByAnyPlayerWithNumPlayers(int index, int numplayers);
void challengeSetCompletedByAnyPlayerWithNumPlayers(int index, int numplayers, bool completed);
bool challengeIsCompletedByPlayerWithNumPlayers(int mpchrnum, int index, int numplayers);
void challengeSetCompletedByPlayerWithNumPlayers(uint32_t mpchrnum, int index, int numplayers, bool completed);
void challengeConsiderMarkingComplete(void);
bool challengeIsFeatureUnlocked(int feature);

#endif
