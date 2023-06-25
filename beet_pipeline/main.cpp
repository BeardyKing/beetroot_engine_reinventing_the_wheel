#include <pipeline/font_atlas.h>
#include <pipeline/shader_compile.h>
#include <shared/log.h>

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
}

int main() {
    build_font_atlas_and_description();
    build_spv_from_source();
}