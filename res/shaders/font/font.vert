#version 450

//===LOCAL===//
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_UV;
//layout (location = 3) in vec3 v_normal;

layout (push_constant) uniform PushConstants {
    mat4 mvp;
    vec2 uv;
} constants;

//===STAGE OUT===//
layout (location = 0) out StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
} stageLayout;

void main() {
    gl_Position = constants.mvp * vec4(v_position.xy, 0.0, 1.0);

    stageLayout.color = v_color;
    stageLayout.uv = v_UV;
}
