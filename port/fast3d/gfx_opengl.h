#ifndef GFX_OPENGL_H
#define GFX_OPENGL_H

#include "gfx_rendering_api.h"

extern struct GfxRenderingAPI gfx_opengl_api;
void writeShadersToFile(const char *fs_buf, const char *filename);
char *loadShaderFile(const char *filepath);
int debug_log(const char *message, int num);
void debug_log_coord(const struct coord *pos);

#endif
