#include <shared/dds_loader.h>
#include <shared/log.h>
#include <shared/assert.h>
#include <shared/texture_formats.h>

#include <filesystem>
#include <iostream>
#include <fstream>

struct PixelFormatDDS {
    uint32_t dwSize;        // dwSize:          Structure size
    uint32_t dwFlags;       // dwFlags:         Values which indicate what type of data is in the surface.
    uint32_t dwFourCC;      // dwFourCC:        Four-character codes for specifying compressed or custom formats. i.e. {'D','X','T','2'}{'B','C','6','H'}
    uint32_t dwRGBBitCount; // dwRGBBitCount:   Number of bits in an RGB.
    uint32_t dwRBitMask;    // dwRBitMask:      Red (or luminance or Y) mask for reading color data.
    uint32_t dwGBitMask;    // dwGBitMask:      Green (or U) mask for reading color data.
    uint32_t dwBBitMask;    // dwBBitMask:      Blue (or V) mask for reading color data.
    uint32_t dwABitMask;    // dwABitMask:      Alpha mask for reading alpha data.
};

struct HeaderDDS {
    uint32_t dwSize;                // dwSize:              Size of structure
    uint32_t dwFlags;               // dwFlags:             Flags to indicate which members contain valid data.
    uint32_t dwHeight;              // dwHeight:            Surface height (in pixels).
    uint32_t dwWidth;               // dwWidth:             Surface width (in pixels).
    uint32_t dwPitchOrLinearSize;   // dwPitchOrLinearSize: The pitch or number of bytes per scan line in an uncompressed texture
    uint32_t dwDepth;               // dwDepth:             Depth of a volume texture (in pixels), otherwise unused.
    uint32_t dwMipMapCount;         // dwMipMapCount:       Number of mipmap levels, otherwise unused.
    uint32_t dwReserved1[11];       // dwReserved1:         Unused.
    PixelFormatDDS ddspf;           // ddspf:               The pixel format see BEET_DXGI_FORMAT
    uint32_t dwCaps;                // dwCaps:
    uint32_t dwCaps2;               // dwCaps2:
    uint32_t dwCaps3;               // dwCaps3:
    uint32_t dwCaps4;               // dwCaps4:
    uint32_t dwReserved2;           // dwReserved2:
};

struct HeaderDDSDXT10 {
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag; // See D3D11_RESOURCE_MISC_FLAG
    uint32_t arraySize;
    uint32_t reserved;
};

TextureFormat internal_dxgi_to_beet_texture_format(const TextureFormatDXGI textureFormat);

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)              \
                ((uint32_t)(uint8_t)(ch0) |         \
                ((uint32_t)(uint8_t)(ch1) << 8) |   \
                ((uint32_t)(uint8_t)(ch2) << 16) |  \
                ((uint32_t)(uint8_t)(ch3) << 24)    \
                )
#endif /* defined(MAKEFOURCC) */

enum D3D11_RESOURCE_DIMENSION {
    D3D11_RESOURCE_DIMENSION_UNKNOWN = 0,
    D3D11_RESOURCE_DIMENSION_BUFFER = 1,
    D3D11_RESOURCE_DIMENSION_TEXTURE1D = 2,
    D3D11_RESOURCE_DIMENSION_TEXTURE2D = 3,
    D3D11_RESOURCE_DIMENSION_TEXTURE3D = 4
};

static size_t BitsPerPixel(_In_ TextureFormatDXGI fmt) {
    switch (fmt) {
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G8X24_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 64;

        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R11G11B10_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R24G8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D24_UNORM_S8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_B8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_G8R8_G8B8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 32;

        case TextureFormatDXGI::DXGI_FORMAT_R8G8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_D16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_B5G6R5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B5G5R5A1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case TextureFormatDXGI::DXGI_FORMAT_R8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_A8_UNORM:
            return 8;

        case TextureFormatDXGI::DXGI_FORMAT_R1_UNORM:
            return 1;

        case TextureFormatDXGI::DXGI_FORMAT_BC1_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_SNORM:
            return 4;

        case TextureFormatDXGI::DXGI_FORMAT_BC2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_SF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
    }
}

