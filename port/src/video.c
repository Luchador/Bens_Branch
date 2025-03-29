#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <PR/gbi.h>
#include "platform.h"
#include "config.h"
#include "system.h"
#include "video.h"

#include "../fast3d/gfx_api.h"
#include "../fast3d/gfx_sdl.h"
#include "../fast3d/gfx_opengl.h"

#ifdef PLATFORM_NSWITCH
#define DEFAULT_VID_WIDTH 1280
#define DEFAULT_VID_HEIGHT 720
#define DEFAULT_VID_FULLSCREEN true
#define DEFAULT_VID_FULLSCREEN_EXCLUSIVE true
#else
#define DEFAULT_VID_WIDTH 640
#define DEFAULT_VID_HEIGHT 480
#define DEFAULT_VID_FULLSCREEN false
#define DEFAULT_VID_FULLSCREEN_EXCLUSIVE false
#endif

static struct GfxWindowManagerAPI *wmAPI;
static struct GfxRenderingAPI *renderingAPI;

static bool initDone = false;

static int vidWidth = DEFAULT_VID_WIDTH;
static int vidHeight = DEFAULT_VID_HEIGHT;
static int vidFramebuffers = true;
static int vidFullscreen = DEFAULT_VID_FULLSCREEN;
static int vidFullscreenExclusive = DEFAULT_VID_FULLSCREEN_EXCLUSIVE;
static int vidMaximize = false;
static int vidCenter = false;
static int vidAllowHiDpi = false;
static int vidVsync = 1;
static int vidMSAA = 1;
static int vidFramerateLimit = 0;

static int vidDisplayFPS = 0;
static float vidDisplayFPSInterval = 1.f;
static float vidAvgFPS = 0;
static double vidLastRenderTime;

static int vidNumModes = 1;
static displaymode vidModeDefault;
static displaymode *vidModes = &vidModeDefault;

static int texFilter = FILTER_LINEAR;
static int texFilter2D = true;
static int texDetail = false;

static uint32_t dlcount = 0;
static uint32_t frames = 0;
static double startTime, endTime;
static double accumDelta = 0.0;
static double fpsTime = 0.0;
static int fpsNumFrames = 0;

static bool framebuffers_enabled;
static bool detail_textures;

static int videoInitDisplayModes(void);

int videoInit(void)
{
	wmAPI = &gfx_sdl;
	renderingAPI = &gfx_opengl_api;

	gfx_current_native_viewport.width = 320;
	gfx_current_native_viewport.height = 220;
	gfx_current_native_aspect = 320.f / 220.f;
	framebuffers_enabled = (bool)vidFramebuffers;
	detail_textures = (bool)texDetail;
	gfx_msaa_level = vidMSAA;

	struct GfxInitSettings set = {
		.wapi = wmAPI,
		.rapi = renderingAPI,
		.window_settings = {
			.title = "Perfect Dark",
			.width = vidWidth,
			.height = vidHeight,
			.x = 100,
			.y = 100,
			.fullscreen = vidFullscreen,
			.fullscreen_is_exclusive = vidFullscreenExclusive,
			.maximized = vidMaximize,
			.centered = vidCenter,
			.allow_hidpi = vidAllowHiDpi
		}
	};

	gfx_init(&set);

	videoInitDisplayModes();
	videoSetVsync(vidVsync);
	videoSetFramerateLimit(vidFramerateLimit);

	gfx_set_texture_filter((enum FilteringMode)texFilter);

	initDone = true;
	return 0;
}

void videoStartFrame(void)
{
	if (initDone) {
		startTime = wmAPI->get_time();
		gfx_start_frame();
	}

	// Synchronize with their backend counterparts.
	vidFullscreen = videoGetFullscreen();
	vidMaximize = videoGetMaximizeWindow();
}

void videoSubmitCommands(Gfx *cmds)
{
	if (initDone) {
		gfx_run(cmds);
		++dlcount;
	}
}

void videoEndFrame(void)
{
	if (!initDone) {
		return;
	}

	gfx_end_frame();

	++frames;
	++fpsNumFrames;

	const double flipTime = wmAPI->get_time();
	accumDelta += flipTime - endTime;
	endTime = flipTime;
	vidLastRenderTime = endTime - startTime;

	if (endTime >= fpsTime) {
		char tmp[128];
		vidAvgFPS = fpsNumFrames ? ((double)fpsNumFrames / accumDelta) : 0.f;
		fpsNumFrames = 0;
		accumDelta = 0.0;
		snprintf(tmp, sizeof(tmp), "fps %4.1f frt %lf frm %u", vidAvgFPS, vidLastRenderTime, frames);
		wmAPI->set_window_title(tmp);
		fpsTime = endTime + vidDisplayFPSInterval;
	}
}

double videoGetLastRenderTime(void)
{
	return vidLastRenderTime;
}

float videoGetAverageFPS(void)
{
	return vidAvgFPS;
}

