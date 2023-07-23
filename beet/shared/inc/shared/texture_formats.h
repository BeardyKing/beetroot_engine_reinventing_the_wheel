#ifndef BEETROOT_TEXTURE_FORMATS_H
#define BEETROOT_TEXTURE_FORMATS_H

#include <cstdint>

//supported engine formats
enum class TextureFormat : uint32_t {
    RGBA8,
    RGBA16,

    BC1RGBA,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6H,
    BC7,
    INVALID_FORMAT
};

struct RawImage {
    TextureFormat textureFormat;
    uint32_t mipMapCount;
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    uint32_t dataSize;
    void* data;
};

#endif //BEETROOT_TEXTURE_FORMATS_H
