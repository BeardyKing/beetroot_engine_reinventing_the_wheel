#ifndef BEETROOT_FONT_SHARED_H
#define BEETROOT_FONT_SHARED_H

#include <pipeline/pipeline_defines.h>

#include <cstdint>

struct GlyphInfo {
    uint32_t glyph;             // character range value/index
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

struct CharRanges {
    uint32_t start{}, end{};
};

const CharRanges basic_latin{0x0020,0x007f};
const CharRanges latin_1_supplement{0x00a0,0x00ff};
const CharRanges latin_extended_a{0x0100,0x017f};
const CharRanges latin_extended_b{0x0180,0x024f};
const CharRanges ipa_extensions{0x0250,0x02af};
const CharRanges spacing_modifier_letters{0x02b0,0x02ff};
const CharRanges combining_diacritical_marks{0x0300,0x036f};
const CharRanges greek_and_coptic{0x0370,0x03ff};
const CharRanges cyrillic{0x0400,0x04ff};
const CharRanges cyrillic_supplementary{0x0500,0x052f};
const CharRanges armenian{0x0530,0x058f};
const CharRanges hebrew{0x0590,0x05ff};
const CharRanges arabic{0x0600,0x06ff};
const CharRanges syriac{0x0700,0x074f};
const CharRanges thaana{0x0780,0x07bf};
const CharRanges devanagari{0x0900,0x097f};
const CharRanges bengali{0x0980,0x09ff};
const CharRanges gurmukhi{0x0a00,0x0a7f};
const CharRanges gujarati{0x0a80,0x0aff};
const CharRanges oriya{0x0b00,0x0b7f};
const CharRanges tamil{0x0b80,0x0bff};
const CharRanges telugu{0x0c00,0x0c7f};
const CharRanges kannada{0x0c80,0x0cff};
const CharRanges malayalam{0x0d00,0x0d7f};
const CharRanges sinhala{0x0d80,0x0dff};
const CharRanges thai{0x0e00,0x0e7f};
const CharRanges lao{0x0e80,0x0eff};
const CharRanges tibetan{0x0f00,0x0fff};
const CharRanges myanmar{0x1000,0x109f};
const CharRanges georgian{0x10a0,0x10ff};
const CharRanges hangul_jamo{0x1100,0x11ff};
const CharRanges ethiopic{0x1200,0x137f};
const CharRanges cherokee{0x13a0,0x13ff};
const CharRanges unified_canadian_aboriginal_syllabics{0x1400,0x167f};
const CharRanges ogham{0x1680,0x169f};
const CharRanges runic{0x16a0,0x16ff};
const CharRanges tagalog{0x1700,0x171f};
const CharRanges hanunoo{0x1720,0x173f};
const CharRanges buhid{0x1740,0x175f};
const CharRanges tagbanwa{0x1760,0x177f};
const CharRanges khmer{0x1780,0x17ff};
const CharRanges mongolian{0x1800,0x18af};
const CharRanges limbu{0x1900,0x194f};
const CharRanges tai_le{0x1950,0x197f};
const CharRanges khmer_symbols{0x19e0,0x19ff};
const CharRanges phonetic_extensions{0x1d00,0x1d7f};
const CharRanges latin_extended_additional{0x1e00,0x1eff};
const CharRanges greek_extended{0x1f00,0x1fff};
const CharRanges general_punctuation{0x2000,0x206f};
const CharRanges superscripts_and_subscripts{0x2070,0x209f};
const CharRanges currency_symbols{0x20a0,0x20cf};
const CharRanges combining_diacritical_marks_for_symbols{0x20d0,0x20ff};
const CharRanges letterlike_symbols{0x2100,0x214f};
const CharRanges number_forms{0x2150,0x218f};
const CharRanges arrows{0x2190,0x21ff};
const CharRanges mathematical_operators{0x2200,0x22ff};
const CharRanges miscellaneous_technical{0x2300,0x23ff};
const CharRanges control_pictures{0x2400,0x243f};
const CharRanges optical_character_recognition{0x2440,0x245f};
const CharRanges enclosed_alphanumerics{0x2460,0x24ff};
const CharRanges box_drawing{0x2500,0x257f};
const CharRanges block_elements{0x2580,0x259f};
const CharRanges geometric_shapes{0x25a0,0x25ff};
const CharRanges miscellaneous_symbols{0x2600,0x26ff};
const CharRanges dingbats{0x2700,0x27bf};
const CharRanges miscellaneous_mathematical_symbols_a{0x27c0,0x27ef};
const CharRanges supplemental_arrows_a{0x27f0,0x27ff};
const CharRanges braille_patterns{0x2800,0x28ff};
const CharRanges supplemental_arrows_b{0x2900,0x297f};
const CharRanges miscellaneous_mathematical_symbols_b{0x2980,0x29ff};
const CharRanges supplemental_mathematical_operators{0x2a00,0x2aff};
const CharRanges miscellaneous_symbols_and_arrows{0x2b00,0x2bff};
const CharRanges cjk_radicals_supplement{0x2e80,0x2eff};
const CharRanges kangxi_radicals{0x2f00,0x2fdf};
const CharRanges ideographic_description_characters{0x2ff0,0x2fff};
const CharRanges cjk_symbols_and_punctuation{0x3000,0x303f};
const CharRanges hiragana{0x3040,0x309f};
const CharRanges katakana{0x30a0,0x30ff};
const CharRanges bopomofo{0x3100,0x312f};
const CharRanges hangul_compatibility_jamo{0x3130,0x318f};
const CharRanges kanbun{0x3190,0x319f};
const CharRanges bopomofo_extended{0x31a0,0x31bf};
const CharRanges katakana_phonetic_extensions{0x31f0,0x31ff};
const CharRanges enclosed_cjk_letters_and_months{0x3200,0x32ff};
const CharRanges cjk_compatibility{0x3300,0x33ff};
const CharRanges cjk_unified_ideographs_extension_a{0x3400,0x4dbf};
const CharRanges yijing_hexagram_symbols{0x4dc0,0x4dff};
const CharRanges cjk_unified_ideographs{0x4e00,0x9fff};
const CharRanges yi_syllables{0xa000,0xa48f};
const CharRanges yi_radicals{0xa490,0xa4cf};
const CharRanges hangul_syllables{0xac00,0xd7af};
const CharRanges high_surrogates{0xd800,0xdb7f};
const CharRanges high_private_use_surrogates{0xdb80,0xdbff};
const CharRanges low_surrogates{0xdc00,0xdfff};
const CharRanges private_use_area{0xe000,0xf8ff};
const CharRanges cjk_compatibility_ideographs{0xf900,0xfaff};
const CharRanges alphabetic_presentation_forms{0xfb00,0xfb4f};
const CharRanges arabic_presentation_forms_a{0xfb50,0xfdff};
const CharRanges variation_selectors{0xfe00,0xfe0f};
const CharRanges combining_half_marks{0xfe20,0xfe2f};
const CharRanges cjk_compatibility_forms{0xfe30,0xfe4f};
const CharRanges small_form_variants{0xfe50,0xfe6f};
const CharRanges arabic_presentation_forms_b{0xfe70,0xfeff};
const CharRanges halfwidth_and_fullwidth_forms{0xff00,0xffef};
const CharRanges specials{0xfff0,0xffff};
const CharRanges linear_b_syllabary{0x10000,0x1007f};
const CharRanges linear_b_ideograms{0x10080,0x100ff};
const CharRanges aegean_numbers{0x10100,0x1013f};
const CharRanges old_italic{0x10300,0x1032f};
const CharRanges gothic{0x10330,0x1034f};
const CharRanges ugaritic{0x10380,0x1039f};
const CharRanges deseret{0x10400,0x1044f};
const CharRanges shavian{0x10450,0x1047f};
const CharRanges osmanya{0x10480,0x104af};
const CharRanges cypriot_syllabary{0x10800,0x1083f};
const CharRanges byzantine_musical_symbols{0x1d000,0x1d0ff};
const CharRanges musical_symbols{0x1d100,0x1d1ff};
const CharRanges tai_xuan_jing_symbols{0x1d300,0x1d35f};
const CharRanges mathematical_alphanumeric_symbols{0x1d400,0x1d7ff};
const CharRanges cjk_unified_ideographs_extension_b{0x20000,0x2a6df};
const CharRanges cjk_compatibility_ideographs_supplement{0x2f800,0x2fa1f};
const CharRanges tags{0xe0000,0xe007f};

#endif //BEETROOT_FONT_SHARED_H
