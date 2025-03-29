/*====================================================================
 * audioInternals.h
 *
 * Synopsis:
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

#ifndef __audioInternals__
#define __audioInternals__

#include <libaudio.h>
#include <stdint.h>

/*
 * filter message ids
 */
enum {
	AL_FILTER_FREE_VOICE,
	AL_FILTER_SET_SOURCE,
	AL_FILTER_ADD_SOURCE,
	AL_FILTER_ADD_UPDATE,
	AL_FILTER_RESET,
	AL_FILTER_SET_WAVETABLE,
	/*    AL_FILTER_SET_DMA_PROC,*/
	/*    AL_FILTER_SKIP_LOOP,*/
	AL_FILTER_SET_DRAM,
	AL_FILTER_SET_PITCH,
	AL_FILTER_SET_UNITY_PITCH,
	AL_FILTER_START,
	/*    AL_FILTER_SET_DECAY,*/
	/*    AL_FILTER_SET_FC,*/
	AL_FILTER_SET_STATE,
	AL_FILTER_SET_VOLUME,
	AL_FILTER_SET_PAN,
	AL_FILTER_START_VOICE_ALT,
	AL_FILTER_START_VOICE,
	AL_FILTER_STOP_VOICE,
	AL_FILTER_SET_FXAMT,
	AL_FILTER_11,
	AL_FILTER_12,
	AL_FILTER_13
};

#define AL_MAX_RSP_SAMPLES      160

/*
 * buffer locations based on AL_MAX_RSP_SAMPLES
 */
#define AL_DECODER_IN	        0
#define	AL_RESAMPLER_OUT	0
#define AL_TEMP_0	        0
#define	AL_DECODER_OUT	        320
#define	AL_TEMP_1	        320
#define	AL_TEMP_2	        640
#define	AL_MAIN_L_OUT	        1088
#define	AL_MAIN_R_OUT	        1408
#define	AL_AUX_L_OUT	        1728
#define	AL_AUX_R_OUT	        2048

/*
 * filter types
 */
enum {
	AL_ADPCM,
	AL_RESAMPLE,
	AL_BUFFER,
	AL_SAVE,
	AL_ENVMIX,
	AL_FX,
	AL_AUXBUS,
	AL_MAINBUS
};

typedef struct ALParam_s {
	struct ALParam_s    *next;
	int                 delta;
	int16_t                 type;
	union {
		float             f;
		int             i;
	} data;
	union {
		float             f;
		int             i;
	} moredata;
	union {
		float             f;
		int             i;
	} stillmoredata;
	union {
		float             f;
		int             i;
	} yetstillmoredata;
	int unk1c;
	int unk20;
#ifdef PLATFORM_64BIT
	uint8_t _pad_[8];
#endif
} ALParam;

typedef struct {
	struct ALParam_s            *next;
	int                         delta;
	int16_t                         type;
	int16_t                         unity;  /* disable resampler */
	float                         pitch;
	int16_t                         volume;
	ALPan                       pan;
	uint8_t                          fxMix;
	uint8_t                          unk14;
	uint8_t                          unk15;
	float                         unk18;
	int                         samples;
	struct ALWaveTable_s        *wave;
} ALStartParamAlt;

typedef struct {
	struct ALParam_s            *next;
	int                         delta;
	int16_t                         type;
	int16_t                         unity;  /* disable resampler */
	struct ALWaveTable_s        *wave;
} ALStartParam;

typedef struct {
	struct ALParam_s    *next;
	int                 delta;
	int16_t                 type;
	struct PVoice_s     *pvoice;
} ALFreeParam;

typedef Acmd *(*ALCmdHandler)(void *, int16_t *, int, int, Acmd *);
typedef int   (*ALSetParam)(void *, int, void *);

typedef struct ALFilter_s {
	struct ALFilter_s   *source;
	ALCmdHandler        handler;
	ALSetParam          setParam;
	int16_t                 inp;
	int16_t                 outp;
	int                 type;
} ALFilter;

void    alFilterNew(ALFilter *f, ALCmdHandler h, ALSetParam s, int type);

#define AL_MAX_ADPCM_STATES     3

typedef struct {
	ALFilter                    filter;
	ADPCM_STATE                 *state;
	ADPCM_STATE                 *lstate;
	ALRawLoop                   loop;
	struct ALWaveTable_s        *table;
	int                         bookSize;
	ALDMAproc                   dma;
	void                        *dmaState;
	int                         sample;
	int                         lastsam;
	int                         first;
	int                         memin;
} ALLoadFilter;

void    alLoadNew(ALLoadFilter *f, ALDMANew dma, ALHeap *hp);
Acmd    *alAdpcmPull(void *f, int16_t *outp, int byteCount, int sampleOffset, Acmd *p);
Acmd    *alRaw16Pull(void *f, int16_t *outp, int byteCount, int sampleOffset, Acmd *p);
int     alLoadParam(void *filter, int paramID, void *param);

typedef struct ALResampler_s {
	ALFilter            filter;
	RESAMPLE_STATE      *state[2];
	float                 ratio;
	int			upitch;
	float		        delta;
	int			first;
	ALParam		*ctrlList;
	ALParam		*ctrlTail;
	int                 motion;
} ALResampler;

