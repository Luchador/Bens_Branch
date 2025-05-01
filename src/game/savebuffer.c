#include <ultra64.h>
#include <stdio.h>
#include "constants.h"
#include "game/camera.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "game/mtxutils.h"
#include "game/savebuffer.h"
#include "game/tex.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"


Vp *g_Viewport = NULL;

int var8009de90;
int var8009de94;
int g_MenuProjectFromX;
int g_MenuProjectFromY;

void func0f0d4690(Mtx *mtx)
{
	struct coord pos;

	mtxIdent(mtx);

	pos.x = -159.75f;
	pos.y = 120.25f;
	pos.z = 0;

	pos.x = (.5f - viGetWidth()) * 0.5f;
	pos.y = (.5f + viGetHeight()) * 0.5f;
	pos.z = 0;

	mtx4SetTranslation(&pos, mtx);
	mtxScaleRow1Full(-1, mtx);
}

Gfx *savebufferSetCustomProjection(Gfx *gdl)
{
	Mtx mtx;
	Mtx *mtx1;
	Mtx *mtx2;

	mtx1 = gfxAllocateMatrix();
	mtx2 = gfxAllocateMatrix();

	func0f0d4690(&mtx);
	mtxScaleRow0Full(0.1f, &mtx);
	mtxScaleRow1Full(0.1f, &mtx);
	mtx4Copy(&mtx, mtx2);
	mtxIdent(&mtx);

	mtxFrustum(&mtx, -(float) viGetWidth() * 0.5f, viGetWidth() * 0.5f, -(float) viGetHeight() * 0.5f, viGetHeight() * 0.5f, 10, 10000, 1);

	mtx4Copy(&mtx, mtx1);

	gfx_Matrix(gdl++, mtx2, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
	gfx_Matrix(gdl++, mtx1, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	if (g_Viewport == NULL) {
		uint32_t size = align16(sizeof(Vp));
		g_Viewport = gfxAllocate(size);

		if (g_Viewport != NULL) {
			g_Viewport->vp.vscale[0] = viGetWidth();
			g_Viewport->vp.vscale[1] = viGetHeight();

			g_Viewport->vp.vtrans[0] = viGetWidth() / 2;
			g_Viewport->vp.vtrans[1] = viGetHeight() / 2;
		}
	}

	gfx_Viewport(gdl++, g_Viewport);

	return gdl;
}

Gfx *savebufferSetup2DRender(Gfx *gdl)
{
	gfx_Viewport(gdl++, viGetCurrentPlayerViewport());
	gfx_Matrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *func0f0d4a3c(Gfx *gdl)
{
	Mtx mtx;
	Mtx *mtxptr = gfxAllocateMatrix();

	texSelect(&gdl, &g_TexGeneralConfigs[6], 2, 0, 2, 1, NULL);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0,
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0,
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0);
	gfx_Set_Geometry_Mode(gdl++, G_SHADE);
	gfx_Set_Geometry_Mode(gdl++, G_SHADING_SMOOTH);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

	func0f0d4690(&mtx);
	mtx4Copy(&mtx, mtxptr);

	gfx_Matrix(gdl++, mtxptr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *func0f0d4c80(Gfx *gdl)
{
	Mtx mtx;
	Mtx*mtxptr = gfxAllocateMatrix();

	func0f0d4690(&mtx);
	mtxScaleRow0Full(0.1f, &mtx);
	mtxScaleRow1Full(0.1f, &mtx);
	mtx4Copy(&mtx, mtxptr);

	gfx_Matrix(gdl++, mtxptr, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

Gfx *menugfxDrawPlane(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2, int type)
{
	Col *colours;
	Vtx *vertices;
	float tmp1;
	int16_t a1;
	int16_t t1;
	float sp34;
	float sp30;
	int16_t sp2e;
	int16_t sp2c;
	int16_t sp2a;
	int16_t sp28;
	int16_t a1_2;
	int16_t scale = 10;
	float tmp2;

	static uint32_t depthsub = 1000;
	static uint32_t txmul = 20;
	static uint32_t rsub = 5;

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(4);

	sp34 = 1.0f;
	sp30 = 1.0f;

	if (y1 < var8009de90 && y2 < var8009de90) {
		return gdl;
	}

	if (y1 > var8009de94 && y2 > var8009de94) {
		return gdl;
	}

	if (y1 < var8009de90) {
		y1 = var8009de90;
	}

	if (y2 < var8009de90) {
		y2 = var8009de90;
	}

	if (y1 > var8009de94) {
		y1 = var8009de94;
	}

	if (y2 > var8009de94) {
		y2 = var8009de94;
	}

	sp2e = (x1 + y1) * txmul;
	sp2c = (x2 + y2) * txmul;
	sp2a = 0;
	sp28 = 16384;

	if (type == MENUPLANE_01) {
		sp30 = 2.0f;
	}

	a1 = 200;

	if (type == MENUPLANE_02 || type == MENUPLANE_03) {
		if (type == MENUPLANE_02) {
			sp2e = 0;
			sp2c = 1024;
		} else {
			sp2e = 1024;
			sp2c = 2048;
		}

		sp34 = 4.0f;
		sp30 = 4.0f;
		a1 = 6000;
	}

	if (type == MENUPLANE_08 || type == MENUPLANE_09 || type == MENUPLANE_11) {
		sp2e = 0;
		sp2c = 2048;
		a1 = 2000;
		sp34 = 4.0f;
		sp30 = 4.0f;

		if (type == MENUPLANE_09) {
			sp30 = 2.0f;
		}
	}

	if (type == MENUPLANE_04) {
		a1 = 2000;
		sp34 = 1.0f;
		sp30 = 1.0f;
	}

	if (type == MENUPLANE_05 || type == MENUPLANE_06 || type == MENUPLANE_10) {
		a1 = 1000;
		sp2e = 0;
		sp2c = 4096;
		sp30 = 4.0f;

		if (type == MENUPLANE_06) {
			sp2e = 384;
			sp2c = 4480;
			sp30 = 8.0f;
		} else if (type == MENUPLANE_10) {
			sp2e = 384;
			sp2c = 4480;
			sp30 = 8.0f;
		} else {
			sp34 = 2.0f;
		}
	}

	if (type == MENUPLANE_07) {
		a1 = -rsub;
		sp30 = 8.0f;
		sp2a = 256;
		sp28 = 0;
	}

	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = -10;

	vertices[1].x = x2;
	vertices[1].y = y2;
	vertices[1].z = -10;

	tmp1 = (float) g_MenuProjectFromX * a1 / scale;
	tmp2 = (float) g_MenuProjectFromY * a1 / scale;

	vertices[2].x = vertices[0].v[0] + (int16_t) tmp1;
	vertices[2].y = vertices[0].v[1] + (int16_t) tmp2;
	vertices[2].z = -10 - a1;

	vertices[3].x = vertices[1].v[0] + (int16_t) tmp1;
	vertices[3].y = vertices[1].v[1] + (int16_t) tmp2;
	vertices[3].z = -10 - a1;

	if (type == MENUPLANE_10) {
		t1 = g_20SecIntervalFrac * sp34 * 64.0f * 32.0f;
	} else {
		t1 = (g_20SecIntervalFrac - 0.5f) * sp34 * 64.0f * 32.0f;
	}

	if (type == MENUPLANE_10) {
		a1_2 = (g_20SecIntervalFrac - 0.5f) * sp30 * 64.0f * 32.0f;
	} else {
		a1_2 = g_20SecIntervalFrac * sp30 * 64.0f * 32.0f;
	}

	vertices[0].s = sp2e + t1;
	vertices[0].t = sp2a + a1_2;
	vertices[1].s = sp2c + t1;
	vertices[1].t = sp2a + a1_2;
	vertices[3].s = sp2c + t1;
	vertices[3].t = sp28 + a1_2;
	vertices[2].s = sp2e + t1;
	vertices[2].t = sp28 + a1_2;

	if (type == MENUPLANE_07) {
		vertices[0].colour = 0;
		vertices[1].colour = 0;
		vertices[2].colour = 4;
		vertices[3].colour = 4;
	} else {
		vertices[0].colour = 0;
		vertices[1].colour = 4;
		vertices[2].colour = 0;
		vertices[3].colour = 4;
	}

	colours[0].word = PD_BE32(colour1);
	colours[1].word = PD_BE32(colour2);

	gfx_Color(gdl++, colours, 2);
	gfx_Vertex(gdl++, vertices, 4, 0);
	gfx_Tri2(gdl++, 0, 1, 3, 3, 2, 0);

	return gdl;
}

/**
 * Write the specified amount of bits to the buffer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 *
 * This function only sets bits to on and does not unset them.
 */
void savebufferOr(struct savebuffer *buffer, uint32_t value, int numbits)
{
	uint32_t bit = 1 << (numbits - 1);

	for (; bit; bit >>= 1) {
		if (bit & value) {
			int bitindex = buffer->bitpos % 8;
			uint8_t mask = 1 << (7 - bitindex);
			int byteindex = buffer->bitpos / 8;

			buffer->bytes[byteindex] |= mask;
		}

		buffer->bitpos++;
	}
}

/**
 * Write the specified amount of bits to the buffer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 */
void savebufferWriteBits(struct savebuffer *buffer, uint32_t value, int numbits, uint8_t *dst)
{
	uint32_t bit = 1 << (numbits - 1);

	for (; bit; bit >>= 1) {
		int bitindex = buffer->bitpos % 8;
		uint8_t mask = 1 << (7 - bitindex);
		int byteindex = buffer->bitpos / 8;

		if (bit & value) {
			dst[byteindex] |= mask;
		} else {
			dst[byteindex] &= ~mask;
		}

		buffer->bitpos++;
	}
}

/**
 * Read the specified amount of bits from the buffer and return it as an
 * integer, advancing the internal pointer.
 *
 * numbits is expected to be 32 or less.
 */
uint32_t savebufferReadBits(struct savebuffer *buffer, int numbits)
{
	uint32_t bit = 1 << (numbits - 1);
	uint32_t value = 0;

	for (; bit; bit >>= 1) {
		int bitindex = buffer->bitpos % 8;
		uint8_t mask = 1 << (7 - bitindex);
		int byteindex = buffer->bitpos / 8;

		if (buffer->bytes[byteindex] & mask) {
			value |= bit;
		}

		buffer->bitpos++;
	}

	return value;
}

void savebufferClear(struct savebuffer *buffer)
{
	int i;

	buffer->bitpos = 0;

	for (i = 0; i < sizeof(buffer->bytes);) {
		buffer->bytes[i] = 0;
		i++;
	}
}

void savebufferWriteData(struct savebuffer *buffer, uint8_t *data, uint8_t len)
{
	int i;

	buffer->bitpos = 0;

	for (i = 0; i < len; i++) {
		buffer->bytes[i] = data[i];
	}
}

/**
 * Read a zero-terminated string from the buffer and move the buffer's internal
 * pointer past the end of the string.
 */
void savebufferReadString(struct savebuffer *buffer, char *dst, bool addlinebreak)
{
	bool foundnull = false;
	int index = 0;
	int i;

	for (i = 0; i < 10; i++) {
		int byte = savebufferReadBits(buffer, 8);

		if (!foundnull) {
			if (byte == '\0') {
				foundnull = true;
			} else {
				dst[i] = byte;
				index = i;
			}
		}
	}

	if (addlinebreak) {
		index++;
		dst[index] = '\n';
	}

	index++;
	dst[index] = '\0';
}

void func0f0d55a4(struct savebuffer *buffer, char *src)
{
	bool done = false;
	int i;

	for (i = 0; i < 10; i++) {
		if (!done) {
			if (src[i] == '\0') {
				done = true;
			} else if (src[i] == '\n') {
				done = true;
			} else {
				uint32_t c = src[i];
				savebufferOr(buffer, c, 8);
			}
		}

		if (done) {
			savebufferOr(buffer, '\0', 8);
		}
	}
}

void func0f0d564c(uint8_t *data, char *dst, bool addlinebreak)
{
	struct savebuffer buffer;

	savebufferWriteData(&buffer, data, 10);
	savebufferReadString(&buffer, dst, addlinebreak);
}

void func0f0d5690(uint8_t *dst, char *src)
{
	struct savebuffer buffer;
	bool done = false;
	int i;

	savebufferWriteData(&buffer, dst, 10);

	for (i = 0; i < 10; i++) {
		if (!done) {
			if (src[i] == '\0') {
				done = true;
			} else if (src[i] == '\n') {
				done = true;
			} else {
				uint32_t c = src[i];
				savebufferWriteBits(&buffer, c, 8, dst);
			}
		}

		if (done) {
			savebufferWriteBits(&buffer, '\0', 8, dst);
		}
	}
}

void savebufferWriteGuid(struct savebuffer *buffer, struct fileguid *guid)
{
	savebufferOr(buffer, guid->fileid, 7);
	savebufferOr(buffer, guid->deviceserial, 13);
}

void savebufferReadGuid(struct savebuffer *buffer, struct fileguid *guid)
{
	guid->fileid = savebufferReadBits(buffer, 7);
	guid->deviceserial = savebufferReadBits(buffer, 13);
}

void formatTime(char *dst, int time60, int precision)
{
	int parts[5];
	bool donefirst = false;
	int len = 0;
	int i;

	parts[4] = time60 % 60 * 100 / 60; // hundredths
	parts[3] = time60 / 60; // seconds
	parts[2] = parts[3] / 60; // minutes
	parts[1] = parts[2] / 60; // hours
	parts[0] = parts[1] / 24; // days

	parts[3] %= 60; // seconds
	parts[2] %= 60; // minutes
	parts[1] %= 24; // hours

	for (i = 0; i <= precision; i++) {
		if (donefirst) {
			len += sprintf(&dst[len], ":%02d", parts[i]);
		} else if (parts[i] != 0 || i >= TIMEPRECISION_MINUTES) {
			len += sprintf(&dst[len], "%d", parts[i]);
			donefirst = true;
		}
	}
}

void savebufferResetVp(void)
{
	// Set viewport to NULL
	g_Viewport = 0;
}
