#ifndef BEETROOT_GFX_RESOURCE_DB_H
#define BEETROOT_GFX_RESOURCE_DB_H

#include <shared/db_types.h>
#include <gfx/gfx_types.h>

void gfx_db_create();
void gfx_db_cleanup();

uint32_t gfx_db_add_camera(const Camera &camera);
Camera *gfx_db_get_camera(uint32_t index);

uint32_t gfx_db_add_camera_entity(const CameraEntity &camera);
CameraEntity *gfx_db_get_camera_entity(uint32_t index);

uint32_t gfx_db_add_lit_entity(const LitEntity &litEntity);
LitEntity *gfx_db_get_lit_entity(uint32_t index);
uint32_t gfx_db_get_lit_entity_count();

uint32_t gfx_db_add_lit_material(const LitMaterial &litMaterial);
LitMaterial *gfx_db_get_lit_material(uint32_t index);

uint32_t gfx_db_add_texture(const GfxTexture &gfxTexture);
GfxTexture *gfx_db_get_texture(uint32_t index);
uint32_t gfx_db_get_texture_count();

uint32_t gfx_db_add_mesh(const GfxMesh &gfxMesh);
GfxMesh *gfx_db_get_mesh(uint32_t index);
uint32_t gfx_db_get_mesh_count();

uint32_t gfx_db_add_descriptor_set(const VkDescriptorSet &descriptorSet);
VkDescriptorSet *gfx_db_get_descriptor_set(uint32_t index);

uint32_t gfx_db_add_transform(const Transform &litMaterial);
Transform *gfx_db_get_transform(uint32_t index);

#endif //BEETROOT_GFX_RESOURCE_DB_H
