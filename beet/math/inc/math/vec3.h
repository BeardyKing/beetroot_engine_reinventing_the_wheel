#ifndef BEETROOT_VEC3_H
#define BEETROOT_VEC3_H

struct vec3f;

vec3f vec3f_mul(const vec3f &lhs, float scalar);
vec3f vec3f_sub(const vec3f &lhs, const vec3f &rhs);
vec3f vec3f_add(const vec3f &lhs, const vec3f &rhs);

vec3f vec3f_normalized(const vec3f &rhs);
float vec3f_dot(const vec3f &lhs, const vec3f &rhs);
vec3f vec3f_cross(const vec3f &lhs, const vec3f &rhs);

struct vec3f {
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

    vec3f operator+(const vec3f &rhs) const { return vec3f_add(*this, rhs); }
    vec3f operator-(const vec3f &rhs) const { return vec3f_sub(*this, rhs); }
    vec3f operator*(float s) const { return vec3f_mul(*this, s); }
};


#endif //BEETROOT_VEC3_H
