#ifndef _IN_PREPROCESS_GBI_H
#define _IN_PREPROCESS_GBI_H

#include "preprocess/common.h"
#include <stdint.h>

// gbi related functions
void gbiReset(void);
void gbiSetSegment(int segment, uint32_t offset);
void gbiSetVtx(uint32_t src_offset, uint32_t dst_offset);
uint32_t gbiConvertGdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int segment_cmds);
void gbiConvertVtx(uint8_t* dst, uint32_t dstpos, int count);
void gbiGdlRewriteAddrs(uint8_t* dst, uint32_t offset);
void gbiAddTexAddr(uint32_t src_offset, uint32_t dst_offset);

#endif
