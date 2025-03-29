/*====================================================================
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

#ifndef __N_SYNTHINTERNALS__
#define __N_SYNTHINTERNALS__

#include <n_libaudio.h>
#include <synthInternals.h>
#include "n_abi.h"
#include "types.h"

#define SAMPLE_ROUND
#undef  SAMPLE_ROUND

#define FINAL_ROUND

#define SAMPLES               184
#define SAMPLE184(delta)      (((delta) + SAMPLES - 1) / SAMPLES) * SAMPLES
#define FIXED_SAMPLE          SAMPLES

#define N_AL_DECODER_IN	        0
#define	N_AL_RESAMPLER_OUT	0
#define N_AL_TEMP_0	        0
#define	N_AL_DECODER_OUT        368
#define	N_AL_TEMP_1	        368
#define	N_AL_TEMP_2	        736
#define	N_AL_MAIN_L_OUT	        1248
#define	N_AL_MAIN_R_OUT	        1616
#define	N_AL_AUX_L_OUT	        1984
#define	N_AL_AUX_R_OUT	        2352

#define N_AL_DIVIDED            368

typedef struct {
	struct ALParam_s    *next;
	int                 delta;
	int16_t                 type;
	struct N_PVoice_s     *pvoice;
} N_ALFreeParam;

typedef struct N_PVoice_s {
	ALLink               node;
	struct N_ALVoice_s    *vvoice;
	/** ALLoadFilter *********************************/
	ADPCM_STATE                 *dc_state;
	ADPCM_STATE                 *dc_lstate;
	ALRawLoop                   dc_loop;
	struct ALWaveTable_s        *dc_table;
	int                         dc_bookSize;
	ALDMAproc                   dc_dma;
	void                        *dc_dmaState;
	int                         dc_sample;
	int                         dc_lastsam;
	int                         dc_first;
	intptr_t                    dc_memin;
	/** ALResampler *********************************/
	RESAMPLE_STATE      *rs_state;
	float                 rs_ratio;
	int			rs_upitch;
	float		        rs_delta;
	int			rs_first;
	/** ALEnvMixer *********************************/
	ENVMIX_STATE	*em_state;
	int16_t		        em_pan;
	int16_t		        em_volume;
	int16_t		        em_cvolL;
	int16_t		        em_cvolR;
	int16_t		        em_dryamt;
	int16_t		        em_wetamt;
	uint16_t                 em_lratl;
	int16_t                 em_lratm;
	int16_t                 em_ltgt;
	uint16_t                 em_rratl;
	int16_t                 em_rratm;
	int16_t                 em_rtgt;
	int                 em_delta;
	int                 em_segEnd;
	int			em_first;
	ALParam		*em_ctrlList;
	ALParam		*em_ctrlTail;
	int                 em_motion;
	int                 offset;
	uint8_t unk8c;
	uint16_t unk8e;
	struct fx fx;
	int unkb8;
	void *unkbc; // size 8
} N_PVoice;


typedef Acmd *(*N_ALCmdHandler)(int, Acmd *, int bus);

typedef struct N_ALFilter_s {
	struct N_ALFilter_s   *source;
	N_ALCmdHandler        handler;
	ALSetParam          setParam;
	int16_t                 inp;
	int16_t                 outp;
	int                 type;
} N_ALFilter;

typedef struct N_ALMainBus_s {
	N_ALFilter           filter;
} N_ALMainBus;

struct auxbus44 {
	struct fx fx;
	uint32_t unk28;
	POLEF_STATE *unk2c;
	POLEF_STATE *unk30;
	uint32_t unk34;
};

typedef struct N_ALAuxBus_s {
	ALFilter            filter;
	int                 sourceCount;
	int                 maxSources;
	N_PVoice           **sources;
	ALFx                *fx;
	ALFx		*fx_array[AL_MAX_AUX_BUS_SOURCES];
	struct auxbus44 *unk44;
} N_ALAuxBus;

void alN_PVoiceNew(N_PVoice *mv, ALDMANew dmaNew, ALHeap *hp);

ALParam         *__n_allocParam(void);
void            _n_freeParam(ALParam *param);
void            _n_freePVoice(N_PVoice *pvoice);
void            _n_collectPVoices(void);
int             _n_timeToSamples(int micros);
ALMicroTime     _n_samplesToTime(int samples);


Acmd    *n_alAdpcmPull(N_PVoice *f,int16_t *outp, int byteCount, Acmd *p);
int     n_alLoadParam(N_PVoice *filter, int paramID, void *param);

Acmd    *n_alResamplePull(N_PVoice *f, int16_t *outp, Acmd *p);
Acmd    *n_alResamplePull2(N_PVoice *f, int16_t *outp, int outCount, Acmd *p);
int     n_alResampleParam(N_PVoice *f, int paramID, void *param);
int     n_alResampleParam2(N_PVoice *f, int paramID, void *param);

Acmd    *n_alEnvmixerPull(N_PVoice *f, int sampleOffset, Acmd *p);
int     n_alEnvmixerParam(N_PVoice *p, int paramID, void *param);

int n_alAuxBusParam( int paramID, void *param);
Acmd *n_alAuxBusPull(int sampleOffset, Acmd *cmdptr, int fxBus, int *numpulls);

Acmd *n_alFxPull( int sampleOffset, Acmd *p, int arg2);
int n_alFxParamHdl(void *filter, int paramID, void *param);
void n_alFxNew(ALFx **r, ALSynConfig *c, int16_t bus, ALHeap *hp);

Acmd *n_alMainBusPull( int sampleOffset, Acmd *p);
int n_alMainBusParam( int paramID, void *param);

Acmd *n_alSavePull(int sampleOffset, Acmd *p);
int n_alSaveParam( int paramID, void *param);

void n_alSaveNew(void);

void n_alSynNew(ALSynConfig *c);
void n_alSynDelete(void);

#endif /*  __N_SYNTHINTERNALS__ */
