#pragma once

#include "data.h"
#include "types.h"

extern bool g_MainIsBooting;

void mainInit(void);
void mainProc(void);
void mainLoop(void);
void mainTick(void);
void mainEndStage(void);
void mainChangeToStage(int stagenum);
void mainFinalObjectiveCheck(void);
int mainGetStageNum(void);
