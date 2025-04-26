#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct RGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

void gfx_Set_Prim_Color(Gfx *pkt, RGBA color);

extern float g_ModelViewProj[4][4];

extern struct RGBA g_FillColor;
extern struct RGBA g_FogColor;

#ifdef __cplusplus
}
#endif
