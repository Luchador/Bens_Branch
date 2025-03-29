#ifndef IN_GAME_PROPSND_H
#define IN_GAME_PROPSND_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void psReset(void);

void psStop(void);

bool psPropHasSoundWithContext(struct prop *prop, int type);
void psStopSound(struct prop *prop, int type, uint16_t flags);
int psCalculateVolumeFromDistance(float playerdist, float dist1, float dist2, float dist3, int fullvolume);
int psGetVolume(int channelnum);
void psCreateIfNotDupe(struct prop *prop, int16_t soundnum, int type);
void psStopChannel(int channelnum);
int psGetSubtitleOpacity(int channelnum);
void psTickChannel(int channelnum);
void psTick(void);
void psSetPitch(struct prop *prop, float targetpitch, int changespeed);
void psSetVolume(struct prop *prop, int vol);
void psStopOneShootChannel(struct prop *prop);
int16_t psCreate(struct pschannel *channel, struct prop *prop, int16_t soundnum, int16_t padnum, int vol, uint16_t flags, uint16_t flags2, int type, struct coord *pos, float pitch, RoomNum *rooms, int room, float dist1, float dist2, float dist3);
int psPlayFromProp(int channelnum, int16_t soundnum, int vol, struct prop *prop, int16_t type, uint16_t flags);
void psMuteChannel(int channelnum);
bool psIsChannelFree(int channelnum);
void psModify(int channelnum, int arg1, int16_t padnum, struct prop *prop, int volchangetimer60, int dist2, int dist3, uint16_t flags);
int psCalculateVol(struct coord *pos, float arg1, float arg2, float arg3, RoomNum *rooms, int16_t soundnum, int arg6, float *arg7);
int psCalculatePan3(int degrees, float arg1, struct pschannel *channel);
int psCalculatePan2(struct coord *pos, int arg1, float arg2, struct pschannel *channel);
int psCalculatePan(struct coord *pos, float dist1, float dist2, float dist3, float playerdist, bool arg5, struct pschannel *channel);
void psGetTheoreticalVolPan(struct coord *pos, RoomNum *rooms, int16_t soundnum, int *vol, int *pan);
void psApplyVolPan(struct sndstate *handle, struct coord *pos, float dist1, float dist2, float dist3, RoomNum *rooms, int16_t soundnum, int arg7, float *distanceptr);
int psGetRandomSparkSound(void);
int psGetDuration60(int channelnum);

#endif
