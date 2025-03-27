#include <ultra64.h>
#include "constants.h"
#include "game/pak.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/joy.h"
#include "data.h"
#include "types.h"

/**
 * PD polls the controllers from the scheduler's thread. The scheduler polls the
 * controllers on each retrace and stores the results inside g_JoyData->samples.
 * This allows the main thread to access a history of controller states since
 * the last rendered frame. For example, under laggy conditions the player might
 * press and release a button between two frames and the main thread can tell
 * that this has happened even if the button was unpressed during both the
 * previous and current frame.
 *
 * The samples array contains 20 elements and is written to in a cyclic manner.
 * These samples are split into two partitions: cur and next. cur refers to
 * samples which are currently visible to the main thread on this frame, and
 * samples in next are samples which have been added since the start of the
 * current frame and will be made visible on the next frame.
 *
 * At the start of a frame, the main thread informs the joy system that it's
 * ready to consume more samples. The joy system then moves the partition
 * boundaries so that the old next partition becomes the new cur, and everything
 * else becomes available for next.
 *
 * If all 20 samples are in use, the joy system will overwrite the most recent
 * sample in the next partition.
 */

#define NUM_DATA    2
#define NUM_SAMPLES 20
#define NUM_PADS    MAXCONTROLLERS

struct contsample {
	OSContPad pads[NUM_PADS];
};

struct joydata {
	struct contsample samples[NUM_SAMPLES];
	int curlast;
	int curstart;
	int nextlast;
	int nextsecondlast;
	unsigned int buttonspressed[NUM_PADS];
	unsigned int buttonsreleased[NUM_PADS];
	int unk200;
};

struct joydata g_JoyData[NUM_DATA];
int g_JoyDisableCooldown[NUM_PADS];
OSContStatus g_JoyContStatuses[NUM_PADS];
uint8_t g_JoyPfsStates[100];

struct joydata *g_JoyDataPtr = &g_JoyData[0];
bool g_JoyBusy = false;

// Number of times per pad that different inputs were attempted to be read
// when controller was disconnected or not ready.
unsigned int g_JoyBadReadsStickX[NUM_PADS] = {0};
unsigned int g_JoyBadReadsStickY[NUM_PADS] = {0};
unsigned int g_JoyBadReadsRStickX[NUM_PADS] = {0};
unsigned int g_JoyBadReadsRStickY[NUM_PADS] = {0};
unsigned int g_JoyBadReadsButtons[NUM_PADS] = {0};
unsigned int g_JoyBadReadsButtonsPressed[NUM_PADS] = {0};

uint8_t g_JoyConnectedControllers = 0;
bool g_JoyQueuesCreated = false;
bool g_JoyInitDone = false;
bool g_JoyNeedsInit = true;
unsigned int g_JoyCyclicPollDisableCount = 0;
unsigned int var8005eec0 = 1;
int g_JoyNextPfsStateIndex = 0;

bool g_JoyPfsPollMasterEnabled = true;
int g_JoyPfsPollInterval = 0;
int g_JoyPfsPollTimeRemaining = -1;
unsigned int g_JoyPfsPollCount = 0;
bool g_JoyPfsPollEnabled = false;
bool g_JoyCyclicPollingLocked = true;

void joyLockCyclicPolling(void)
{
	if (g_JoyCyclicPollingLocked) {
		joyDisableCyclicPolling();
		g_JoyCyclicPollingLocked = false;
	}
}

void joyUnlockCyclicPolling(void)
{
	if (!g_JoyCyclicPollingLocked) {
		joyEnableCyclicPolling();
		g_JoyCyclicPollingLocked = true;
	}
}

void joySetPfsPollEnabled(bool enabled)
{
	g_JoyPfsPollEnabled = enabled;
}

bool joyIsPfsPollEnabled(void)
{
	return g_JoyPfsPollEnabled;
}

void joySetPfsPollInterval(int value)
{
	g_JoyPfsPollTimeRemaining = g_JoyPfsPollInterval = value * 11000;
}

