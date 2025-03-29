#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess/common.h"

struct n64_fontchar {
	uint8_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int kerningindex;
	uint32_t pixeldata;
};

uint8_t *preprocessFont(uint8_t *src, uint32_t srclen, uint32_t *outSize)
{
	int num_chars = 94;

	size_t dstlen = srclen - (num_chars * sizeof(struct n64_fontchar)) + (num_chars * sizeof(struct fontchar)) + 0x20;
	uint8_t* dst = sysMemZeroAlloc(dstlen);

	// Kerning table
	int *src_kerning_table = (int *)src;
	int *dst_kerning_table = (int *)dst;

	for (int i = 0; i < 13 * 13; i++) {
		dst_kerning_table[i] = PD_BE32(src_kerning_table[i]);
	}

	uint32_t src_offset = PD_ALIGN(13 * 13 * 4, sizeof(uint32_t));
	uint32_t dst_offset = PD_ALIGN(13 * 13 * 4, sizeof(uintptr_t));

	// Character table
		struct n64_fontchar *src_char_table = (struct n64_fontchar *) &src[src_offset];
		struct fontchar *dst_char_table = (struct fontchar *) &dst[dst_offset];
		uint32_t diff = (dst_offset + num_chars * sizeof(struct fontchar)) - (src_offset + num_chars * sizeof(struct n64_fontchar));

		for (int i = 0; i < num_chars; i++) {
			dst_char_table[i].index = src_char_table[i].index;
			dst_char_table[i].baseline = src_char_table[i].baseline;
			dst_char_table[i].height = src_char_table[i].height;
			dst_char_table[i].width = src_char_table[i].width;
			dst_char_table[i].kerningindex = PD_BE32(src_char_table[i].kerningindex);
			dst_char_table[i].pixeldata = (void *)(uintptr_t)(src_char_table[i].pixeldata ? PD_BE32(src_char_table[i].pixeldata) + diff : 0);
		}

		src_offset += num_chars * sizeof(struct n64_fontchar);
		dst_offset += num_chars * sizeof(struct fontchar);

	// Pixel data
	uint32_t len = srclen - src_offset;
	memcpy(&dst[dst_offset], &src[src_offset], len);

	if (outSize) *outSize = ALIGN16(dst_offset + len);

	return dst;
}