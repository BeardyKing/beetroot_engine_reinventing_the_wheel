#ifndef BEETROOT_DB_TYPES_H
#define BEETROOT_DB_TYPES_H

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/vec2.h>
#include <cstdint>

#define MAX_DB_CAMERA_ENTITIES 1
#define MAX_DB_LIT_ENTITIES 64
#define MAX_DB_FONT_ENTITIES 64

#define MAX_DB_CAMERAS 1
#define MAX_DB_TRANSFORMS 64
#define MAX_DB_UI_TRANSFORMS 64
#define MAX_DB_GFX_TEXTURES 64
#define MAX_DB_GFX_MESHES 64

#define MAX_DB_VK_DESCRIPTOR_SETS 64

#define MAX_DB_LIT_MATERIALS 64
#define MAX_DB_FONT_MATERIALS 16

struct LitEntity {
    uint32_t transformIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
};

struct FontEntity {
    uint32_t uiTransformIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
};

struct CameraEntity {
    uint32_t transformIndex;
    uint32_t cameraIndex;
};

struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f, 1.0f};
};

struct UiTransform {
    vec2f position{0.0f, 0.0f};
    vec2f rotation{0.0f, 0.0f};
    vec2f scale{1.0f, 1.0f};
    vec2f size{1.0f, 1.0f};
    vec2f pivot{0.5f, 0.5f};
    vec4f anchor{0.5f, 0.5f, 0.5f, 0.5f};
};

struct Camera {
    float fov{60.0f};
    float zNear{0.1f};
    float zFar{800.0f};
};

struct FontMaterial {
    uint32_t descriptorSetIndex{0};
    uint32_t atlasIndex{0};
};

struct LitMaterial {
    uint32_t descriptorSetIndex{0};
    uint32_t albedoIndex{0};
    //TODO:GFX
    //uint32_t normalIndex{0};
    //uint32_t metallicIndex{0};
    //uint32_t roughnessIndex{0};
    //uint32_t occlusionIndex{0};
    //
    //vec4f albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    //vec2f textureTiling{1.0f, 1.0f};
    //
    //float albedoScalar{1.0f};
    //float normalScalar{1.0f};
    //float metallicScalar{1.0f};
    //float roughnessScalar{1.0f};
    //float occlusionScalar{1.0f};
};

#endif //BEETROOT_DB_TYPES_H