static void GetSurfaceInfo(
        _In_ size_t width,
        _In_ size_t height,
        _In_ TextureFormatDXGI fmt,
        _Out_opt_ size_t *outNumBytes,
        _Out_opt_ size_t *outRowBytes,
        _Out_opt_ size_t *outNumRows
) {
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed = false;
    size_t bcnumBytesPerBlock = 0;
    switch (fmt) {
        case TextureFormatDXGI::DXGI_FORMAT_BC1_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bcnumBytesPerBlock = 8;
            break;

        case TextureFormatDXGI::DXGI_FORMAT_BC2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_SF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bcnumBytesPerBlock = 16;
            break;

        case TextureFormatDXGI::DXGI_FORMAT_R8G8_B8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_G8R8_G8B8_UNORM:
            packed = true;
            break;
    }

    if (bc) {
        size_t numBlocksWide = 0;
        if (width > 0) {
            numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
        }
        size_t numBlocksHigh = 0;
        if (height > 0) {
            numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
        }
        rowBytes = numBlocksWide * bcnumBytesPerBlock;
        numRows = numBlocksHigh;
    } else if (packed) {
        rowBytes = ((width + 1) >> 1) * 4;
        numRows = height;
    } else {
        size_t bpp = BitsPerPixel(fmt);
        rowBytes = (width * bpp + 7) / 8; // Round up to the nearest byte.
        numRows = height;
    }

    numBytes = rowBytes * numRows;
    if (outNumBytes) {
        *outNumBytes = numBytes;
    }
    if (outRowBytes) {
        *outRowBytes = rowBytes;
    }
    if (outNumRows) {
        *outNumRows = numRows;
    }
}

void load_dds_image(const char *path, RawImage *outRawImage) {
    log_info("loading file %s\n", path);
    size_t out_fileSize{};
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        log_warning("failed to find path: %s\n", path);
        out_fileSize = 0;
    }

    out_fileSize = static_cast<size_t>(file.tellg());
    log_info("file size %zu \n", out_fileSize);

    char *out_data = new char[out_fileSize]();
    char ext[4]{};

    file.seekg(0);
    file.read(ext, sizeof(uint32_t));
    file.read(out_data, out_fileSize - sizeof(uint32_t));
    const char expectedHeaderFmt[4] = {'D', 'D', 'S', ' '};
    if ((uint32_t) *ext == (uint32_t) *expectedHeaderFmt) {
        log_info("is dds file\n");
    }
    file.close();
    const HeaderDDS *header = reinterpret_cast<const HeaderDDS *> (out_data);

    size_t width = header->dwWidth;
    size_t height = header->dwHeight;
    size_t depth = header->dwDepth;

    uint32_t resDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    size_t arraySize = 1;
    TextureFormatDXGI format{};
    bool isCubeMap = false;

    size_t mipCount = header->dwMipMapCount;
    if (0 == mipCount) {
        mipCount = 1;
    }
    size_t outNumBytes{};
    size_t outRowBytes{};
    size_t outNumRows{};
    const HeaderDDSDXT10 *d3d10ext = reinterpret_cast<const HeaderDDSDXT10 *>((const char *) header + sizeof(HeaderDDS));

    if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.dwFourCC) {
        log_info("is dx10\n");
        arraySize = d3d10ext->arraySize;
        format = (TextureFormatDXGI) d3d10ext->dxgiFormat;
        resDim = d3d10ext->resourceDimension;
        GetSurfaceInfo(width, height, format, &outNumBytes, &outRowBytes, &outNumRows);
    }
    uint32_t headerSpace = (sizeof(HeaderDDS) - sizeof(HeaderDDSDXT10) - sizeof(uint32_t));
    log_info("header space bytes: %u, full dataSize: %u \n", headerSpace, out_fileSize - headerSpace)
    void *imageStartPos = (unsigned char *) header + sizeof(HeaderDDS) + sizeof(HeaderDDSDXT10);
    outRawImage->textureFormat = internal_dxgi_to_beet_texture_format(format);
    outRawImage->mipMapCount = mipCount;
    outRawImage->width = width;
    outRawImage->height = height;
    outRawImage->depth = depth;
    outRawImage->dataSize = mipCount > 1 ? out_fileSize - headerSpace : outNumBytes;
    outRawImage->data = (unsigned char *) malloc(outRawImage->dataSize);
    memset(outRawImage->data, 0, outRawImage->dataSize);
    memcpy(outRawImage->data, imageStartPos, outRawImage->dataSize);

    log_info("%p \n", header);
    log_info("%p \n", imageStartPos);
}

TextureFormat internal_dxgi_to_beet_texture_format(const TextureFormatDXGI textureFormat) {
    switch (textureFormat) {
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM:
            return TextureFormat::RGBA8;
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UNORM:
            return TextureFormat::RGBA16;

        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
            return TextureFormat::BC1RGBA;
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
            return TextureFormat::BC2;
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
            return TextureFormat::BC3;
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
            return TextureFormat::BC4;
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
            return TextureFormat::BC5;
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
            return TextureFormat::BC6H;
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
            return TextureFormat::BC7;
        default: SANITY_CHECK();
    };
    return TextureFormat::INVALID_FORMAT;
}
