#include <pipeline/font_atlas.h>
#include <pipeline/shader_compile.h>
#include <pipeline/texture_compression.h>

#include <shared/log.h>
#include <shared/texture_formats.h>

#include <fmt/format.h>

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

void build_compressed_textures() {
    pipeline_build_compressed_textures("UV_Grid/UV_Grid_test.png", "UV_Grid/UV_Grid_test.dds", TextureFormat::BC7, false);
    pipeline_build_compressed_textures("sky/herkulessaulen_1k.hdr", "sky/herkulessaulen_1k.dds", TextureFormat::BC6H, false);
    pipeline_build_compressed_textures("hi_16x16.png", "hi_16x16.dds", TextureFormat::RGBA8, true);
    pipeline_build_compressed_textures("oct_map/oct_map_test.png", "oct_map/oct_map_test.dds", TextureFormat::BC7, true);
}

int main() {
    build_font_atlas_and_description();
    build_spv_from_source();
    build_compressed_textures();
}