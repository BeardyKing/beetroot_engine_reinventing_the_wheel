#include <pipeline/font_shared.h>
#include <pipeline/font_atlas.h>

void build_font_atlas_and_description() {
    pipeline_build_font_atlas(PIPELINE_FONT_DIR, "JetBrainsMono/JetBrainsMono-Regular", ".ttf", 48, 512);
    pipeline_build_font_atlas(PIPELINE_FONT_DIR, "Unifont/unifont-15.0.06", ".ttf", 48, 512);
}

int main() {
    build_font_atlas_and_description();
}
