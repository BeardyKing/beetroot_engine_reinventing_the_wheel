#ifndef BEETROOT_GFX_TEXTURE_H
#define BEETROOT_GFX_TEXTURE_H

#include <gfx/gfx_types.h>

void gfx_create_texture_immediate(const char* path, GfxTexture& outTexture);
void gfx_cleanup_texture(GfxTexture& gfxTexture);

#endif //BEETROOT_GFX_TEXTURE_H
