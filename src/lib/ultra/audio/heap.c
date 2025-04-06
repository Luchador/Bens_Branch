#include "synthInternals.h"
#include <libaudio.h>

void alHeapInit(ALHeap *hp, uint8_t *base, int len)
{
	hp->base = base;
	hp->len = len;
	hp->cur = hp->base;
	hp->count = 0;
}

void *alHeapDBAlloc(uint8_t *file, int line, ALHeap *hp, int num, int size)
{
	int bytes;
	uint8_t *ptr = 0;

	bytes = (num * size + 0xf) & ~0xf;

	if (hp->cur + bytes <= hp->base + hp->len) {
		ptr = hp->cur;
		hp->cur += bytes;
	} else {

	}

	return ptr;
}
