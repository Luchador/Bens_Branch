#pragma once

#include <PR/ultrasched.h>
#include "types.h"

void schedInitArtifacts(void);
void schedResetArtifacts(void);
struct artifact *schedGetWriteArtifacts(void);
struct artifact *schedGetFrontArtifacts(void);
void schedIncrementWriteArtifacts(void);
void schedIncrementFrontArtifacts(void);
void schedUpdatePendingArtifacts(void);
void schedConsiderScreenshot(void);
void schedEndFrame();
