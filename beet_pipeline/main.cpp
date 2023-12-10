#include <pipeline/font_atlas.h>
#include <pipeline/shader_compile.h>
#include <pipeline/texture_compression.h>
#include <pipeline/pipeline_commandlines.h>

#include <shared/log.h>
#include <shared/texture_formats.h>

#include <fmt/format.h>

void build_font_atlas_and_description() {
    pipeline_font_atlas_log();
    {
        pipeline_build_font_atlas("fonts/JetBrainsMono/JetBrainsMono-Regular", ".ttf", 48, 512);
        pipeline_build_font_atlas("fonts/Unifont/unifont-15.0.06", ".ttf", 48, 512);
    }
}

void build_spv_from_source() {
    pipeline_shader_log();
    {
        pipeline_build_shader_spv("shaders/lit/lit.vert", "shaders/lit/lit.vert.spv");
        pipeline_build_shader_spv("shaders/lit/lit.frag", "shaders/lit/lit.frag.spv");
    }
    {
        pipeline_build_shader_spv("shaders/font/font.vert", "shaders/font/font.vert.spv");
        pipeline_build_shader_spv("shaders/font/font.frag", "shaders/font/font.frag.spv");
    }
}

void build_compressed_textures() {
    // compress generated font atlas.
    pipeline_build_compressed_textures("fonts/JetBrainsMono/JetBrainsMono-Regular.png", "fonts/JetBrainsMono/JetBrainsMono-Regular.dds", TextureFormat::BC7, false, true);

    pipeline_build_compressed_textures("textures/UV_Grid/UV_Grid_test.png", "textures/UV_Grid/UV_Grid_test.dds", TextureFormat::BC7, false);
    pipeline_build_compressed_textures("textures/sky/herkulessaulen_1k.hdr", "textures/sky/herkulessaulen_1k.dds", TextureFormat::BC6H, false);
    pipeline_build_compressed_textures("textures/hi_16x16.png", "textures/hi_16x16.dds", TextureFormat::RGBA8, true);
    pipeline_build_compressed_textures("textures/oct_map/oct_map_test.png", "textures/oct_map/oct_map_test.dds", TextureFormat::BC7, true);

}

int32_t main(int32_t argc, char **argv) {
    commandline_init(argc, argv);
    if(commandline_get_arg(CLArgs::help).enabled){
        commandline_show_commands();
        return 0;
    }

    build_font_atlas_and_description();
    build_spv_from_source();
    build_compressed_textures();
}