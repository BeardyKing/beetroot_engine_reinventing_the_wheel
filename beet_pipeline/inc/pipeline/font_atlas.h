#ifndef BEETROOT_FONT_ATLAS_H
#define BEETROOT_FONT_ATLAS_H

#include <pipeline/font_shared.h>

#include <cstdint>
#include <string>

void pipeline_build_font_atlas(const std::string &fontName,
                               const std::string &fontExt,
                               uint32_t fontSize,
                               uint32_t atlasSize);

void pipeline_save_atlas_info(const AtlasInfo *header, const std::string &fileSrc);

AtlasInfo *pipeline_load_atlas_info(const std::string &fileSrc);

void pipeline_font_atlas_log();

#endif //BEETROOT_FONT_ATLAS_H
