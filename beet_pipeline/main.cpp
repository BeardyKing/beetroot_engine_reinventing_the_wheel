#include <pipeline/font_atlas.h>
#include <pipeline/shader_compile.h>
#include <shared/log.h>

#include <pipeline/pipeline_defines.h>
#include <shared/assert.h>

#include <shared/texture_formats.h>
#include <shared/dds_loader.h>

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


#include <pipeline/texture_compression.h>
#include <shared/texture_formats.h>


void build_compressed_textures() {
    {
        pipeline_build_compressed_textures("UV_Grid/UV_Grid_test.png", "UV_Grid/UV_Grid_test.dds", TextureFormat::BC7, false);
        pipeline_build_compressed_textures("sky/herkulessaulen_1k.hdr", "sky/herkulessaulen_1k.dds", TextureFormat::BC6H, false);
        pipeline_build_compressed_textures("hi_16x16.png", "hi_16x16.dds", TextureFormat::RGBA8, true);
    }
}

#include <cuttlefish/Image.h>
#include <cuttlefish/Texture.h>

#include <pipeline/pipeline_defines.h>
#include <pipeline/pipeline_cache.h>

#include <shared/assert.h>

#include <ostream>
#include <fstream>
#include <format>

#define TINYDDSLOADER_IMPLEMENTATION
#include <shared/tinyddsloader.h>

int main() {
    build_font_atlas_and_description();
    build_spv_from_source();
//    build_compressed_textures();

//    const std::string inPath = std::format("{}{}", CLIENT_RUNTIME_TEXTURE_DIR, "hi_16x16.dds");
    const std::string inPath = std::format("{}{}", CLIENT_RUNTIME_TEXTURE_DIR, "UV_Grid/UV_Grid_test.dds");
    RawImage image{};
    load_dds_image(inPath.c_str(), &image);
    log_info("img %u %u\n", image.width, image.height);

    using namespace tinyddsloader;
    DDSFile dds;
    auto ret = dds.Load(inPath.c_str());
    if (tinyddsloader::Result::Success != ret) {
        std::cout << "Failed to load.[" << inPath << "]\n";
        std::cout << "Result : " << int(ret) << "\n";
        return 1;
    }
    auto imageData = dds.GetImageData(0);
    auto rawImageData = (unsigned char *)imageData->m_mem;
    int32_t valid = memcmp(image.data, rawImageData, image.dataSize);
    log_info("hello image %i\n", valid);
    log_info("\n");
}

