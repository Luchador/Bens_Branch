#ifndef _IN_BOOT_SCHED_H
#define _IN_BOOT_SCHED_H
#include <ultra64.h>
#include <PR/ultrasched.h>
#include "types.h"

void schedSubmitTask(OSSched *sc, OSScTask *t);
void schedInitArtifacts(void);
void schedResetArtifacts(void);
struct artifact *schedGetWriteArtifacts(void);
struct artifact *schedGetFrontArtifacts(void);
void schedIncrementWriteArtifacts(void);
void schedIncrementFrontArtifacts(void);
void schedUpdatePendingArtifacts(void);
void schedConsiderScreenshot(void);
void schedStartFrame(OSSched *sc);
void schedEndFrame(OSSched *sc);

#endif
