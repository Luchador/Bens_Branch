#ifndef _IN_AUDIO_H
#define _IN_AUDIO_H

#include <stdint.h>

int audioInit(void);
int audioGetBytesBuffered(void);
int audioGetSamplesBuffered(void);
void audioSetNextBuffer(const int16_t *buf, uint32_t len);
void audioEndFrame(void);

#endif
