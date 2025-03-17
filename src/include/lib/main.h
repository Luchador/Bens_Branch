#ifndef _IN_LIB_MAIN_H
#define _IN_LIB_MAIN_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern s32 g_MainIsBooting;

void mainInit(void);
void mainProc(void);
void mainLoop(void);
void mainTick(void);
void mainEndStage(void);
void mainChangeToStage(s32 stagenum);
void mainFinalObjectiveCheck(void);
s32 mainGetStageNum(void);

#endif
