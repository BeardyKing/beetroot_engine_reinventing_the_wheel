#ifndef BEETROOT_GFX_SAMPLERS_H
#define BEETROOT_GFX_SAMPLERS_H

#include <vulkan/vulkan_core.h>

struct TextureSamplers {
    VkSampler linearSampler{};
    VkSampler pointSampler{};
    VkSampler linearMipSampler{};
};

TextureSamplers* gfx_samplers();

void gfx_create_samplers();
void gfx_cleanup_samplers();

#endif //BEETROOT_GFX_SAMPLERS_H
