#ifndef BEETROOT_DB_TYPES_H
#define BEETROOT_DB_TYPES_H

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/vec2.h>
#include <cstdint>

struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f};
};

struct Camera {
    vec3f lookTarget{0.0f, 0.0f, -1.0f};
    float fov{60.0f};
    float zNear{0.1f};
    float zFar{800.0f};
};

struct LitMaterial {
    uint32_t descriptorSetIndex{0};
    uint32_t albedoIndex{0};
    //TODO:GFX
//    uint32_t normalIndex{0};
//    uint32_t metallicIndex{0};
//    uint32_t roughnessIndex{0};
//    uint32_t occlusionIndex{0};
//
//    vec4f albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
//    vec2f textureTiling{1.0f, 1.0f};
//
//    float albedoScalar{1.0f};
//    float normalScalar{1.0f};
//    float metallicScalar{1.0f};
//    float roughnessScalar{1.0f};
//    float occlusionScalar{1.0f};
};

struct LitEntity {
    uint32_t transformIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
};

#endif //BEETROOT_DB_TYPES_H
