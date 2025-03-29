#ifndef IN_GAME_MENUITEM_H
#define IN_GAME_MENUITEM_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

int menuitem0f0e5d2c(int arg0, struct menuitem *item);
int16_t menuitemListGetOffsetY(int16_t optionindex, struct menuitem *item);
Gfx *menuitemListRenderHeader(Gfx *gdl, int16_t x1, int16_t y1, int16_t width, int16_t arg4, int16_t height, char *text, struct menudialog *dialog);
Gfx *menuitemListOverlay(Gfx *gdl, int16_t x, int16_t y, int16_t x2, int16_t y2);
Gfx *menuitemListRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemListTick(struct menuitem *item, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);

void menuitemDropdownInit(struct menuitem *item, union menuitemdata *data);
Gfx *menuitemDropdownRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemDropdownTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);

Gfx *menuitemDropdownOverlay(Gfx *gdl, int16_t x, int16_t y, int16_t x2, int16_t y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data);
bool menuitemKeyboardIsStringEmptyOrSpaces(char *text);
Gfx *menuitemKeyboardRender(Gfx *gdl, struct menurendercontext *thing);
bool menuitemKeyboardTick(struct menuitem *item, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
void menuitemKeyboardInit(struct menuitem *item, union menuitemdata *data);

Gfx *menuitemSeparatorRender(Gfx *gdl, struct menurendercontext *context);

Gfx *menuitemObjectivesRenderOne(Gfx *gdl, struct menudialog *dialog, int index, int position, int16_t objx, int16_t objy, int16_t width, int16_t height, bool withstatus, bool narrow);
Gfx *menuitemObjectivesRender(Gfx *gdl, struct menurendercontext *context);

Gfx *menuitemModelRender(Gfx *gdl, struct menurendercontext *context);

Gfx *menuitemLabelRender(Gfx *gdl, struct menurendercontext *context);

Gfx *menuitemMeterRender(Gfx *gdl, struct menurendercontext *context);

#ifndef PLATFORM_N64
Gfx *menuitemColorBoxRender(Gfx *gdl, struct menurendercontext *context);
#endif

Gfx *menuitemSelectableRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemSelectableTick(struct menuitem *item, struct menuinputs *inputs, uint32_t tickflags);

Gfx *menuitemSliderRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemSliderTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
void menuitemSliderInit(union menuitemdata *data);

Gfx *menuitemCarouselRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemCarouselTick(struct menuitem *item, struct menuinputs *inputs, uint32_t tickflags);

Gfx *menuitemCheckboxRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemCheckboxTick(struct menuitem *item, struct menuinputs *inputs, uint32_t tickflags);

char *menuitemScrollableGetText(uint32_t type);
Gfx *menuitemScrollableRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemScrollableTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
void menuitemScrollableInit(union menuitemdata *data);

Gfx *menuitemMarqueeRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemMarqueeTick(struct menuitem *item, union menuitemdata *data);
void menuitemMarqueeInit(union menuitemdata *data);

Gfx *menuitem07Render(Gfx *gdl);

Gfx *menuitemRankingRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemRankingTick(struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
void menuitemRankingInit(union menuitemdata *data);

Gfx *menuitemPlayerStatsRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemPlayerStatsTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
Gfx *menuitemPlayerStatsOverlay(Gfx *gdl, int16_t x, int16_t y, int16_t x2, int16_t y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data);
void menuitemPlayerStatsInit(struct menuitem *item, union menuitemdata *data);

Gfx *menuitemControllerRenderLine(Gfx *gdl, int speed, int x1, int y1, int x2, int y2);
Gfx *menuitemControllerRenderTexture(Gfx *gdl, int x, int y, int texturenum, uint32_t alpha);
Gfx *menuitemControllerRenderLines(Gfx *gdl, struct menurendercontext *context, int firstindex, int lastindex, int padx, int pady, uint32_t alpha);
uint16_t menuitemControllerGetButtonAction(int mode, int buttonnum);
Gfx *menuitemControllerRenderText(Gfx *gdl, int curmode, struct menurendercontext *context, int padx, int pady, uint32_t valuecolour, uint32_t labelcolour, int8_t prevmode);
Gfx *menuitemControllerRenderPad(Gfx *gdl, struct menurendercontext *context, int padx, int pady, int curmode, uint32_t alpha, uint32_t colour1, uint32_t colour2, int8_t prevmode);
Gfx *menuitemControllerRender(Gfx *gdl, struct menurendercontext *context);
void menuitemControllerInit(union menuitemdata *data);

Gfx *menuitemRender(Gfx *gdl, struct menurendercontext *context);
bool menuitemTick(struct menuitem *item, struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags, union menuitemdata *data);
void menuitemInit(struct menuitem *item, union menuitemdata *data);
Gfx *menuitemOverlay(Gfx *gdl, int16_t x, int16_t y, int16_t x2, int16_t y2, struct menuitem *item, struct menudialog *dialog, union menuitemdata *data);

#ifndef PLATFORM_N64
int menuitemGetTop(struct menuitem *item, struct menudialog *dialog);
#endif

#endif
