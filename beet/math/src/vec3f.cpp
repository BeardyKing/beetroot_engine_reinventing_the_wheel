#include <mat4.h>
#include <cmath>

vec3f vec3f_sub(const vec3f &lhs, const vec3f &rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

vec3f vec3f_add(const vec3f &lhs, const vec3f &rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

vec3f vec3f_mul(const vec3f &lhs, float scalar) {
    return {lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

vec3f vec3f_normalized(const vec3f &rhs) {
    return (rhs) * (1.f / sqrt(rhs.x * rhs.x + rhs.y * rhs.y + rhs.z * rhs.z));
}

float vec3f_dot(const vec3f &lhs, const vec3f &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

vec3f vec3f_cross(const vec3f &lhs, const vec3f &rhs) {
    return {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
    };
}