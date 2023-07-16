#include <gfx/gfx_resource_db.h>
#include <gfx/gfx_types.h>

#include <shared/db_types.h>
#include <shared/assert.h>

#include <cstring>

#define MAX_CAMERA_ENTITIES 1
static CameraEntity s_dbCameraEntities[MAX_CAMERA_ENTITIES];
static uint32_t s_dbCameraEntitiesCount{0};

#define MAX_LIT_ENTITIES 64
static LitEntity s_dbLitEntities[MAX_LIT_ENTITIES];
static uint32_t s_dbLitEntitiesCount{0};

#define MAX_CAMERAS 1
static Camera s_dbCameras[MAX_CAMERAS];
static uint32_t s_dbCameraCount{0};

#define MAX_TRANSFORMS 64
static Transform s_dbTransforms[MAX_TRANSFORMS];
static uint32_t s_dbTransformsCount{0};

#define MAX_LIT_MATERIALS 64
static LitMaterial s_dbLitMaterials[MAX_LIT_MATERIALS];
static uint32_t s_dbLitMaterialsCount{0};

#define MAX_GFX_TEXTURES 64
static GfxTexture s_dbTextures[MAX_GFX_TEXTURES];
static uint32_t s_dbTexturesCount{0};

#define MAX_VK_DESCRIPTOR_SETS 64
static VkDescriptorSet s_dbDescriptorSet[MAX_VK_DESCRIPTOR_SETS];
static uint32_t s_dbDescriptorSetCount{0};

void gfx_db_create() {
    memset(s_dbCameraEntities, 0, sizeof(CameraEntity) * MAX_CAMERA_ENTITIES);
    memset(s_dbLitEntities, 0, sizeof(LitEntity) * MAX_LIT_ENTITIES);

    memset(s_dbCameras, 0, sizeof(Camera) * MAX_CAMERAS);
    memset(s_dbTransforms, 0, sizeof(Transform) * MAX_TRANSFORMS);
    memset(s_dbLitMaterials, 0, sizeof(LitMaterial) * MAX_LIT_MATERIALS);
    memset(s_dbTextures, 0, sizeof(GfxTexture) * MAX_GFX_TEXTURES);
    memset(s_dbDescriptorSet, 0, sizeof(VkDescriptorSet) * MAX_VK_DESCRIPTOR_SETS);
}

void gfx_db_cleanup() {}

uint32_t gfx_db_add_camera(const Camera &camera) {
    ASSERT_MSG(s_dbCameraCount < MAX_CAMERAS, "Err: exceeded pre-allocated amount of cameras %u, max amount [%u]", s_dbCameraCount, MAX_CAMERAS);
    uint32_t currentCameraIndex = s_dbCameraCount;
    s_dbCameras[currentCameraIndex] = camera;
    s_dbCameraCount++;
    return currentCameraIndex;
}

Camera *gfx_db_get_camera(uint32_t index) {
    ASSERT_MSG(index < MAX_CAMERAS, "Err: invalid db index %u, max index [%u]", index, MAX_CAMERAS);
    return &s_dbCameras[index];
}

uint32_t gfx_db_add_camera_entity(const CameraEntity &camera) {
    ASSERT_MSG(s_dbCameraEntitiesCount < MAX_CAMERA_ENTITIES, "Err: exceeded pre-allocated amount of cameras %u, max amount [%u]",
               s_dbCameraEntitiesCount, MAX_CAMERA_ENTITIES);
    uint32_t currentCameraEntityIndex = s_dbCameraEntitiesCount;
    s_dbCameraEntities[currentCameraEntityIndex] = camera;
    s_dbCameraEntitiesCount++;
    return currentCameraEntityIndex;
}

CameraEntity *gfx_db_get_camera_entity(uint32_t index) {
    ASSERT_MSG(index < MAX_CAMERA_ENTITIES, "Err: invalid db index %u, max index [%u]", index, MAX_CAMERA_ENTITIES);
    return &s_dbCameraEntities[index];
}

