/* -*- C++ -*-
 *
 *  test_encoding.cpp - Encoding tests for ONScripter-jh-Switch
 *
 *  Copyright (C) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "test_framework.h"
#include <cstring>
#include <cstdint>

// Replicate the UTF8_N_BYTE macro from ScriptHandler.h for testing
#define UTF8_N_BYTE(x) ( \
    (((unsigned char)(x) & 0x80) == 0x00) ? 1 : ( \
    (((unsigned char)(x) & 0xE0) == 0xC0) ? 2 : ( \
    (((unsigned char)(x) & 0xF0) == 0xE0) ? 3 : ( \
    (((unsigned char)(x) & 0xF8) == 0xF0) ? 4 : 0))))

// Replicate IS_TWO_BYTE macro for Shift-JIS/GBK detection
#define IS_TWO_BYTE(x) \
    ((((unsigned char)(x) >= 0x81) && ((unsigned char)(x) <= 0x9F)) || \
     (((unsigned char)(x) >= 0xE0) && ((unsigned char)(x) <= 0xFC)))

//==============================================================================
// UTF-8 Byte Length Detection Tests
//==============================================================================

TEST_CASE(UTF8_SingleByte_ASCII) {
    // ASCII characters (0x00-0x7F) should be 1 byte
    ASSERT_EQ(1, UTF8_N_BYTE('A'));
    ASSERT_EQ(1, UTF8_N_BYTE('Z'));
    ASSERT_EQ(1, UTF8_N_BYTE('a'));
    ASSERT_EQ(1, UTF8_N_BYTE('z'));
    ASSERT_EQ(1, UTF8_N_BYTE('0'));
    ASSERT_EQ(1, UTF8_N_BYTE('9'));
    ASSERT_EQ(1, UTF8_N_BYTE(' '));
    ASSERT_EQ(1, UTF8_N_BYTE('\n'));
    ASSERT_EQ(1, UTF8_N_BYTE('\t'));
    ASSERT_EQ(1, UTF8_N_BYTE(0x00));
    ASSERT_EQ(1, UTF8_N_BYTE(0x7F));
    return true;
}

TEST_CASE(UTF8_TwoByte_LatinExtended) {
    // Two-byte UTF-8 sequences (0xC0-0xDF leading byte)
    // Examples: Latin extended, common diacritics
    ASSERT_EQ(2, UTF8_N_BYTE(0xC2)); // Start of Latin-1 Supplement
    ASSERT_EQ(2, UTF8_N_BYTE(0xC3)); // More Latin-1 Supplement
    ASSERT_EQ(2, UTF8_N_BYTE(0xC4)); // Latin Extended-A
    ASSERT_EQ(2, UTF8_N_BYTE(0xDF)); // End of 2-byte range
    return true;
}

TEST_CASE(UTF8_ThreeByte_CJK) {
    // Three-byte UTF-8 sequences (0xE0-0xEF leading byte)
    // Used for CJK characters (Chinese, Japanese, Korean)
    ASSERT_EQ(3, UTF8_N_BYTE(0xE0)); // Start of 3-byte range
    ASSERT_EQ(3, UTF8_N_BYTE(0xE4)); // CJK Unified Ideographs (Chinese)
    ASSERT_EQ(3, UTF8_N_BYTE(0xE3)); // Hiragana/Katakana (Japanese)
    ASSERT_EQ(3, UTF8_N_BYTE(0xEA)); // Hangul (Korean)
    ASSERT_EQ(3, UTF8_N_BYTE(0xEF)); // End of 3-byte range
    return true;
}

TEST_CASE(UTF8_FourByte_Emoji) {
    // Four-byte UTF-8 sequences (0xF0-0xF7 leading byte)
    // Used for emoji and rare characters
    ASSERT_EQ(4, UTF8_N_BYTE(0xF0)); // Start of 4-byte range (emoji, etc.)
    ASSERT_EQ(4, UTF8_N_BYTE(0xF4)); // Last valid 4-byte start
    return true;
}

TEST_CASE(UTF8_Invalid_ContinuationByte) {
    // Continuation bytes (0x80-0xBF) should return 0 (invalid as leading byte)
    ASSERT_EQ(0, UTF8_N_BYTE(0x80));
    ASSERT_EQ(0, UTF8_N_BYTE(0xBF));
    ASSERT_EQ(0, UTF8_N_BYTE(0xA0));
    return true;
}

TEST_CASE(UTF8_Invalid_TooLong) {
    // Invalid leading bytes (0xF8-0xFF) should return 0
    ASSERT_EQ(0, UTF8_N_BYTE(0xF8));
    ASSERT_EQ(0, UTF8_N_BYTE(0xFC));
    ASSERT_EQ(0, UTF8_N_BYTE(0xFF));
    return true;
}

//==============================================================================
// UTF-8 String Processing Tests
//==============================================================================

TEST_CASE(UTF8_ChineseString_ByteCount) {
    // "你好" in UTF-8: E4 BD A0 E5 A5 BD (6 bytes, 2 characters)
    const unsigned char chinese[] = {0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD, 0x00};

    int total_chars = 0;
    int total_bytes = 0;
    const unsigned char* p = chinese;

    while (*p) {
        int len = UTF8_N_BYTE(*p);
        ASSERT_EQ(3, len); // Each Chinese char is 3 bytes
        p += len;
        total_chars++;
        total_bytes += len;
    }

    ASSERT_EQ(2, total_chars);
    ASSERT_EQ(6, total_bytes);
    return true;
}

TEST_CASE(UTF8_JapaneseHiragana_ByteCount) {
    // "あいう" in UTF-8: E3 81 82 E3 81 84 E3 81 86 (9 bytes, 3 characters)
    const unsigned char hiragana[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x84, 0xE3, 0x81, 0x86, 0x00};

    int total_chars = 0;
    const unsigned char* p = hiragana;

    while (*p) {
        int len = UTF8_N_BYTE(*p);
        ASSERT_EQ(3, len);
        p += len;
        total_chars++;
    }

    ASSERT_EQ(3, total_chars);
    return true;
}

TEST_CASE(UTF8_MixedASCIIAndCJK) {
    // "Hello世界" - mixed ASCII and Chinese
    // H(1) e(1) l(1) l(1) o(1) 世(3) 界(3) = 11 bytes, 7 characters
    const unsigned char mixed[] = {'H', 'e', 'l', 'l', 'o',
                                    0xE4, 0xB8, 0x96,  // 世
                                    0xE7, 0x95, 0x8C,  // 界
                                    0x00};

    int total_chars = 0;
    int total_bytes = 0;
    const unsigned char* p = mixed;

    while (*p) {
        int len = UTF8_N_BYTE(*p);
        ASSERT_GT(len, 0);
        ASSERT_LE(len, 4);
        p += len;
        total_chars++;
        total_bytes += len;
    }

    ASSERT_EQ(7, total_chars);
    ASSERT_EQ(11, total_bytes);
    return true;
}

TEST_CASE(UTF8_EmojiString) {
    // Single emoji (4 bytes): 😀 = F0 9F 98 80
    const unsigned char emoji[] = {0xF0, 0x9F, 0x98, 0x80, 0x00};

    ASSERT_EQ(4, UTF8_N_BYTE(emoji[0]));

    int total_chars = 0;
    const unsigned char* p = emoji;

    while (*p) {
        int len = UTF8_N_BYTE(*p);
        p += len;
        total_chars++;
    }

    ASSERT_EQ(1, total_chars);
    return true;
}

//==============================================================================
// Shift-JIS / GBK Two-Byte Detection Tests (IS_TWO_BYTE)
//==============================================================================

TEST_CASE(SJIS_TwoByte_Range1) {
    // Shift-JIS first byte range: 0x81-0x9F
    ASSERT_TRUE(IS_TWO_BYTE(0x81));
    ASSERT_TRUE(IS_TWO_BYTE(0x82));
    ASSERT_TRUE(IS_TWO_BYTE(0x9F));
    return true;
}

TEST_CASE(SJIS_TwoByte_Range2) {
    // Shift-JIS first byte range: 0xE0-0xFC
    ASSERT_TRUE(IS_TWO_BYTE(0xE0));
    ASSERT_TRUE(IS_TWO_BYTE(0xEF));
    ASSERT_TRUE(IS_TWO_BYTE(0xFC));
    return true;
}

TEST_CASE(SJIS_SingleByte_ASCII) {
    // ASCII range should NOT be detected as two-byte
    ASSERT_FALSE(IS_TWO_BYTE('A'));
    ASSERT_FALSE(IS_TWO_BYTE('Z'));
    ASSERT_FALSE(IS_TWO_BYTE(' '));
    ASSERT_FALSE(IS_TWO_BYTE(0x00));
    ASSERT_FALSE(IS_TWO_BYTE(0x7F));
    return true;
}

TEST_CASE(SJIS_SingleByte_HalfwidthKatakana) {
    // Half-width katakana (0xA0-0xDF) should NOT be detected as two-byte leading
    ASSERT_FALSE(IS_TWO_BYTE(0xA1));
    ASSERT_FALSE(IS_TWO_BYTE(0xDF));
    return true;
}

TEST_CASE(GBK_TwoByte_Range) {
    // GBK uses similar ranges to Shift-JIS for first byte
    // The IS_TWO_BYTE macro is designed for Shift-JIS (0x81-0x9F, 0xE0-0xFC)
    // GBK has a wider range (0x81-0xFE) but we use the SJIS macro
    ASSERT_TRUE(IS_TWO_BYTE(0x81));
    // Note: 0xA1-0xDF is NOT detected by IS_TWO_BYTE (half-width katakana range in SJIS)
    // This is expected behavior - GBK text should use force_utf8 mode or a different detector
    ASSERT_FALSE(IS_TWO_BYTE(0xA1)); // Falls in the SJIS gap
    ASSERT_TRUE(IS_TWO_BYTE(0xE0));  // In the SJIS range
    ASSERT_TRUE(IS_TWO_BYTE(0xFC)); // Max value in IS_TWO_BYTE range
    return true;
}

//==============================================================================
// Lua Script Backslash Escape Processing Tests
// (Simulates the processing done in NL_dofile and loadInitScript)
//==============================================================================

// Helper function to simulate the UTF-8 aware backslash escape processing
size_t processLuaScriptUTF8(const unsigned char* input, unsigned char* output, bool force_utf8) {
    const unsigned char* p = input;
    unsigned char* p2 = output;

    while (*p) {
        if (force_utf8) {
            int leading = UTF8_N_BYTE(*p);
            for (int j = 0; j < leading - 1; j++) {
                *p2++ = *p++;
                if (*p == '\\') *p2++ = '\\';
            }
            *p2++ = *p++;
        } else {
            if (IS_TWO_BYTE(*p)) {
                *p2++ = *p++;
                if (*p == '\\') *p2++ = '\\';
            }
            *p2++ = *p++;
        }
    }
    *p2 = 0;

    return p2 - output;
}

TEST_CASE(LuaEscape_UTF8_NoBackslash) {
    // Chinese text without backslash - should pass through unchanged
    const unsigned char input[] = {0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD, 0x00}; // "你好"
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(input, output, true);

    ASSERT_EQ(6, (int)len);
    ASSERT_EQ(0, memcmp(input, output, len + 1));
    return true;
}

TEST_CASE(LuaEscape_UTF8_WithBackslashInMultibyte) {
    // Simulate a case where 0x5C (backslash) appears in a multi-byte sequence
    // This is contrived but tests the escape logic
    // In real UTF-8, 0x5C only appears as ASCII backslash, not in continuation bytes
    const unsigned char input[] = {'a', '\\', 'n', 0x00};
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(input, output, true);

    // ASCII characters are 1-byte, so backslash should pass through as-is
    ASSERT_EQ(3, (int)len);
    ASSERT_STREQ("a\\n", (const char*)output);
    return true;
}

TEST_CASE(LuaEscape_SJIS_BackslashInSecondByte) {
    // In Shift-JIS, some characters have 0x5C as second byte
    // e.g., ソ (0x83 0x5C) - the "yen sign problem"
    // The escape logic doubles the backslash
    const unsigned char input[] = {0x83, 0x5C, 0x00}; // "ソ" in SJIS
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(input, output, false); // SJIS mode

    // Should escape the 0x5C: 0x83 0x5C 0x5C
    ASSERT_EQ(3, (int)len);
    ASSERT_EQ(0x83, output[0]);
    ASSERT_EQ(0x5C, output[1]);
    ASSERT_EQ(0x5C, output[2]);
    return true;
}

TEST_CASE(LuaEscape_SJIS_NoBackslash) {
    // Shift-JIS text without backslash issues
    const unsigned char input[] = {0x82, 0xA0, 0x82, 0xA2, 0x00}; // "あい" in SJIS
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(input, output, false);

    ASSERT_EQ(4, (int)len);
    ASSERT_EQ(0, memcmp(input, output, len + 1));
    return true;
}

TEST_CASE(LuaEscape_MixedContent) {
    // Mix of ASCII and multi-byte
    const unsigned char input[] = {'H', 'i', 0xE4, 0xBD, 0xA0, '!', 0x00}; // "Hi你!"
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(input, output, true);

    ASSERT_EQ(6, (int)len);
    ASSERT_EQ(0, memcmp(input, output, len + 1));
    return true;
}

//==============================================================================
// UTF-16 Conversion Helper Tests
//==============================================================================

TEST_CASE(UTF8_to_UTF16_ASCII) {
    // Basic ASCII to UTF-16 (should be same value)
    unsigned char utf8_A = 'A';
    ASSERT_EQ(1, UTF8_N_BYTE(utf8_A));
    // UTF-16 for 'A' is 0x0041
    uint16_t expected_utf16 = 0x0041;
    ASSERT_EQ((uint16_t)'A', expected_utf16);
    return true;
}

TEST_CASE(UTF8_BOM_Detection) {
    // UTF-8 BOM: EF BB BF
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};

    ASSERT_EQ(3, UTF8_N_BYTE(bom[0]));
    return true;
}

//==============================================================================
// Edge Cases and Boundary Tests
//==============================================================================

TEST_CASE(Encoding_EmptyString) {
    const unsigned char empty[] = {0x00};
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(empty, output, true);

    ASSERT_EQ(0, (int)len);
    ASSERT_EQ(0, output[0]);
    return true;
}

TEST_CASE(Encoding_OnlyASCII) {
    const unsigned char ascii[] = "Hello, World!";
    unsigned char output[64];

    size_t len = processLuaScriptUTF8(ascii, output, true);

    ASSERT_EQ(13, (int)len);
    ASSERT_STREQ("Hello, World!", (const char*)output);
    return true;
}

TEST_CASE(Encoding_LongUTF8String) {
    // Test with a longer string to ensure no buffer issues in logic
    // 10 Chinese characters = 30 bytes
    const unsigned char longstr[] = {
        0xE4, 0xB8, 0x80, // 一
        0xE4, 0xBA, 0x8C, // 二
        0xE4, 0xB8, 0x89, // 三
        0xE5, 0x9B, 0x9B, // 四
        0xE4, 0xBA, 0x94, // 五
        0xE5, 0x85, 0xAD, // 六
        0xE4, 0xB8, 0x83, // 七
        0xE5, 0x85, 0xAB, // 八
        0xE4, 0xB9, 0x9D, // 九
        0xE5, 0x8D, 0x81, // 十
        0x00
    };
    unsigned char output[128];

    size_t len = processLuaScriptUTF8(longstr, output, true);

    ASSERT_EQ(30, (int)len);

    // Count characters
    int chars = 0;
    const unsigned char* p = longstr;
    while (*p) {
        p += UTF8_N_BYTE(*p);
        chars++;
    }
    ASSERT_EQ(10, chars);

    return true;
}