void joySetDefaultPfsPollInterval(void)
{
	joySetPfsPollInterval(10);
}

/**
 * Remove an item from the beginning of the g_JoyPfsStates array,
 * shift the rest of the array back and return the removed item.
 */
int joyShiftPfsStates(void)
{
	int pfsstate = 0;
	int i;

	if (g_JoyNextPfsStateIndex) {
		pfsstate = g_JoyPfsStates[0];

		if (g_JoyNextPfsStateIndex > 1) {
			for (i = 0; i < g_JoyNextPfsStateIndex; i++) {
				g_JoyPfsStates[i] = g_JoyPfsStates[i + 1];
			}

			g_JoyNextPfsStateIndex--;
		}
	}

	return pfsstate;
}

void joyRecordPfsState(uint8_t pfsstate)
{
	if (g_JoyNextPfsStateIndex + 1 >= ARRAYCOUNT(g_JoyPfsStates)) {
		joyShiftPfsStates();
	}

	if (g_JoyNextPfsStateIndex == 0 || pfsstate != g_JoyPfsStates[g_JoyNextPfsStateIndex - 1]) {
		g_JoyPfsStates[g_JoyNextPfsStateIndex] = pfsstate;
		g_JoyNextPfsStateIndex++;
	}
}

/**
 * Scan controllers for controller paks, but only under certain conditions.
 *
 * force 0 = poll based on the configured poll frequency and if not disabled
 * force 1 = poll based on the configured poll frequency even if disabled
 * force 2 = poll now
 */
void joyPollPfs(int force)
{
	static unsigned int thiscount = 0;
	static unsigned int prevcount = 0;
	static unsigned int doingit = false;
	unsigned int diffcount;
	unsigned int value;

	if (g_JoyPfsPollMasterEnabled
			&& (force == 2 || (g_JoyPfsPollInterval && (force || ((g_JoyCyclicPollDisableCount == 0 || !g_JoyCyclicPollingLocked) && g_JoyPfsPollEnabled))))
			&& !doingit) {
		doingit = true;
		prevcount = thiscount;
		thiscount = osGetCount();
		diffcount = (thiscount - prevcount) / 256;
		value = g_JoyPfsPollInterval * 2;

		if (diffcount > value) {
			diffcount = value;
		}

		g_JoyPfsPollTimeRemaining -= diffcount;

		if (g_JoyPfsPollTimeRemaining < 0
				|| force == 2
				|| (force == 1 && g_JoyPfsPollTimeRemaining < 0 && -g_JoyPfsPollTimeRemaining > g_JoyPfsPollInterval)) {
			uint8_t bitpattern = 0;

			g_JoyPfsPollCount++;

			if (force) {
				joyDisableCyclicPolling();
			}

			bitpattern =
   			(inputRumbleSupported(0) << 0) |
   			(inputRumbleSupported(1) << 1) |
    		(inputRumbleSupported(2) << 2) |
    		(inputRumbleSupported(3) << 3);

			int i = 0;
			for (i = 0; i < MAXCONTROLLERS; ++i) {
				if (inputRumbleSupported(i)) {
					bitpattern |= 1 << i;
				}
			}

			if (force) {
				joyEnableCyclicPolling();
			}

			bitpattern |= 0x10; // eeprom

			joyRecordPfsState(bitpattern);

			g_JoyPfsPollTimeRemaining = g_JoyPfsPollInterval;
		}

		doingit = false;
	}
}

void joySetPfsTemporarilyPlugged(int8_t index)
{
	joyRecordPfsState(0);
}

void joyInit(void)
{
	int i;
	int j;

	g_JoyQueuesCreated = true;

	for (i = 0; i < NUM_DATA; i++) {
		g_JoyData[i].curlast = 0;
		g_JoyData[i].curstart = 0;
		g_JoyData[i].nextlast = 0;
		g_JoyData[i].nextsecondlast = 0;
		g_JoyData[i].unk200 = -1;

		for (j = 0; j < NUM_PADS; j++) {
			g_JoyData[i].samples[0].pads[j].button = 0;
			g_JoyData[i].samples[0].pads[j].stick_x = 0;
			g_JoyData[i].samples[0].pads[j].stick_y = 0;
			g_JoyData[i].samples[0].pads[j].errnum = 0;
		}
	}

	for (i = 0; i < NUM_PADS; i++) {
		g_JoyDisableCooldown[i] = 0;
	}
}

