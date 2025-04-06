#pragma once

#include "data.h"
#include "types.h"

void modelmgrReset(void);
void modelmgrSetLvResetting(bool value);
void modelmgrAllocateSlots(int numobjs, int numchrs);
bool modelmgrLoadProjectileModeldefs(int weaponnum);
void playerInitEyespy(void);
void playerReset(void);