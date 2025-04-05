#pragma once

#include "data.h"
#include "types.h"

extern struct menudialogdef g_MpScenarioMenuDialog;
extern struct menudialogdef g_MpQuickTeamScenarioMenuDialog;

struct mpscenariooverview {
	uint16_t name;
	uint16_t shortname;
	uint8_t requirefeature;
	uint8_t teamonly;
};

extern struct mpscenariooverview g_MpScenarioOverviews[6];

MenuItemHandlerResult menuhandlerMpOpenOptions(int operation, struct menuitem *item, union handlerdata *data);
void scenarioReadSave(struct savebuffer *buffer);
void scenarioWriteSave(struct savebuffer *buffer);
void scenarioInit(void);
int scenarioNumProps(void);
void scenarioInitProps(void);
void scenarioTick(void);
void scenarioTickChr(struct chrdata *chr);
Gfx *scenarioRadarExtra(Gfx *gdl);
bool scenarioRadarChr(Gfx **gdl, struct prop *prop);
float scenarioChooseSpawnLocation(float chrradius, struct coord *pos, RoomNum *rooms, struct prop *prop);
int scenarioGetMaxTeams(void);
void scenarioHighlightRoom(RoomNum room, int *arg1, int *arg2, int *arg3);