uint32_t gfx_db_add_lit_entity(const LitEntity &litEntity) {
    ASSERT_MSG(s_dbLitEntitiesCount < MAX_LIT_ENTITIES, "Err: exceeded pre-allocated amount of lit entities %u, max amount [%u]",
               s_dbLitEntitiesCount, MAX_LIT_ENTITIES);
    uint32_t currentLitEntityIndex = s_dbLitEntitiesCount;
    s_dbLitEntities[currentLitEntityIndex] = litEntity;
    s_dbLitEntitiesCount++;
    return currentLitEntityIndex;
}

LitEntity *gfx_db_get_lit_entity(uint32_t index) {
    ASSERT_MSG(index < MAX_LIT_ENTITIES, "Err: invalid db index %u, max index [%u]", index, MAX_LIT_ENTITIES);
    return &s_dbLitEntities[index];
}

uint32_t gfx_db_add_texture(const GfxTexture &gfxTexture) {
    ASSERT_MSG(s_dbTexturesCount < MAX_GFX_TEXTURES, "Err: exceeded pre-allocated amount of gfx textures %u, max amount [%u]",
               s_dbTexturesCount, MAX_GFX_TEXTURES);
    uint32_t currentGfxTextureIndex = s_dbTexturesCount;
    s_dbTextures[currentGfxTextureIndex] = gfxTexture;
    s_dbTexturesCount++;
    return currentGfxTextureIndex;
}

GfxTexture *gfx_db_get_texture(uint32_t index) {
    ASSERT_MSG(index < MAX_GFX_TEXTURES, "Err: invalid db index %u, max index [%u]", index, MAX_GFX_TEXTURES);
    return &s_dbTextures[index];
}

uint32_t gfx_db_add_descriptor_set(const VkDescriptorSet &descriptorSet) {
    ASSERT_MSG(s_dbDescriptorSetCount < MAX_VK_DESCRIPTOR_SETS, "Err: exceeded pre-allocated amount of vk descriptor sets %u, max amount [%u]",
               s_dbDescriptorSetCount, MAX_VK_DESCRIPTOR_SETS);
    uint32_t currentDescriptorSetIndex = s_dbDescriptorSetCount;
    s_dbDescriptorSet[currentDescriptorSetIndex] = descriptorSet;
    s_dbDescriptorSetCount++;
    return currentDescriptorSetIndex;
}

VkDescriptorSet *gfx_db_get_descriptor_set(uint32_t index) {
    ASSERT_MSG(index < MAX_VK_DESCRIPTOR_SETS, "Err: invalid db index %u, max index [%u]", index, MAX_VK_DESCRIPTOR_SETS);
    return &s_dbDescriptorSet[index];
}

uint32_t gfx_db_add_lit_material(const LitMaterial &litMaterial) {
    ASSERT_MSG(s_dbLitMaterialsCount < MAX_LIT_MATERIALS, "Err: exceeded pre-allocated amount of lit materials sets %u, max amount [%u]",
               s_dbLitMaterialsCount, MAX_LIT_MATERIALS);
    uint32_t currentLitMaterialsIndex = s_dbLitMaterialsCount;
    s_dbLitMaterials[currentLitMaterialsIndex] = litMaterial;
    s_dbLitMaterialsCount++;
    return currentLitMaterialsIndex;
}

LitMaterial *gfx_db_get_lit_material(uint32_t index) {
    ASSERT_MSG(index < MAX_LIT_MATERIALS, "Err: invalid db index %u, max index [%u]", index, MAX_LIT_MATERIALS);
    return &s_dbLitMaterials[index];
}

uint32_t gfx_db_add_transform(const Transform &litMaterial) {
    ASSERT_MSG(s_dbTransformsCount < MAX_TRANSFORMS, "Err: exceeded pre-allocated amount of lit materials sets %u, max amount [%u]",
               s_dbTransformsCount, MAX_TRANSFORMS);
    uint32_t currentLitMaterialsIndex = s_dbTransformsCount;
    s_dbTransforms[currentLitMaterialsIndex] = litMaterial;
    s_dbTransformsCount++;
    return currentLitMaterialsIndex;
}

Transform *gfx_db_get_transform(uint32_t index) {
    ASSERT_MSG(index < MAX_TRANSFORMS, "Err: invalid db index %u, max index [%u]", index, MAX_TRANSFORMS);
    return &s_dbTransforms[index];
}

