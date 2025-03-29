#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <PR/os.h>
#include "platform.h"
#include "system.h"
#include "input.h"
#include "video.h"
#include "audio.h"
#include "fs.h"

#define EEPROM_SIZE (256 * 8)
#define EEPROM_FNAME "eeprom.bin"
#define EEPROM_PATH "$S/" EEPROM_FNAME

static uint8_t eeprom[EEPROM_SIZE];
static char eepromPath[FS_MAXPATH + 1];
static int eepromLoaded = 0;

/* Cont */

int osContInit(uint8_t *bitpattern, OSContStatus *data)
{
	if (bitpattern) {
		*bitpattern = inputControllerMask();
	}
	if (data) {
		for (int i = 0; i < MAXCONTROLLERS; ++i, ++data) {
			if (inputControllerConnected(i)) {
				data->errnum = 0;
				data->type = CONT_ABSOLUTE;
				data->status = CONT_CARD_ON;
			} else {
				data->errnum = CONT_NO_RESPONSE_ERROR;
				data->type = 0;
				data->status = 0;
			}
		}
	}
	return 0;
}

/* Motor */

int osMotorProbe(OSPfs* pfs, int channel)
{
	if (pfs && inputRumbleSupported(channel)) {
		pfs->channel = channel;
		pfs->activebank = 0xff;
		pfs->status = 0x8; // PFS_MOTOR_INITIALIZED
		return 0;
	}
	return PFS_ERR_NOPACK;
}

int __osMotorAccess(OSPfs *pfs, int cmd)
{
	if (!pfs || pfs->channel < 0 || pfs->channel >= INPUT_MAX_CONTROLLERS) {
		return PFS_ERR_NOPACK;
	}

	const float strength = (float)(cmd == 1);
	inputRumble(pfs->channel, strength, 5.f); // hope someone turns it off in those 5 seconds

	return 0;
}

/* Eeprom */

static inline void osEepromSetPath(void)
{
	const char *extPath = sysArgGetString("--eeprom-file");
	if (extPath && extPath[0]) {
		if (extPath[0] == '$' || fsPathIsAbsolute(extPath) || fsPathIsCwdRelative(extPath)) {
			strncpy(eepromPath, extPath, FS_MAXPATH);
		} else {
			// just a filename, look for it in the save dir
			snprintf(eepromPath, FS_MAXPATH, "$S/%s", extPath);
		}
	} else {
		strncpy(eepromPath, EEPROM_PATH, FS_MAXPATH);
	}
}

static inline void osEeepromLoad(const char *fname)
{
	if (!eepromLoaded) {
		eepromLoaded = 1;
		FILE *fp = fsFileOpenRead(fname);
		if (fp) {
			fread(eeprom, 1, EEPROM_SIZE, fp);
			fsFileFree(fp);
		} else {
			sysLogPrintf(LOG_NOTE, "could not read EEPROM from `%s`: %s", fsFullPath(fname), strerror(errno));
		}
	}
}

static inline void osEeepromSave(const char *fname)
{
	FILE* fp = fsFileOpenWrite(fname);
	if (fp) {
		fwrite(eeprom, 1, EEPROM_SIZE, fp);
		fsFileFree(fp);
	} else {
		sysLogPrintf(LOG_ERROR, "could not save EEPROM to `%s`: %s", fsFullPath(fname), strerror(errno));
	}
}

int osEepromLongRead(uint8_t address, uint8_t *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(buffer, eeprom + address * 8, nbytes);

	return 0;
}

int osEepromLongWrite(uint8_t address, uint8_t *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(eeprom + address * 8, buffer, nbytes);

	osEeepromSave(eepromPath);

	return 0;
}

/* libc compatibility wrappers */

#ifndef PLATFORM_OSX

void bzero(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

void bcopy(const void *src, void *dst, size_t n)
{
	memcpy(dst, src, n);
}

int bcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}

#endif
