// see https://github.com/n64decomp/007/blob/master/tools/mktex/src/libpdtex/reader.c
// and https://github.com/doomhack/perfect_dark/blob/master/src/lib/rzip.c

#include <zlib.h>
#include "types.h"

#include "lib/rzip.h"

void *var80091558; // g_RzipUnused

bool rzipIs1172(void *buffer)
{
	const uint8_t* src = buffer;
	return (src[0] == 0x11 && src[1] == 0x72);
}

bool rzipIs1173(void *buffer)
{
	const uint8_t* src = buffer;
	return (src[0] == 0x11 && src[1] == 0x73);
}

static inline int rzipInflate1172(z_stream *strm, uint8_t *src, void *dst)
{
	strm->avail_in = 0x2000;
	strm->next_in = src;

	do {
		strm->avail_out = 0x2000;
		strm->next_out = dst;
		if (inflate(strm, Z_FINISH) == Z_STREAM_ERROR) {
			return 0;
		}
	} while (strm->avail_out == 0);

	return strm->total_out;
}

static inline int rzipInflate1173(z_stream *strm, uint8_t *src, void *dst, uint32_t dstLen)
{
	strm->avail_in = -1; // compressed size unknown
	strm->next_in = src;
	strm->avail_out = dstLen;
	strm->next_out = dst;

	if (inflate(strm, Z_SYNC_FLUSH) == Z_STREAM_ERROR) {
		return 0;
	}

	return strm->total_out;
}

int rzipInflate(void *srcp, void *dst, void *scratch)
{
	int ret = 0;
	uint8_t *src = srcp;
	z_stream strm = { 0 };

	ret = inflateInit2(&strm, -15);
	if (ret != Z_OK) {
		return 0;
	}

	if (rzipIs1173(src)) {
		// 1173, we know the uncompressed length
		const uint32_t dstLen = ((uint32_t)src[2] << 16) | ((uint32_t)src[3] << 8) | (uint32_t)src[4];
		ret = rzipInflate1173(&strm, src + 5, dst, dstLen);
	} else if (rzipIs1172(src)) {
		// 1172, uncompressed length unknown
		ret = rzipInflate1172(&strm, src + 2, dst);
	} else {
		ret = 0;
	}

	inflateEnd(&strm);

	if (ret) {
		var80091558 = strm.next_in;
		return strm.total_out;
	} else {
		return 0;
	}
}

void *rzipGetSomething(void)
{
	return var80091558;
}
