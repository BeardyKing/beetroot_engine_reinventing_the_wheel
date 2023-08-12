#ifndef BEETROOT_GFX_FALLBACK_H
#define BEETROOT_GFX_FALLBACK_H

#include <gfx/gfx_types.h>

void gfx_fallback_record_render_pass(VkCommandBuffer& cmdBuffer);
void gfx_fallback_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture& albedoTexture);

void gfx_create_fallback_pipeline_layout();

void gfx_create_fallback_pipeline();
void gfx_create_fallback_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat);
void gfx_create_fallback_framebuffer();

void gfx_create_texture_immediate(const char* path, GfxTexture& outTexture);
void gfx_cleanup_fallback_texture(GfxTexture& gfxTexture);

void gfx_create_fallback_mesh(GfxMesh& outMesh);
void gfx_cleanup_fallback_mesh(GfxMesh& gfxMesh);

void gfx_create_fallback_descriptors();
void gfx_cleanup_fallback_descriptors();

void gfx_destroy_fallback();

#endif //BEETROOT_GFX_FALLBACK_H
