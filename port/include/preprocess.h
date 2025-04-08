#ifndef _IN_PREPROCESS_H
#define _IN_PREPROCESS_H

#include <stdint.h>

typedef uint8_t *(*preprocessfunc)(uint8_t *data, uint32_t size, uint32_t *outSize);

uint8_t* preprocessAnimations(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t* preprocessMpConfigs(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t* preprocessFont(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t* preprocessALBankFile(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t *preprocessALCMidiHdr(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t* preprocessSequences(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t* preprocessTexturesList(uint8_t *data, uint32_t size, uint32_t *outSize);

void preprocessBgSection1(uint8_t* data, uint32_t size, uint32_t ofs);
void preprocessBgSection3(uint8_t* data, uint32_t size);
void preprocessBgSection1Header(uint8_t *data, uint32_t size);
void preprocessBgSection2Header(uint8_t *data, uint32_t size);
void preprocessBgSection3Header(uint8_t *data, uint32_t size);
uint32_t preprocessBgRoom(uint8_t* data, uint32_t size, uint32_t room_ofs);

uint8_t *preprocessPadsFile(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t *preprocessTilesFile(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t *preprocessSetupFile(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t *preprocessModelFile(uint8_t *data, uint32_t size, uint32_t *outSize);
uint8_t *preprocessGunFile(uint8_t *data, uint32_t size, uint32_t *outSize);

#endif
