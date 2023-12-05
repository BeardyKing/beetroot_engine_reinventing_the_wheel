#version 450

//===STAGE IN===//
layout (location = 0) in StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
} stageLayout;

layout (push_constant) uniform PushConstants {
    mat4 mvp;
    vec2 uvOffset;
    vec2 uvScale;
} constants;

//===LOCAL===//
layout (set = 0, binding = 1) uniform sampler2D u_albedo;

//===OUT===//
layout (location = 0) out vec4 outFragColor;

void main(){
    vec2 uv = (stageLayout.uv + constants.uvOffset) * constants.uvScale;
    vec4 outCol = vec4(texture(u_albedo, uv).xyz, 1.0f);
    outCol * vec4(1.0f, 0.0f, 0.0f, 1.0f);

    outFragColor = outCol;
}
