#include <PR/ultratypes.h>
#include <PR/ultrasched.h>
#include "lib/sched.h"
#include "constants.h"
#include "game/menugfx.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/audiomgr.h"
#include "lib/rzip.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "data.h"
#include "types.h"
#include "game/debug.h"

#include "video.h"
#include "audio.h"
#include "input.h"
#include "mixer.h"

struct artifact g_ArtifactLists[3][240];
u8 g_SchedSpecialArtifactIndexes[3];
s32 g_SchedWriteArtifactsIndex;
s32 g_SchedFrontArtifactsIndex;
s32 g_SchedPendingArtifactsIndex;

s32 var8005ce74 = 0;
f32 g_ViXScalesBySlot[NUM_GFXTASKS] = {1, 1};
f32 g_ViYScalesBySlot[NUM_GFXTASKS] = {1, 1};
bool g_SchedViModesPending[NUM_GFXTASKS] = {false, false};
s32 g_ViUnblackTimer = NUM_FRAMEBUFFERS + 1;
s32 g_ViShakeDirection = 1;
s32 g_ViShakeIntensity = 0;
f32 g_ViShakeIntensityMult = 1.f;
s32 g_ViShakeTimer = 0;
bool g_SchedIsFirstTask = true;

s32 g_PrevFrameFb = -1;
s32 g_BlurFb = -1;
s32 g_BlurFbCapTimer = -1;
bool g_BlurFbDirty = true;

void __scUpdateViMode(void)
{
	if (g_SchedIsFirstTask) {
		g_SchedIsFirstTask = false;
	}

	var8005ce74 = (var8005ce74 + 1) % 2;

	if (g_SchedViModesPending[1 - var8005ce74]) {
		// TODO: make this a little less awkward
		extern struct rend_vidat *g_ViBackData;
		videoUpdateNativeResolution(g_ViBackData->bufx, g_ViBackData->bufy);
		g_SchedViModesPending[1 - var8005ce74] = false;
	}

	if (g_ViUnblackTimer != 0 && g_ViUnblackTimer <= NUM_FRAMEBUFFERS) {
		g_ViUnblackTimer--;
	}
}

/**
 * Nintendo's sheduler accepts tasks on a "command" message queue.
 * This isn't used here.
 *
 * In PD, the main and audio threads submit tasks by calling this function
 * instead. It temporarily increases the calling thread's priority above the
 * scheduler, adds the task to the linked list directly and attempts to execute
 * it. This is faster than the queue method because it avoids switching threads.
 */
void schedSubmitTask(OSSched *sc, OSScTask *t)
{
	if (t->list.t.type == M_GFXTASK) {
		videoSubmitCommands((Gfx *)t->list.t.data_ptr);
	}
}

void schedStartFrame(OSSched *sc)
{
	videoStartFrame();
}

void schedAudioFrame(OSSched *sc)
{
	s32 i;

	if (!g_SndDisabled) {
		for (i = 0; i < g_Vars.diffframe60; i++) {
			amgrFrame();
			audioEndFrame();
		}
	}
}

/**
 * Handle a retrace (vsync) event.
 *
 * Audio tasks are scheduled based on retrace + a timer (approximately 6ms).
 * On NTSC, this is done on every second frame if 8MB, or every second frame
 * if 4MB. I guess less memory means the audio queue has to be kept smaller
 * and processed more frequently. On PAL, it's every second frame regardless.
 *
 * Controller input is polled here.
 *
 * Lastly, if there's crash information available then it will be checked and
 * rendered periodically (once every 16 retraces). I guess this makes it render
 * if the RDP has hung.
 */
void schedEndFrame(OSSched *sc)
{
	sc->frameCount++;

	viHandleRetrace();

	inputUpdate();

	joyReadData();
	joy00014238();

	schedAudioFrame(sc);
	videoEndFrame();

	if (g_MainIsBooting == 0) {
		schedConsiderScreenshot();
	}

	// check for vid mode changes
	__scUpdateViMode();
}

void schedInitArtifacts(void)
{
	s32 i;
	s32 j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < MAX_ARTIFACTS; j++) {
			g_ArtifactLists[i][j].type = ARTIFACTTYPE_FREE;
		}

		g_SchedSpecialArtifactIndexes[i] = 0;
	}
}

/**
 * The write list is an artifact list that is not currently being displayed on
 * the screen. Update logic can write here to put artifacts on the next frame.
 */
struct artifact *schedGetWriteArtifacts(void)
{
	return g_ArtifactLists[g_SchedWriteArtifactsIndex];
}

/**
 * The front list is the artifact list that is currently being displayed on the
 * screen. Rendering logic reads this list. The list may be re-used for multiple
 * frames in a row during lag.
 */
struct artifact *schedGetFrontArtifacts(void)
{
	return g_ArtifactLists[g_SchedFrontArtifactsIndex];
}

/**
 * The pending list is possibly misnamed. I'm not sure how this list works.
 *
 * @TODO: Investigate.
 */
struct artifact *schedGetPendingArtifacts(void)
{
	return g_ArtifactLists[g_SchedPendingArtifactsIndex];
}

void schedIncrementWriteArtifacts(void)
{
	g_SchedWriteArtifactsIndex = (g_SchedWriteArtifactsIndex + 1) % 3;
}

void schedIncrementFrontArtifacts(void)
{
	g_SchedFrontArtifactsIndex = (g_SchedFrontArtifactsIndex + 1) % 3;
}

void schedIncrementPendingArtifacts(void)
{
	g_SchedPendingArtifactsIndex = (g_SchedPendingArtifactsIndex + 1) % 3;
}

void schedResetArtifacts(void)
{
	g_SchedWriteArtifactsIndex = 0;
	g_SchedFrontArtifactsIndex = 1;
	g_SchedPendingArtifactsIndex = 0;
}

void schedUpdatePendingArtifacts(void)
{
	g_SchedSpecialArtifactIndexes[g_SchedPendingArtifactsIndex] = 0;
	schedIncrementPendingArtifacts();
}

void schedConsiderScreenshot(void)
{
	if (g_MenuData.screenshottimer == 1) {
		menugfxCreateBlur();

		g_MenuData.screenshottimer = 0;
	}

	if (g_BlurFbCapTimer == 0) {
		videoCopyFramebuffer(g_BlurFb, 0, -1, -1);
		g_BlurFbCapTimer = -1;
		g_BlurFbDirty = false;
	} else if (g_BlurFbCapTimer > 0) {
		--g_BlurFbCapTimer;
	} else if (g_BlurFbCapTimer < 0) {
		// no blur requested this frame, mark blur fb dirty
		g_BlurFbDirty = true;
	}

	if (g_MenuData.screenshottimer >= 2) {
		g_MenuData.screenshottimer--;
	}
}
