#include <pipeline/font_atlas.h>
#include <pipeline/pipeline_cache.h>

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

#include <fmt/format.h>

void pipeline_build_font_atlas(const std::string &fontName,
                               const std::string &fontExt,
                               uint32_t fontSize,
                               uint32_t atlasSize) {

    const std::string readPath = PIPELINE_RES_DIR;
    const std::string savePath = CLIENT_RUNTIME_RES_DIR;

    std::string fontSrc = fmt::format("{}{}{}", readPath, fontName, fontExt);
    std::string fontAtlasOutName = fmt::format("{}{}{}", savePath, fontName, ".png");
    std::string fontAtlasDescOutName = fmt::format("{}{}{}", savePath, fontName, ".desc");

    if (!pipeline_cache_should_convert(fontAtlasOutName, fontSrc) &&
        !pipeline_cache_should_convert(fontAtlasDescOutName, fontSrc)) {
        return;
    }

    const uint32_t dotSize = 6;

    const CharRanges invalid_character{0xFFFD, 0xFFFE};
    const std::array<CharRanges, 2> supportedRanges{
            invalid_character,
            basic_latin,
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
    memset(pixels, 0, sizeof(char) * pixelCount);

    int32_t pen_x = 0, pen_y = 0;

    GlyphInfo *glyphInfo = new GlyphInfo[glyphCount];
    memset(glyphInfo, 0, sizeof(GlyphInfo) * glyphCount);

    const char *fontFile = fontSrc.c_str();
    FT_Library library;

    FT_Error initRes = FT_Init_FreeType(&library);
    ASSERT_MSG(initRes == FT_Err_Ok, "Err: failed to init freetype");

    FT_Face face;
    FT_Error faceRes = FT_New_Face(library, fontFile, 0, &face);
    ASSERT_MSG(faceRes == FT_Err_Ok, "Err could not load font %s \n", fontFile);

    FT_Error sizeRes = FT_Set_Char_Size(face, fontSize << dotSize, fontSize << dotSize, 0, 0);
    ASSERT_MSG(sizeRes == FT_Err_Ok, "Err failed to set font size %u", fontSize);

    for (uint32_t i = 0; i < glyphCount; ++i) {
        FT_Error glyphRes = FT_Load_Char(face, glyphText[i], FT_LOAD_RENDER);
        ASSERT_MSG(glyphRes == FT_Err_Ok, "Err failed to load char %u", glyphText[i]);

        FT_Bitmap *bmp = &face->glyph->bitmap;
        if (pen_x + bmp->width >= maxTexWidth) {
            pen_x = 0;
            pen_y += ((face->size->metrics.height >> dotSize) + 1);
        }

        for (uint32_t row = 0; row < bmp->rows; ++row) {
            for (uint32_t col = 0; col < bmp->width; ++col) {
                int x = pen_x + col;
                int y = pen_y + row;
                const uint32_t pixelIndex = y * maxTexWidth + x;
                ASSERT_MSG(pixelIndex < pixelCount, "Err: did not allocate enough space [%u] of [%u] for glyph index [%u] of [%u]\n", pixelIndex,
                           pixelCount, i, glyphCount);
                pixels[pixelIndex] = (char) bmp->buffer[row * bmp->pitch + col];
            }
        }
        {
            //font .desc info
            glyphInfo[i].glyph = glyphText[i];
            glyphInfo[i].x0 = pen_x;
            glyphInfo[i].y0 = pen_y;
            glyphInfo[i].x1 = int32_t(pen_x + bmp->width);
            glyphInfo[i].y1 = int32_t(pen_y + bmp->rows);

            glyphInfo[i].x_off = face->glyph->bitmap_left;
            glyphInfo[i].y_off = face->glyph->bitmap_top;
            glyphInfo[i].advance = face->glyph->advance.x >> dotSize;
        }
        pen_x += int32_t(bmp->width + 1);
    }

    {
        char *png_data = new char[maxTexWidth * maxTexHeight * 4];
        memset(png_data, 0, sizeof(char) * (maxTexWidth * maxTexHeight * 4));

        for (uint32_t i = 0; i < (maxTexWidth * maxTexHeight); ++i) {
            png_data[i * 4 + 0] |= pixels[i];
            png_data[i * 4 + 1] |= pixels[i];
            png_data[i * 4 + 2] |= pixels[i];
            png_data[i * 4 + 3] = pixels[i];
        }
        stbi_write_png(fontAtlasOutName.c_str(), int32_t(maxTexWidth), int32_t(maxTexHeight), 4, png_data, int32_t(maxTexWidth) * 4);
        delete[] png_data;
    }
    log_info("font atlas: %s \n", fontAtlasOutName.c_str());
    {
        AtlasInfo info{};
        info.version = ATLAS_INFO_VERSION_0;
        info.glyphs = glyphInfo;
        info.glyphCount = glyphCount;
        info.fontSize = fontSize;
        info.atlasWidth = maxTexWidth;
        info.atlasHeight = maxTexHeight;
        pipeline_save_atlas_info(&info, fontAtlasDescOutName);
    }
    log_info("font desc: %s \n", fontAtlasDescOutName.c_str());

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
    pipeline_build_font_atlas("JetBrainsMono-Regular", ".ttf", 48, 512);
    auto info = pipeline_load_atlas_info(CLIENT_RUNTIME_FONT_DIR "JetBrainsMono-Regular" ".desc");
    delete[] info->glyphs;
    delete info;
}

void pipeline_font_atlas_log() {
    log_info("\n")
    log_info("===========================\n")
    log_info("===BUILDING FONTS==========\n")
    log_info("===========================\n")
    log_info("\n")
}