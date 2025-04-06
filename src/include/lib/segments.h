#pragma once

void *segGetDataStart(void);
void *segGetDatazipRomStart(void);
void *segGetInflateRomStart(void);
void *segGetInflateRomStart2(void);
void *segGetGamezipsRomStart(void);
void segInflate(void *src, void *dst, void *scratch);