void videoClearScreen(void)
{
	videoStartFrame();
	// TODO: clear
	videoEndFrame();
}

void *videoGetWindowHandle(void)
{
	if (initDone) {
		return wmAPI->get_window_handle();
	}
	return NULL;
}

void videoUpdateNativeResolution(int w, int h)
{
	gfx_current_native_viewport.width = w;
	gfx_current_native_viewport.height = h;
	gfx_current_native_aspect = (float)w / (float)h;
}

int videoGetNativeWidth(void)
{
	return gfx_current_native_viewport.width;
}

int videoGetNativeHeight(void)
{
	return gfx_current_native_viewport.height;
}

int videoGetWidth(void)
{
	return gfx_current_dimensions.width;
}

int videoGetHeight(void)
{
	return gfx_current_dimensions.height;
}

int videoGetFullscreen(void)
{
	vidFullscreen = wmAPI->get_fullscreen_state();
	return vidFullscreen;
}

int videoGetFullscreenMode(void)
{
	vidFullscreenExclusive = wmAPI->get_fullscreen_flag_mode();
	return vidFullscreenExclusive;
}

int videoGetMaximizeWindow(void)
{
	vidMaximize = wmAPI->get_maximized_state();
	return vidMaximize;
}

int videoGetCenterWindow(void)
{
	return vidCenter;
}

float videoGetAspect(void)
{
	return gfx_current_dimensions.aspect_ratio;
}

int videoGetDisplayModeIndex(void)
{
	for (int i = 1; i < vidNumModes; ++i) {
		if (vidModes[i].width == gfx_current_dimensions.width &&
		    vidModes[i].height == gfx_current_dimensions.height) {
			return i;
		}
	}
	// Current dimensions don't match any known mode, so return index 0, "Custom".
	return 0;
}

int videoGetMSAA(void)
{
	vidMSAA = (int)gfx_msaa_level;
	return vidMSAA;
}

int videoGetVsync(void)
{
	vidVsync = wmAPI->get_swap_interval();
	return vidVsync;
}

int videoGetFramerateLimit(void)
{
	vidFramerateLimit = wmAPI->get_target_fps();
	return vidFramerateLimit;
}

int videoGetDisplayFPS(void)
{
	return vidDisplayFPS;
}

static int videoInitDisplayModes(void)
{
	if (!wmAPI->get_current_display_mode(&vidModeDefault.width, &vidModeDefault.height)) {
		vidModeDefault.width = 640;
		vidModeDefault.height = 480;
		return false;
	}

	const int numBaseModes = wmAPI->get_num_display_modes();
	if (!numBaseModes) {
		return false;
	}

	const int numCustomModes = 1;
	displaymode *modeList = sysMemZeroAlloc((numBaseModes + numCustomModes) * sizeof(displaymode));
	if (!modeList) {
		return false;
	}

	modeList[0].width = 0;
	modeList[0].height = 0;

	int numModes = 1;
	int w = -1, h = w, neww = w, newh = w;

	// SDL modes are guaranteed to be sorted high to low
	for (int i = 0; i < numBaseModes; ++i) {
		wmAPI->get_display_mode(i, &neww, &newh);

		if (neww != w || newh != h) {
			w = neww;
			h = newh;
			modeList[numModes].width = w;
			modeList[numModes].height = h;
			++numModes;
		}
	}

	modeList = sysMemRealloc(modeList, numModes * sizeof(displaymode));
	if (!modeList) {
		return false;
	}

	vidModes = modeList;
	vidNumModes = numModes;

	return true;
}

int videoGetDisplayMode(displaymode *out, const int index)
{
	if (index >= 0 && index < vidNumModes) {
		*out = vidModes[index];
		return true;
	}
	return false;
}

int videoGetNumDisplayModes(void)
{
	return vidNumModes;
}

void videoSetDisplayMode(const int index)
{
	const displaymode dm = vidModes[index];

	if (index == 0) {
		// "Custom" video mode.
		return;
	}

	vidWidth = dm.width;
	vidHeight = dm.height;

	int posX = 100;
	int posY = 100;
	if (vidCenter) {
		wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
	}

	if (vidFullscreen) {
		wmAPI->set_closest_resolution(vidWidth, vidHeight, vidCenter);
	} else {
		if (vidMaximize) {
			videoSetMaximizeWindow(false);
		} else {
			wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
		}
	}
}

int videoGetTextureFilter2D(void)
{
	return texFilter2D;
}

uint32_t videoGetTextureFilter(void)
{
	return texFilter;
}

int videoGetDetailTextures(void)
{
	return texDetail;
}

void videoSetWindowOffset(int x, int y)
{
	gfx_current_game_window_viewport.x = x;
	gfx_current_game_window_viewport.y = y;
}

void videoSetFullscreen(int fs)
{
	if (fs != vidFullscreen) {
		vidFullscreen = !!fs;
		wmAPI->set_closest_resolution(vidWidth, vidHeight, vidCenter);
		wmAPI->set_fullscreen(vidFullscreen);
		if (!vidFullscreen && vidMaximize) {
			wmAPI->set_maximize(false);
			wmAPI->set_maximize(true);
		}
	}
}

