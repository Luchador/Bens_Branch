#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MAX_BUFFERED 256
#define MAX_LIGHTS 4
#define MAX_VERTICES 128
#define MAX_VERTEX_COLORS 64

typedef struct RGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

struct NormalColor {
    union {
        struct { uint8_t r, g, b, a; };
        struct { int8_t x, y, z, w; };
    };
};

struct LoadedVertex {
    float x, y, z, w;
    float u, v;
    struct RGBA color;
    uint8_t fog;
    uint8_t clip_rej;
};

typedef struct {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float v[3];
	};
	uint8_t flags;
	uint8_t colour;
	float s;
	float t;
} VtxF;

struct RSP {
    float modelview_matrix_stack[11][4][4];
    uint8_t modelview_matrix_stack_size;

    float MP_matrix[4][4];
    float P_matrix[4][4];

    Light_t lookat[2];
    bool lookat_enabled;

    Light_t current_lights[MAX_LIGHTS + 1];
    float current_lights_coeffs[MAX_LIGHTS][3];
    float current_lookat_coeffs[2][3]; // lookat_x, lookat_y
    uint8_t current_num_lights;        // includes ambient light
    bool lights_changed;

    uint32_t geometry_mode;
    int16_t fog_mul, fog_offset;

    uint32_t extra_geometry_mode;

    uint32_t aspect_mode;
    float aspect_ofs;
    float aspect_scale;

    struct {
        float s, t;
    } texture_scaling_factor;

    struct LoadedVertex loaded_vertices[MAX_VERTICES + 4];

    const struct NormalColor *vertex_colors; //[MAX_VERTEX_COLORS];
};

extern struct RSP rsp;

/* Extended commands */

#define G_SETFB_EXT                  0x21
#define G_SETTIMG_FB_EXT             0x23
#define G_INVALTEXCACHE_EXT          0x34
#define G_FILLRECT_WIDE_EXT          0x38
#define G_EXTRAGEOMETRYMODE_EXT      0x3a
#define G_COPYFB_EXT                 0x41
#define G_IMAGERECT_EXT              0x42
#define G_RDPFLUSH_EXT               0x43
#define G_CLEAR_DEPTH_EXT            0x44
#define G_SETSUBPIXELOFFSET_EXT      0x45

/* G_EXTRAGEOMETRYMODE flags */

#define G_ASPECT_LEFT_EXT        0x00000010
#define G_ASPECT_RIGHT_EXT       0x00000020
#define G_ASPECT_WIDE_EXT        0x00000040
#define G_ASPECT_CENTER_EXT      (G_ASPECT_LEFT_EXT | G_ASPECT_RIGHT_EXT)
#define G_ASPECT_MODE_EXT        (G_ASPECT_CENTER_EXT | G_ASPECT_WIDE_EXT)
#define G_NO_CLIPPING_EXT        0x00000100
#define G_MODULATE_EXT           0x00000200 // this should really go into OTHERMODE_H, but for some reason I can't get it to work

/* Extra texture filtering mode */

#define G_TF_BLUR_EXT (1 << G_MDSFT_TEXTFILT)

