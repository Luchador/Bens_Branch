#pragma once

#include "data.h"
#include "types.h"

void playermgrInit(void);
void playermgrReset(void);
void playermgrAllocatePlayers(int count);
void playermgrAllocatePlayer(int index);
void playermgrCalculateAiBuddyNums(void);
void setCurrentPlayerNum(int playernum);
int playermgrGetPlayerNumByProp(struct prop *prop);
void playermgrSetViewSize(int viewx, int viewy);
void playermgrSetViewPosition(int viewleft, int viewtop);
void playermgrSetFovY(float fovy);
void playermgrSetAspectRatio(float aspect);
int playermgrGetModelOfWeapon(int weapon);
void playermgrDeleteWeapon(int hand);
void playermgrCreateWeapon(int hand);
void playermgrShuffle(void);
int playermgrGetOrderOfPlayer(int playernum);
int playermgrGetPlayerAtOrder(int ordernum);