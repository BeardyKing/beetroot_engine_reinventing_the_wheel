#include <math/mat4.h>
#include <math/vec3.h>

#include <cmath>

mat4 mat4f_mul(const mat4 &lhs, const mat4 &rhs) {
    return {
            lhs.data[0][0] * rhs.data[0][0] + lhs.data[0][1] * rhs.data[1][0] + lhs.data[0][2] * rhs.data[2][0] + lhs.data[0][3] * rhs.data[3][0],
            lhs.data[0][0] * rhs.data[0][1] + lhs.data[0][1] * rhs.data[1][1] + lhs.data[0][2] * rhs.data[2][1] + lhs.data[0][3] * rhs.data[3][1],
            lhs.data[0][0] * rhs.data[0][2] + lhs.data[0][1] * rhs.data[1][2] + lhs.data[0][2] * rhs.data[2][2] + lhs.data[0][3] * rhs.data[3][2],
            lhs.data[0][0] * rhs.data[0][3] + lhs.data[0][1] * rhs.data[1][3] + lhs.data[0][2] * rhs.data[2][3] + lhs.data[0][3] * rhs.data[3][3],

            lhs.data[1][0] * rhs.data[0][0] + lhs.data[1][1] * rhs.data[1][0] + lhs.data[1][2] * rhs.data[2][0] + lhs.data[1][3] * rhs.data[3][0],
            lhs.data[1][0] * rhs.data[0][1] + lhs.data[1][1] * rhs.data[1][1] + lhs.data[1][2] * rhs.data[2][1] + lhs.data[1][3] * rhs.data[3][1],
            lhs.data[1][0] * rhs.data[0][2] + lhs.data[1][1] * rhs.data[1][2] + lhs.data[1][2] * rhs.data[2][2] + lhs.data[1][3] * rhs.data[3][2],
            lhs.data[1][0] * rhs.data[0][3] + lhs.data[1][1] * rhs.data[1][3] + lhs.data[1][2] * rhs.data[2][3] + lhs.data[1][3] * rhs.data[3][3],

            lhs.data[2][0] * rhs.data[0][0] + lhs.data[2][1] * rhs.data[1][0] + lhs.data[2][2] * rhs.data[2][0] + lhs.data[2][3] * rhs.data[3][0],
            lhs.data[2][0] * rhs.data[0][1] + lhs.data[2][1] * rhs.data[1][1] + lhs.data[2][2] * rhs.data[2][1] + lhs.data[2][3] * rhs.data[3][1],
            lhs.data[2][0] * rhs.data[0][2] + lhs.data[2][1] * rhs.data[1][2] + lhs.data[2][2] * rhs.data[2][2] + lhs.data[2][3] * rhs.data[3][2],
            lhs.data[2][0] * rhs.data[0][3] + lhs.data[2][1] * rhs.data[1][3] + lhs.data[2][2] * rhs.data[2][3] + lhs.data[2][3] * rhs.data[3][3],

            lhs.data[3][0] * rhs.data[0][0] + lhs.data[3][1] * rhs.data[1][0] + lhs.data[3][2] * rhs.data[2][0] + lhs.data[3][3] * rhs.data[3][0],
            lhs.data[3][0] * rhs.data[0][1] + lhs.data[3][1] * rhs.data[1][1] + lhs.data[3][2] * rhs.data[2][1] + lhs.data[3][3] * rhs.data[3][1],
            lhs.data[3][0] * rhs.data[0][2] + lhs.data[3][1] * rhs.data[1][2] + lhs.data[3][2] * rhs.data[2][2] + lhs.data[3][3] * rhs.data[3][2],
            lhs.data[3][0] * rhs.data[0][3] + lhs.data[3][1] * rhs.data[1][3] + lhs.data[3][2] * rhs.data[2][3] + lhs.data[3][3] * rhs.data[3][3]
    };
}

mat4 mat4f_rotation_y(float angle) {
    const float s = sin(angle), c = cos(angle);
    return {
            c, 0.f, -s, 0.f,
            0.f, 1.f, 0.f, 0.f,
            s, 0.f, c, 0.f,
            0.f, 0.f, 0.f, 1.f};
}

mat4 mat4f_perspective(float fovY, float aspectRatio, float zNear, float zFar) {
    float yScale = 1.0f / tan(fovY * 0.5f);
    float xScale = yScale / aspectRatio;
    return {
            xScale, 0.0f, 0.0f, 0.0f,
            0.0f, yScale, 0.0f, 0.0f,
            0.0f, 0.0f, zFar / (zFar - zNear), 1.0f,
            0.0f, 0.0f, -zNear * zFar / (zFar - zNear), 0.0f};
}

mat4 mat4f_look_at(vec3 at, vec3 eye, vec3 up) {
    vec3 zAxis = vec3f_normalized(at - eye);
    vec3 xAxis = vec3f_normalized(vec3f_cross(up, zAxis));
    vec3 yAxis = vec3f_cross(zAxis, xAxis);
    return {
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            -vec3f_dot(xAxis, eye), -vec3f_dot(yAxis, eye), -vec3f_dot(zAxis, eye), 1.0f};
}