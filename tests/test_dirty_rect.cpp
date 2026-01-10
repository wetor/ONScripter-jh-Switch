/* -*- C++ -*-
 *
 *  test_dirty_rect.cpp - DirtyRect tests for ONScripter-jh-Switch
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
#include <algorithm>

//==============================================================================
// Mock SDL_Rect structure (simplified version for testing)
//==============================================================================

struct SDL_Rect {
    int x, y, w, h;
};

//==============================================================================
// Mock DirtyRect class (replicates the logic from DirtyRect.cpp)
//==============================================================================

class MockDirtyRect {
public:
    int screen_width;
    int screen_height;
    SDL_Rect bounding_box;

    MockDirtyRect() {
        screen_width = screen_height = 0;
        bounding_box.x = bounding_box.y = 0;
        bounding_box.w = bounding_box.h = 0;
    }

    MockDirtyRect(const MockDirtyRect& d) {
        screen_width = d.screen_width;
        screen_height = d.screen_height;
        bounding_box = d.bounding_box;
    }

    MockDirtyRect& operator=(const MockDirtyRect& d) {
        screen_width = d.screen_width;
        screen_height = d.screen_height;
        bounding_box = d.bounding_box;
        return *this;
    }

    void setDimension(int w, int h) {
        screen_width = w;
        screen_height = h;
    }

    void add(SDL_Rect src) {
        if (src.w == 0 || src.h == 0) return;

        // Clip negative x
        if (src.x < 0) {
            if (src.w < -src.x) return;
            src.w += src.x;
            src.x = 0;
        }
        // Clip negative y
        if (src.y < 0) {
            if (src.h < -src.y) return;
            src.h += src.y;
            src.y = 0;
        }

        // Clip right edge
        if (src.x >= screen_width) return;
        if (src.x + src.w >= screen_width)
            src.w = screen_width - src.x;

        // Clip bottom edge
        if (src.y >= screen_height) return;
        if (src.y + src.h >= screen_height)
            src.h = screen_height - src.y;

        bounding_box = calcBoundingBox(bounding_box, src);
    }

    static SDL_Rect calcBoundingBox(SDL_Rect src1, SDL_Rect& src2) {
        if (src2.w == 0 || src2.h == 0) {
            return src1;
        }
        if (src1.w == 0 || src1.h == 0) {
            return src2;
        }

        if (src1.x > src2.x) {
            src1.w += src1.x - src2.x;
            src1.x = src2.x;
        }
        if (src1.y > src2.y) {
            src1.h += src1.y - src2.y;
            src1.y = src2.y;
        }
        if (src1.x + src1.w < src2.x + src2.w) {
            src1.w = src2.x + src2.w - src1.x;
        }
        if (src1.y + src1.h < src2.y + src2.h) {
            src1.h = src2.y + src2.h - src1.y;
        }

        return src1;
    }

    void clear() {
        bounding_box.w = bounding_box.h = 0;
    }

    void fill(int w, int h) {
        bounding_box.x = bounding_box.y = 0;
        bounding_box.w = w;
        bounding_box.h = h;
    }

    bool isEmpty() const {
        return bounding_box.w == 0 || bounding_box.h == 0;
    }
};

//==============================================================================
// Basic Initialization Tests
//==============================================================================

TEST_CASE(DirtyRect_DefaultConstruction) {
    MockDirtyRect dr;

    ASSERT_EQ(0, dr.screen_width);
    ASSERT_EQ(0, dr.screen_height);
    ASSERT_EQ(0, dr.bounding_box.w);
    ASSERT_EQ(0, dr.bounding_box.h);
    ASSERT_TRUE(dr.isEmpty());

    return true;
}

TEST_CASE(DirtyRect_SetDimension) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    ASSERT_EQ(640, dr.screen_width);
    ASSERT_EQ(480, dr.screen_height);

    return true;
}

TEST_CASE(DirtyRect_CopyConstruction) {
    MockDirtyRect dr1;
    dr1.setDimension(800, 600);
    dr1.bounding_box = {10, 20, 100, 200};

    MockDirtyRect dr2(dr1);

    ASSERT_EQ(800, dr2.screen_width);
    ASSERT_EQ(600, dr2.screen_height);
    ASSERT_EQ(10, dr2.bounding_box.x);
    ASSERT_EQ(20, dr2.bounding_box.y);
    ASSERT_EQ(100, dr2.bounding_box.w);
    ASSERT_EQ(200, dr2.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Assignment) {
    MockDirtyRect dr1;
    dr1.setDimension(1920, 1080);
    dr1.bounding_box = {50, 60, 300, 400};

    MockDirtyRect dr2;
    dr2 = dr1;

    ASSERT_EQ(1920, dr2.screen_width);
    ASSERT_EQ(1080, dr2.screen_height);
    ASSERT_EQ(50, dr2.bounding_box.x);
    ASSERT_EQ(60, dr2.bounding_box.y);
    ASSERT_EQ(300, dr2.bounding_box.w);
    ASSERT_EQ(400, dr2.bounding_box.h);

    return true;
}

//==============================================================================
// Add Rectangle Tests
//==============================================================================

TEST_CASE(DirtyRect_AddFirstRect) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {100, 100, 50, 50};
    dr.add(rect);

    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_AddMultipleRects_NonOverlapping) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect1 = {10, 10, 30, 30};
    SDL_Rect rect2 = {200, 200, 40, 40};

    dr.add(rect1);
    dr.add(rect2);

    // Bounding box should encompass both
    ASSERT_EQ(10, dr.bounding_box.x);
    ASSERT_EQ(10, dr.bounding_box.y);
    ASSERT_EQ(230, dr.bounding_box.w);  // 200 + 40 - 10 = 230
    ASSERT_EQ(230, dr.bounding_box.h);  // 200 + 40 - 10 = 230

    return true;
}

TEST_CASE(DirtyRect_AddMultipleRects_Overlapping) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect1 = {100, 100, 100, 100};
    SDL_Rect rect2 = {150, 150, 100, 100};

    dr.add(rect1);
    dr.add(rect2);

    // Bounding box should merge overlapping rects
    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(150, dr.bounding_box.w);  // 150 + 100 - 100 = 150
    ASSERT_EQ(150, dr.bounding_box.h);  // 150 + 100 - 100 = 150

    return true;
}

TEST_CASE(DirtyRect_AddEmptyRect_ZeroWidth) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect existing = {100, 100, 50, 50};
    dr.add(existing);

    SDL_Rect empty = {200, 200, 0, 50};
    dr.add(empty);

    // Should remain unchanged
    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_AddEmptyRect_ZeroHeight) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect existing = {100, 100, 50, 50};
    dr.add(existing);

    SDL_Rect empty = {200, 200, 50, 0};
    dr.add(empty);

    // Should remain unchanged
    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

//==============================================================================
// Clipping Tests - Negative Coordinates
//==============================================================================

TEST_CASE(DirtyRect_Add_NegativeX_PartiallyVisible) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect starts at x=-20 with width 50, so 30 pixels are visible
    SDL_Rect rect = {-20, 100, 50, 30};
    dr.add(rect);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(30, dr.bounding_box.w);  // 50 - 20 = 30
    ASSERT_EQ(30, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Add_NegativeY_PartiallyVisible) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect starts at y=-30 with height 60, so 30 pixels are visible
    SDL_Rect rect = {100, -30, 40, 60};
    dr.add(rect);

    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(40, dr.bounding_box.w);
    ASSERT_EQ(30, dr.bounding_box.h);  // 60 - 30 = 30

    return true;
}

TEST_CASE(DirtyRect_Add_NegativeX_FullyOutside) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect is completely outside (x=-100, w=50 means it ends at -50)
    SDL_Rect rect = {-100, 100, 50, 30};
    dr.add(rect);

    // Should remain empty
    ASSERT_TRUE(dr.isEmpty());

    return true;
}

TEST_CASE(DirtyRect_Add_NegativeY_FullyOutside) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect is completely outside vertically
    SDL_Rect rect = {100, -100, 40, 50};
    dr.add(rect);

    // Should remain empty
    ASSERT_TRUE(dr.isEmpty());

    return true;
}

TEST_CASE(DirtyRect_Add_BothNegative_PartiallyVisible) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect with both x and y negative
    SDL_Rect rect = {-10, -15, 50, 60};
    dr.add(rect);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(40, dr.bounding_box.w);  // 50 - 10 = 40
    ASSERT_EQ(45, dr.bounding_box.h);  // 60 - 15 = 45

    return true;
}

//==============================================================================
// Clipping Tests - Beyond Screen Bounds
//==============================================================================

TEST_CASE(DirtyRect_Add_BeyondRightEdge) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect extends past right edge
    SDL_Rect rect = {600, 100, 100, 50};
    dr.add(rect);

    ASSERT_EQ(600, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(40, dr.bounding_box.w);  // Clipped to screen_width - x = 640 - 600 = 40
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Add_BeyondBottomEdge) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect extends past bottom edge
    SDL_Rect rect = {100, 450, 50, 100};
    dr.add(rect);

    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(450, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(30, dr.bounding_box.h);  // Clipped to screen_height - y = 480 - 450 = 30

    return true;
}

TEST_CASE(DirtyRect_Add_FullyBeyondRight) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect is completely outside (starts at x=640 or beyond)
    SDL_Rect rect = {700, 100, 50, 50};
    dr.add(rect);

    ASSERT_TRUE(dr.isEmpty());

    return true;
}

TEST_CASE(DirtyRect_Add_FullyBeyondBottom) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Rect is completely outside (starts at y=480 or beyond)
    SDL_Rect rect = {100, 500, 50, 50};
    dr.add(rect);

    ASSERT_TRUE(dr.isEmpty());

    return true;
}

TEST_CASE(DirtyRect_Add_AllEdgesClipped) {
    MockDirtyRect dr;
    dr.setDimension(100, 100);

    // Rect extends beyond all edges
    SDL_Rect rect = {-20, -30, 200, 200};
    dr.add(rect);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(100, dr.bounding_box.w);  // Clipped to screen width
    ASSERT_EQ(100, dr.bounding_box.h);  // Clipped to screen height

    return true;
}

//==============================================================================
// Clear and Fill Tests
//==============================================================================

TEST_CASE(DirtyRect_Clear) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {100, 100, 200, 200};
    dr.add(rect);

    ASSERT_FALSE(dr.isEmpty());

    dr.clear();

    ASSERT_TRUE(dr.isEmpty());
    ASSERT_EQ(0, dr.bounding_box.w);
    ASSERT_EQ(0, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Fill) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    dr.fill(320, 240);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(320, dr.bounding_box.w);
    ASSERT_EQ(240, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_FillFullScreen) {
    MockDirtyRect dr;
    dr.setDimension(1920, 1080);

    dr.fill(1920, 1080);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(1920, dr.bounding_box.w);
    ASSERT_EQ(1080, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_ClearThenAdd) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Add first rect
    SDL_Rect rect1 = {0, 0, 100, 100};
    dr.add(rect1);

    // Clear
    dr.clear();
    ASSERT_TRUE(dr.isEmpty());

    // Add second rect
    SDL_Rect rect2 = {200, 200, 50, 50};
    dr.add(rect2);

    // Should only contain second rect
    ASSERT_EQ(200, dr.bounding_box.x);
    ASSERT_EQ(200, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

//==============================================================================
// Bounding Box Calculation Tests
//==============================================================================

TEST_CASE(DirtyRect_CalcBoundingBox_BothEmpty) {
    SDL_Rect src1 = {0, 0, 0, 0};
    SDL_Rect src2 = {0, 0, 0, 0};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(0, result.w);
    ASSERT_EQ(0, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_FirstEmpty) {
    SDL_Rect src1 = {0, 0, 0, 0};
    SDL_Rect src2 = {100, 100, 50, 50};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(100, result.x);
    ASSERT_EQ(100, result.y);
    ASSERT_EQ(50, result.w);
    ASSERT_EQ(50, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_SecondEmpty) {
    SDL_Rect src1 = {100, 100, 50, 50};
    SDL_Rect src2 = {0, 0, 0, 0};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(100, result.x);
    ASSERT_EQ(100, result.y);
    ASSERT_EQ(50, result.w);
    ASSERT_EQ(50, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_Identical) {
    SDL_Rect src1 = {100, 100, 50, 50};
    SDL_Rect src2 = {100, 100, 50, 50};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(100, result.x);
    ASSERT_EQ(100, result.y);
    ASSERT_EQ(50, result.w);
    ASSERT_EQ(50, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_Contained) {
    // src2 is completely contained within src1
    SDL_Rect src1 = {0, 0, 200, 200};
    SDL_Rect src2 = {50, 50, 50, 50};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(0, result.x);
    ASSERT_EQ(0, result.y);
    ASSERT_EQ(200, result.w);
    ASSERT_EQ(200, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_Src2Larger) {
    // src1 is contained within src2
    SDL_Rect src1 = {50, 50, 50, 50};
    SDL_Rect src2 = {0, 0, 200, 200};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(0, result.x);
    ASSERT_EQ(0, result.y);
    ASSERT_EQ(200, result.w);
    ASSERT_EQ(200, result.h);

    return true;
}

TEST_CASE(DirtyRect_CalcBoundingBox_DiagonallyOpposite) {
    SDL_Rect src1 = {0, 0, 50, 50};
    SDL_Rect src2 = {100, 100, 50, 50};

    SDL_Rect result = MockDirtyRect::calcBoundingBox(src1, src2);

    ASSERT_EQ(0, result.x);
    ASSERT_EQ(0, result.y);
    ASSERT_EQ(150, result.w);  // 100 + 50 = 150
    ASSERT_EQ(150, result.h);  // 100 + 50 = 150

    return true;
}

//==============================================================================
// Edge Case Tests
//==============================================================================

TEST_CASE(DirtyRect_Add_SinglePixel) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {100, 100, 1, 1};
    dr.add(rect);

    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(100, dr.bounding_box.y);
    ASSERT_EQ(1, dr.bounding_box.w);
    ASSERT_EQ(1, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Add_FullScreenRect) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {0, 0, 640, 480};
    dr.add(rect);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(640, dr.bounding_box.w);
    ASSERT_EQ(480, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Add_AtOrigin) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {0, 0, 50, 50};
    dr.add(rect);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_Add_AtBottomRightCorner) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    SDL_Rect rect = {590, 430, 50, 50};
    dr.add(rect);

    ASSERT_EQ(590, dr.bounding_box.x);
    ASSERT_EQ(430, dr.bounding_box.y);
    ASSERT_EQ(50, dr.bounding_box.w);
    ASSERT_EQ(50, dr.bounding_box.h);

    return true;
}

//==============================================================================
// Stress Tests
//==============================================================================

TEST_CASE(DirtyRect_AddManyRects) {
    MockDirtyRect dr;
    dr.setDimension(1000, 1000);

    // Add 100 small rects scattered across the screen
    for (int i = 0; i < 100; i++) {
        SDL_Rect rect = {(i * 10) % 900, (i * 7) % 900, 20, 20};
        dr.add(rect);
    }

    // Should have a valid bounding box
    ASSERT_FALSE(dr.isEmpty());
    ASSERT_GE(dr.bounding_box.x, 0);
    ASSERT_GE(dr.bounding_box.y, 0);
    ASSERT_LE(dr.bounding_box.x + dr.bounding_box.w, 1000);
    ASSERT_LE(dr.bounding_box.y + dr.bounding_box.h, 1000);

    return true;
}

TEST_CASE(DirtyRect_AddClearCycle) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    for (int cycle = 0; cycle < 100; cycle++) {
        SDL_Rect rect = {cycle % 600, cycle % 440, 40, 40};
        dr.add(rect);

        if (cycle % 10 == 9) {
            dr.clear();
            ASSERT_TRUE(dr.isEmpty());
        }
    }

    // Final state should be empty (since 99 % 10 == 9, clear was called on last iteration)
    ASSERT_TRUE(dr.isEmpty());

    return true;
}

//==============================================================================
// Switch-specific Resolution Tests
//==============================================================================

TEST_CASE(DirtyRect_SwitchDocked_1920x1080) {
    MockDirtyRect dr;
    dr.setDimension(1920, 1080);

    // Test common UI element positions
    SDL_Rect textBox = {100, 800, 1720, 200};
    SDL_Rect nameBox = {100, 750, 300, 50};

    dr.add(textBox);
    dr.add(nameBox);

    ASSERT_EQ(100, dr.bounding_box.x);
    ASSERT_EQ(750, dr.bounding_box.y);
    ASSERT_EQ(1720, dr.bounding_box.w);
    ASSERT_EQ(250, dr.bounding_box.h);  // From 750 to 1000

    return true;
}

TEST_CASE(DirtyRect_SwitchHandheld_1280x720) {
    MockDirtyRect dr;
    dr.setDimension(1280, 720);

    // Test letterboxed content (e.g., 4:3 in 16:9)
    // 4:3 at 720p height would be 960x720, centered
    int offsetX = (1280 - 960) / 2;  // 160
    SDL_Rect gameArea = {offsetX, 0, 960, 720};

    dr.add(gameArea);

    ASSERT_EQ(160, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(960, dr.bounding_box.w);
    ASSERT_EQ(720, dr.bounding_box.h);

    return true;
}

TEST_CASE(DirtyRect_GameOriginal_640x480) {
    MockDirtyRect dr;
    dr.setDimension(640, 480);

    // Typical ONScripter game resolution
    SDL_Rect sprite = {100, 50, 200, 300};
    SDL_Rect text = {50, 350, 540, 120};

    dr.add(sprite);
    dr.add(text);

    // Bounding box should cover both
    ASSERT_EQ(50, dr.bounding_box.x);
    ASSERT_EQ(50, dr.bounding_box.y);
    ASSERT_EQ(540, dr.bounding_box.w);   // From 50 to 590
    ASSERT_EQ(420, dr.bounding_box.h);   // From 50 to 470

    return true;
}

TEST_CASE(DirtyRect_GameOriginal_800x600) {
    MockDirtyRect dr;
    dr.setDimension(800, 600);

    SDL_Rect background = {0, 0, 800, 450};
    SDL_Rect textbox = {50, 450, 700, 140};

    dr.add(background);
    dr.add(textbox);

    ASSERT_EQ(0, dr.bounding_box.x);
    ASSERT_EQ(0, dr.bounding_box.y);
    ASSERT_EQ(800, dr.bounding_box.w);
    ASSERT_EQ(590, dr.bounding_box.h);  // From 0 to 590 (450+140)

    return true;
}