void videoSetFullscreenMode(int mode)
{
	vidFullscreenExclusive = mode;
	wmAPI->set_fullscreen_flag(mode);
	if (vidFullscreen) {
		wmAPI->set_fullscreen(false);
		wmAPI->set_fullscreen(true);
	}
}

void videoSetMaximizeWindow(int fs)
{
	if (fs != vidMaximize) {
		vidMaximize = !!fs;
		wmAPI->set_maximize(vidMaximize);
		if (vidCenter && !vidMaximize) {
			int posX = 0;
			int posY = 0;
			wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
			wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
		}
	}
}

void videoSetCenterWindow(int center)
{
	vidCenter = center;
	if (vidCenter && !vidMaximize) {
		int posX = 0;
		int posY = 0;
		wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
		wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
	}
}

void videoSetTextureFilter(uint32_t filter)
{
	if (filter > FILTER_THREE_POINT) filter = FILTER_THREE_POINT;
	if (texFilter == filter) return;
	texFilter = filter;
	gfx_set_texture_filter((enum FilteringMode)filter);
}

void videoSetTextureFilter2D(int filter)
{
	texFilter2D = !!filter;
}

void videoSetDetailTextures(int detail)
{
	texDetail = !!detail;
	detail_textures = (bool)texDetail;
}

int videoCreateFramebuffer(uint32_t w, uint32_t h, int upscale, int autoresize)
{
	return gfx_create_framebuffer(w, h, upscale, autoresize);
}

void videoSetMSAA(const int msaa)
{
	vidMSAA = msaa;
	gfx_msaa_level = (uint32_t)vidMSAA;
}

void videoSetVsync(const int vsync)
{
	vidVsync = wmAPI->set_swap_interval(vsync) ? vsync : 0;

	if (vidVsync == 0 && vidFramerateLimit == 0) {
		// cap FPS if there's no vsync to prevent the game from exploding
		videoSetFramerateLimit(VIDEO_MAX_FPS);
	}
}

void videoSetFramerateLimit(const int limit)
{
	vidFramerateLimit = (vidVsync == 0 && limit == 0) ? VIDEO_MAX_FPS : limit;
	wmAPI->set_target_fps(vidFramerateLimit);
}

void videoSetDisplayFPS(const int displayfps)
{
	vidDisplayFPS = displayfps;
}

void videoSetFramebuffer(int target)
{
	return gfx_set_framebuffer(target, 1.f);
}

void videoResetFramebuffer(void)
{
	return gfx_reset_framebuffer();
}

int videoFramebuffersSupported(void)
{
	return framebuffers_enabled;
}

void videoResizeFramebuffer(int target, uint32_t w, uint32_t h, int upscale, int autoresize)
{
	gfx_resize_framebuffer(target, w, h, upscale, autoresize);
}

void videoCopyFramebuffer(int dst, int src, int left, int top)
{
	// assume immediate copies always read the front buffer
	gfx_copy_framebuffer(dst, src, left, top, false);
}

void videoResetTextureCache(void)
{
	gfx_texture_cache_clear();
}

void videoFreeCachedTexture(const void *texptr)
{
	gfx_texture_cache_delete(texptr);
}

void videoShutdown(void)
{
	free(vidModes);
}

PD_CONSTRUCTOR static void videoConfigInit(void)
{
	configRegisterInt("Video.DefaultFullscreen", &vidFullscreen, 0, 1);
	configRegisterInt("Video.DefaultMaximize", &vidMaximize, 0, 1);
	configRegisterInt("Video.DefaultWidth", &vidWidth, 0, 32767);
	configRegisterInt("Video.DefaultHeight", &vidHeight, 0, 32767);
	configRegisterInt("Video.ExclusiveFullscreen", &vidFullscreenExclusive, 0, 1);
	configRegisterInt("Video.CenterWindow", &vidCenter, 0, 1);
	configRegisterInt("Video.AllowHiDpi", &vidAllowHiDpi, 0, 1);
	configRegisterInt("Video.VSync", &vidVsync, -1, 10);
	configRegisterInt("Video.FramebufferEffects", &vidFramebuffers, 0, 1);
	configRegisterInt("Video.FramerateLimit", &vidFramerateLimit, 0, VIDEO_MAX_FPS);
	configRegisterInt("Video.DisplayFPS", &vidDisplayFPS, 0, 1);
	configRegisterFloat("Video.DisplayFPSInterval", &vidDisplayFPSInterval, 0.01f, 32.f);
	configRegisterInt("Video.MSAA", &vidMSAA, 1, 16);
	configRegisterInt("Video.TextureFilter", &texFilter, 0, 2);
	configRegisterInt("Video.TextureFilter2D", &texFilter2D, 0, 1);
	configRegisterInt("Video.DetailTextures", &texDetail, 0, 1);
}
