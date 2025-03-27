#ifndef _IN_GAME_TITLE_H
#define _IN_GAME_TITLE_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

#define TITLE_ALLOCSIZE 1024 * 378

void titleInit(void);

void titleReset(void);

Gfx *titleClear(Gfx *gdl);

char *mpPlayerGetWeaponOfChoiceName(unsigned int playernum, unsigned int slot);
void titleSetLight(Lights1 *light, int8_t r, int8_t g, int8_t b, float luminosity, struct coord *dir);
void titleInitLegal(void);
void titleTickLegal(void);
Gfx *titleRenderLegal(Gfx *gdl);
void titleInitPdLogo(void);
void titleExitPdLogo(void);
void titleTickPdLogo(void);
Gfx *titleRenderPdLogoModel(Gfx *gdl, struct model *model, int arg2, float arg3, int arg4, float arg5, Mtxf *arg6, Vtx *vertices, Col *colours);
void titleSkipToPdTitle(void);
Gfx *titleRenderPdLogo(Gfx *gdl);
void titleInitNintendoLogo(void);
void titleExitNintendoLogo(void);
void titleTickNintendoLogo(void);
Gfx *titleRenderNintendoLogo(Gfx *gdl);
void titleInitRareLogo(void);
void titleExitRareLogo(void);
void titleTickRareLogo(void);
float titleRotateClockwise(float arg0);
Gfx *titleRenderRareLogo(Gfx *gdl);
void titleInitSkip(void);
void titleSetNextMode(int mode);
void titleTick(void);
bool titleIsChangingMode(void);
bool titleIsKeepingMode(void);
void titleExit(void);
void titleInitFromAiCmd(unsigned int arg0);
bool func0f01ad5c(void);
Gfx *titleRender(Gfx *gdl);
int getNumPlayers(void);
void setNumPlayers(int numplayers);
void titleSetNextStage(int stagenum);

#endif