void gfx_Immp1(Gfx *pkt, uint8_t command, uint32_t param);
void gfx_Immp21(Gfx *pkt, uint8_t command, uint16_t p0, uint8_t p1, uintptr_t data);
void gfx_Dma1p(Gfx *pkt, uint8_t command, uintptr_t src, uint16_t length, uint8_t param);
void gfx_MoveWord(Gfx *pkt, uint8_t index, uint16_t offset, uintptr_t data);
void gfx_Segment(Gfx *pkt, uint8_t segment, uintptr_t base);
void gfx_Num_Lights(Gfx *pkt, uint32_t numlights);
void gfx_Light(Gfx *pkt, const Light *light, uint32_t n);
void gfx_Set_Lights1(Gfx *pkt, const Lights1 *lights);
void gfx_Matrix(Gfx *pkt, const Mtx *matrix, uint32_t flags);
void gfx_Pop_Matrix(Gfx *pkt, uint32_t count);
void gfx_Clear_Geometry_Mode(Gfx *pkt, uint32_t word);
void gfx_Set_Geometry_Mode(Gfx *pkt, uint32_t word);
void gfx_Extra_Geometry_Mode_EXT(Gfx *pkt, uint32_t clearbits, uint32_t setbits);
void gfx_Vertex(Gfx *pkt, const Vtx *v, uint8_t n, uint8_t v0);
void gfx_VertexF(Gfx *pkt, const VtxF *v, uint8_t n, uint8_t v0);
void gfx_1Triangle(Gfx *pkt, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t flag);
void gfx_Tri4(Gfx *pkt, uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, uint8_t x3, uint8_t y3, uint8_t z3, uint8_t x4, uint8_t y4, uint8_t z4);
void gfx_Tri3(Gfx *pkt, uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, uint8_t x3, uint8_t y3, uint8_t z3);
void gfx_Tri2(Gfx *pkt, uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2);
void gfx_Tri1(Gfx *pkt, uint8_t x1, uint8_t y1, uint8_t z1);
int  gfx_Fill_Rectangle(Gfx *pkt, float ulx, float uly, float lrx, float lry);
int  gfx_HUD_Rectangle(Gfx *pkt, float x1, float y1, float x2, float y2);
int  gfx_Texture_Rectangle(Gfx *pkt, float xl, float yl, float xh, float yh, uint8_t tile, float s, float t, float dsdx, float dtdy, bool flip);
int  gfx_Image_Rectangle_EXT(Gfx *pkt, float x0, float y0, float s0, float t0, float x1, float y1, float s1, float t1, float iw, float ih);
void gfx_Set_Subpixel_Offset_EXT(Gfx *pkt, int16_t x, int16_t y);
void gfx_Color(Gfx *pkt, const Col *colors, uint32_t count);
void gfx_Set_Prim_Color(Gfx *pkt, RGBA color);
void gfx_Set_Fill_Color(Gfx *pkt, RGBA color);
void gfx_Set_Env_Color(Gfx *pkt, RGBA color);
void gfx_Set_Fog_Color(Gfx *pkt, RGBA color);
void gfx_Texture(Gfx *pkt, uint16_t s, uint16_t t, uint8_t level, uint8_t tile, uint8_t on);
void gfx_Copy_Framebuffer_EXT(Gfx *pkt, uint16_t dst, uint16_t src, uint16_t uls, uint16_t ult, uint8_t back);
void gfx_Set_Framebuffer_Texture_EXT(Gfx *pkt, uint8_t fmt, uint8_t siz, uint16_t width, uintptr_t image);
void gfx_Set_Framebuffer_Target_EXT(Gfx *pkt, uint8_t fmt, uint8_t siz, uint16_t width, uintptr_t image);
void gfx_Set_Image(Gfx *pkt, uint8_t cmd, uint8_t fmt, uint8_t siz, uint16_t width, uintptr_t address);
void gfx_Set_Color_Image(Gfx *pkt, uint8_t fmt, uint8_t siz, uint16_t width, uintptr_t address);
void gfx_Set_Texture_Image(Gfx *pkt, uint8_t fmt, uint8_t siz, uint16_t width, uintptr_t address);
void gfx_Set_Tile_Size(Gfx *pkt, uint8_t tile, uint16_t uls, uint16_t ult, uint16_t lrs, uint16_t lrt);
void gfx_Load_Block(Gfx *pkt, uint8_t tile, uint16_t uls, uint16_t ult, uint16_t lrs, uint16_t dxt);
void gfx_Set_Tile(Gfx *pkt, uint8_t fmt, uint8_t siz, uint16_t line, uint16_t tmem, uint8_t tile, uint8_t palette, uint8_t cmt, uint8_t maskt, uint8_t shiftt, uint8_t cms, uint8_t masks, uint8_t shifts);
void gfx_Load_TLUT06(Gfx *pkt, uint16_t a, uint16_t b, uint16_t c, uint16_t d);
void gfx_Load_TLUT(Gfx *pkt, uint16_t count);
void gfx_Fog_Position(Gfx *pkt, uint16_t min, uint16_t max);
void gfx_Viewport(Gfx *pkt, const Vp *v);
int  gfx_Set_Scissor(Gfx *pkt, int ulx, int uly, int lrx, int lry);
void gfx_LookAtX(Gfx *pkt, Light *l);
void gfx_LookAtY(Gfx *pkt, Light *l);
void gfx_LookAt(Gfx *pkt, LookAt *la);
void gfx_Set_Other_Mode(Gfx *pkt, uint8_t cmd, uint8_t shift, uint8_t length, uint32_t data);
void gfx_Set_Texture_LOD(Gfx *pkt, uint32_t type);
void gfx_Set_Texture_LUT(Gfx *pkt, uint32_t type);
void gfx_Set_Texture_Persp(Gfx *pkt, uint32_t type);
void gfx_Set_Texture_Filter(Gfx *pkt, uint32_t type);
void gfx_Set_Texture_Convert(Gfx *pkt, uint32_t type);
void gfx_Display_List(Gfx *pkt, const void *dl);
void gfx_Branch_List(Gfx *pkt, const void *dl);
void gfx_Pipeline_Mode(Gfx *pkt, uint32_t mode);
void gfx_Set_Cycle_Type(Gfx *pkt, uint32_t type);
void gfx_Set_Combine_Key(Gfx *pkt, uint32_t type);
void gfx_Set_Combine_Mode(Gfx *pkt, uint32_t w0bits, uint32_t w1bits);
void gfx_Set_Combine_LERP(Gfx *pkt, uint8_t a0, uint8_t b0, uint8_t c0, uint8_t d0, uint8_t Aa0, uint8_t Ab0, uint8_t Ac0, uint8_t Ad0, uint8_t a1, uint8_t b1, uint8_t c1, uint8_t d1, uint8_t Aa1, uint8_t Ab1, uint8_t Ac1, uint8_t Ad1);
void gfx_Set_Alpha_Compare(Gfx *pkt, uint32_t type);
void gfx_Set_Render_Mode(Gfx *pkt, uint32_t c0, uint32_t c1);
void gfx_No_Param(Gfx *pkt, uint8_t cmd);
void gfx_End_Display_List(Gfx *pkt);

void gfx_normalize_vector(float v[3]);
void gfx_transposed_matrix_mul(float res[3], const float a[3], const float b[4][4]);
void gfx_matrix_mul(float res[4][4], const float a[4][4], const float b[4][4]);
void gfx_sp_matrix(uint8_t parameters, const int32_t *addr);

#define gSPSetExtraGeometryModeEXT(pkt, word) gfx_Extra_Geometry_Mode_EXT((pkt), 0, word)
#define gSPClearExtraGeometryModeEXT(pkt, word) gfx_Extra_Geometry_Mode_EXT((pkt), word, 0)

extern float g_ModelViewProj[4][4];

extern struct RGBA g_FillColor;
extern struct RGBA g_FogColor;

#ifdef __cplusplus
}
#endif
