#ifndef BEETROOT_GFX_FONT_H
#define BEETROOT_GFX_FONT_H

#include <gfx/gfx_types.h>

void gfx_font_record_render_pass(VkCommandBuffer& cmdBuffer);
void gfx_font_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture& albedoTexture);

void gfx_create_font_pipeline_layout();

void gfx_create_font_pipeline();
void gfx_create_font_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat);
void gfx_create_font_framebuffer();

void gfx_create_font_descriptors();
void gfx_cleanup_font_descriptors();

void gfx_destroy_font();

#endif //BEETROOT_GFX_FONT_H
