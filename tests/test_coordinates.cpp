/* -*- C++ -*-
 *
 *  test_coordinates.cpp - Button position and touch coordinate tests for ONScripter-jh-Switch
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
#include <cmath>

//==============================================================================
// Mock SDL_Rect structure
//==============================================================================

struct MockRect {
    int x, y, w, h;
};

//==============================================================================
// Mock screen parameters (simulating ONScripter member variables)
//==============================================================================

struct MockScreenParams {
    int screen_width;         // Logical game width (e.g., 800, 1280)
    int screen_height;        // Logical game height (e.g., 600, 720)
    int screen_device_width;  // Physical device width (e.g., 1920, 1280)
    int screen_device_height; // Physical device height (e.g., 1080, 720)
    int device_width;         // Total device width including letterbox
    int device_height;        // Total device height including letterbox
    MockRect render_view_rect; // Viewport rectangle for rendering
    float screen_scale_ratio1; // Scale ratio numerator
    float screen_scale_ratio2; // Scale ratio denominator

    MockScreenParams() {
        // Default: 800x600 game on 1920x1080 screen
        screen_width = 800;
        screen_height = 600;
        screen_device_width = 1920;
        screen_device_height = 1080;
        device_width = 1920;
        device_height = 1080;
        render_view_rect = {0, 0, 1920, 1080};
        screen_scale_ratio1 = 1.0f;
        screen_scale_ratio2 = 1.0f;
    }

    // Configure for Switch docked mode (1920x1080)
    void setDockedMode(int game_w, int game_h) {
        screen_width = game_w;
        screen_height = game_h;
        device_width = 1920;
        device_height = 1080;

        // Calculate aspect ratio preserving viewport
        float game_aspect = (float)game_w / game_h;
        float device_aspect = 1920.0f / 1080.0f;

        if (game_aspect > device_aspect) {
            // Game is wider, letterbox top/bottom
            screen_device_width = 1920;
            screen_device_height = (int)(1920.0f / game_aspect);
            render_view_rect.x = 0;
            render_view_rect.y = (1080 - screen_device_height) / 2;
        } else {
            // Game is taller, pillarbox left/right
            screen_device_height = 1080;
            screen_device_width = (int)(1080.0f * game_aspect);
            render_view_rect.x = (1920 - screen_device_width) / 2;
            render_view_rect.y = 0;
        }
        render_view_rect.w = screen_device_width;
        render_view_rect.h = screen_device_height;

        screen_scale_ratio1 = (float)screen_width / screen_device_width;
        screen_scale_ratio2 = (float)screen_height / screen_device_height;
    }

    // Configure for Switch handheld mode (1280x720)
    void setHandheldMode(int game_w, int game_h) {
        screen_width = game_w;
        screen_height = game_h;
        device_width = 1280;
        device_height = 720;

        float game_aspect = (float)game_w / game_h;
        float device_aspect = 1280.0f / 720.0f;

        if (game_aspect > device_aspect) {
            screen_device_width = 1280;
            screen_device_height = (int)(1280.0f / game_aspect);
            render_view_rect.x = 0;
            render_view_rect.y = (720 - screen_device_height) / 2;
        } else {
            screen_device_height = 720;
            screen_device_width = (int)(720.0f * game_aspect);
            render_view_rect.x = (1280 - screen_device_width) / 2;
            render_view_rect.y = 0;
        }
        render_view_rect.w = screen_device_width;
        render_view_rect.h = screen_device_height;

        screen_scale_ratio1 = (float)screen_width / screen_device_width;
        screen_scale_ratio2 = (float)screen_height / screen_device_height;
    }
};

//==============================================================================
// Simulated shiftCursorOnButton coordinate transformation (original)
//==============================================================================

void originalShiftCursor(int button_x, int button_y, int* out_x, int* out_y,
                         const MockScreenParams& params) {
    int x = button_x;
    int y = button_y;

    *out_x = x * params.screen_device_width / params.screen_width;
    *out_y = y * params.screen_device_height / params.screen_height;
}

//==============================================================================
// Simulated shiftCursorOnButton coordinate transformation (fixed from OnscripterYuri)
//==============================================================================

void fixedShiftCursor(int button_x, int button_y, int* out_x, int* out_y,
                      const MockScreenParams& params) {
    int x = button_x;
    int y = button_y;

    // Bounds checking from OnscripterYuri
    if (x < 0) x = 0;
    else if (x >= params.screen_width) x = params.screen_width - 1;
    if (y < 0) y = 0;
    else if (y >= params.screen_height) y = params.screen_height - 1;

    // Add render_view_rect offset for proper coordinate mapping
    *out_x = x * params.screen_device_width / params.screen_width + params.render_view_rect.x;
    *out_y = y * params.screen_device_height / params.screen_height + params.render_view_rect.y;
}

//==============================================================================
// Simulated touch coordinate transformation (original Switch code)
//==============================================================================

void originalTouchToLogical(float touch_x, float touch_y, int* out_x, int* out_y,
                            const MockScreenParams& params) {
    *out_x = (int)(params.device_width * touch_x);
    *out_y = (int)(params.device_height * touch_y);
}

//==============================================================================
// Simulated touch coordinate transformation (fixed from OnscripterYuri)
//==============================================================================

void fixedTouchToLogical(float touch_x, float touch_y, int* out_x, int* out_y,
                         const MockScreenParams& params) {
    *out_x = (int)((params.device_width * touch_x - params.render_view_rect.x) * params.screen_scale_ratio1);
    *out_y = (int)((params.device_height * touch_y - params.render_view_rect.y) * params.screen_scale_ratio2);
}

//==============================================================================
// Helper: Check if coordinate is within game bounds
//==============================================================================

bool isWithinGameBounds(int x, int y, const MockScreenParams& params) {
    return x >= 0 && x < params.screen_width && y >= 0 && y < params.screen_height;
}

//==============================================================================
// Button Position Tests - Basic
//==============================================================================

TEST_CASE(Button_CenterPosition_FullScreen) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    // Button at center of game screen (400, 300)
    fixedShiftCursor(400, 300, &out_x, &out_y, params);

    // Should map to center of device screen (960, 540)
    ASSERT_EQ(960, out_x);
    ASSERT_EQ(540, out_y);
    return true;
}

TEST_CASE(Button_TopLeftCorner) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    fixedShiftCursor(0, 0, &out_x, &out_y, params);

    ASSERT_EQ(0, out_x);
    ASSERT_EQ(0, out_y);
    return true;
}

TEST_CASE(Button_BottomRightCorner) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    // Max valid coordinate is screen_width - 1, screen_height - 1
    fixedShiftCursor(799, 599, &out_x, &out_y, params);

    // Should be close to device max
    ASSERT_GT(out_x, 1900);
    ASSERT_GT(out_y, 1070);
    return true;
}

//==============================================================================
// Button Position Tests - With Letterboxing/Pillarboxing
//==============================================================================

TEST_CASE(Button_WithLetterbox_DockedMode) {
    MockScreenParams params;
    params.setDockedMode(800, 600); // 4:3 game on 16:9 screen = letterbox

    int out_x, out_y;

    // Button at center of game
    fixedShiftCursor(400, 300, &out_x, &out_y, params);

    // Center should still be at device center
    ASSERT_EQ(960, out_x);

    // Y should be offset by letterbox
    ASSERT_GT(out_y, 500);
    ASSERT_LT(out_y, 580);

    return true;
}

TEST_CASE(Button_WithPillarbox_DockedMode) {
    MockScreenParams params;
    // 9:16 game (portrait) on 16:9 screen = pillarbox
    params.setDockedMode(540, 960);

    int out_x, out_y;

    // Button at center of game
    fixedShiftCursor(270, 480, &out_x, &out_y, params);

    // Should be at center of device
    ASSERT_GT(out_x, 900);
    ASSERT_LT(out_x, 1020);
    ASSERT_EQ(540, out_y);

    return true;
}

TEST_CASE(Button_HandheldMode_720p) {
    MockScreenParams params;
    params.setHandheldMode(800, 600);

    int out_x, out_y;

    // Button at center
    fixedShiftCursor(400, 300, &out_x, &out_y, params);

    // Should map to center of 1280x720 with letterbox offset
    ASSERT_EQ(640, out_x);
    ASSERT_GT(out_y, 300);
    ASSERT_LT(out_y, 420);

    return true;
}

//==============================================================================
// Button Position Tests - Bounds Checking
//==============================================================================

TEST_CASE(Button_NegativeX_Clamped) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    // Negative coordinate should be clamped to 0
    fixedShiftCursor(-100, 300, &out_x, &out_y, params);

    ASSERT_EQ(0, out_x);
    return true;
}

TEST_CASE(Button_NegativeY_Clamped) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    fixedShiftCursor(400, -50, &out_x, &out_y, params);

    ASSERT_EQ(0, out_y);
    return true;
}

TEST_CASE(Button_ExceedsWidth_Clamped) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    // X >= screen_width should be clamped
    fixedShiftCursor(1000, 300, &out_x, &out_y, params);

    // Should be at max valid position
    int expected_x = 799 * 1920 / 800;
    ASSERT_EQ(expected_x, out_x);
    return true;
}

TEST_CASE(Button_ExceedsHeight_Clamped) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};

    int out_x, out_y;

    fixedShiftCursor(400, 800, &out_x, &out_y, params);

    int expected_y = 599 * 1080 / 600;
    ASSERT_EQ(expected_y, out_y);
    return true;
}

//==============================================================================
// Touch Coordinate Tests - Basic
//==============================================================================

TEST_CASE(Touch_CenterScreen_NoOffset) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};
    params.screen_scale_ratio1 = 800.0f / 1920.0f;
    params.screen_scale_ratio2 = 600.0f / 1080.0f;

    int out_x, out_y;

    // Touch at center of screen (0.5, 0.5)
    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);

    // Should map to center of game (400, 300)
    ASSERT_NEAR(400, out_x, 2);
    ASSERT_NEAR(300, out_y, 2);
    return true;
}

TEST_CASE(Touch_TopLeftCorner) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};
    params.screen_scale_ratio1 = 800.0f / 1920.0f;
    params.screen_scale_ratio2 = 600.0f / 1080.0f;

    int out_x, out_y;

    fixedTouchToLogical(0.0f, 0.0f, &out_x, &out_y, params);

    ASSERT_EQ(0, out_x);
    ASSERT_EQ(0, out_y);
    return true;
}

TEST_CASE(Touch_BottomRightCorner) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};
    params.screen_scale_ratio1 = 800.0f / 1920.0f;
    params.screen_scale_ratio2 = 600.0f / 1080.0f;

    int out_x, out_y;

    fixedTouchToLogical(1.0f, 1.0f, &out_x, &out_y, params);

    ASSERT_NEAR(800, out_x, 2);
    ASSERT_NEAR(600, out_y, 2);
    return true;
}

//==============================================================================
// Touch Coordinate Tests - With Letterbox Offset
//==============================================================================

TEST_CASE(Touch_WithLetterbox_Center) {
    MockScreenParams params;
    params.setDockedMode(800, 600); // Creates letterbox

    int out_x, out_y;

    // Touch at center of device
    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);

    // Should map to center of game
    ASSERT_NEAR(400, out_x, 10);
    ASSERT_NEAR(300, out_y, 10);
    return true;
}

TEST_CASE(Touch_WithLetterbox_InBlackBar) {
    MockScreenParams params;
    params.setDockedMode(800, 600);

    int out_x, out_y;

    // Touch in top letterbox area (y close to 0)
    // The letterbox offset means this maps to negative or very small game Y
    fixedTouchToLogical(0.5f, 0.0f, &out_x, &out_y, params);

    // Y should be small or negative (near top of or in letterbox area)
    // The exact value depends on letterbox size calculation
    ASSERT_LT(out_y, 50);  // Should be near top of game area or in letterbox
    return true;
}

TEST_CASE(Touch_WithPillarbox_Center) {
    MockScreenParams params;
    params.setDockedMode(540, 960); // Creates pillarbox

    int out_x, out_y;

    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);

    ASSERT_NEAR(270, out_x, 10);
    ASSERT_NEAR(480, out_y, 10);
    return true;
}

//==============================================================================
// Touch Coordinate Tests - Handheld Mode
//==============================================================================

TEST_CASE(Touch_Handheld_Center) {
    MockScreenParams params;
    params.setHandheldMode(800, 600);

    int out_x, out_y;

    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);

    ASSERT_NEAR(400, out_x, 10);
    ASSERT_NEAR(300, out_y, 10);
    return true;
}

TEST_CASE(Touch_Handheld_AllCorners) {
    MockScreenParams params;
    params.setHandheldMode(800, 600);

    int out_x, out_y;

    // Approximate game area on 1280x720
    // With 4:3 on 16:9, we get letterbox
    float left_x = (float)params.render_view_rect.x / params.device_width;
    float right_x = (float)(params.render_view_rect.x + params.render_view_rect.w) / params.device_width;
    float top_y = (float)params.render_view_rect.y / params.device_height;
    float bottom_y = (float)(params.render_view_rect.y + params.render_view_rect.h) / params.device_height;

    // Touch at top-left of game area
    fixedTouchToLogical(left_x, top_y, &out_x, &out_y, params);
    ASSERT_NEAR(0, out_x, 10);
    ASSERT_NEAR(0, out_y, 10);

    // Touch at bottom-right of game area
    fixedTouchToLogical(right_x, bottom_y, &out_x, &out_y, params);
    ASSERT_NEAR(800, out_x, 15);
    ASSERT_NEAR(600, out_y, 15);

    return true;
}

//==============================================================================
// Comparison: Original vs Fixed Implementation
//==============================================================================

TEST_CASE(Comparison_Button_WithOffset_Difference) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 1440;
    params.screen_device_height = 1080;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {240, 0, 1440, 1080}; // Pillarbox

    int orig_x, orig_y, fixed_x, fixed_y;

    originalShiftCursor(400, 300, &orig_x, &orig_y, params);
    fixedShiftCursor(400, 300, &fixed_x, &fixed_y, params);

    // Original doesn't add offset, fixed does
    ASSERT_EQ(fixed_x, orig_x + 240);
    ASSERT_EQ(fixed_y, orig_y);
    return true;
}

TEST_CASE(Comparison_Touch_WithOffset_Difference) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.device_width = 1920;
    params.device_height = 1080;
    params.screen_device_width = 1440;
    params.screen_device_height = 1080;
    params.render_view_rect = {240, 0, 1440, 1080};
    params.screen_scale_ratio1 = 800.0f / 1440.0f;
    params.screen_scale_ratio2 = 600.0f / 1080.0f;

    int orig_x, orig_y, fixed_x, fixed_y;

    // Touch at device center
    originalTouchToLogical(0.5f, 0.5f, &orig_x, &orig_y, params);
    fixedTouchToLogical(0.5f, 0.5f, &fixed_x, &fixed_y, params);

    // Fixed version should give correct game coordinates
    ASSERT_NEAR(400, fixed_x, 10);
    ASSERT_NEAR(300, fixed_y, 10);

    // Original version would be off
    ASSERT_EQ(960, orig_x); // Just device_width * 0.5
    return true;
}

//==============================================================================
// Round-trip Tests (Button -> Screen -> Touch -> Game should be consistent)
//==============================================================================

TEST_CASE(Roundtrip_ButtonToTouchAndBack) {
    MockScreenParams params;
    params.setDockedMode(800, 600);

    // Start with a game coordinate
    int game_x = 400, game_y = 300;

    // Convert to device coordinate (as shiftCursorOnButton does)
    int device_x, device_y;
    fixedShiftCursor(game_x, game_y, &device_x, &device_y, params);

    // Convert device position to touch normalized (0-1)
    float touch_x = (float)device_x / params.device_width;
    float touch_y = (float)device_y / params.device_height;

    // Convert touch back to game coordinate
    int result_x, result_y;
    fixedTouchToLogical(touch_x, touch_y, &result_x, &result_y, params);

    // Should get back approximately the same game coordinate
    ASSERT_NEAR(game_x, result_x, 5);
    ASSERT_NEAR(game_y, result_y, 5);
    return true;
}

TEST_CASE(Roundtrip_MultiplePositions) {
    MockScreenParams params;
    params.setDockedMode(1280, 720);

    // Test several positions
    int test_positions[][2] = {
        {0, 0}, {640, 360}, {1279, 719},
        {100, 100}, {500, 400}, {1000, 600}
    };

    for (int i = 0; i < 6; i++) {
        int game_x = test_positions[i][0];
        int game_y = test_positions[i][1];

        int device_x, device_y;
        fixedShiftCursor(game_x, game_y, &device_x, &device_y, params);

        float touch_x = (float)device_x / params.device_width;
        float touch_y = (float)device_y / params.device_height;

        int result_x, result_y;
        fixedTouchToLogical(touch_x, touch_y, &result_x, &result_y, params);

        ASSERT_NEAR(game_x, result_x, 5);
        ASSERT_NEAR(game_y, result_y, 5);
    }

    return true;
}

//==============================================================================
// Edge Cases
//==============================================================================

TEST_CASE(EdgeCase_ZeroSizeViewport) {
    MockScreenParams params;
    params.screen_width = 800;
    params.screen_height = 600;
    params.screen_device_width = 0; // Invalid
    params.screen_device_height = 0;
    params.render_view_rect = {0, 0, 0, 0};

    // This would cause division by zero in real code
    // The test documents the edge case - real implementation should guard against this
    return true;
}

TEST_CASE(EdgeCase_1to1_Scaling) {
    MockScreenParams params;
    params.screen_width = 1920;
    params.screen_height = 1080;
    params.screen_device_width = 1920;
    params.screen_device_height = 1080;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {0, 0, 1920, 1080};
    params.screen_scale_ratio1 = 1.0f;
    params.screen_scale_ratio2 = 1.0f;

    int out_x, out_y;

    // With 1:1 scaling, coordinates should match exactly
    fixedShiftCursor(960, 540, &out_x, &out_y, params);
    ASSERT_EQ(960, out_x);
    ASSERT_EQ(540, out_y);

    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);
    ASSERT_EQ(960, out_x);
    ASSERT_EQ(540, out_y);

    return true;
}

TEST_CASE(EdgeCase_VerySmallGameResolution) {
    MockScreenParams params;
    params.screen_width = 320;
    params.screen_height = 240;
    params.screen_device_width = 1440;  // Actual render width (pillarboxed)
    params.screen_device_height = 1080;
    params.device_width = 1920;
    params.device_height = 1080;
    params.render_view_rect = {240, 0, 1440, 1080}; // Pillarbox
    params.screen_scale_ratio1 = 320.0f / 1440.0f;
    params.screen_scale_ratio2 = 240.0f / 1080.0f;

    int out_x, out_y;

    // Button at center of 320x240 game
    fixedShiftCursor(160, 120, &out_x, &out_y, params);
    // 160 * 1440 / 320 + 240 = 720 + 240 = 960
    ASSERT_EQ(960, out_x); // Center with offset
    ASSERT_EQ(540, out_y);

    // Touch at center of device
    fixedTouchToLogical(0.5f, 0.5f, &out_x, &out_y, params);
    // (1920*0.5 - 240) * (320/1440) = (960-240) * 0.222 = 720 * 0.222 = 160
    ASSERT_NEAR(160, out_x, 10);
    ASSERT_NEAR(120, out_y, 10);

    return true;
}
