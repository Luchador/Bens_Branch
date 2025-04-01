#ifndef _IN_LIB_MAIN_H
#define _IN_LIB_MAIN_H
#include "data.h"

extern bool g_MainIsBooting;

void mainInit(void);
void mainProc(void);
void mainLoop(void);
void mainTick(void);
void mainEndStage(void);
void mainChangeToStage(int stagenum);
void mainFinalObjectiveCheck(void);
int mainGetStageNum(void);

#endif
