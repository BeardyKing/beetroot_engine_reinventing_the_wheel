#ifndef BEETROOT_VEC3_H
#define BEETROOT_VEC3_H

struct vec3;

vec3 vec3f_mul(const vec3 &lhs, float scalar);
vec3 vec3f_sub(const vec3 &lhs, const vec3 &rhs);
vec3 vec3f_add(const vec3 &lhs, const vec3 &rhs);

vec3 vec3f_normalized(const vec3 &rhs);
float vec3f_dot(const vec3 &lhs, const vec3 &rhs);
vec3 vec3f_cross(const vec3 &lhs, const vec3 &rhs);

struct vec3 {
    union {
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
        struct {
            float data[3];
        };
    };

    vec3 operator+(const vec3 &rhs) const { return vec3f_add(*this, rhs); }
    vec3 operator-(const vec3 &rhs) const { return vec3f_sub(*this, rhs); }
    vec3 operator*(float s) const { return vec3f_mul(*this, s); }
};


#endif //BEETROOT_VEC3_H
