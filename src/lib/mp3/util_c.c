#include <ultra64.h>
#include "internal.h"
#include "mp3.h"
#include "data.h"
#include "platform.h"

extern struct mp3decfourbytes *var8009c650[];

int mp3util000461c0(uint8_t *buffer, int *a2, int a3, struct mp3decfourbytes *v8, int t0)
{
	int v4 = t0;
	int v5 = *buffer;
	int v6 = (a3 << 8) + (uint8_t)(((v5 << 8) | (uint32_t)buffer[1]) >> (8 - (v4 & 7)));
	if (*(uint8_t *)(var8005f6fc + v6)) {
		return *(uint8_t *)(var8005f6f8 + v6) & 0xF;
	}
	uint32_t v9 = 0x80u >> (v4 & 7);

	do {
		int v10;
		if ((v5 & v9) != 0) {
			v10 = v8->bytes[1];
		} else {
			v10 = v8->bytes[0];
		}
		v9 >>= 1;
		v8 += v10;
		if (!v9) {
			v9 = 128;
			v5 = *++buffer;
		}
		++v4;
	} while (v8->unk02 == -1);

	return v8->unk02;
}

int mp3util00046290(uint32_t *ptr, uint8_t s1, uint8_t t0)
{
	return *ptr << (t0 & 7) >> (32 - s1);
}

int mp3utilGetBits(uint8_t *buffer, int *count, int numbits)
{
	const int result = PD_BE32(*(uint32_t *)(buffer + (*count >> 3))) << (*count & 7) >> (32 - numbits);
	*count += numbits;
	return result;
}

int mp3util000462f8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int arg5, int16_t **arg6, uint8_t **arg7)
{
	int8_t cVar1;
	uint32_t uVar2;
	uint32_t uVar3;
	int *piVar4;
	uint32_t uVar5;
	int16_t *puVar6;
	uint8_t *puVar7;
	int iVar8;

	struct mp3decfourbytes *s0 = var8009c650[arg2];

	uVar5 = *arg1;
	puVar6 = (int16_t *)*arg6;
	puVar7 = (uint8_t *)*arg7;
	piVar4 = (int *)(arg0 + (uVar5 >> 3));
	iVar8 = -arg3;

	while (iVar8 = arg5 + iVar8, iVar8 > 0) {
		uVar2 = mp3util000461c0((uint8_t *)piVar4, arg1, arg2, s0, uVar5);
		if ((arg4 != 0) && (uVar2 == 0xf)) {
			uVar2 = mp3util00046290(piVar4, arg4, uVar5);
			uVar2 = uVar2 + 0xf;
		}
		if (uVar2 == 0) {
			*puVar7 = 0;
		} else {
			cVar1 = *(int8_t *)piVar4;
			uVar3 = uVar5 & 7;
			piVar4 = (int *)((uint8_t *)piVar4 + ((uVar3 + 1) >> 3));
			uVar5 = uVar5 + 1;
			if (((uint32_t)cVar1 >> ((7 - uVar3) & 0x1f) & 1) == 0) {
				*puVar7 = 0;
			} else {
				*puVar7 = 1;
			}
		}
		if ((arg4 != 0) && (iVar8 == 0xf)) {
			uVar3 = mp3util00046290(piVar4, arg4, uVar5);
			iVar8 = uVar3 + 0xf;
		}
		*puVar6 = (int16_t)uVar2;
		puVar6[1] = (int16_t)iVar8;
		puVar6 = puVar6 + 2;
		if (iVar8 == 0) {
			puVar7[1] = 0;
		} else {
			cVar1 = *(int8_t *)piVar4;
			uVar2 = uVar5 & 7;
			piVar4 = (int *)((uint8_t *)piVar4 + ((uVar2 + 1) >> 3));
			uVar5 = uVar5 + 1;
			if (((uint32_t)cVar1 >> ((7 - uVar2) & 0x1f) & 1) == 0) {
				puVar7[1] = 0;
			} else {
				puVar7[1] = 1;
			}
		}
		arg3 = arg3 + 2;
		puVar7 = puVar7 + 2;
		iVar8 = -arg3;
	}

	*arg1 = uVar5;
	*arg6 = puVar6;
	*arg7 = puVar7;

	return arg3;
}

int mp3util000464a8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int16_t **arg5, uint8_t **arg6)
{
	uint32_t uVar1;
	uint32_t uVar2;
	int *piVar3;
	int iVar4;
	uint32_t uVar5;
	uint32_t uVar6;
	uint32_t uVar7;
	uint32_t uVar8;
	uint32_t uVar9;
	int16_t *puVar10;
	uint8_t *puVar11;

	struct mp3decfourbytes *s0 = var8009c650[arg2];

	uVar5 = *arg1;
	puVar10 = *arg5;
	puVar11 = *arg6;
	piVar3 = (int *)(arg0 + (uVar5 >> 3));

	if (((int)(arg4 - uVar5) > 0) && (iVar4 = arg3, arg3 + -0x240 < 0)) {
		do {
			uVar1 = mp3util000461c0((uint8_t *)piVar3, arg1, arg2, s0, uVar5);
			uVar6 = uVar1 >> 3 & 1;
			uVar7 = uVar1 >> 2 & 1;
			uVar8 = uVar1 >> 1 & 1;
			uVar1 = uVar1 & 1;
			*puVar10 = (int16_t)uVar6;
			puVar10[1] = (int16_t)uVar7;
			puVar10[2] = (int16_t)uVar8;
			puVar10[3] = (int16_t)uVar1;
			uVar2 = mp3util00046290(piVar3, 4, uVar5);
			uVar9 = 3;
			*puVar11 = (uint8_t)uVar2;
			if (uVar6 != 0) {
				uVar6 = uVar6 & uVar2 >> 3;
				uVar9 = 2;
			}
			*puVar11 = (uint8_t)uVar6;
			if (uVar7 != 0) {
				uVar7 = uVar7 & uVar2 >> uVar9;
				uVar9 = uVar9 - 1;
			}
			puVar11[1] = (uint8_t)uVar7;
			if (uVar8 != 0) {
				uVar8 = uVar8 & uVar2 >> (uVar9 & 0x1f);
				uVar9 = uVar9 - 1;
			}
			puVar11[2] = (uint8_t)uVar8;
			if (uVar1 != 0) {
				uVar1 = uVar1 & uVar2 >> (uVar9 & 0x1f);
				uVar9 = uVar9 - 1;
			}
			puVar11[3] = (uint8_t)uVar1;
			uVar1 = uVar5 & 7;
			uVar5 = uVar5 - (uVar9 + 1);
			if ((int)(uVar1 - (uVar9 + 1)) < 0) {
				piVar3 = (int *)((uint8_t *)piVar3 - 1);
			}
			puVar10 = puVar10 + 4;
			puVar11 = puVar11 + 4;
			arg3 = iVar4 + 4;
		} while ((iVar4 + -0x23c < 0) && (iVar4 = arg3, (int)(arg4 - uVar5) > 0));
	}

	*arg1 = uVar5;
	*arg5 = puVar10;
	*arg6 = puVar11;

	return arg4;
}