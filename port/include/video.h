#ifndef _IN_VIDEO_H
#define _IN_VIDEO_H

#include <stdint.h>
#include <PR/gbi.h>

// maximum framerate; if the game runs faster than this, things will break
#define VIDEO_MAX_FPS 240

typedef struct {
	int width;
	int height;
} displaymode;

int videoInit(void);
void videoStartFrame(void);
void videoSubmitCommands(Gfx *cmds);
void videoClearScreen(void);
void videoEndFrame(void);

void *videoGetWindowHandle(void);

void videoUpdateNativeResolution(int w, int h);
int videoGetNativeWidth(void);
int videoGetNativeHeight(void);

int videoGetWidth(void);
int videoGetHeight(void);
float videoGetAspect(void);
int videoGetFullscreen(void);
int videoGetFullscreenMode(void);
int videoGetMaximizeWindow(void);
void videoSetMaximizeWindow(int fs);
int videoGetCenterWindow(void);
void videoSetCenterWindow(int center);
uint32_t videoGetTextureFilter(void);
int videoGetTextureFilter2D(void);
int videoGetDisplayModeIndex(void);
int videoGetDisplayMode(displaymode *out, const int index);
int videoGetNumDisplayModes(void);
int videoGetVsync(void);
int videoGetFramerateLimit(void);
int videoGetDisplayFPS(void);
int videoGetMSAA(void);

float videoGetAverageFPS(void);
double videoGetLastRenderTime(void);

void videoSetWindowOffset(int x, int y);
void videoSetFullscreen(int fs);
void videoSetFullscreenMode(int mode);
void videoSetTextureFilter(uint32_t filter);
void videoSetTextureFilter2D(int filter);
void videoSetDisplayMode(const int index);
void videoSetVsync(const int vsync);
void videoSetFramerateLimit(const int limit);
void videoSetDisplayFPS(const int displayfps);
void videoSetMSAA(const int msaa);

int videoCreateFramebuffer(uint32_t w, uint32_t h, int upscale, int autoresize);
void videoSetFramebuffer(int target);
void videoResetFramebuffer(void);
void videoCopyFramebuffer(int dst, int src, int left, int top);
void videoResizeFramebuffer(int target, uint32_t w, uint32_t h, int upscale, int autoresize);
int videoFramebuffersSupported(void);

void videoResetTextureCache(void);
void videoFreeCachedTexture(const void *texptr);

void videoShutdown(void);

#endif
