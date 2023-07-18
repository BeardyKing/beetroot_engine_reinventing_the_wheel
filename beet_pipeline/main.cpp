#include <pipeline/font_atlas.h>
#include <pipeline/shader_compile.h>
#include <shared/log.h>

#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>

#include <pipeline/pipeline_defines.h>
#include <shared/assert.h>

#include <ostream>
#include <fstream>

void build_font_atlas_and_description() {
    pipeline_font_atlas_log();
    {
        pipeline_build_font_atlas("JetBrainsMono/JetBrainsMono-Regular", ".ttf", 48, 512);
        pipeline_build_font_atlas("Unifont/unifont-15.0.06", ".ttf", 48, 512);
    }
}

void build_spv_from_source() {
    pipeline_shader_log();
    {
        pipeline_build_shader_spv("fallback/fallback.vert", "fallback/fallback.vert.spv");
        pipeline_build_shader_spv("fallback/fallback.frag", "fallback/fallback.frag.spv");
    }
    {
        pipeline_build_shader_spv("font/font.vert", "font/font.vert.spv");
        pipeline_build_shader_spv("font/font.frag", "font/font.frag.spv");
    }
}

enum class TextureFormat : uint32_t {
    RGBA8 = (uint32_t) cuttlefish::Texture::Format::R8G8B8A8,
    RGB8 = (uint32_t) cuttlefish::Texture::Format::R8G8B8,
    RGB16 = (uint32_t) cuttlefish::Texture::Format::R16G16B16,
    RGBA16 = (uint32_t) cuttlefish::Texture::Format::R16G16B16A16,

    BC1RGB = (uint32_t) cuttlefish::Texture::Format::BC1_RGB,
    BC1RGBA = (uint32_t) cuttlefish::Texture::Format::BC1_RGBA,
    BC2 = (uint32_t) cuttlefish::Texture::Format::BC2,
    BC3 = (uint32_t) cuttlefish::Texture::Format::BC3,
    BC4 = (uint32_t) cuttlefish::Texture::Format::BC4,
    BC5 = (uint32_t) cuttlefish::Texture::Format::BC5,
    BC6H = (uint32_t) cuttlefish::Texture::Format::BC6H,
    BC7 = (uint32_t) cuttlefish::Texture::Format::BC7,
};

#include <pipeline/pipeline_cache.h>

void pipeline_build_compressed_textures(const std::string &readPath,
                                        const std::string &writePath,
                                        const TextureFormat textureFormat,
                                        const bool generateMipsMaps = true) {

    const std::string inPath = std::format("{}{}", PIPELINE_TEXTURE_DIR, readPath);
    const std::string outPath = std::format("{}{}", CLIENT_RUNTIME_TEXTURE_DIR, writePath);

    if (!pipeline_cache_should_convert(outPath, inPath)) {
        return;
    }

    cuttlefish::Image image;
    image.load(inPath.c_str(), cuttlefish::ColorSpace::Linear);
    image.flipVertical();
    ASSERT_MSG(image.isValid() == true, "Err: failed to load texture %s ", inPath.c_str());

    const cuttlefish::Texture::Format targetFormat = (cuttlefish::Texture::Format) textureFormat;

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

    const bool convertRes = texture.convert(targetFormat, cuttlefish::Texture::Type::UNorm, cuttlefish::Texture::Quality::Normal);
    ASSERT_MSG(convertRes, "Err: Failed to convert texture");

    const uint32_t blockX = (texture.width() + cuttlefish::Texture::blockWidth(targetFormat) - 1) / cuttlefish::Texture::blockWidth(targetFormat);
    const uint32_t blockY = (texture.height() + cuttlefish::Texture::blockHeight(targetFormat) - 1) / cuttlefish::Texture::blockHeight(targetFormat);
    ASSERT_MSG(blockX * blockY * cuttlefish::Texture::blockSize(targetFormat) == texture.dataSize(), "Err: block compression size didn't match");

    const bool formatValidation = cuttlefish::Texture::isFormatValid(targetFormat, cuttlefish::Texture::Type::UNorm,
                                                                     cuttlefish::Texture::FileType::DDS);
    ASSERT_MSG(formatValidation, "Err: Invalid format");

    std::ofstream ofs(outPath, std::ios::out | std::ios::binary);
    const auto saveRes = texture.save(ofs, cuttlefish::Texture::FileType::DDS);
    ASSERT_MSG(saveRes == cuttlefish::Texture::SaveResult::Success, "Err: failed to save file %s", outPath.c_str());
}

void build_compressed_textures() {
    {
        pipeline_build_compressed_textures("UV_Grid/UV_Grid_test.png", "UV_Grid/UV_Grid_test.dds", TextureFormat::BC7, false);
    }
}

int main() {
    build_font_atlas_and_description();
    build_spv_from_source();
    build_compressed_textures();
}