#ifndef _IN_SYSTEM_H
#define _IN_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum LogLevel {
  LOG_NOTE,
  LOG_WARNING,
  LOG_ERROR,
};

void sysInitArgs(int argc, const char **argv);
void sysInit(void);

int sysArgCheck(const char *arg);
const char *sysArgGetString(const char *arg);
int sysArgGetInt(const char *arg, int defval);

uint64_t sysGetMicroseconds(void);

void sysFatalError(const char *fmt, ...) __attribute__((noreturn));

int sysLogIsOpen(void);
void sysLogPrintf(int level, const char *fmt, ...);

void sysGetExecutablePath(char *outPath, const uint32_t outLen);
void sysGetHomePath(char *outPath, const uint32_t outLen);

void *sysMemAlloc(const uint32_t size);
void *sysMemZeroAlloc(const uint32_t size);
void *sysMemRealloc(void *ptr, const uint32_t newSize);
void sysMemFree(void *ptr);

// hns is specified in 100ns units
void sysSleep(const int64_t hns);

// yield CPU if supported (e.g. during a busy loop)
void sysCpuRelax(void);

void crashInit(void);
void crashShutdown(void);

#ifdef __cplusplus
}
#endif

#endif