/**
 * Disable all input on all controllers for 60 frames, or until the player has
 * released all inputs.
 *
 * It's used to prevent the player from accidentally skipping cutscenes and
 * progressing past endscreens if they are holding buttons when they are
 * started.
 */
void joyDisableTemporarily(void)
{
	int i;

	for (i = 0; i < NUM_PADS; i++) {
		g_JoyDisableCooldown[i] = TICKS(60);
	}
}

void joyReset(void)
{

	if (g_JoyQueuesCreated) {

		joyCheckStatus();

		var8005eec0 = 1;
	}
}

void joyCheckStatus(void)
{
	static uint8_t prevconnected = 0xff;

	// osContInit should be called only once. The first time this function is
	// called it'll take the first branch here, and all subsequent calls will
	// take the second branch.
	if (g_JoyNeedsInit) {
		int i;
		g_JoyNeedsInit = false;
		osContInit(&g_JoyConnectedControllers, g_JoyContStatuses);
		g_JoyInitDone = true;

		for (i = 0; i < NUM_PADS; i++) {
			joyStopRumble(i, false);
		}
	} else {
		unsigned int slots = 0xf;
		int i;

		for (int i = 0; i < MAXCONTROLLERS; ++i) {
			if (inputControllerConnected(i)) {
				g_JoyContStatuses[i].errnum = 0;
				g_JoyContStatuses[i].type = CONT_ABSOLUTE;
				g_JoyContStatuses[i].status = CONT_CARD_ON;
			} else {
				g_JoyContStatuses[i].errnum = CONT_NO_RESPONSE_ERROR;
				g_JoyContStatuses[i].type = 0;
				g_JoyContStatuses[i].status = 0;
			}
		}

		for (i = 0; i < ARRAYCOUNT(g_JoyContStatuses); i++) {
			if (g_JoyContStatuses[i].errnum & CONT_NO_RESPONSE_ERROR) {
				slots -= 1 << i;
			}
		}

		g_JoyConnectedControllers = slots;
	}

	if (prevconnected != g_JoyConnectedControllers) {
		int i = 0;
		int index = 0;

		for (; i < NUM_PADS; i++) {
			if (g_JoyConnectedControllers & (1 << i)) {
				g_Vars.playertojoymap[index++] = i;
			}
		}

		prevconnected = g_JoyConnectedControllers;
	}
}

int8_t contGetFreeSlot(void)
{
	int i;

	if (g_JoyDataPtr->unk200 >= 0) {
		return g_JoyDataPtr->unk200;
	}

	for (i = 0; i < NUM_PADS; i++) {
		if ((g_JoyConnectedControllers & (1 << i)) == 0) {
			return i;
		}
	}

	return NUM_PADS;
}

unsigned int joyGetConnectedControllers(void)
{
	return g_JoyConnectedControllers;
}

