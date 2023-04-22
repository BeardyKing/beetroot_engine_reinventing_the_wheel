#ifndef BEETROOT_VEC2_H
#define BEETROOT_VEC2_H

struct vec2f {
    union {
        struct {
            float x, y;
        };
        struct {
            float r, g;
        };
    };
};

struct vec2d {
    union {
        struct {
            double x, y;
        };
        struct {
            double r, g;
        };
    };
};

struct vec2i {
    union {
        struct {
            int32_t x, y;
        };
        struct {
            int32_t r, g;
        };
    };
};

struct vec2u {
    union {
        struct {
            uint32_t x, y;
        };
        struct {
            uint32_t r, g;
        };
    };
};

#endif //BEETROOT_VEC2_H
