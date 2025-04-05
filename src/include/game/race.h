#pragma once

#include "data.h"
#include "types.h"

void racesInit(void);

uint16_t raceGetAnimSumAngleAsInt(int16_t animnum, int frame, int endframe);
int raceGetAnimSumForwardAsInt(int16_t animnum, int startframe, int endframe);
int raceInitAnimGroup(struct attackanimconfig *configs);
void raceInitAnimGroups(struct attackanimgroup **groups);
int raceCountAnims(struct animtablerow *rows);
float race0f0005c0(int16_t animnum);
void raceInitAnims(void);
