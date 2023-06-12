#include <string>
#include <array>

#include <shared/log.h>
#include <shared/assert.h>
#include <cstdio>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>

#define PIPELINE_RES_DIR BEET_CMAKE_RES_DIR // defined in preprocess_pipeline/CmakeLists.txt
#define PIPELINE_FONT_DIR BEET_CMAKE_RES_DIR "fonts/"

struct GlyphInfo {
    int x0, y0, x1, y1;     // coords of glyph in the texture atlas
    int x_off, y_off;       // left & top bearing when rendering
    int advance;            // x advance when rendering
};

struct AtlasInfo {
    GlyphInfo *glyphs;
    uint32_t glyphCount;

    uint32_t atlasWidth;
    uint32_t atlasHeight;
};

struct CharRanges {
    uint32_t start, end;
};

const CharRanges basicLatin{0x0000, 0x007F};            //0
const CharRanges latin_1{0x0080, 0x00FF};               //1
const CharRanges latin_ext_a{0x0100, 0x017F};           //2
const CharRanges latin_ext_b{0x0180, 0x024F};           //3
const CharRanges ipa_ext{0x0250, 0x0250};               //4
const CharRanges phonetic_ext{0x1D00, 0x1D7F};          //5
const CharRanges phonetic_ext_sup{0x1D80, 0x1DBF};      //6
const CharRanges spacing_mod_letters{0x02B0, 0x02FF};   //7
const CharRanges hiragana{0x3040, 0x309F};              //49
const CharRanges katakana{0x30A0, 0x30FF};              //50

void pipeline_build_font_atlas(const std::string &path, const std::string &fontName, const std::string &fontExt, uint32_t fontSize);

void pipeline_write_atlas_description(const AtlasInfo *atlasInfo, const std::string &fontName) {
    std::string fileSrc = std::format("{}{}{}", PIPELINE_FONT_DIR, fontName, ".desc");
    FILE *fileOut = fopen(fileSrc.c_str(), "w");
    fwrite(&atlasInfo, (sizeof(AtlasInfo)), 1, fileOut);
    fwrite(atlasInfo->glyphs, (sizeof(GlyphInfo)), atlasInfo->glyphCount, fileOut);
    fclose(fileOut);
}

//caller takes ownership of both atlasInfo & GlyphInfo arr
AtlasInfo *pipeline_read_atlas_description(const std::string &path) {
    AtlasInfo *atlasInfo = new AtlasInfo;
    FILE *fin = fopen(path.c_str(), "r");
    fread(atlasInfo, sizeof(AtlasInfo), 1, fin);
    GlyphInfo *glyphInfo = new GlyphInfo[atlasInfo->glyphCount];
    fread(glyphInfo, sizeof(GlyphInfo), atlasInfo->glyphCount, fin);
    fclose(fin);
    atlasInfo->glyphs = glyphInfo;
    return atlasInfo;
}

int main() {
    pipeline_build_font_atlas(PIPELINE_FONT_DIR, "JetBrainsMono-Regular", ".ttf", 64);
    pipeline_build_font_atlas(PIPELINE_FONT_DIR, "unifont-15.0.06", ".ttf", 64);

    auto info = pipeline_read_atlas_description(PIPELINE_FONT_DIR "unifont-15.0.06" ".desc");

    log_info("count %u \n", info->glyphCount);
    log_info("glyphs[10].advance: %i", info->glyphs[10].advance);

    delete[] info->glyphs;
    delete info;
}

void pipeline_build_font_atlas(const std::string &path, const std::string &fontName, const std::string &fontExt, const uint32_t fontSize) {
    const uint32_t dotSize = 6;

    const std::array<CharRanges, 9> supportedRanges{
            basicLatin, latin_1, latin_ext_a,
            ipa_ext,
            phonetic_ext,
            phonetic_ext_sup,
            spacing_mod_letters,
            hiragana,
            katakana,
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

    GlyphInfo *glyphInfo = new GlyphInfo[glyphCount];

    std::string fontSrc = std::format("{}{}{}", path, fontName, fontExt);
    std::string fontOutName = std::format("{}{}{}{}{}{}", path, "font_atlas_pt", fontSize, "_", fontName, ".png");
    std::string fontOutNameMeta = std::format("{}{}{}{}{}{}", path, "font_atlas_pt", fontSize, "_", fontName, ".meta");

    const char *fontFile = fontSrc.c_str();
    FT_Library library;

    FT_Error initRes = FT_Init_FreeType(&library);
    ASSERT_MSG(initRes == FT_Err_Ok, "Err: failed to init freetype");

    FT_Face face;
    FT_Error faceRes = FT_New_Face(library, fontFile, 0, &face);
    ASSERT_MSG(faceRes == FT_Err_Ok, "Err could not load font %s \n", fontFile);

    FT_Error sizeRes = FT_Set_Char_Size(face, fontSize << dotSize, fontSize << dotSize, 0, 0);
    ASSERT_MSG(faceRes == FT_Err_Ok, "Err failed to set font size %u", fontSize);

    hb_font_t *hbFont = hb_ft_font_create(face, nullptr);
    hb_buffer_t *hbBuffer = hb_buffer_create();

    hb_buffer_add_utf32(hbBuffer, glyphText, int32_t(glyphCount), 0, -1);
    hb_buffer_guess_segment_properties(hbBuffer);
    hb_shape(hbFont, hbBuffer, nullptr, 0);

    uint32_t hbGlyphCount = hb_buffer_get_length(hbBuffer);
    hb_glyph_info_t *bhInfo = hb_buffer_get_glyph_infos(hbBuffer, nullptr);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hbBuffer, nullptr);

    // max possible atlas size
    uint32_t maxTexWidth = (fontSize << dotSize) / 2;
    uint32_t maxTexHeight = maxTexWidth;

    // render glyphs to atlas
    char *pixels = (char *) calloc(maxTexWidth * maxTexHeight, 1);
    int32_t pen_x = 0, pen_y = 0;

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
                pixels[y * maxTexWidth + x] = bmp->buffer[row * bmp->pitch + col];
            }
        }

        //font .desc info
        glyphInfo[i].x0 = pen_x;
        glyphInfo[i].y0 = pen_y;
        glyphInfo[i].x1 = pen_x + bmp->width;
        glyphInfo[i].y1 = pen_y + bmp->rows;

        glyphInfo[i].x_off = face->glyph->bitmap_left;
        glyphInfo[i].y_off = face->glyph->bitmap_top;
        glyphInfo[i].advance = face->glyph->advance.x >> dotSize;

        pen_x += bmp->width + 1;
    }

    char *png_data = (char *) calloc(maxTexWidth * maxTexHeight * 4, 1);
    for (int i = 0; i < (maxTexWidth * maxTexHeight); ++i) {
        png_data[i * 4 + 0] |= pixels[i];
        png_data[i * 4 + 1] |= pixels[i];
        png_data[i * 4 + 2] |= pixels[i];
        png_data[i * 4 + 3] = 0xff;
    }

    stbi_write_png(fontOutName.c_str(), int32_t(maxTexWidth), int32_t(maxTexHeight), 4, png_data, maxTexWidth * 4);

    AtlasInfo info{};
    info.glyphs = glyphInfo;
    info.glyphCount = hbGlyphCount;
    info.atlasWidth = maxTexWidth;
    info.atlasHeight = maxTexHeight;

    pipeline_write_atlas_description(&info, fontName);

    FT_Done_FreeType(library);
    delete[]glyphInfo;
    delete[]glyphText;
    free(png_data);
    free(pixels);
}