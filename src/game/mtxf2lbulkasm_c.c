#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

void mtxF2LBulk(Mtxf *mtx, int count)
{
#ifndef GBI_FLOATS
	do {
		uint32_t m00 = (int) (mtx->m[0][0] * var8005ef10[0]);
		uint32_t m01 = (int) (mtx->m[0][1] * var8005ef10[0]);
		uint32_t m02 = (int) (mtx->m[0][2] * var8005ef10[0]);
		uint32_t m03 = (int) (mtx->m[0][3] * var8005ef10[1]);
		uint32_t m10 = (int) (mtx->m[1][0] * var8005ef10[0]);
		uint32_t m11 = (int) (mtx->m[1][1] * var8005ef10[0]);
		uint32_t m12 = (int) (mtx->m[1][2] * var8005ef10[0]);
		uint32_t m13 = (int) (mtx->m[1][3] * var8005ef10[1]);
		uint32_t m20 = (int) (mtx->m[2][0] * var8005ef10[0]);
		uint32_t m21 = (int) (mtx->m[2][1] * var8005ef10[0]);
		uint32_t m22 = (int) (mtx->m[2][2] * var8005ef10[0]);
		uint32_t m23 = (int) (mtx->m[2][3] * var8005ef10[1]);
		uint32_t m30 = (int) (mtx->m[3][0] * var8005ef10[0]);
		uint32_t m31 = (int) (mtx->m[3][1] * var8005ef10[0]);
		uint32_t m32 = (int) (mtx->m[3][2] * var8005ef10[0]);
		uint32_t m33 = (int) (mtx->m[3][3] * var8005ef10[1]);

		mtx->l[0][0] = (m00 & 0xffff0000) | m01 >> 16;
		mtx->l[0][1] = (m02 & 0xffff0000) | m03 >> 16;
		mtx->l[0][2] = (m10 & 0xffff0000) | m11 >> 16;
		mtx->l[0][3] = (m12 & 0xffff0000) | m13 >> 16;
		mtx->l[1][0] = (m20 & 0xffff0000) | m21 >> 16;
		mtx->l[1][1] = (m22 & 0xffff0000) | m23 >> 16;
		mtx->l[1][2] = (m30 & 0xffff0000) | m31 >> 16;
		mtx->l[1][3] = (m32 & 0xffff0000) | m33 >> 16;
		mtx->l[2][0] = m00 << 16 | (m01 & 0xffff);
		mtx->l[2][1] = m02 << 16 | (m03 & 0xffff);
		mtx->l[2][2] = m10 << 16 | (m11 & 0xffff);
		mtx->l[2][3] = m12 << 16 | (m13 & 0xffff);
		mtx->l[3][0] = m20 << 16 | (m21 & 0xffff);
		mtx->l[3][1] = m22 << 16 | (m23 & 0xffff);
		mtx->l[3][2] = m30 << 16 | (m31 & 0xffff);
		mtx->l[3][3] = m32 << 16 | (m33 & 0xffff);

		mtx++;

		count--;
	} while (count);
#endif
}
