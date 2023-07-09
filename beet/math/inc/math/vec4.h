#ifndef BEETROOT_VEC4_H
#define BEETROOT_VEC4_H

#include <cstdint>

struct vec4f {
    union {
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };
};

struct vec4d {
    union {
        struct {
            double x, y, z, w;
        };
        struct {
            double r, g, b, a;
        };
    };
};

struct vec4i {
    union {
        struct {
            int32_t x, y, z, w;
        };
        struct {
            int32_t r, g, b, a;
        };
    };
};

struct vec4u {
    union {
        struct {
            uint32_t x, y, z, w;
        };
        struct {
            uint32_t r, g, b, a;
        };
    };
};

#endif //BEETROOT_VEC4_H
