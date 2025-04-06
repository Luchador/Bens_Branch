#pragma once

#include "data.h"
#include "types.h"

uint8_t hudmsgsAreActive(void);
int hudmsgIsZoomRangeVisible(void);
Gfx *hudmsgRenderMissionTimer(Gfx *gdl, uint32_t alpha);
Gfx *hudmsgRenderZoomRange(Gfx *gdl, uint32_t alpha);
Gfx *hudmsgRenderBox(Gfx *gdl, int x1, int y1, int x2, int y2, float bgopacity, uint32_t bordercolour, float textopacity);
int hudmsgCalcXPos(int *arg0, int arg1);
void hudmsgsHideByChannel(int value);
void hudmsgsReset(void);
void hudmsgRemoveAll(void);
int hudmsgGetNext(int refid);
void hudmsgCreate(char *text, int type);
void hudmsgCreateWithFlags(char *text, int type, uint32_t flags);
void hudmsgCreateWithColour(char *text, int type, uint8_t colour);
void hudmsgCreateWithDuration(char *text, int type, struct hudmsgtype *config, int duration60);
void hudmsgCreateAsSubtitle(char *text, int type, uint8_t colourindex, int audiochannelnum);
void hudmsgCalculatePosition(struct hudmessage *msg);
void hudmsgCreateFromArgs(char *text, int type, int conf00, int conf01, int conf02,
		struct fontchar **conf04, struct font **conf08,
		uint32_t textcolour, uint32_t shadowcolour,
		uint32_t alignh, int conf16, uint32_t alignv, int conf18, int arg14, uint32_t flags);
void hudmsgsTick(void);
void hudmsgsSetOn(uint32_t reason);
void hudmsgsSetOff(uint32_t reason);
void hudmsgsRemoveForDeadPlayer(int playernum);
Gfx *hudmsgsRender(Gfx *gdl);
void hudmsgsStop(void);