void joyConsumeSamples(struct joydata *joydata)
{
	int8_t i;
	int samplenum;
	unsigned int buttons1;
	unsigned int buttons2;

	joydata->curstart = joydata->curlast;
	joydata->curlast = joydata->nextlast;

	for (i = 0; i < NUM_PADS; i++) {
		joydata->buttonspressed[i] = 0;
		joydata->buttonsreleased[i] = 0;

		if (joydata->curlast != joydata->curstart) {
			samplenum = (joydata->curstart + 1) % NUM_SAMPLES; \
			while (true) {
				buttons1 = joydata->samples[samplenum].pads[i].button;
				buttons2 = joydata->samples[(samplenum + NUM_SAMPLES - 1) % NUM_SAMPLES].pads[i].button;

				joydata->buttonspressed[i] |= buttons1 & ~buttons2;
				joydata->buttonsreleased[i] |= ~buttons1 & buttons2;

				if (g_JoyDisableCooldown[i] > 0) {
					if (joydata->samples[samplenum].pads[i].button == 0
							&& joydata->samples[samplenum].pads[i].stick_x < 15
							&& joydata->samples[samplenum].pads[i].stick_x > -15
							&& joydata->samples[samplenum].pads[i].stick_y < 15
							&& joydata->samples[samplenum].pads[i].stick_y > -15
							&& joydata->samples[samplenum].pads[i].rstick_x < 15
							&& joydata->samples[samplenum].pads[i].rstick_x > -15
							&& joydata->samples[samplenum].pads[i].rstick_y < 15
							&& joydata->samples[samplenum].pads[i].rstick_y > -15
					) {
						g_JoyDisableCooldown[i] = 0;
					} else {
						g_JoyDisableCooldown[i]--;
					}
				}

				if (samplenum == joydata->curlast) {
					break;
				}

				samplenum = (samplenum + 1) % NUM_SAMPLES;
			}
		}
	}
}

/**
 * The use of the static variable suggests that the function is able to be
 * called recursively, but its behaviour should not be run when recursing.
 */
void joy00014238(void)
{
	static bool doingit = false;
	int i;

	if (!doingit) {
		doingit = true;

		for (i = 0; i < NUM_PADS; i++) {
			if (joyGetPakState2(i) == PAKSTATE_13) {
				pakSetState(i, PAKSTATE_READY);
			}
		}

		joysTickRumble();

		doingit = false;
	}
}

void joyDebugJoy(void)
{
	static unsigned int var8005ef08 = 0;

	if (g_Vars.paksneededformenu) {
		joyPollPfs(1);
	}

	joyConsumeSamples(&g_JoyData[0]);

	if (joyIsCyclicPollingEnabled() && var8005eec0 && joyGetNumSamples() <= 0) {
		joyDisableCyclicPolling();
		joy00014238();
		joyEnableCyclicPolling();
		joyConsumeSamples(&g_JoyData[0]);
	}
}

void joyReadData(void)
{
	int index = (g_JoyData[0].nextlast + 1) % NUM_SAMPLES;

	if (index == g_JoyData[0].curstart) {
		// If the sample queue is full, don't overwrite the oldest sample.
		// Instead, overwrite the most recent.
		index = g_JoyData[0].nextlast;
	}

	for (int i = 0; i < MAXCONTROLLERS; ++i) {
		g_JoyData[0].samples[index].pads[i].button = 0;
		g_JoyData[0].samples[index].pads[i].stick_x = 0;
		g_JoyData[0].samples[index].pads[i].stick_y = 0;
		g_JoyData[0].samples[index].pads[i].rstick_x = 0;
		g_JoyData[0].samples[index].pads[i].rstick_y = 0;
		if (inputReadController(i, &g_JoyData[0].samples[index].pads[i]) < 0) {
			g_JoyData[0].samples[index].pads[i].errnum = CONT_NO_RESPONSE_ERROR;
		} else {
			g_JoyData[0].samples[index].pads[i].errnum = 0;
		}
	}

	//osContGetReadData(g_JoyData[0].samples[index].pads);

	g_JoyData[0].nextlast = index;
	g_JoyData[0].nextsecondlast = (g_JoyData[0].nextlast + NUM_SAMPLES - 1) % NUM_SAMPLES;
}

void joySetAllowTitleInput(bool value)
{
	var8005eec0 = value;
}

int joyGetNumSamples(void)
{
	return (g_JoyDataPtr->curlast - g_JoyDataPtr->curstart + NUM_SAMPLES) % NUM_SAMPLES;
}

int joyGetRStickXOnSample(int samplenum, int8_t contpadnum) {
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsRStickX[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].rstick_x;
}

int joyGetRStickYOnSample(int samplenum, int8_t contpadnum) {
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].rstick_y;
}

int joyGetStickXOnSample(int samplenum, int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickX[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].stick_x;
}

int joyGetStickYOnSample(int samplenum, int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].stick_y;
}

