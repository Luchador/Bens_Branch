#ifndef _IN_FS_H
#define _IN_FS_H

#include <stdio.h>
#include <stdint.h>

#define FS_MAXPATH 1024

int fsInit(void);

const char *fsFullPath(const char *relPath);

int fsPathIsAbsolute(const char *path);
int fsPathIsCwdRelative(const char *path);

void *fsFileLoad(const char *name, uint32_t *outSize);
int fsFileLoadTo(const char *name, void *dst, uint32_t dstSize);
int fsFileSize(const char *name);

FILE *fsFileOpenWrite(const char *name);
FILE *fsFileOpenRead(const char *name);
void fsFileFree(FILE *f);

const char *fsGetModDir(void);

#endif
