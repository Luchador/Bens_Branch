#pragma once

#include "data.h"
#include "types.h"

int musicHandlePlayEvent(struct musicevent *event, int result);
int musicHandleStopEvent(struct musicevent *event, int result);
int musicHandleFadeEvent(struct musicevent *event, int result);
int musicHandleStopAllEvent(int result);
int musicHandleSetIntervalEvent(struct musicevent *event, int result);
void musicTickEvents(void);
void musicTick(void);
