#ifndef _IN_LIB_MUSIC_H
#define _IN_LIB_MUSIC_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

int musicHandlePlayEvent(struct musicevent *event, int result);
int musicHandleStopEvent(struct musicevent *event, int result);
int musicHandleFadeEvent(struct musicevent *event, int result);
int musicHandleStopAllEvent(int result);
int musicHandleSetIntervalEvent(struct musicevent *event, int result);
void musicTickEvents(void);
void musicTick(void);

#endif