typedef struct {
	int16_t		        fc;
	int16_t		        fgain;
	union {
		int16_t		fccoef[16];
		int64_t             force_aligned;
	} fcvec;
	int			first;
	POLEF_STATE *fstate[2];
	int unk34;
} ALLowPass;

typedef struct {
	uint32_t		input;
	uint32_t		output;
	int16_t		ffcoef;
	int16_t		fbcoef;
	int16_t		gain;
	float		rsinc;
	float		rsval;
	int		rsdelta;
	float		rsgain;
	ALLowPass	*lp;
	ALResampler	*rs;
} ALDelay;

typedef int   (*ALSetFXParam)(void *, int, void *);
typedef struct {
	uint32_t			length; // 0
	ALDelay		*delay; // 4
	uint8_t			section_count; // 8
	struct ALFilter_s   filter;
	int16_t			*base[2]; // 20
	int16_t			*input[2]; // 28
} ALFx;

void    alFxNew(ALFx *r, ALSynConfig *c, ALHeap *hp);
Acmd    *alFxPull(void *f, int16_t *outp, int out, int sampleOffset, Acmd *p);
int     alFxParam(void *filter, int paramID, void *param);
int     alFxParamHdl(void *filter, int paramID, void *param);

#define AL_MAX_MAIN_BUS_SOURCES       1
typedef struct ALMainBus_s {
	ALFilter            filter;
	int                 sourceCount;
	int                 maxSources;
	ALFilter            **sources;
} ALMainBus;

void    alMainBusNew(ALMainBus *m, void *ptr, int len);
Acmd    *alMainBusPull(void *f, int16_t *outp, int outCount, int sampleOffset, Acmd *p);
int     alMainBusParam(void *filter, int paramID, void *param);

#define AL_MAX_AUX_BUS_SOURCES       8
#define AL_MAX_AUX_BUS_FX	     1
typedef struct ALAuxBus_s {
	ALFilter            filter;
	int                 sourceCount;
	int                 maxSources;
	ALFilter            **sources;
	ALFx		fx[AL_MAX_AUX_BUS_FX];
} ALAuxBus;

void    alAuxBusNew(ALAuxBus *m, void *ptr, int len);
Acmd    *alAuxBusPull(void *f, int16_t *outp, int outCount, int sampleOffset, Acmd *p);
int     alAuxBusParam(void *filter, int paramID, void *param);

void    alResampleNew(ALResampler *r, ALHeap *hp);
Acmd    *alResamplePull(void *f, int16_t *outp, int out, int sampleOffset, Acmd *p);
int     alResampleParam(void *f, int paramID, void *param);

typedef struct ALSave_s {
	ALFilter            filter;
	int	       		dramout;
	int                 first;
} ALSave;

void    alSaveNew(ALSave *r);
Acmd    *alSavePull(void *f, int16_t *outp, int outCount, int sampleOffset, Acmd *p);
int     alSaveParam(void *f, int paramID, void *param);

typedef struct ALEnvMixer_s {
	ALFilter            filter;
	ENVMIX_STATE	*state;
	int16_t		        pan;
	int16_t		        volume;
	int16_t		        cvolL;
	int16_t		        cvolR;
	int16_t		        dryamt;
	int16_t		        wetamt;
	uint16_t                 lratl;
	int16_t                 lratm;
	int16_t                 ltgt;
	uint16_t                 rratl;
	int16_t                 rratm;
	int16_t                 rtgt;
	int                 delta;
	int                 segEnd;
	int			first;
	ALParam		*ctrlList;
	ALParam		*ctrlTail;
	ALFilter            **sources;
	int                 motion;
} ALEnvMixer;

void    alEnvmixerNew(ALEnvMixer *e, ALHeap *hp);
Acmd    *alEnvmixerPull(void *f, int16_t *outp, int out, int sampleOffset, Acmd *p);
int     alEnvmixerParam(void *filter, int paramID, void *param);


/*
 * heap stuff
 */
typedef struct {
	int         magic;  /* check structure integrety                    */
	int         size;   /* size of this allocated block                 */
	uint8_t          *file;  /* file that this alloc was called from         */
	int         line;   /* line that it was called from                 */
	int         count;  /* heap call number                             */
	int         pad0;
	int         pad1;
	int         pad2;   /* Make it 32 bytes                             */
} HeapInfo;

#define AL_CACHE_ALIGN  15

/*
 * synth stuff
 */

typedef struct PVoice_s {
	ALLink               node;
	struct ALVoice_s    *vvoice;
	ALFilter            *channelKnob;
	ALLoadFilter        decoder;
	ALResampler         resampler;
	ALEnvMixer		envmixer;
	int                 offset;
} PVoice;

/*
 * prototypes for private driver functions
 */
ALParam         *__allocParam(void);
void            __freeParam(ALParam *param);
void            _freePVoice(ALSynth *drvr, PVoice *pvoice);
void            _collectPVoices(ALSynth *drvr);

int             _timeToSamples(ALSynth *ALSynth, int micros);
ALMicroTime     _samplesToTime(ALSynth *synth, int samples);

void            _init_lpfilter(ALLowPass *lp);

#endif

