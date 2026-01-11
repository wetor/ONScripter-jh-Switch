/* -*- C++ -*-
 *
 *  test_utf_conversion.cpp - UTF-16/UTF-8 conversion tests for ONScripter-jh-Switch
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

//==============================================================================
// UTF-16 to UTF-8 Conversion Function (replicates coding2utf16.cpp logic)
//==============================================================================

int convUTF16ToUTF8(unsigned char dst[4], uint16_t src) {
    if (src & 0xff80) {
        if (src & 0xf800) {
            // UCS-2 = U+0800 - U+FFFF -> UTF-8 (3 bytes)
            dst[0] = 0xe0 | (src >> 12);
            dst[1] = 0x80 | ((src >> 6) & 0x3f);
            dst[2] = 0x80 | (src & 0x3f);
            dst[3] = 0;
            return 3;
        }
        // UCS-2 = U+0080 - U+07FF -> UTF-8 (2 bytes)
        dst[0] = 0xc0 | (src >> 6);
        dst[1] = 0x80 | (src & 0x3f);
        dst[2] = 0;
        return 2;
    }
    // UCS-2 = U+0000 - U+007F -> UTF-8 (1 byte)
    dst[0] = src;
    dst[1] = 0;
    return 1;
}

//==============================================================================
// UTF-8 to UTF-16 Conversion Function (replicates coding2utf16.cpp logic)
//==============================================================================

unsigned short convUTF8ToUTF16(const char** src) {
    unsigned short utf16 = 0;
    const unsigned char* p = (const unsigned char*)*src;

    if (*p & 0x80) {
        if (*p & 0x20) {
            if (*p & 0x10) {
                // 4-byte UTF-8 sequence (U+10000 - U+10FFFF)
                // Note: This only handles the BMP portion
                (*src)++;
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f)) << 12;
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f)) << 6;
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f));
            } else {
                // 3-byte UTF-8 sequence (U+0800 - U+FFFF)
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x0f)) << 12;
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f)) << 6;
                utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f));
            }
        } else {
            // 2-byte UTF-8 sequence (U+0080 - U+07FF)
            utf16 |= ((unsigned short)((*((const unsigned char*)(*src)++)) & 0x1f)) << 6;
            utf16 |= (unsigned short)((*((const unsigned char*)(*src)++)) & 0x3f);
        }
    } else {
        // 1-byte UTF-8 sequence (U+0000 - U+007F)
        utf16 |= (unsigned short)(*((const unsigned char*)(*src)++));
    }

    return utf16;
}

//==============================================================================
// Helper: Get UTF-8 byte length from leading byte
//==============================================================================

int getUTF8ByteLength(unsigned char lead) {
    if ((lead & 0x80) == 0x00) return 1;      // 0xxxxxxx
    if ((lead & 0xE0) == 0xC0) return 2;      // 110xxxxx
    if ((lead & 0xF0) == 0xE0) return 3;      // 1110xxxx
    if ((lead & 0xF8) == 0xF0) return 4;      // 11110xxx
    return 0;  // Invalid
}

//==============================================================================
// UTF-16 to UTF-8 Tests - ASCII Range
//==============================================================================

TEST_CASE(UTF16ToUTF8_ASCII_A) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x0041);  // 'A'

    ASSERT_EQ(1, len);
    ASSERT_EQ('A', dst[0]);
    ASSERT_EQ(0, dst[1]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ASCII_Zero) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x0000);

    ASSERT_EQ(1, len);
    ASSERT_EQ(0, dst[0]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ASCII_DEL) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x007F);  // DEL character

    ASSERT_EQ(1, len);
    ASSERT_EQ(0x7F, dst[0]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ASCII_AllPrintable) {
    unsigned char dst[4];

    for (uint16_t c = 0x20; c <= 0x7E; c++) {
        int len = convUTF16ToUTF8(dst, c);
        ASSERT_EQ(1, len);
        ASSERT_EQ(c, dst[0]);
    }

    return true;
}

//==============================================================================
// UTF-16 to UTF-8 Tests - 2-byte Range (U+0080 - U+07FF)
//==============================================================================

TEST_CASE(UTF16ToUTF8_TwoByte_FirstChar) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x0080);  // First 2-byte character

    ASSERT_EQ(2, len);
    ASSERT_EQ(0xC2, dst[0]);  // 110 00010
    ASSERT_EQ(0x80, dst[1]);  // 10 000000

    return true;
}

TEST_CASE(UTF16ToUTF8_TwoByte_Copyright) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x00A9);  // © Copyright symbol

    ASSERT_EQ(2, len);
    // U+00A9 = 10101001 -> 110 00010 10 101001 -> C2 A9
    ASSERT_EQ(0xC2, dst[0]);
    ASSERT_EQ(0xA9, dst[1]);

    return true;
}

TEST_CASE(UTF16ToUTF8_TwoByte_LastChar) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x07FF);  // Last 2-byte character

    ASSERT_EQ(2, len);
    // U+07FF = 11111 111111 -> 110 11111 10 111111 -> DF BF
    ASSERT_EQ(0xDF, dst[0]);
    ASSERT_EQ(0xBF, dst[1]);

    return true;
}

TEST_CASE(UTF16ToUTF8_TwoByte_LatinExtended) {
    unsigned char dst[4];

    // ñ (U+00F1) - Spanish ñ
    int len = convUTF16ToUTF8(dst, 0x00F1);
    ASSERT_EQ(2, len);
    ASSERT_EQ(0xC3, dst[0]);
    ASSERT_EQ(0xB1, dst[1]);

    return true;
}

TEST_CASE(UTF16ToUTF8_TwoByte_Greek) {
    unsigned char dst[4];

    // Ω (U+03A9) - Greek capital omega
    int len = convUTF16ToUTF8(dst, 0x03A9);
    ASSERT_EQ(2, len);
    // U+03A9 = 1110101001 -> 110 01110 10 101001 -> CE A9
    ASSERT_EQ(0xCE, dst[0]);
    ASSERT_EQ(0xA9, dst[1]);

    return true;
}

//==============================================================================
// UTF-16 to UTF-8 Tests - 3-byte Range (U+0800 - U+FFFF)
//==============================================================================

TEST_CASE(UTF16ToUTF8_ThreeByte_FirstChar) {
    unsigned char dst[4];
    int len = convUTF16ToUTF8(dst, 0x0800);  // First 3-byte character

    ASSERT_EQ(3, len);
    // U+0800 = 0000 100000 000000 -> 1110 0000 10 100000 10 000000 -> E0 A0 80
    ASSERT_EQ(0xE0, dst[0]);
    ASSERT_EQ(0xA0, dst[1]);
    ASSERT_EQ(0x80, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_ChineseChar) {
    unsigned char dst[4];

    // 中 (U+4E2D) - Chinese character for "middle"
    int len = convUTF16ToUTF8(dst, 0x4E2D);
    ASSERT_EQ(3, len);
    // U+4E2D = 0100 111000 101101 -> 1110 0100 10 111000 10 101101 -> E4 B8 AD
    ASSERT_EQ(0xE4, dst[0]);
    ASSERT_EQ(0xB8, dst[1]);
    ASSERT_EQ(0xAD, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_JapaneseHiragana) {
    unsigned char dst[4];

    // あ (U+3042) - Hiragana 'a'
    int len = convUTF16ToUTF8(dst, 0x3042);
    ASSERT_EQ(3, len);
    // U+3042 = 0011 000001 000010 -> 1110 0011 10 000001 10 000010 -> E3 81 82
    ASSERT_EQ(0xE3, dst[0]);
    ASSERT_EQ(0x81, dst[1]);
    ASSERT_EQ(0x82, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_JapaneseKatakana) {
    unsigned char dst[4];

    // ア (U+30A2) - Katakana 'a'
    int len = convUTF16ToUTF8(dst, 0x30A2);
    ASSERT_EQ(3, len);
    // E3 82 A2
    ASSERT_EQ(0xE3, dst[0]);
    ASSERT_EQ(0x82, dst[1]);
    ASSERT_EQ(0xA2, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_KoreanHangul) {
    unsigned char dst[4];

    // 한 (U+D55C) - Korean character
    int len = convUTF16ToUTF8(dst, 0xD55C);
    ASSERT_EQ(3, len);
    // ED 95 9C
    ASSERT_EQ(0xED, dst[0]);
    ASSERT_EQ(0x95, dst[1]);
    ASSERT_EQ(0x9C, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_EuroSign) {
    unsigned char dst[4];

    // € (U+20AC) - Euro sign
    int len = convUTF16ToUTF8(dst, 0x20AC);
    ASSERT_EQ(3, len);
    // E2 82 AC
    ASSERT_EQ(0xE2, dst[0]);
    ASSERT_EQ(0x82, dst[1]);
    ASSERT_EQ(0xAC, dst[2]);

    return true;
}

TEST_CASE(UTF16ToUTF8_ThreeByte_MaxBMP) {
    unsigned char dst[4];

    // U+FFFF - Last BMP character
    int len = convUTF16ToUTF8(dst, 0xFFFF);
    ASSERT_EQ(3, len);
    // EF BF BF
    ASSERT_EQ(0xEF, dst[0]);
    ASSERT_EQ(0xBF, dst[1]);
    ASSERT_EQ(0xBF, dst[2]);

    return true;
}

//==============================================================================
// UTF-8 to UTF-16 Tests - ASCII Range
//==============================================================================

TEST_CASE(UTF8ToUTF16_ASCII_A) {
    const char* str = "A";
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x0041, result);
    ASSERT_EQ(str + 1, p);  // Pointer advanced by 1

    return true;
}

TEST_CASE(UTF8ToUTF16_ASCII_Space) {
    const char* str = " ";
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x0020, result);

    return true;
}

TEST_CASE(UTF8ToUTF16_ASCII_Digits) {
    const char* str = "0123456789";
    const char* p = str;

    for (int i = 0; i < 10; i++) {
        uint16_t result = convUTF8ToUTF16(&p);
        ASSERT_EQ((uint16_t)('0' + i), result);
    }

    return true;
}

//==============================================================================
// UTF-8 to UTF-16 Tests - 2-byte Sequences
//==============================================================================

TEST_CASE(UTF8ToUTF16_TwoByte_Copyright) {
    // © = C2 A9
    const char str[] = {(char)0xC2, (char)0xA9, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x00A9, result);
    ASSERT_EQ(str + 2, p);

    return true;
}

TEST_CASE(UTF8ToUTF16_TwoByte_LatinN) {
    // ñ = C3 B1
    const char str[] = {(char)0xC3, (char)0xB1, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x00F1, result);

    return true;
}

TEST_CASE(UTF8ToUTF16_TwoByte_GreekOmega) {
    // Ω = CE A9
    const char str[] = {(char)0xCE, (char)0xA9, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x03A9, result);

    return true;
}

//==============================================================================
// UTF-8 to UTF-16 Tests - 3-byte Sequences
//==============================================================================

TEST_CASE(UTF8ToUTF16_ThreeByte_Chinese) {
    // 中 = E4 B8 AD
    const char str[] = {(char)0xE4, (char)0xB8, (char)0xAD, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x4E2D, result);
    ASSERT_EQ(str + 3, p);

    return true;
}

TEST_CASE(UTF8ToUTF16_ThreeByte_JapaneseA) {
    // あ = E3 81 82
    const char str[] = {(char)0xE3, (char)0x81, (char)0x82, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x3042, result);

    return true;
}

TEST_CASE(UTF8ToUTF16_ThreeByte_Euro) {
    // € = E2 82 AC
    const char str[] = {(char)0xE2, (char)0x82, (char)0xAC, 0};
    const char* p = str;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0x20AC, result);

    return true;
}

//==============================================================================
// Round-trip Tests (UTF-16 -> UTF-8 -> UTF-16)
//==============================================================================

TEST_CASE(UTF_RoundTrip_ASCII) {
    for (uint16_t original = 0x0001; original <= 0x007F; original++) {
        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, original);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);

        ASSERT_EQ(original, result);
    }

    return true;
}

TEST_CASE(UTF_RoundTrip_TwoByteRange) {
    // Test a sample of 2-byte range
    uint16_t test_chars[] = {0x0080, 0x00A9, 0x00F1, 0x03A9, 0x07FF};

    for (int i = 0; i < 5; i++) {
        uint16_t original = test_chars[i];
        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, original);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);

        ASSERT_EQ(original, result);
    }

    return true;
}

TEST_CASE(UTF_RoundTrip_ThreeByteRange) {
    // Test common CJK and symbol characters
    uint16_t test_chars[] = {0x0800, 0x3042, 0x30A2, 0x4E2D, 0x20AC, 0xD55C, 0xFFFF};

    for (int i = 0; i < 7; i++) {
        uint16_t original = test_chars[i];
        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, original);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);

        ASSERT_EQ(original, result);
    }

    return true;
}

TEST_CASE(UTF_RoundTrip_FullBMP_Sample) {
    // Test a sampling across the BMP range
    // Use uint32_t to avoid overflow issues with uint16_t
    for (uint32_t original = 0x0001; original <= 0xFF01; original += 0x0100) {
        // Skip surrogate range
        if (original >= 0xD800 && original <= 0xDFFF) continue;

        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, (uint16_t)original);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);

        ASSERT_EQ((uint16_t)original, result);
    }

    return true;
}

//==============================================================================
// Multi-character String Tests
//==============================================================================

TEST_CASE(UTF_MultiChar_HelloWorld) {
    // "Hello" in UTF-8
    const char* str = "Hello";
    const char* p = str;

    ASSERT_EQ((uint16_t)'H', convUTF8ToUTF16(&p));
    ASSERT_EQ((uint16_t)'e', convUTF8ToUTF16(&p));
    ASSERT_EQ((uint16_t)'l', convUTF8ToUTF16(&p));
    ASSERT_EQ((uint16_t)'l', convUTF8ToUTF16(&p));
    ASSERT_EQ((uint16_t)'o', convUTF8ToUTF16(&p));

    return true;
}

TEST_CASE(UTF_MultiChar_Japanese) {
    // "あいう" = E3 81 82, E3 81 84, E3 81 86
    const char str[] = {
        (char)0xE3, (char)0x81, (char)0x82,
        (char)0xE3, (char)0x81, (char)0x84,
        (char)0xE3, (char)0x81, (char)0x86,
        0
    };
    const char* p = str;

    ASSERT_EQ(0x3042, convUTF8ToUTF16(&p));  // あ
    ASSERT_EQ(0x3044, convUTF8ToUTF16(&p));  // い
    ASSERT_EQ(0x3046, convUTF8ToUTF16(&p));  // う

    return true;
}

TEST_CASE(UTF_MultiChar_Mixed) {
    // "Aあ" - mixed ASCII and Japanese
    const char str[] = {'A', (char)0xE3, (char)0x81, (char)0x82, 0};
    const char* p = str;

    ASSERT_EQ((uint16_t)'A', convUTF8ToUTF16(&p));
    ASSERT_EQ(0x3042, convUTF8ToUTF16(&p));

    return true;
}

//==============================================================================
// UTF-8 Byte Length Detection Tests
//==============================================================================

TEST_CASE(UTF8ByteLength_OneByte) {
    for (unsigned char c = 0x00; c <= 0x7F; c++) {
        ASSERT_EQ(1, getUTF8ByteLength(c));
    }

    return true;
}

TEST_CASE(UTF8ByteLength_TwoByte) {
    for (unsigned char c = 0xC0; c <= 0xDF; c++) {
        ASSERT_EQ(2, getUTF8ByteLength(c));
    }

    return true;
}

TEST_CASE(UTF8ByteLength_ThreeByte) {
    for (unsigned char c = 0xE0; c <= 0xEF; c++) {
        ASSERT_EQ(3, getUTF8ByteLength(c));
    }

    return true;
}

TEST_CASE(UTF8ByteLength_FourByte) {
    for (unsigned char c = 0xF0; c <= 0xF7; c++) {
        ASSERT_EQ(4, getUTF8ByteLength(c));
    }

    return true;
}

TEST_CASE(UTF8ByteLength_Invalid_ContinuationByte) {
    // Continuation bytes (0x80-0xBF) are invalid as leading bytes
    for (unsigned char c = 0x80; c <= 0xBF; c++) {
        ASSERT_EQ(0, getUTF8ByteLength(c));
    }

    return true;
}

//==============================================================================
// BOM (Byte Order Mark) Tests
//==============================================================================

TEST_CASE(UTF_BOM_UTF8) {
    // UTF-8 BOM: EF BB BF
    const char bom[] = {(char)0xEF, (char)0xBB, (char)0xBF, 0};
    const char* p = bom;
    uint16_t result = convUTF8ToUTF16(&p);

    ASSERT_EQ(0xFEFF, result);  // UTF-8 BOM decodes to U+FEFF

    return true;
}

TEST_CASE(UTF_BOM_RoundTrip) {
    unsigned char utf8[4];
    int len = convUTF16ToUTF8(utf8, 0xFEFF);

    ASSERT_EQ(3, len);
    ASSERT_EQ(0xEF, utf8[0]);
    ASSERT_EQ(0xBB, utf8[1]);
    ASSERT_EQ(0xBF, utf8[2]);

    return true;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

TEST_CASE(UTF_EdgeCase_NullChar) {
    unsigned char utf8[4];
    int len = convUTF16ToUTF8(utf8, 0x0000);

    ASSERT_EQ(1, len);
    ASSERT_EQ(0x00, utf8[0]);

    return true;
}

TEST_CASE(UTF_EdgeCase_MaxOneByteMinTwoByte) {
    unsigned char utf8[4];

    // U+007F (last 1-byte)
    int len1 = convUTF16ToUTF8(utf8, 0x007F);
    ASSERT_EQ(1, len1);

    // U+0080 (first 2-byte)
    int len2 = convUTF16ToUTF8(utf8, 0x0080);
    ASSERT_EQ(2, len2);

    return true;
}

TEST_CASE(UTF_EdgeCase_MaxTwoByteMinThreeByte) {
    unsigned char utf8[4];

    // U+07FF (last 2-byte)
    int len1 = convUTF16ToUTF8(utf8, 0x07FF);
    ASSERT_EQ(2, len1);

    // U+0800 (first 3-byte)
    int len2 = convUTF16ToUTF8(utf8, 0x0800);
    ASSERT_EQ(3, len2);

    return true;
}

//==============================================================================
// ONScripter-specific Character Tests
//==============================================================================

TEST_CASE(UTF_ONS_FullWidthSpace) {
    // Full-width space (U+3000) used in Japanese text
    unsigned char utf8[4];
    int len = convUTF16ToUTF8(utf8, 0x3000);

    ASSERT_EQ(3, len);
    // E3 80 80
    ASSERT_EQ(0xE3, utf8[0]);
    ASSERT_EQ(0x80, utf8[1]);
    ASSERT_EQ(0x80, utf8[2]);

    return true;
}

TEST_CASE(UTF_ONS_FullWidthBrackets) {
    // 「」brackets commonly used in ONScripter
    unsigned char utf8[4];

    // 「 U+300C
    int len1 = convUTF16ToUTF8(utf8, 0x300C);
    ASSERT_EQ(3, len1);

    // 」 U+300D
    int len2 = convUTF16ToUTF8(utf8, 0x300D);
    ASSERT_EQ(3, len2);

    return true;
}

TEST_CASE(UTF_ONS_FullWidthDigits) {
    // Full-width digits 0-9 (U+FF10 - U+FF19)
    for (uint16_t digit = 0xFF10; digit <= 0xFF19; digit++) {
        unsigned char utf8[4];
        int len = convUTF16ToUTF8(utf8, digit);
        ASSERT_EQ(3, len);

        // Round-trip
        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);
        ASSERT_EQ(digit, result);
    }

    return true;
}

TEST_CASE(UTF_ONS_CommonKanji) {
    // Common kanji used in visual novels
    uint16_t kanji[] = {
        0x540D,  // 名 (name)
        0x524D,  // 前 (front/before)
        0x4EBA,  // 人 (person)
        0x65E5,  // 日 (day/sun)
        0x6708,  // 月 (month/moon)
        0x5E74,  // 年 (year)
        0x6642,  // 時 (time)
        0x5206,  // 分 (minute)
        0x79D2,  // 秒 (second)
    };

    for (int i = 0; i < 9; i++) {
        unsigned char utf8[4];
        int len = convUTF16ToUTF8(utf8, kanji[i]);
        ASSERT_EQ(3, len);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);
        ASSERT_EQ(kanji[i], result);
    }

    return true;
}

//==============================================================================
// Stress Tests
//==============================================================================

TEST_CASE(UTF_Stress_ManyConversions) {
    // Convert many characters in sequence
    for (int i = 0; i < 10000; i++) {
        uint16_t original = (i % 0xFFFF) + 1;

        // Skip surrogate range
        if (original >= 0xD800 && original <= 0xDFFF) continue;

        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, original);

        const char* p = (const char*)utf8;
        uint16_t result = convUTF8ToUTF16(&p);

        ASSERT_EQ(original, result);
    }

    return true;
}

TEST_CASE(UTF_Stress_LongString) {
    // Create a long UTF-8 string with mixed content
    char buffer[1024];
    int pos = 0;

    // Add 100 Japanese characters
    for (int i = 0; i < 100 && pos < 1000; i++) {
        unsigned char utf8[4];
        convUTF16ToUTF8(utf8, 0x3042 + (i % 83));  // Hiragana range

        int len = getUTF8ByteLength(utf8[0]);
        for (int j = 0; j < len; j++) {
            buffer[pos++] = utf8[j];
        }
    }
    buffer[pos] = 0;

    // Parse the string back
    const char* p = buffer;
    int count = 0;
    while (*p) {
        uint16_t result = convUTF8ToUTF16(&p);
        ASSERT_GE(result, 0x3042);
        ASSERT_LT(result, 0x3042 + 83);
        count++;
    }

    ASSERT_EQ(100, count);

    return true;
}
