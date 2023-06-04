#include <mat4.h>
#include <cmath>

vec3 vec3f_sub(const vec3 &lhs, const vec3 &rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

vec3 vec3f_add(const vec3 &lhs, const vec3 &rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

vec3 vec3f_mul(const vec3 &lhs, float scalar) {
    return {lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

vec3 vec3f_normalized(const vec3 &rhs) {
    return (rhs) * (1.f / sqrt(rhs.x * rhs.x + rhs.y * rhs.y + rhs.z * rhs.z));
}

float vec3f_dot(const vec3 &lhs, const vec3 &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

vec3 vec3f_cross(const vec3 &lhs, const vec3 &rhs) {
    return {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
    };
}