#include <pipeline/texture_compression.h>

#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>

#include <pipeline/pipeline_defines.h>
#include <pipeline/pipeline_cache.h>

#include <shared/assert.h>

#include <ostream>
#include <fstream>
#include <format>

cuttlefish::Texture::Format beet_to_cuttlefish_texture_format(const TextureFormat &textureFormat);

void pipeline_build_compressed_textures(const std::string &readPath,
                                        const std::string &writePath,
                                        const TextureFormat format,
                                        const bool generateMipsMaps = true) {

    const std::string inPath = std::format("{}{}", PIPELINE_TEXTURE_DIR, readPath);
    const std::string outPath = std::format("{}{}", CLIENT_RUNTIME_TEXTURE_DIR, writePath);

//    if (!pipeline_cache_should_convert(outPath, inPath)) {
//        return;
//    }

    cuttlefish::Image image;
    image.load(inPath.c_str(), cuttlefish::ColorSpace::Linear);
    image.flipVertical();
    ASSERT_MSG(image.isValid() == true, "Err: failed to load texture %s ", inPath.c_str());

    const cuttlefish::Texture::Format targetFormat = beet_to_cuttlefish_texture_format(format);

    uint32_t mipCount = 1;
    if (generateMipsMaps) {
        mipCount = cuttlefish::Texture::maxMipmapLevels(cuttlefish::Texture::Dimension::Dim2D, image.width(), image.height());
    }

    cuttlefish::Texture texture(cuttlefish::Texture::Dimension::Dim2D, image.width(), image.height(), 0, mipCount);
    texture.setImage(image);

    if (mipCount > 1) {
        const bool mipGenRes = texture.generateMipmaps();
        ASSERT_MSG(mipGenRes, "Err: failed to generate mipmaps");
    }

    cuttlefish::Texture::Type pixelType = cuttlefish::Texture::Type::UNorm;
    switch (format) {
        case TextureFormat::BC6H:
            pixelType = cuttlefish::Texture::Type::UFloat;
            break;
        default:
            pixelType = cuttlefish::Texture::Type::UNorm;
            break;
    }

    const bool convertRes = texture.convert(targetFormat, pixelType, cuttlefish::Texture::Quality::Normal);
    ASSERT_MSG(convertRes, "Err: Failed to convert texture");

    const uint32_t blockX = (texture.width() + cuttlefish::Texture::blockWidth(targetFormat) - 1) / cuttlefish::Texture::blockWidth(targetFormat);
    const uint32_t blockY = (texture.height() + cuttlefish::Texture::blockHeight(targetFormat) - 1) / cuttlefish::Texture::blockHeight(targetFormat);
    ASSERT_MSG(blockX * blockY * cuttlefish::Texture::blockSize(targetFormat) == texture.dataSize(), "Err: block compression size didn't match");



    const bool formatValidation = cuttlefish::Texture::isFormatValid(targetFormat, pixelType,
                                                                     cuttlefish::Texture::FileType::DDS);
    ASSERT_MSG(formatValidation, "Err: Invalid format");

    std::ofstream ofs(outPath, std::ios::out | std::ios::binary);
    const auto saveRes = texture.save(ofs, cuttlefish::Texture::FileType::DDS);
    ASSERT_MSG(saveRes == cuttlefish::Texture::SaveResult::Success, "Err: failed to save file %s", outPath.c_str());
}

cuttlefish::Texture::Format beet_to_cuttlefish_texture_format(const TextureFormat &textureFormat) {
    switch (textureFormat) {
        case TextureFormat::RGBA8:
            return cuttlefish::Texture::Format::R8G8B8A8;
        case TextureFormat::RGBA16:
            return cuttlefish::Texture::Format::R16G16B16A16;

        case TextureFormat::BC1RGBA:
            return cuttlefish::Texture::Format::BC1_RGBA;
        case TextureFormat::BC2:
            return cuttlefish::Texture::Format::BC2;
        case TextureFormat::BC3:
            return cuttlefish::Texture::Format::BC3;
        case TextureFormat::BC4:
            return cuttlefish::Texture::Format::BC4;
        case TextureFormat::BC5:
            return cuttlefish::Texture::Format::BC5;
        case TextureFormat::BC6H:
            return cuttlefish::Texture::Format::BC6H;
        case TextureFormat::BC7:
            return cuttlefish::Texture::Format::BC7;
        default: {
            SANITY_CHECK();
        }
    };
    return cuttlefish::Texture::Format::Unknown;
}