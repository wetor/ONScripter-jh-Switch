/* -*- C++ -*-
 *
 *  test_lua_animation.cpp - Lua animation callback tests for ONScripter-jh-Switch
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
// Mock structures to simulate LUAHandler behavior
//==============================================================================

struct MockLuaHandler {
    bool is_animatable;
    int duration_time;
    int next_time;
    bool callback_enabled;
    int callback_count;

    MockLuaHandler() : is_animatable(false), duration_time(15),
                       next_time(0), callback_enabled(true), callback_count(0) {}

    bool isCallbackEnabled() const { return callback_enabled; }

    int callFunction() {
        callback_count++;
        return 0; // Success
    }
};

//==============================================================================
// Simulated proceedAnimation logic (original version - for comparison)
//==============================================================================

int simulateOriginalProceedAnimation(MockLuaHandler& handler, int current_time) {
    int iterations = 0;

    if (handler.is_animatable) {
        while (handler.next_time <= current_time) {
            if (handler.isCallbackEnabled()) {
                handler.callFunction();
            }

            handler.next_time += handler.duration_time;
            iterations++;

            if (handler.duration_time <= 0) {
                handler.next_time = current_time;
                break;
            }
        }
    }

    return iterations;
}

//==============================================================================
// Simulated proceedAnimation logic (optimized version from OnscripterYuri)
//==============================================================================

int simulateOptimizedProceedAnimation(MockLuaHandler& handler, int current_time) {
    int iterations = 0;

    if (handler.is_animatable) {
        while (handler.next_time <= current_time) {
            if (handler.isCallbackEnabled()) {
                handler.callFunction();
            }
            iterations++;

            if (handler.duration_time <= 0) {
                handler.next_time = current_time;
                break;
            }

            // Exit the loop not to decrease the performance
            // Skip multiple frames at once if we're behind
            do {
                handler.next_time += handler.duration_time;
            } while (handler.next_time <= current_time);
        }
    }

    return iterations;
}

//==============================================================================
// Basic Animation Callback Tests
//==============================================================================

TEST_CASE(LuaAnimation_Disabled) {
    MockLuaHandler handler;
    handler.is_animatable = false;
    handler.next_time = 0;
    handler.duration_time = 16;

    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(0, iterations);
    ASSERT_EQ(0, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_Enabled_SingleFrame) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16;

    // Current time is 10ms, next_time is 0, so callback should fire once
    int iterations = simulateOptimizedProceedAnimation(handler, 10);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    ASSERT_GT(handler.next_time, 10); // next_time should be advanced past current_time
    return true;
}

TEST_CASE(LuaAnimation_NotYetTime) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 100;
    handler.duration_time = 16;

    // Current time is 50ms, but next_time is 100ms, so no callback
    int iterations = simulateOptimizedProceedAnimation(handler, 50);

    ASSERT_EQ(0, iterations);
    ASSERT_EQ(0, handler.callback_count);
    ASSERT_EQ(100, handler.next_time);
    return true;
}

//==============================================================================
// Frame Skip Performance Tests (Key optimization from OnscripterYuri)
//==============================================================================

TEST_CASE(LuaAnimation_Optimized_SkipsMultipleFrames) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16; // ~60fps

    // Simulate being 100ms behind (about 6 frames)
    // Optimized version should only call callback ONCE and skip ahead
    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    ASSERT_GT(handler.next_time, 100);
    return true;
}

TEST_CASE(LuaAnimation_Original_CallsEveryFrame) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16;

    // Original version would call callback multiple times
    int iterations = simulateOriginalProceedAnimation(handler, 100);

    // Should iterate approximately 100/16 = 6-7 times
    ASSERT_GE(iterations, 6);
    ASSERT_LE(iterations, 7);
    ASSERT_EQ(iterations, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_PerformanceComparison) {
    // Test that optimized version calls callback fewer times than original
    MockLuaHandler original_handler;
    original_handler.is_animatable = true;
    original_handler.next_time = 0;
    original_handler.duration_time = 16;

    MockLuaHandler optimized_handler;
    optimized_handler.is_animatable = true;
    optimized_handler.next_time = 0;
    optimized_handler.duration_time = 16;

    // Simulate being very far behind (1000ms = 1 second)
    simulateOriginalProceedAnimation(original_handler, 1000);
    simulateOptimizedProceedAnimation(optimized_handler, 1000);

    // Original should call many times (~62 times for 1000ms at 16ms interval)
    ASSERT_GT(original_handler.callback_count, 50);

    // Optimized should call only once
    ASSERT_EQ(1, optimized_handler.callback_count);

    // Both should end up with next_time > current_time
    ASSERT_GT(original_handler.next_time, 1000);
    ASSERT_GT(optimized_handler.next_time, 1000);

    return true;
}

//==============================================================================
// Duration Time Edge Cases
//==============================================================================

TEST_CASE(LuaAnimation_ZeroDuration) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 0;

    // Zero duration should not cause infinite loop
    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    ASSERT_EQ(100, handler.next_time); // Should be set to current_time
    return true;
}

TEST_CASE(LuaAnimation_NegativeDuration) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = -10;

    // Negative duration should not cause infinite loop
    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(100, handler.next_time);
    return true;
}

TEST_CASE(LuaAnimation_VerySmallDuration) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 1; // 1ms duration

    // Even with tiny duration, optimized version should only call once
    int iterations = simulateOptimizedProceedAnimation(handler, 1000);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_LargeDuration) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 1000; // 1 second duration

    // With large duration, should still work correctly
    int iterations = simulateOptimizedProceedAnimation(handler, 500);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    ASSERT_GE(handler.next_time, 1000);
    return true;
}

//==============================================================================
// Callback Disabled Tests
//==============================================================================

TEST_CASE(LuaAnimation_CallbackDisabled) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.callback_enabled = false;
    handler.next_time = 0;
    handler.duration_time = 16;

    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(0, handler.callback_count); // Callback should not be called
    return true;
}

//==============================================================================
// Time Precision Tests
//==============================================================================

TEST_CASE(LuaAnimation_ExactTimeMatch) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 100;
    handler.duration_time = 16;

    // Exact match: current_time == next_time
    int iterations = simulateOptimizedProceedAnimation(handler, 100);

    ASSERT_EQ(1, iterations);
    ASSERT_EQ(1, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_JustBeforeTime) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 100;
    handler.duration_time = 16;

    // Just before: current_time < next_time
    int iterations = simulateOptimizedProceedAnimation(handler, 99);

    ASSERT_EQ(0, iterations);
    ASSERT_EQ(0, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_ConsecutiveCalls) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16;

    // Simulate multiple frame updates
    simulateOptimizedProceedAnimation(handler, 16);
    ASSERT_EQ(1, handler.callback_count);
    int saved_next_time = handler.next_time;

    simulateOptimizedProceedAnimation(handler, 32);
    ASSERT_EQ(2, handler.callback_count);
    ASSERT_GT(handler.next_time, saved_next_time);

    simulateOptimizedProceedAnimation(handler, 48);
    ASSERT_EQ(3, handler.callback_count);

    return true;
}

//==============================================================================
// Stress Tests
//==============================================================================

TEST_CASE(LuaAnimation_ManyIterations) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16;

    // Simulate 60 seconds of gameplay at 60fps
    // Without optimization, this would call callback ~3600 times
    // With optimization, should be much fewer
    for (int frame = 0; frame < 3600; frame++) {
        int current_time = frame * 16;
        simulateOptimizedProceedAnimation(handler, current_time);
    }

    // Each frame should trigger exactly one callback
    ASSERT_EQ(3600, handler.callback_count);
    return true;
}

TEST_CASE(LuaAnimation_CatchUpAfterPause) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16;

    // Normal frame at t=0
    simulateOptimizedProceedAnimation(handler, 0);
    ASSERT_EQ(1, handler.callback_count);

    // Simulate a long pause (e.g., game minimized for 5 seconds)
    simulateOptimizedProceedAnimation(handler, 5000);

    // Should only call callback once, not 312 times (5000/16)
    ASSERT_EQ(2, handler.callback_count);

    return true;
}

//==============================================================================
// Integration-like Tests
//==============================================================================

TEST_CASE(LuaAnimation_RealisticGameLoop) {
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 0;
    handler.duration_time = 16; // ~60fps animation

    int total_callbacks = 0;
    int current_time = 0;

    // Simulate 1 second of gameplay with variable frame times
    int frame_times[] = {16, 17, 15, 16, 18, 14, 16, 33, 16, 16, // First 10 frames
                         16, 16, 16, 16, 16, 16, 16, 16, 16, 16, // Next 10 frames
                         50, 16, 16, 16, 16, 16, 16, 16, 16, 16, // Spike at frame 21
                         16, 16, 16, 16, 16, 16, 16, 16, 16, 16}; // Last 10 frames

    for (int i = 0; i < 40; i++) {
        current_time += frame_times[i];
        int prev_count = handler.callback_count;
        simulateOptimizedProceedAnimation(handler, current_time);
        if (handler.callback_count > prev_count) {
            total_callbacks++;
        }
    }

    // Despite frame time variations, should have reasonable callback count
    ASSERT_GT(total_callbacks, 30);
    ASSERT_LE(total_callbacks, 40);

    return true;
}

TEST_CASE(LuaAnimation_StatePreservation) {
    // Test that animation state is properly maintained across calls
    MockLuaHandler handler;
    handler.is_animatable = true;
    handler.next_time = 100;
    handler.duration_time = 50;

    // Before threshold
    simulateOptimizedProceedAnimation(handler, 50);
    ASSERT_EQ(0, handler.callback_count);
    ASSERT_EQ(100, handler.next_time);

    // At threshold
    simulateOptimizedProceedAnimation(handler, 100);
    ASSERT_EQ(1, handler.callback_count);
    ASSERT_GE(handler.next_time, 150);

    // After threshold
    simulateOptimizedProceedAnimation(handler, 200);
    ASSERT_EQ(2, handler.callback_count);

    return true;
}
