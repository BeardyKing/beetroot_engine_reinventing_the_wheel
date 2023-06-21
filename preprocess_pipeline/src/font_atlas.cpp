#include <pipeline/font_atlas.h>

#include <string>
#include <array>
#include <bit>

#include <shared/log.h>
#include <shared/assert.h>
#include <cstdio>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>

void pipeline_build_font_atlas(const std::string &path,
                               const std::string &fontName,
                               const std::string &fontExt,
                               uint32_t fontSize,
                               uint32_t atlasSize) {
    const uint32_t dotSize = 6;

    const std::array<CharRanges, 1> supportedRanges{
            basicLatin,
    };

    uint32_t glyphCount{};
    for (const auto &range: supportedRanges) {
        glyphCount += range.end - range.start;
    }

    uint32_t *glyphText = new uint32_t[glyphCount];
    uint32_t textCount{};
    for (const auto &glyphRange: supportedRanges) {
        for (uint32_t glyphIndex = glyphRange.start; glyphIndex < glyphRange.end; ++glyphIndex) {
            glyphText[textCount] = glyphIndex;
            textCount++;
        }
    }

    uint32_t maxTexWidth = std::bit_ceil(atlasSize);
    uint32_t maxTexHeight = maxTexWidth;
    const uint32_t pixelCount = maxTexWidth * maxTexHeight;
    // render glyphs to atlas
    char *pixels = (char *) new char[pixelCount];
    int32_t pen_x = 0, pen_y = 0;

    GlyphInfo *glyphInfo = new GlyphInfo[glyphCount];

    std::string fontSrc = std::format("{}{}{}", path, fontName, fontExt);
    std::string fontOutName = std::format("{}{}{}", path, fontName, ".png");
    std::string fontOutNameMeta = std::format("{}{}{}", path, fontName, ".desc");

    const char *fontFile = fontSrc.c_str();
    FT_Library library;

    FT_Error initRes = FT_Init_FreeType(&library);
    ASSERT_MSG(initRes == FT_Err_Ok, "Err: failed to init freetype");

    FT_Face face;
    FT_Error faceRes = FT_New_Face(library, fontFile, 0, &face);
    ASSERT_MSG(faceRes == FT_Err_Ok, "Err could not load font %s \n", fontFile);

    FT_Error sizeRes = FT_Set_Char_Size(face, fontSize << dotSize, fontSize << dotSize, 0, 0);
    ASSERT_MSG(sizeRes == FT_Err_Ok, "Err failed to set font size %u", fontSize);

    hb_font_t *hbFont = hb_ft_font_create(face, nullptr);
    hb_buffer_t *hbBuffer = hb_buffer_create();

    hb_buffer_add_utf32(hbBuffer, glyphText, int32_t(glyphCount), 0, -1);
    hb_buffer_guess_segment_properties(hbBuffer);
    hb_shape(hbFont, hbBuffer, nullptr, 0);

    uint32_t hbGlyphCount = hb_buffer_get_length(hbBuffer);
    hb_glyph_info_t *bhInfo = hb_buffer_get_glyph_infos(hbBuffer, &glyphCount);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

    for (uint32_t i = 0; i < hbGlyphCount; ++i) {
        FT_Error glyphRes = FT_Load_Glyph(face, bhInfo[i].codepoint, FT_LOAD_RENDER);
        ASSERT_MSG(glyphRes == FT_Err_Ok, "Err failed to load char %u", bhInfo[i].codepoint);

        FT_Bitmap *bmp = &face->glyph->bitmap;
        if (pen_x + bmp->width >= maxTexWidth) {
            pen_x = 0;
            pen_y += ((face->size->metrics.height >> dotSize) + 1);
        }

        for (int row = 0; row < bmp->rows; ++row) {
            for (int col = 0; col < bmp->width; ++col) {
                int x = pen_x + col;
                int y = pen_y + row;
                const uint32_t pixelIndex = y * maxTexWidth + x;
                ASSERT_MSG(pixelIndex < pixelCount, "Err: did not allocate enough space [%u] of [%u] for glyph index [%u] of [%u]\n", pixelIndex,
                           pixelCount, i, hbGlyphCount);
                pixels[pixelIndex] = bmp->buffer[row * bmp->pitch + col];
            }
        }

        {
            //font .desc info
            glyphInfo[i].glyph = bhInfo[i].codepoint;

            glyphInfo[i].x0 = pen_x;
            glyphInfo[i].y0 = pen_y;
            glyphInfo[i].x1 = pen_x + bmp->width;
            glyphInfo[i].y1 = pen_y + bmp->rows;

            glyphInfo[i].x_off = pos[i].x_offset;//face->glyph->bitmap_left;
            glyphInfo[i].y_off = pos[i].y_offset;//face->glyph->bitmap_top;
            glyphInfo[i].advance = pos[i].x_advance >> dotSize;
        }
        pen_x += int32_t(bmp->width + 1);
    }

    {
        char *png_data = new char[maxTexWidth * maxTexHeight * 4];
        for (int i = 0; i < (maxTexWidth * maxTexHeight); ++i) {
            png_data[i * 4 + 0] |= pixels[i];
            png_data[i * 4 + 1] |= pixels[i];
            png_data[i * 4 + 2] |= pixels[i];
            png_data[i * 4 + 3] = 0xff;
        }
        stbi_write_png(fontOutName.c_str(), int32_t(maxTexWidth), int32_t(maxTexHeight), 4, png_data, int32_t(maxTexWidth) * 4);
        delete[] png_data;
    }
    {
        AtlasInfo info{};
        info.version = ATLAS_INFO_VERSION_0;
        info.glyphs = glyphInfo;
        info.glyphCount = hbGlyphCount;
        info.fontSize = fontSize;
        info.atlasWidth = maxTexWidth;
        info.atlasHeight = maxTexHeight;
        pipeline_save_atlas_info(&info, fontOutNameMeta);
    }

    delete[] glyphInfo;
    delete[] glyphText;
    delete[] pixels;
    FT_Done_FreeType(library);
}

void pipeline_save_atlas_info(const AtlasInfo *header, const std::string &fileSrc) {
    FILE *fileWrite = fopen(fileSrc.c_str(), "wb");
    ASSERT_MSG(fileWrite != nullptr, "Err: failed to write atlas info at path: %s ", fileSrc.c_str())

    fwrite(header, sizeof(AtlasInfo), 1, fileWrite);
    fwrite(&header->glyphs[0], sizeof(GlyphInfo) * header->glyphCount, 1, fileWrite);

    fclose(fileWrite);
}

AtlasInfo *pipeline_load_atlas_info(const std::string &fileSrc) {
    AtlasInfo *atlasInfo = new AtlasInfo;
    FILE *fileRead = fopen(fileSrc.c_str(), "rb");
    ASSERT_MSG(fileRead != nullptr, "Err: failed to load atlas info at path: %s ", fileSrc.c_str())

    fread(atlasInfo, sizeof(AtlasInfo), 1, fileRead);
    atlasInfo->glyphs = new GlyphInfo[atlasInfo->glyphCount];
    fread(&atlasInfo->glyphs[0], sizeof(GlyphInfo) * atlasInfo->glyphCount, 1, fileRead);

    fclose(fileRead);
    return atlasInfo;
}

//===pipeline tests==========
void test_load_atlas_info() {
    pipeline_build_font_atlas(PIPELINE_FONT_DIR, "JetBrainsMono-Regular", ".ttf", 48, 512);
    auto info = pipeline_load_atlas_info(PIPELINE_FONT_DIR "JetBrainsMono-Regular" ".desc");
    delete[] info->glyphs;
    delete info;
}