#include <stdint.h>
#include "constants.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

void crcCalculateU16Pair(uint8_t *start, uint8_t *end, uint16_t *checksum)
{
	uint8_t *ptr;
	uint32_t salt = 0;
	uint64_t seed = 0x8f809f473108b3c1;
	uint32_t sum1 = 0;
	uint32_t sum2 = 0;

	for (ptr = start; ptr < end; ptr++, salt += 7) {
		seed += *ptr << (salt & 0x0f);
		sum1 ^= rngRotateSeed(&seed);
	}

	for (ptr = end - 1; ptr >= start; ptr--, salt += 3) {
		seed += *ptr << (salt & 0x0f);
		sum2 ^= rngRotateSeed(&seed);
	}

	checksum[0] = sum1 & 0xffff;
	checksum[1] = sum2 & 0xffff;
}
