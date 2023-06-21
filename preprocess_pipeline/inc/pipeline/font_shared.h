#ifndef BEETROOT_FONT_SHARED_H
#define BEETROOT_FONT_SHARED_H

#define PIPELINE_RES_DIR BEET_CMAKE_RES_DIR // defined in preprocess_pipeline/CmakeLists.txt
#define PIPELINE_FONT_DIR BEET_CMAKE_RES_DIR "fonts/"

#include <cstdint>

struct CharRanges {
    uint32_t start{}, end{};
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
//TODO add standard character ranges
//TODO add non+standard ligature ranges

struct GlyphInfo {
    int32_t glyph;              // character range value/index
    int32_t x0, y0, x1, y1;     // coords of glyph in the texture atlas
    int32_t x_off, y_off;       // left & top bearing when rendering
    int32_t advance;            // x advance when rendering
};

#define ATLAS_INFO_VERSION_0 0
struct AtlasInfo {
    uint32_t version;

    uint32_t fontSize;
    uint32_t atlasWidth;
    uint32_t atlasHeight;

    uint32_t glyphCount;
    GlyphInfo *glyphs;
};

#endif //BEETROOT_FONT_SHARED_H
