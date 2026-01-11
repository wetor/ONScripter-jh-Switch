/* -*- C++ -*-
 *
 *  test_rendering.cpp - Rendering and sharpness tests for ONScripter-jh-Switch
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
// Sharpness Parameter Tests
//==============================================================================

// Sharpness value validation (should be 0.0 to 1.0, or NAN for disabled)
bool isValidSharpness(float value) {
    if (std::isnan(value)) return true;  // NAN means disabled
    return value >= 0.0f && value <= 1.0f;
}

// Check if sharpness is enabled (not NAN)
bool isSharpnessEnabled(float value) {
    return !std::isnan(value);
}

TEST_CASE(Sharpness_ValidRange_Zero) {
    float sharpness = 0.0f;
    ASSERT_TRUE(isValidSharpness(sharpness));
    ASSERT_TRUE(isSharpnessEnabled(sharpness));
    return true;
}

TEST_CASE(Sharpness_ValidRange_One) {
    float sharpness = 1.0f;
    ASSERT_TRUE(isValidSharpness(sharpness));
    ASSERT_TRUE(isSharpnessEnabled(sharpness));
    return true;
}

TEST_CASE(Sharpness_ValidRange_Mid) {
    float sharpness = 0.5f;
    ASSERT_TRUE(isValidSharpness(sharpness));
    ASSERT_TRUE(isSharpnessEnabled(sharpness));
    return true;
}

TEST_CASE(Sharpness_Disabled_NAN) {
    float sharpness = NAN;
    ASSERT_TRUE(isValidSharpness(sharpness));
    ASSERT_FALSE(isSharpnessEnabled(sharpness));
    return true;
}

TEST_CASE(Sharpness_InvalidRange_Negative) {
    float sharpness = -0.5f;
    ASSERT_FALSE(isValidSharpness(sharpness));
    return true;
}

TEST_CASE(Sharpness_InvalidRange_OverOne) {
    float sharpness = 1.5f;
    ASSERT_FALSE(isValidSharpness(sharpness));
    return true;
}

TEST_CASE(Sharpness_EdgeCases) {
    // Test boundary values
    ASSERT_TRUE(isValidSharpness(0.0f));
    ASSERT_TRUE(isValidSharpness(0.001f));
    ASSERT_TRUE(isValidSharpness(0.999f));
    ASSERT_TRUE(isValidSharpness(1.0f));

    // Just outside valid range
    ASSERT_FALSE(isValidSharpness(-0.001f));
    ASSERT_FALSE(isValidSharpness(1.001f));
    return true;
}

//==============================================================================
// CAS (Contrast Adaptive Sharpening) Parameter Calculation Tests
// Based on AMD FidelityFX CAS algorithm
//==============================================================================

// CAS sharpness to internal parameter conversion
// sharpness 0.0 = minimal sharpening
// sharpness 1.0 = maximum sharpening
float casSharpnessToParameter(float sharpness) {
    // CAS uses a "peak" value that's derived from sharpness
    // Lower values = more sharpening (counterintuitive but that's how CAS works)
    // Typical range: 0.0 (max sharp) to 1.0 (min sharp)
    return 1.0f - sharpness;
}

// Calculate CAS constants for shader
void calculateCASConstants(float sharpness, float* const0, float* const1) {
    float peak = casSharpnessToParameter(sharpness);

    // These constants are used in the CAS shader
    // const0: peak adjustment for the filter
    // const1: usually fixed or derived from peak
    *const0 = peak;
    *const1 = 1.0f / (1.0f + 4.0f * peak);  // Normalization factor
}

TEST_CASE(CAS_Parameter_MinSharpening) {
    float sharpness = 0.0f;
    float param = casSharpnessToParameter(sharpness);
    ASSERT_NEAR(1.0f, param, 0.001f);  // Minimal sharpening
    return true;
}

TEST_CASE(CAS_Parameter_MaxSharpening) {
    float sharpness = 1.0f;
    float param = casSharpnessToParameter(sharpness);
    ASSERT_NEAR(0.0f, param, 0.001f);  // Maximum sharpening
    return true;
}

TEST_CASE(CAS_Parameter_MidSharpening) {
    float sharpness = 0.5f;
    float param = casSharpnessToParameter(sharpness);
    ASSERT_NEAR(0.5f, param, 0.001f);
    return true;
}

TEST_CASE(CAS_Constants_Calculation) {
    float const0, const1;

    // Test with mid sharpness
    calculateCASConstants(0.5f, &const0, &const1);
    ASSERT_NEAR(0.5f, const0, 0.001f);
    ASSERT_NEAR(1.0f / 3.0f, const1, 0.001f);  // 1/(1+4*0.5) = 1/3

    return true;
}

TEST_CASE(CAS_Constants_Range) {
    float const0, const1;

    // Test full range
    for (float s = 0.0f; s <= 1.0f; s += 0.1f) {
        calculateCASConstants(s, &const0, &const1);
        ASSERT_GE(const0, 0.0f);
        ASSERT_LE(const0, 1.0f);
        ASSERT_GT(const1, 0.0f);
        ASSERT_LE(const1, 1.0f);
    }

    return true;
}

//==============================================================================
// Render View Rectangle Tests
//==============================================================================

struct RenderViewRect {
    int x, y, w, h;
};

// Calculate render view rectangle for aspect ratio preservation
RenderViewRect calculateRenderViewRect(int game_w, int game_h, int device_w, int device_h) {
    RenderViewRect rect;

    float game_aspect = (float)game_w / game_h;
    float device_aspect = (float)device_w / device_h;

    if (game_aspect > device_aspect) {
        // Game is wider - letterbox (bars top/bottom)
        rect.w = device_w;
        rect.h = (int)(device_w / game_aspect);
        rect.x = 0;
        rect.y = (device_h - rect.h) / 2;
    } else if (game_aspect < device_aspect) {
        // Game is taller - pillarbox (bars left/right)
        rect.h = device_h;
        rect.w = (int)(device_h * game_aspect);
        rect.x = (device_w - rect.w) / 2;
        rect.y = 0;
    } else {
        // Same aspect ratio - no bars
        rect.x = 0;
        rect.y = 0;
        rect.w = device_w;
        rect.h = device_h;
    }

    return rect;
}

TEST_CASE(RenderView_SameAspect) {
    // 16:9 game on 16:9 screen
    RenderViewRect rect = calculateRenderViewRect(1280, 720, 1920, 1080);

    ASSERT_EQ(0, rect.x);
    ASSERT_EQ(0, rect.y);
    ASSERT_EQ(1920, rect.w);
    ASSERT_EQ(1080, rect.h);
    return true;
}

TEST_CASE(RenderView_4_3_On_16_9_Letterbox) {
    // 4:3 game (800x600) on 16:9 screen (1920x1080)
    // 4:3 = 1.333, 16:9 = 1.777, so game is taller (pillarbox, not letterbox)
    RenderViewRect rect = calculateRenderViewRect(800, 600, 1920, 1080);

    // 4:3 < 16:9, so we get pillarbox (left/right bars)
    // rect.h = 1080, rect.w = 1080 * (800/600) = 1080 * 1.333 = 1440
    ASSERT_GT(rect.x, 0);  // Left bar
    ASSERT_EQ(0, rect.y);
    ASSERT_LT(rect.w, 1920);  // Not full width
    ASSERT_EQ(1080, rect.h);

    // Check symmetry
    int right_bar = 1920 - rect.x - rect.w;
    ASSERT_EQ(rect.x, right_bar);

    return true;
}

TEST_CASE(RenderView_9_16_On_16_9_Pillarbox) {
    // 9:16 game (540x960) on 16:9 screen (1920x1080)
    // 9:16 = 0.5625, 16:9 = 1.777, so game is much taller (pillarbox)
    RenderViewRect rect = calculateRenderViewRect(540, 960, 1920, 1080);

    // Should have pillarbox (left/right bars)
    // rect.h = 1080, rect.w = 1080 * (540/960) = 1080 * 0.5625 = 607.5
    ASSERT_GT(rect.x, 0);  // Left bar
    ASSERT_EQ(0, rect.y);
    ASSERT_LT(rect.w, 1920);  // Not full width
    ASSERT_EQ(1080, rect.h);

    // Check symmetry (with tolerance for integer rounding)
    int right_bar = 1920 - rect.x - rect.w;
    int diff = (rect.x > right_bar) ? (rect.x - right_bar) : (right_bar - rect.x);
    ASSERT_LE(diff, 1);  // Allow 1 pixel difference due to rounding

    return true;
}

TEST_CASE(RenderView_DockedVsHandheld) {
    // Same game on different screen sizes
    RenderViewRect docked = calculateRenderViewRect(800, 600, 1920, 1080);
    RenderViewRect handheld = calculateRenderViewRect(800, 600, 1280, 720);

    // Aspect ratios should be the same
    float docked_aspect = (float)docked.w / docked.h;
    float handheld_aspect = (float)handheld.w / handheld.h;
    ASSERT_NEAR(docked_aspect, handheld_aspect, 0.01f);

    // But actual sizes differ
    ASSERT_GT(docked.w, handheld.w);
    ASSERT_GT(docked.h, handheld.h);

    return true;
}

TEST_CASE(RenderView_CentersCorrectly) {
    RenderViewRect rect = calculateRenderViewRect(800, 600, 1920, 1080);

    // Center point of viewport should be at center of device
    int viewport_center_x = rect.x + rect.w / 2;
    int viewport_center_y = rect.y + rect.h / 2;

    ASSERT_EQ(960, viewport_center_x);
    ASSERT_EQ(540, viewport_center_y);

    return true;
}

//==============================================================================
// Screen Scale Ratio Tests
//==============================================================================

struct ScaleRatios {
    float ratio1;  // screen_width / render_width
    float ratio2;  // screen_height / render_height
};

ScaleRatios calculateScaleRatios(int game_w, int game_h, int render_w, int render_h) {
    ScaleRatios ratios;
    ratios.ratio1 = (float)game_w / render_w;
    ratios.ratio2 = (float)game_h / render_h;
    return ratios;
}

TEST_CASE(ScaleRatio_1to1) {
    ScaleRatios ratios = calculateScaleRatios(1920, 1080, 1920, 1080);
    ASSERT_NEAR(1.0f, ratios.ratio1, 0.001f);
    ASSERT_NEAR(1.0f, ratios.ratio2, 0.001f);
    return true;
}

TEST_CASE(ScaleRatio_Upscale) {
    // 800x600 game rendered at 1920x1080
    ScaleRatios ratios = calculateScaleRatios(800, 600, 1920, 1080);
    ASSERT_LT(ratios.ratio1, 1.0f);
    ASSERT_LT(ratios.ratio2, 1.0f);
    ASSERT_NEAR(800.0f/1920.0f, ratios.ratio1, 0.001f);
    ASSERT_NEAR(600.0f/1080.0f, ratios.ratio2, 0.001f);
    return true;
}

TEST_CASE(ScaleRatio_Downscale) {
    // 4K game rendered at 1080p (unlikely but valid)
    ScaleRatios ratios = calculateScaleRatios(3840, 2160, 1920, 1080);
    ASSERT_GT(ratios.ratio1, 1.0f);
    ASSERT_GT(ratios.ratio2, 1.0f);
    ASSERT_NEAR(2.0f, ratios.ratio1, 0.001f);
    ASSERT_NEAR(2.0f, ratios.ratio2, 0.001f);
    return true;
}

TEST_CASE(ScaleRatio_NonUniform) {
    // Test when scale ratios differ (stretch mode)
    ScaleRatios ratios = calculateScaleRatios(800, 600, 1920, 1080);
    // For 4:3 to 16:9, ratios will be different
    ASSERT_NE(ratios.ratio1, ratios.ratio2);
    return true;
}

//==============================================================================
// Texture Format Tests (for GLES renderer)
//==============================================================================

enum TextureFormat {
    TEXTURE_FORMAT_UNKNOWN = 0,
    TEXTURE_FORMAT_RGBA8888,
    TEXTURE_FORMAT_RGB888,
    TEXTURE_FORMAT_RGBA4444,
    TEXTURE_FORMAT_RGB565
};

int getBytesPerPixel(TextureFormat format) {
    switch (format) {
        case TEXTURE_FORMAT_RGBA8888: return 4;
        case TEXTURE_FORMAT_RGB888: return 3;
        case TEXTURE_FORMAT_RGBA4444: return 2;
        case TEXTURE_FORMAT_RGB565: return 2;
        default: return 0;
    }
}

bool hasAlphaChannel(TextureFormat format) {
    return format == TEXTURE_FORMAT_RGBA8888 || format == TEXTURE_FORMAT_RGBA4444;
}

TEST_CASE(TextureFormat_BytesPerPixel) {
    ASSERT_EQ(4, getBytesPerPixel(TEXTURE_FORMAT_RGBA8888));
    ASSERT_EQ(3, getBytesPerPixel(TEXTURE_FORMAT_RGB888));
    ASSERT_EQ(2, getBytesPerPixel(TEXTURE_FORMAT_RGBA4444));
    ASSERT_EQ(2, getBytesPerPixel(TEXTURE_FORMAT_RGB565));
    ASSERT_EQ(0, getBytesPerPixel(TEXTURE_FORMAT_UNKNOWN));
    return true;
}

TEST_CASE(TextureFormat_AlphaChannel) {
    ASSERT_TRUE(hasAlphaChannel(TEXTURE_FORMAT_RGBA8888));
    ASSERT_TRUE(hasAlphaChannel(TEXTURE_FORMAT_RGBA4444));
    ASSERT_FALSE(hasAlphaChannel(TEXTURE_FORMAT_RGB888));
    ASSERT_FALSE(hasAlphaChannel(TEXTURE_FORMAT_RGB565));
    return true;
}

TEST_CASE(TextureFormat_MemoryCalculation) {
    int width = 1920;
    int height = 1080;

    size_t rgba8888_size = width * height * getBytesPerPixel(TEXTURE_FORMAT_RGBA8888);
    size_t rgb565_size = width * height * getBytesPerPixel(TEXTURE_FORMAT_RGB565);

    // RGBA8888 should use twice as much memory as RGB565
    ASSERT_EQ(rgba8888_size, rgb565_size * 2);

    // Verify actual sizes
    ASSERT_EQ(1920 * 1080 * 4, (int)rgba8888_size);  // ~8 MB
    ASSERT_EQ(1920 * 1080 * 2, (int)rgb565_size);    // ~4 MB

    return true;
}

//==============================================================================
// Cursor Double Display Fix Tests
// (From OnscripterYuri: if dst_rect.w > dst_rect.h then dst_rect.w = dst_rect.h)
//==============================================================================

struct CursorRect {
    int x, y, w, h;
};

void fixCursorDoubleDisplay(CursorRect* rect) {
    if (rect->w > rect->h) {
        rect->w = rect->h;
    }
}

TEST_CASE(Cursor_NormalSize_NoChange) {
    CursorRect rect = {100, 100, 32, 32};
    fixCursorDoubleDisplay(&rect);
    ASSERT_EQ(32, rect.w);
    ASSERT_EQ(32, rect.h);
    return true;
}

TEST_CASE(Cursor_WiderThanTall_Fixed) {
    CursorRect rect = {100, 100, 64, 32};  // Double width (buggy)
    fixCursorDoubleDisplay(&rect);
    ASSERT_EQ(32, rect.w);  // Should be fixed to height
    ASSERT_EQ(32, rect.h);
    return true;
}

TEST_CASE(Cursor_TallerThanWide_NoChange) {
    CursorRect rect = {100, 100, 32, 64};
    fixCursorDoubleDisplay(&rect);
    ASSERT_EQ(32, rect.w);  // No change - w < h
    ASSERT_EQ(64, rect.h);
    return true;
}

TEST_CASE(Cursor_ZeroSize_NoChange) {
    CursorRect rect = {100, 100, 0, 0};
    fixCursorDoubleDisplay(&rect);
    ASSERT_EQ(0, rect.w);
    ASSERT_EQ(0, rect.h);
    return true;
}

//==============================================================================
// Refresh Mode Flag Tests
//==============================================================================

#define REFRESH_NORMAL_MODE    (1 << 0)
#define REFRESH_SAYA_MODE      (1 << 1)
#define REFRESH_CURSOR_MODE    (1 << 2)
#define REFRESH_TEXT_MODE      (1 << 3)

int combineRefreshModes(int base_mode, bool draw_cursor) {
    return base_mode | (draw_cursor ? REFRESH_CURSOR_MODE : 0);
}

TEST_CASE(RefreshMode_NormalOnly) {
    int mode = combineRefreshModes(REFRESH_NORMAL_MODE, false);
    ASSERT_EQ(REFRESH_NORMAL_MODE, mode);
    ASSERT_FALSE(mode & REFRESH_CURSOR_MODE);
    return true;
}

TEST_CASE(RefreshMode_WithCursor) {
    int mode = combineRefreshModes(REFRESH_NORMAL_MODE, true);
    ASSERT_TRUE(mode & REFRESH_NORMAL_MODE);
    ASSERT_TRUE(mode & REFRESH_CURSOR_MODE);
    return true;
}

TEST_CASE(RefreshMode_MultipleModes) {
    int mode = REFRESH_NORMAL_MODE | REFRESH_TEXT_MODE;
    mode = combineRefreshModes(mode, true);

    ASSERT_TRUE(mode & REFRESH_NORMAL_MODE);
    ASSERT_TRUE(mode & REFRESH_TEXT_MODE);
    ASSERT_TRUE(mode & REFRESH_CURSOR_MODE);
    ASSERT_FALSE(mode & REFRESH_SAYA_MODE);
    return true;
}

//==============================================================================
// Frame Timing Tests
//==============================================================================

// Calculate time until next animation frame
int calcDurationToNextAnimation(int current_time, int next_time) {
    int duration = next_time - current_time;
    return (duration < 0) ? 0 : duration;
}

TEST_CASE(FrameTiming_FutureFrame) {
    int duration = calcDurationToNextAnimation(100, 150);
    ASSERT_EQ(50, duration);
    return true;
}

TEST_CASE(FrameTiming_PastFrame) {
    int duration = calcDurationToNextAnimation(200, 150);
    ASSERT_EQ(0, duration);  // Clamped to 0
    return true;
}

TEST_CASE(FrameTiming_ExactTime) {
    int duration = calcDurationToNextAnimation(100, 100);
    ASSERT_EQ(0, duration);
    return true;
}

TEST_CASE(FrameTiming_60FPS) {
    // At 60 FPS, frame duration is ~16.67ms
    int frame_duration = 16;
    int current = 0;
    int next = frame_duration;

    int duration = calcDurationToNextAnimation(current, next);
    ASSERT_EQ(16, duration);
    return true;
}

//==============================================================================
// GLES Renderer State Tests
//==============================================================================

struct MockGlesRendererState {
    bool initialized;
    int texture_id;
    float sharpness;
    int viewport_x, viewport_y, viewport_w, viewport_h;
};

bool shouldUseGlesRenderer(const MockGlesRendererState& state) {
    return state.initialized && !std::isnan(state.sharpness);
}

TEST_CASE(GLESRenderer_UseWhenInitializedWithSharpness) {
    MockGlesRendererState state = {true, 1, 0.5f, 0, 0, 1920, 1080};
    ASSERT_TRUE(shouldUseGlesRenderer(state));
    return true;
}

TEST_CASE(GLESRenderer_DontUseWhenNotInitialized) {
    MockGlesRendererState state = {false, 0, 0.5f, 0, 0, 1920, 1080};
    ASSERT_FALSE(shouldUseGlesRenderer(state));
    return true;
}

TEST_CASE(GLESRenderer_DontUseWhenSharpnessNAN) {
    MockGlesRendererState state = {true, 1, NAN, 0, 0, 1920, 1080};
    ASSERT_FALSE(shouldUseGlesRenderer(state));
    return true;
}

TEST_CASE(GLESRenderer_FallbackToSDL) {
    // When GLES is not available, should fall back to SDL
    MockGlesRendererState state = {false, 0, NAN, 0, 0, 0, 0};
    ASSERT_FALSE(shouldUseGlesRenderer(state));
    // In real code, this would trigger SDL_RenderCopy instead of GLES
    return true;
}
