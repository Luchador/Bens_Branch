#ifndef _IN_PREPROCESS_COMMON_H
#define _IN_PREPROCESS_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <PR/ultratypes.h>
#include "types.h"
#include "constants.h"

#include "platform.h"
#include "system.h"
#include "romdata.h"
#include "preprocess.h"

#define MAX_PTR_MARKERS (1024 * 8)

#define PD_ALIGN(val, size) (((val) + ((size) - 1)) & ~((size) - 1))

#define PD_PTR_BASE(x, b) (void *)((u8 *)b + (uintptr_t)x)
#define PD_PTR_BASEOFS(x, b, d) (void *)((u8 *)b - d + (uintptr_t)x)

static inline float swapF32(float x) { *(uint32_t*)&x = PD_BE32(*(uint32_t*)&x); return x; }
static inline uint32_t swapU32(uint32_t x) { return PD_BE32(x); }
static inline int swapS32(int x) { return PD_BE32(x); }
static inline uint16_t swapU16(uint16_t x) { return PD_BE16(x); }
static inline int16_t swapS16(int16_t x) { return PD_BE16(x); }
static inline void* swapPtr(void** x) { return (void*)PD_BEPTR((uintptr_t)x); }
static inline struct coord swapCrd(struct coord crd) { crd.x = swapF32(crd.x); crd.y = swapF32(crd.y); crd.z = swapF32(crd.z); return crd; }
static inline uint32_t swapUnk(uint32_t x) { assert(0 && "unknown type"); return x; }

#define PD_SWAPPED_VAL(x) _Generic((x), \
	float: swapF32, \
	u32: swapU32, \
	int: swapS32, \
	u16: swapU16, \
	s16: swapS16, \
	struct coord: swapCrd, \
	default: swapUnk	\
)(x)

#define PD_SWAP_VAL(x) x = PD_SWAPPED_VAL(x)

// ptr marker functions

struct ptrmarker {
	uint32_t ptr_src;
	uintptr_t ptr_host;
};

void ptrAdd(uint32_t ptr_src, uintptr_t ptr_host);
struct ptrmarker* ptrFind(uintptr_t ptr_src);
void ptrReset(void);

#endif
