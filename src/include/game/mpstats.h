#ifndef IN_GAME_MPSTATS_H
#define IN_GAME_MPSTATS_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void mpstatsIncrementPlayerShotCount(struct gset *gset, int region);
void mpstatsIncrementPlayerShotCount2(struct gset *gset, int region);
void mpstatsResetShotsShouldCount(void);
int mpstatsGetPlayerShotCountByRegion(uint32_t type);
void mpstatsIncrementTotalKillCount(void);
void mpstatsIncrementTotalKnockoutCount(void);
void mpstatsDecrementTotalKnockoutCount(void);
uint8_t mpstatsGetTotalKnockoutCount(void);
uint32_t mpstatsGetTotalKillCount(void);
void mpstatsRecordPlayerKill(void);
int mpstatsGetPlayerKillCount(void);
void mpstatsRecordPlayerDeath(void);
void mpstatsRecordPlayerSuicide(void);
void mpstatsRecordDeath(int aplayernum, int vplayernum);

#endif
