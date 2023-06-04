#ifndef BEETROOT_MAT4_H
#define BEETROOT_MAT4_H

#include <math/vec3.h>

struct mat4;

mat4 mat4f_mul(const mat4 &lhs, const mat4 &rhs);

mat4 mat4f_rotation_y(float angle);
mat4 mat4f_perspective(float fovY, float aspectRatio, float zNear, float zFar);
mat4 mat4f_look_at(vec3 at, vec3 eye, vec3 up);

struct mat4 {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float data[4][4]; // [row][column]
    };


    mat4 operator*(const mat4 &rhs) const {
        return mat4f_mul(*this, rhs);
    };
};


#endif //BEETROOT_MAT4_H