int joyGetRStickYOnSampleIndex(int samplenum, int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum) % NUM_SAMPLES].pads[contpadnum].rstick_y;
}

int joyGetStickYOnSampleIndex(int samplenum, int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum) % NUM_SAMPLES].pads[contpadnum].stick_y;
}

unsigned int joyGetButtonsOnSample(int samplenum, int8_t contpadnum, unsigned int mask)
{
	unsigned int button;

	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsButtons[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	button = g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].button;

	return button & mask;
}

unsigned int joyGetButtonsPressedOnSample(int samplenum, int8_t contpadnum, unsigned int mask)
{
	unsigned int button1;
	unsigned int button2;

	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsButtonsPressed[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	button1 = g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum + 1) % NUM_SAMPLES].pads[contpadnum].button;
	button2 = g_JoyDataPtr->samples[(g_JoyDataPtr->curstart + samplenum) % NUM_SAMPLES].pads[contpadnum].button;

	return (button1 & ~button2) & mask;
}

/**
 * Count the number of times the buttons specified by mask were held during the
 * specific samples given in checksamples.
 *
 * For example, if checksamples[5] is nonzero and a button was pressed on
 * samplenum 5 which matches the mask, count is incremented.
 */
int joyCountButtonsOnSpecificSamples(unsigned int *checksamples, int8_t contpadnum, unsigned int mask)
{
	int count = 0;
	int index = 0;
	int i;
	unsigned int button;

	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsButtons[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	i = (g_JoyDataPtr->curstart + 1) % NUM_SAMPLES;

	while (true) {
		if (checksamples == NULL || checksamples[index]) {
			button = g_JoyDataPtr->samples[i].pads[contpadnum].button;

			if (button & mask) {
				count++;
			}
		}

		if (i == g_JoyDataPtr->curlast) {
			break;
		}

		i = (i + 1) % NUM_SAMPLES;
		index++;
	}

	return count;
}

int8_t joyGetStickX(int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickX[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[g_JoyDataPtr->curlast].pads[contpadnum].stick_x;
}

int8_t joyGetRStickX(int8_t contpadnum) {
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsRStickX[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[g_JoyDataPtr->curlast].pads[contpadnum].rstick_x;
}

int8_t joyGetRStickY(int8_t contpadnum) {
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsRStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[g_JoyDataPtr->curlast].pads[contpadnum].rstick_y;
}

int8_t joyGetStickY(int8_t contpadnum)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsStickY[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[g_JoyDataPtr->curlast].pads[contpadnum].stick_y;
}

unsigned int joyGetButtons(int8_t contpadnum, unsigned int mask)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsButtons[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->samples[g_JoyDataPtr->curlast].pads[contpadnum].button & mask;
}

unsigned int joyGetButtonsPressedThisFrame(int8_t contpadnum, unsigned int mask)
{
	if (g_JoyDataPtr->unk200 < 0 && (g_JoyConnectedControllers >> contpadnum & 1) == 0) {
		g_JoyBadReadsButtonsPressed[contpadnum]++;
		return 0;
	}

	if (g_JoyDisableCooldown[contpadnum] > 0) {
		return 0;
	}

	return g_JoyDataPtr->buttonspressed[contpadnum] & mask;
}

bool joyIsCyclicPollingEnabled(void)
{
	return g_JoyCyclicPollDisableCount ? false : true;
}

/**
 * If cyclic polling is enabled, send a message to the scheduler thread telling
 * it to update the joy state (connected controllers, PFS etc). Then block while
 * waiting for its done message to come back, and increment the disable count.
 *
 * If cyclic polling was already disabled, simply increase the disable count.
 */
void joyDisableCyclicPolling(void)
{
	g_JoyCyclicPollDisableCount++;
}

/**
 * Indicate that the caller is done with cyclic polling being disabled,
 * and enable cyclic polling if there are no callers left who want it disabled.
 */
void joyEnableCyclicPolling(void)
{

	g_JoyCyclicPollDisableCount--;
}

void joyDestroy(void)
{
	int i;

	for (i = 0; i < NUM_PADS; i++) {
		if (osMotorProbe(PFS(i), i) == 0) {
			osMotorStop(PFS(i));
			osMotorStop(PFS(i));
			osMotorStop(PFS(i));
		}
	}
}

void joyGetContpadNumsForPlayer(int8_t playernum, int *pad1, int *pad2)
{
	if (g_Vars.normmplayerisrunning) {
		*pad1 = g_Vars.playerstats[playernum].mpindex;
		*pad2 = -1;
		return;
	}

	*pad1 = playernum;

	uint8_t controlmode = g_PlayerConfigsArray[g_Vars.playerstats[playernum].mpindex].controlmode;
	if (controlmode >= CONTROLMODE_21 && controlmode < CONTROLMODE_PC) {
		*pad2 = PLAYERCOUNT() + playernum;
		return;
	}

	*pad2 = -1;
}

void joyStopRumble(int8_t arg0, bool disablepolling)
{
	if (arg0 != SAVEDEVICE_GAMEPAK) {
		int device = arg0;

		if (g_Paks[device].type != PAKTYPE_MEMORY && g_Paks[device].type != PAKTYPE_GAMEBOY) {
			if (disablepolling) {
				joyDisableCyclicPolling();
			}

			if (osMotorProbe(PFS(device), device) == 0) {
				osMotorStop(PFS(device));
				osMotorStop(PFS(device));
				osMotorStop(PFS(device));
			}

			if (disablepolling) {
				joyEnableCyclicPolling();
			}

			if (g_Paks[device].rumblestate != RUMBLESTATE_DISABLED_STOPPING
					&& g_Paks[device].rumblestate != RUMBLESTATE_DISABLED_STOPPED) {
				g_Paks[device].rumblestate = RUMBLESTATE_ENABLED_STOPPING;
			}

			g_Paks[device].rumblettl = -1;
		}
	}
}

int joyGetPakState(int8_t device)
{
	return g_Paks[device].state;
}

int joyGetPakState2(int8_t device)
{
	return joyGetPakState(device);
}

void joysTickRumble(void)
{
	int i;

	for (i = 0; i < NUM_PADS; i++) {
		if (g_Paks[i].state == PAKSTATE_READY && g_Paks[i].type == PAKTYPE_RUMBLE) {
			switch (g_Paks[i].rumblestate) {
			case RUMBLESTATE_ENABLED_STARTING:
				g_Paks[i].rumblestate = RUMBLESTATE_ENABLED_RUMBLING;
				osMotorStart(PFS(i));
				break;
			case RUMBLESTATE_ENABLED_RUMBLING:
				if (g_Paks[i].rumblepulsestopat != -1) {
					if (g_Paks[i].rumblepulsetimer == 0) {
						osMotorStart(PFS(i));
					} else if (g_Paks[i].rumblepulsestopat == g_Paks[i].rumblepulsetimer) {
						osMotorStop(PFS(i));
					}

					g_Paks[i].rumblepulsetimer++;

					if (g_Paks[i].rumblepulselen == g_Paks[i].rumblepulsetimer) {
						g_Paks[i].rumblepulsetimer = 0;
					}
				}

				g_Paks[i].rumblettl--;

				if (g_Paks[i].rumblettl < 0) {
					g_Paks[i].rumblestate = RUMBLESTATE_ENABLED_STOPPING;
				}
				break;
			case RUMBLESTATE_ENABLED_STOPPING:
				g_Paks[i].rumblestate = RUMBLESTATE_ENABLED_STOPPED;
				osMotorStop(PFS(i));
				break;
			case RUMBLESTATE_DISABLED_STOPPING:
				osMotorStop(PFS(i));
				g_Paks[i].rumblestate = RUMBLESTATE_DISABLED_STOPPED;
				break;
			case RUMBLESTATE_ENABLING:
				g_Paks[i].rumblestate = RUMBLESTATE_ENABLED_STOPPED;
				g_Paks[i].rumblettl = -1;
				break;
			}
		}
	}
}
