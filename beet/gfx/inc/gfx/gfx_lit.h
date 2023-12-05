#ifndef BEETROOT_GFX_LIT_H
#define BEETROOT_GFX_LIT_H

#include <gfx/gfx_types.h>

void gfx_lit_record_render_pass(VkCommandBuffer& cmdBuffer);
void gfx_lit_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture& albedoTexture);

void gfx_create_lit_pipeline_layout();
void gfx_create_lit_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat);
void gfx_create_lit_framebuffer();
void gfx_create_lit_pipeline();

void gfx_create_lit_descriptors();
void gfx_cleanup_lit_descriptors();

void gfx_destroy_lit();

#endif //BEETROOT_GFX_LIT_H
