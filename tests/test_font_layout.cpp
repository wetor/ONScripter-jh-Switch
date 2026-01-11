/* -*- C++ -*-
 *
 *  test_font_layout.cpp - Font layout and positioning tests for ONScripter-jh-Switch
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
// Mock FontInfo class (replicates core logic from FontInfo.cpp)
//==============================================================================

class MockFontInfo {
public:
    // Display modes
    static const int YOKO_MODE = 0;  // Horizontal (left-to-right)
    static const int TATE_MODE = 1;  // Vertical (top-to-bottom)

    int tateyoko_mode;

    // Position (in half-width character units)
    int xy[2];           // Current position [x, y]
    int num_xy[2];       // Number of characters [cols, rows]
    int top_xy[2];       // Top-left position in pixels
    int pitch_xy[2];     // Character pitch (spacing) in pixels
    int font_size_xy[2]; // Font size in pixels [width, height]
    int line_offset_xy[2]; // Line offset for ruby text etc.

    // Color settings
    unsigned char color[3];
    unsigned char on_color[3];
    unsigned char off_color[3];

    // Flags
    bool rubyon_flag;
    bool is_bold;
    bool is_shadow;
    bool is_transparent;
    bool is_newline_accepted;

    MockFontInfo() {
        tateyoko_mode = YOKO_MODE;

        xy[0] = xy[1] = 0;
        num_xy[0] = 23;  // Default columns
        num_xy[1] = 16;  // Default rows
        top_xy[0] = 8;
        top_xy[1] = 16;
        pitch_xy[0] = 26;
        pitch_xy[1] = 26;
        font_size_xy[0] = 26;
        font_size_xy[1] = 26;
        line_offset_xy[0] = line_offset_xy[1] = 0;

        color[0] = color[1] = color[2] = 0xff;
        on_color[0] = on_color[1] = on_color[2] = 0xff;
        off_color[0] = 0xaa;
        off_color[1] = 0xaa;
        off_color[2] = 0xaa;

        rubyon_flag = false;
        is_bold = true;
        is_shadow = true;
        is_transparent = true;
        is_newline_accepted = false;
    }

    void reset() {
        tateyoko_mode = YOKO_MODE;
        clear();
        is_bold = true;
        is_shadow = true;
        is_transparent = true;
        is_newline_accepted = false;
    }

    void setTateyokoMode(int mode) {
        tateyoko_mode = mode;
        clear();
    }

    int getTateyokoMode() const {
        return tateyoko_mode;
    }

    int getRemainingLine() const {
        if (tateyoko_mode == YOKO_MODE)
            return num_xy[1] - xy[1] / 2;
        else
            return num_xy[1] - num_xy[0] + xy[0] / 2 + 1;
    }

    int x(bool use_ruby_offset = true) const {
        int x_val = xy[0] * pitch_xy[0] / 2 + top_xy[0] + line_offset_xy[0];
        if (use_ruby_offset && rubyon_flag && tateyoko_mode == TATE_MODE)
            x_val += font_size_xy[0] - pitch_xy[0];
        return x_val;
    }

    int y(bool use_ruby_offset = true) const {
        int y_val = xy[1] * pitch_xy[1] / 2 + top_xy[1] + line_offset_xy[1];
        if (use_ruby_offset && rubyon_flag && tateyoko_mode == YOKO_MODE)
            y_val += pitch_xy[1] - font_size_xy[1];
        return y_val;
    }

    void setXY(int x_val, int y_val) {
        if (x_val != -1)
            xy[0] = x_val * 2;
        if (y_val != -1)
            xy[1] = y_val * 2;
    }

    void clear() {
        if (tateyoko_mode == YOKO_MODE)
            setXY(0, 0);
        else
            setXY(num_xy[0] - 1, 0);
        line_offset_xy[0] = line_offset_xy[1] = 0;
    }

    void newLine() {
        if (tateyoko_mode == YOKO_MODE) {
            xy[0] = 0;
            xy[1] += 2;
        } else {
            xy[0] -= 2;
            xy[1] = 0;
        }
        line_offset_xy[0] = line_offset_xy[1] = 0;
    }

    void setLineArea(int num) {
        num_xy[tateyoko_mode] = num;
        num_xy[1 - tateyoko_mode] = 1;
    }

    bool isEndOfLine(int margin = 0) const {
        if (xy[tateyoko_mode] + margin >= num_xy[tateyoko_mode] * 2)
            return true;
        return false;
    }

    bool isLineEmpty() const {
        if (xy[tateyoko_mode] == 0)
            return true;
        return false;
    }

    void advanceCharInHankaku(int offset) {
        xy[tateyoko_mode] += offset;
    }

    void addLineOffset(int offset) {
        line_offset_xy[tateyoko_mode] += offset;
    }

    // Calculate pixel position for character at given grid position
    int getPixelX(int grid_x) const {
        return grid_x * pitch_xy[0] / 2 + top_xy[0];
    }

    int getPixelY(int grid_y) const {
        return grid_y * pitch_xy[1] / 2 + top_xy[1];
    }
};

//==============================================================================
// Basic Initialization Tests
//==============================================================================

TEST_CASE(Font_DefaultState) {
    MockFontInfo font;

    ASSERT_EQ(MockFontInfo::YOKO_MODE, font.tateyoko_mode);
    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);
    ASSERT_TRUE(font.is_bold);
    ASSERT_TRUE(font.is_shadow);
    ASSERT_TRUE(font.is_transparent);

    return true;
}

TEST_CASE(Font_DefaultDimensions) {
    MockFontInfo font;

    ASSERT_EQ(23, font.num_xy[0]);  // Columns
    ASSERT_EQ(16, font.num_xy[1]);  // Rows
    ASSERT_EQ(26, font.font_size_xy[0]);
    ASSERT_EQ(26, font.font_size_xy[1]);
    ASSERT_EQ(26, font.pitch_xy[0]);
    ASSERT_EQ(26, font.pitch_xy[1]);

    return true;
}

TEST_CASE(Font_Reset) {
    MockFontInfo font;

    font.xy[0] = 10;
    font.xy[1] = 5;
    font.is_bold = false;

    font.reset();

    ASSERT_EQ(MockFontInfo::YOKO_MODE, font.tateyoko_mode);
    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);
    ASSERT_TRUE(font.is_bold);

    return true;
}

//==============================================================================
// Horizontal (Yoko) Mode Tests
//==============================================================================

TEST_CASE(Font_Yoko_InitialPosition) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);

    return true;
}

TEST_CASE(Font_Yoko_AdvanceChar) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    font.advanceCharInHankaku(2);  // One full-width character
    ASSERT_EQ(2, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);

    font.advanceCharInHankaku(1);  // Half-width character
    ASSERT_EQ(3, font.xy[0]);

    return true;
}

TEST_CASE(Font_Yoko_NewLine) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    font.advanceCharInHankaku(10);
    font.newLine();

    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(2, font.xy[1]);

    font.newLine();
    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(4, font.xy[1]);

    return true;
}

TEST_CASE(Font_Yoko_EndOfLine) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);
    font.num_xy[0] = 10;  // 10 columns

    // At position 0, not end of line
    ASSERT_FALSE(font.isEndOfLine());

    // Move to position 9 (half of 18)
    font.xy[0] = 18;
    ASSERT_FALSE(font.isEndOfLine());

    // Move to position 10 (half of 20) - at end
    font.xy[0] = 20;
    ASSERT_TRUE(font.isEndOfLine());

    return true;
}

TEST_CASE(Font_Yoko_EndOfLineWithMargin) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);
    font.num_xy[0] = 10;

    font.xy[0] = 16;  // Position 8

    // Without margin, not at end
    ASSERT_FALSE(font.isEndOfLine(0));

    // With margin of 4, would be at end (8 + 4 = 12 >= 10*2)
    ASSERT_TRUE(font.isEndOfLine(4));

    return true;
}

TEST_CASE(Font_Yoko_LineEmpty) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    ASSERT_TRUE(font.isLineEmpty());

    font.advanceCharInHankaku(1);
    ASSERT_FALSE(font.isLineEmpty());

    return true;
}

TEST_CASE(Font_Yoko_RemainingLines) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);
    font.num_xy[1] = 16;  // 16 rows

    // At row 0
    ASSERT_EQ(16, font.getRemainingLine());

    // Move to row 5 (xy[1] = 10)
    font.xy[1] = 10;
    ASSERT_EQ(11, font.getRemainingLine());  // 16 - 10/2 = 11

    // Move to last row
    font.xy[1] = 30;
    ASSERT_EQ(1, font.getRemainingLine());

    return true;
}

//==============================================================================
// Vertical (Tate) Mode Tests
//==============================================================================

TEST_CASE(Font_Tate_InitialPosition) {
    MockFontInfo font;
    font.num_xy[0] = 10;  // Set columns before switching mode
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    // In tate mode, starts at rightmost column
    ASSERT_EQ((10 - 1) * 2, font.xy[0]);  // (num_xy[0] - 1) * 2
    ASSERT_EQ(0, font.xy[1]);

    return true;
}

TEST_CASE(Font_Tate_AdvanceChar) {
    MockFontInfo font;
    font.num_xy[0] = 10;
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    int initial_y = font.xy[1];
    font.advanceCharInHankaku(2);

    // In tate mode, advancing moves down (increases y)
    ASSERT_EQ(initial_y + 2, font.xy[1]);

    return true;
}

TEST_CASE(Font_Tate_NewLine) {
    MockFontInfo font;
    font.num_xy[0] = 10;
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    int initial_x = font.xy[0];  // Should be 18 (rightmost)
    font.advanceCharInHankaku(4);
    font.newLine();

    // In tate mode, new line moves left (decreases x)
    ASSERT_EQ(initial_x - 2, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);  // y resets to 0

    return true;
}

TEST_CASE(Font_Tate_EndOfLine) {
    MockFontInfo font;
    font.num_xy[1] = 20;  // 20 rows (vertical length)
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    // At position 0, not end of line
    ASSERT_FALSE(font.isEndOfLine());

    // Move near end
    font.xy[1] = 38;
    ASSERT_FALSE(font.isEndOfLine());

    // Move to end
    font.xy[1] = 40;
    ASSERT_TRUE(font.isEndOfLine());

    return true;
}

//==============================================================================
// Pixel Position Calculation Tests
//==============================================================================

TEST_CASE(Font_PixelPosition_Origin) {
    MockFontInfo font;

    // At grid position (0, 0)
    int px = font.x();
    int py = font.y();

    ASSERT_EQ(font.top_xy[0], px);
    ASSERT_EQ(font.top_xy[1], py);

    return true;
}

TEST_CASE(Font_PixelPosition_Offset) {
    MockFontInfo font;

    // Move to grid position (2, 3)
    font.setXY(2, 3);

    int px = font.x();
    int py = font.y();

    // Expected: top_xy + xy * pitch / 2
    int expected_x = font.top_xy[0] + 4 * font.pitch_xy[0] / 2;  // xy[0] = 2*2 = 4
    int expected_y = font.top_xy[1] + 6 * font.pitch_xy[1] / 2;  // xy[1] = 3*2 = 6

    ASSERT_EQ(expected_x, px);
    ASSERT_EQ(expected_y, py);

    return true;
}

TEST_CASE(Font_PixelPosition_LineOffset) {
    MockFontInfo font;

    font.addLineOffset(10);

    int px = font.x();

    // In yoko mode, line offset affects x
    ASSERT_EQ(font.top_xy[0] + 10, px);

    return true;
}

TEST_CASE(Font_PixelPosition_Helper) {
    MockFontInfo font;

    // Test helper functions
    int px0 = font.getPixelX(0);
    int py0 = font.getPixelY(0);

    ASSERT_EQ(font.top_xy[0], px0);
    ASSERT_EQ(font.top_xy[1], py0);

    int px4 = font.getPixelX(4);
    int py4 = font.getPixelY(4);

    ASSERT_EQ(font.top_xy[0] + 2 * font.pitch_xy[0], px4);
    ASSERT_EQ(font.top_xy[1] + 2 * font.pitch_xy[1], py4);

    return true;
}

//==============================================================================
// SetXY Tests
//==============================================================================

TEST_CASE(Font_SetXY_Both) {
    MockFontInfo font;

    font.setXY(5, 10);

    ASSERT_EQ(10, font.xy[0]);  // 5 * 2
    ASSERT_EQ(20, font.xy[1]);  // 10 * 2

    return true;
}

TEST_CASE(Font_SetXY_OnlyX) {
    MockFontInfo font;

    font.xy[1] = 8;
    font.setXY(5, -1);  // -1 means don't change

    ASSERT_EQ(10, font.xy[0]);
    ASSERT_EQ(8, font.xy[1]);  // Unchanged

    return true;
}

TEST_CASE(Font_SetXY_OnlyY) {
    MockFontInfo font;

    font.xy[0] = 6;
    font.setXY(-1, 10);

    ASSERT_EQ(6, font.xy[0]);  // Unchanged
    ASSERT_EQ(20, font.xy[1]);

    return true;
}

TEST_CASE(Font_SetXY_Zero) {
    MockFontInfo font;

    font.xy[0] = 10;
    font.xy[1] = 10;
    font.setXY(0, 0);

    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);

    return true;
}

//==============================================================================
// Line Area Tests
//==============================================================================

TEST_CASE(Font_SetLineArea_Yoko) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    font.setLineArea(30);

    ASSERT_EQ(30, font.num_xy[0]);  // Columns set
    ASSERT_EQ(1, font.num_xy[1]);   // Rows set to 1

    return true;
}

TEST_CASE(Font_SetLineArea_Tate) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    font.setLineArea(30);

    ASSERT_EQ(1, font.num_xy[0]);   // Columns set to 1
    ASSERT_EQ(30, font.num_xy[1]);  // Rows set

    return true;
}

//==============================================================================
// Ruby Text Tests (Furigana)
//==============================================================================

TEST_CASE(Font_Ruby_YokoMode) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);
    font.rubyon_flag = true;

    // In yoko mode with ruby, y position should be adjusted
    int y_no_ruby = font.y(false);
    int y_with_ruby = font.y(true);

    // With ruby on, y should be offset by (pitch - font_size)
    int expected_offset = font.pitch_xy[1] - font.font_size_xy[1];
    ASSERT_EQ(y_no_ruby + expected_offset, y_with_ruby);

    return true;
}

TEST_CASE(Font_Ruby_TateMode) {
    MockFontInfo font;
    font.num_xy[0] = 10;
    font.setTateyokoMode(MockFontInfo::TATE_MODE);
    font.rubyon_flag = true;

    // In tate mode with ruby, x position should be adjusted
    int x_no_ruby = font.x(false);
    int x_with_ruby = font.x(true);

    // With ruby on, x should be offset
    int expected_offset = font.font_size_xy[0] - font.pitch_xy[0];
    ASSERT_EQ(x_no_ruby + expected_offset, x_with_ruby);

    return true;
}

TEST_CASE(Font_Ruby_Disabled) {
    MockFontInfo font;
    font.rubyon_flag = false;

    int x1 = font.x(true);
    int x2 = font.x(false);

    // Without ruby, both should be the same
    ASSERT_EQ(x1, x2);

    return true;
}

//==============================================================================
// Color Tests
//==============================================================================

TEST_CASE(Font_Color_Default) {
    MockFontInfo font;

    // Default color is white
    ASSERT_EQ(0xff, font.color[0]);
    ASSERT_EQ(0xff, font.color[1]);
    ASSERT_EQ(0xff, font.color[2]);

    return true;
}

TEST_CASE(Font_Color_OnColor) {
    MockFontInfo font;

    // On color (selected/active)
    ASSERT_EQ(0xff, font.on_color[0]);
    ASSERT_EQ(0xff, font.on_color[1]);
    ASSERT_EQ(0xff, font.on_color[2]);

    return true;
}

TEST_CASE(Font_Color_OffColor) {
    MockFontInfo font;

    // Off color (unselected/inactive)
    ASSERT_EQ(0xaa, font.off_color[0]);
    ASSERT_EQ(0xaa, font.off_color[1]);
    ASSERT_EQ(0xaa, font.off_color[2]);

    return true;
}

TEST_CASE(Font_Color_Set) {
    MockFontInfo font;

    font.color[0] = 0x12;
    font.color[1] = 0x34;
    font.color[2] = 0x56;

    ASSERT_EQ(0x12, font.color[0]);
    ASSERT_EQ(0x34, font.color[1]);
    ASSERT_EQ(0x56, font.color[2]);

    return true;
}

//==============================================================================
// Character Width Tests
//==============================================================================

TEST_CASE(Font_CharWidth_FullWidth) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    int start_x = font.xy[0];
    font.advanceCharInHankaku(2);  // Full-width = 2 hankaku

    ASSERT_EQ(start_x + 2, font.xy[0]);

    return true;
}

TEST_CASE(Font_CharWidth_HalfWidth) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    int start_x = font.xy[0];
    font.advanceCharInHankaku(1);  // Half-width = 1 hankaku

    ASSERT_EQ(start_x + 1, font.xy[0]);

    return true;
}

TEST_CASE(Font_CharWidth_MixedText) {
    MockFontInfo font;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    // Simulate: "Aあ" (ASCII + Hiragana)
    font.advanceCharInHankaku(1);  // 'A' - half-width
    font.advanceCharInHankaku(2);  // 'あ' - full-width

    ASSERT_EQ(3, font.xy[0]);

    return true;
}

//==============================================================================
// Text Layout Boundary Tests
//==============================================================================

TEST_CASE(Font_Layout_TextBoxBounds) {
    MockFontInfo font;

    // Set up a text box: 600x400 starting at (20, 300)
    font.top_xy[0] = 20;
    font.top_xy[1] = 300;
    font.num_xy[0] = 22;  // Characters per line
    font.num_xy[1] = 8;   // Lines

    // First character position
    ASSERT_EQ(20, font.x());
    ASSERT_EQ(300, font.y());

    // Move to end of first line
    font.xy[0] = 44;  // 22 * 2
    int end_x = font.x();
    ASSERT_TRUE(font.isEndOfLine());

    return true;
}

TEST_CASE(Font_Layout_WordWrap) {
    MockFontInfo font;
    font.num_xy[0] = 10;  // 10 chars per line

    // Simulate typing until we need to wrap
    for (int i = 0; i < 9; i++) {
        font.advanceCharInHankaku(2);  // Full-width chars
    }

    // After 9 full-width chars (18 hankaku), check if near end
    ASSERT_TRUE(font.isEndOfLine(2));  // Can't fit another full-width

    // Wrap to new line
    font.newLine();
    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(2, font.xy[1]);

    return true;
}

//==============================================================================
// Multi-line Text Tests
//==============================================================================

TEST_CASE(Font_MultiLine_Sequential) {
    MockFontInfo font;
    font.num_xy[0] = 5;
    font.num_xy[1] = 10;

    // Line 1
    font.advanceCharInHankaku(8);
    ASSERT_EQ(8, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);

    // Line 2
    font.newLine();
    font.advanceCharInHankaku(4);
    ASSERT_EQ(4, font.xy[0]);
    ASSERT_EQ(2, font.xy[1]);

    // Line 3
    font.newLine();
    font.advanceCharInHankaku(6);
    ASSERT_EQ(6, font.xy[0]);
    ASSERT_EQ(4, font.xy[1]);

    return true;
}

TEST_CASE(Font_MultiLine_Clear) {
    MockFontInfo font;

    font.advanceCharInHankaku(10);
    font.newLine();
    font.advanceCharInHankaku(5);

    ASSERT_NE(0, font.xy[0]);
    ASSERT_NE(0, font.xy[1]);

    font.clear();

    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);

    return true;
}

//==============================================================================
// Switch-specific Resolution Tests
//==============================================================================

TEST_CASE(Font_Switch_DockedLayout) {
    MockFontInfo font;

    // Typical layout for 1920x1080 docked mode
    // Assuming 3x scale from 640x480
    font.top_xy[0] = 24;    // 8 * 3
    font.top_xy[1] = 48;    // 16 * 3
    font.pitch_xy[0] = 78;  // 26 * 3
    font.pitch_xy[1] = 78;
    font.font_size_xy[0] = 78;
    font.font_size_xy[1] = 78;
    font.num_xy[0] = 23;
    font.num_xy[1] = 12;

    // Text should fit within 1920 width
    font.xy[0] = 46;  // At end of line (23 * 2)
    int end_x = font.x();
    ASSERT_LT(end_x, 1920);

    return true;
}

TEST_CASE(Font_Switch_HandheldLayout) {
    MockFontInfo font;

    // Typical layout for 1280x720 handheld mode
    // Assuming 2x scale from 640x480
    font.top_xy[0] = 16;    // 8 * 2
    font.top_xy[1] = 32;    // 16 * 2
    font.pitch_xy[0] = 52;  // 26 * 2
    font.pitch_xy[1] = 52;
    font.font_size_xy[0] = 52;
    font.font_size_xy[1] = 52;
    font.num_xy[0] = 23;
    font.num_xy[1] = 12;

    // Text should fit within 1280 width
    font.xy[0] = 46;
    int end_x = font.x();
    ASSERT_LT(end_x, 1280);

    return true;
}

TEST_CASE(Font_Switch_ScaledPosition) {
    MockFontInfo font;

    // Original position at 640x480
    int orig_x = 100;
    int orig_y = 200;

    // Scale factors for docked (3x) and handheld (2x)
    int docked_x = orig_x * 3;
    int docked_y = orig_y * 3;
    int handheld_x = orig_x * 2;
    int handheld_y = orig_y * 2;

    ASSERT_EQ(300, docked_x);
    ASSERT_EQ(600, docked_y);
    ASSERT_EQ(200, handheld_x);
    ASSERT_EQ(400, handheld_y);

    return true;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

TEST_CASE(Font_EdgeCase_ZeroSize) {
    MockFontInfo font;
    font.num_xy[0] = 0;
    font.num_xy[1] = 0;

    // Should handle zero size gracefully
    ASSERT_TRUE(font.isEndOfLine());
    ASSERT_TRUE(font.isLineEmpty());

    return true;
}

TEST_CASE(Font_EdgeCase_SingleChar) {
    MockFontInfo font;
    font.num_xy[0] = 1;
    font.num_xy[1] = 1;

    // Can fit one character
    ASSERT_FALSE(font.isEndOfLine());

    font.advanceCharInHankaku(2);
    ASSERT_TRUE(font.isEndOfLine());

    return true;
}

TEST_CASE(Font_EdgeCase_LargeOffset) {
    MockFontInfo font;

    font.addLineOffset(1000);
    int x = font.x();

    ASSERT_EQ(font.top_xy[0] + 1000, x);

    return true;
}

TEST_CASE(Font_EdgeCase_NegativeOffset) {
    MockFontInfo font;

    font.addLineOffset(-5);
    int x = font.x();

    ASSERT_EQ(font.top_xy[0] - 5, x);

    return true;
}

//==============================================================================
// Text Style Flag Tests
//==============================================================================

TEST_CASE(Font_Style_Bold) {
    MockFontInfo font;

    ASSERT_TRUE(font.is_bold);

    font.is_bold = false;
    ASSERT_FALSE(font.is_bold);

    return true;
}

TEST_CASE(Font_Style_Shadow) {
    MockFontInfo font;

    ASSERT_TRUE(font.is_shadow);

    font.is_shadow = false;
    ASSERT_FALSE(font.is_shadow);

    return true;
}

TEST_CASE(Font_Style_Transparent) {
    MockFontInfo font;

    ASSERT_TRUE(font.is_transparent);

    font.is_transparent = false;
    ASSERT_FALSE(font.is_transparent);

    return true;
}

TEST_CASE(Font_Style_NewlineAccepted) {
    MockFontInfo font;

    ASSERT_FALSE(font.is_newline_accepted);

    font.is_newline_accepted = true;
    ASSERT_TRUE(font.is_newline_accepted);

    return true;
}

//==============================================================================
// Mode Switching Tests
//==============================================================================

TEST_CASE(Font_ModeSwitch_YokoToTate) {
    MockFontInfo font;
    font.num_xy[0] = 10;
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    font.advanceCharInHankaku(10);
    font.newLine();
    font.advanceCharInHankaku(5);

    // Switch to tate mode
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    // Position should be cleared to tate mode default
    ASSERT_EQ((10 - 1) * 2, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);
    ASSERT_EQ(MockFontInfo::TATE_MODE, font.getTateyokoMode());

    return true;
}

TEST_CASE(Font_ModeSwitch_TateToYoko) {
    MockFontInfo font;
    font.num_xy[0] = 10;
    font.setTateyokoMode(MockFontInfo::TATE_MODE);

    font.advanceCharInHankaku(10);

    // Switch to yoko mode
    font.setTateyokoMode(MockFontInfo::YOKO_MODE);

    // Position should be cleared to yoko mode default
    ASSERT_EQ(0, font.xy[0]);
    ASSERT_EQ(0, font.xy[1]);
    ASSERT_EQ(MockFontInfo::YOKO_MODE, font.getTateyokoMode());

    return true;
}
