/* -*- C++ -*-
 *
 *  test_animation_info.cpp - AnimationInfo tests for ONScripter-jh-Switch
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
// Mock AnimationInfo class (replicates core logic from AnimationInfo.cpp)
//==============================================================================

class MockAnimationInfo {
public:
    bool visible;
    bool is_animatable;
    int num_of_cells;
    int current_cell;
    int direction;      // 1 = forward, -1 = backward
    int loop_mode;      // 0 = loop, 1 = stop at end, 2 = ping-pong, 3 = no animation
    int next_time;
    int* duration_list;

    // Position and size
    int pos_x, pos_y;
    int orig_pos_x, orig_pos_y;
    int width, height;

    // Affine transformation
    int scale_x, scale_y;  // 100 = 100% scale
    int rot;               // rotation in degrees

    MockAnimationInfo()
        : visible(true), is_animatable(false), num_of_cells(1),
          current_cell(0), direction(1), loop_mode(0), next_time(0),
          duration_list(nullptr), pos_x(0), pos_y(0), orig_pos_x(0), orig_pos_y(0),
          width(0), height(0), scale_x(100), scale_y(100), rot(0) {}

    ~MockAnimationInfo() {
        if (duration_list) delete[] duration_list;
    }

    void setNumOfCells(int num) {
        if (duration_list) delete[] duration_list;
        num_of_cells = num;
        duration_list = new int[num];
        for (int i = 0; i < num; i++) {
            duration_list[i] = 100;  // Default 100ms per frame
        }
    }

    void setCell(int cell) {
        if (cell < 0) cell = 0;
        else if (cell >= num_of_cells) cell = num_of_cells - 1;
        current_cell = cell;
    }

    bool proceedAnimation(int current_time) {
        if (!visible || !is_animatable || next_time > current_time) return false;
        if (!duration_list || num_of_cells <= 0) return false;

        bool is_changed = false;

        while (next_time <= current_time) {
            if (loop_mode != 3 && num_of_cells > 0) {
                current_cell += direction;
                is_changed = true;
            }

            if (current_cell < 0) {
                // loop_mode must be 2 (ping-pong)
                if (num_of_cells == 1)
                    current_cell = 0;
                else
                    current_cell = 1;
                direction = 1;
            }
            else if (current_cell >= num_of_cells) {
                if (loop_mode == 0) {
                    // Loop back to start
                    current_cell = 0;
                }
                else if (loop_mode == 1) {
                    // Stop at end
                    current_cell = num_of_cells - 1;
                    is_changed = false;
                }
                else {
                    // Ping-pong
                    current_cell = num_of_cells - 2;
                    if (current_cell < 0) current_cell = 0;
                    direction = -1;
                }
            }

            next_time += duration_list[current_cell];

            if (duration_list[current_cell] <= 0) {
                next_time = current_time;
                break;
            }
        }

        return is_changed;
    }

    // Calculate if within visible bounds
    bool isVisible(int screen_w, int screen_h) const {
        if (!visible) return false;
        if (pos_x + width <= 0 || pos_x >= screen_w) return false;
        if (pos_y + height <= 0 || pos_y >= screen_h) return false;
        return true;
    }
};

//==============================================================================
// Basic Cell Management Tests
//==============================================================================

TEST_CASE(AnimInfo_DefaultState) {
    MockAnimationInfo anim;

    ASSERT_TRUE(anim.visible);
    ASSERT_FALSE(anim.is_animatable);
    ASSERT_EQ(1, anim.num_of_cells);
    ASSERT_EQ(0, anim.current_cell);
    ASSERT_EQ(1, anim.direction);
    ASSERT_EQ(0, anim.loop_mode);

    return true;
}

TEST_CASE(AnimInfo_SetNumCells) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);

    ASSERT_EQ(5, anim.num_of_cells);
    ASSERT_TRUE(anim.duration_list != nullptr);

    // Check default duration
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(100, anim.duration_list[i]);
    }

    return true;
}

TEST_CASE(AnimInfo_SetCell_Valid) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);

    anim.setCell(3);
    ASSERT_EQ(3, anim.current_cell);

    anim.setCell(0);
    ASSERT_EQ(0, anim.current_cell);

    anim.setCell(4);
    ASSERT_EQ(4, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_SetCell_ClampNegative) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);

    anim.setCell(-1);
    ASSERT_EQ(0, anim.current_cell);

    anim.setCell(-100);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_SetCell_ClampOverflow) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);

    anim.setCell(5);
    ASSERT_EQ(4, anim.current_cell);

    anim.setCell(100);
    ASSERT_EQ(4, anim.current_cell);

    return true;
}

//==============================================================================
// Loop Mode Tests
// Note: while(next_time <= current_time) means when time exactly matches,
// another frame advances. Use t-1 to advance single frames.
//==============================================================================

TEST_CASE(AnimInfo_Loop_LoopMode) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 0;  // Loop
    anim.current_cell = 0;
    anim.next_time = 0;

    // Frame 0 -> 1 (call at t=99, next_time becomes 100)
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);

    // Frame 1 -> 2 (call at t=199)
    anim.proceedAnimation(199);
    ASSERT_EQ(2, anim.current_cell);

    // Frame 2 -> 0 (loop, call at t=299)
    anim.proceedAnimation(299);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Loop_StopMode) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 1;  // Stop at end
    anim.current_cell = 0;
    anim.next_time = 0;

    // Advance to end
    anim.proceedAnimation(99);
    anim.proceedAnimation(199);
    anim.proceedAnimation(299);

    // Should stop at last frame
    ASSERT_EQ(2, anim.current_cell);

    // Further advances should not change
    bool changed = anim.proceedAnimation(399);
    ASSERT_FALSE(changed);
    ASSERT_EQ(2, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Loop_PingPong) {
    MockAnimationInfo anim;
    anim.setNumOfCells(4);
    anim.is_animatable = true;
    anim.loop_mode = 2;  // Ping-pong
    anim.current_cell = 0;
    anim.next_time = 0;

    // Use times that don't exactly match next_time to advance single frames
    // Forward: 0 -> 1 -> 2 -> 3
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);
    ASSERT_EQ(1, anim.direction);

    anim.proceedAnimation(199);
    ASSERT_EQ(2, anim.current_cell);

    anim.proceedAnimation(299);
    ASSERT_EQ(3, anim.current_cell);

    // Hit end, reverse: 3 -> 2 (direction becomes -1)
    anim.proceedAnimation(399);
    ASSERT_EQ(2, anim.current_cell);
    ASSERT_EQ(-1, anim.direction);

    // Continue reverse: 2 -> 1
    anim.proceedAnimation(499);
    ASSERT_EQ(1, anim.current_cell);

    // 1 -> 0
    anim.proceedAnimation(599);
    ASSERT_EQ(0, anim.current_cell);

    // Hit start, forward again: 0 -> 1
    anim.proceedAnimation(699);
    ASSERT_EQ(1, anim.current_cell);
    ASSERT_EQ(1, anim.direction);

    return true;
}

TEST_CASE(AnimInfo_Loop_NoAnimation) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 3;  // No animation
    anim.current_cell = 1;
    anim.next_time = 0;

    // Should not change cell
    anim.proceedAnimation(100);
    ASSERT_EQ(1, anim.current_cell);

    anim.proceedAnimation(1000);
    ASSERT_EQ(1, anim.current_cell);

    return true;
}

//==============================================================================
// Timing Tests
//==============================================================================

TEST_CASE(AnimInfo_Timing_NotYetTime) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.next_time = 100;
    anim.current_cell = 0;

    bool changed = anim.proceedAnimation(50);
    ASSERT_FALSE(changed);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Timing_ExactTime) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.next_time = 100;
    anim.current_cell = 0;

    // Exact time match triggers advancement
    bool changed = anim.proceedAnimation(100);
    ASSERT_TRUE(changed);
    ASSERT_EQ(1, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Timing_SkipFrames) {
    MockAnimationInfo anim;
    anim.setNumOfCells(10);
    anim.is_animatable = true;
    anim.next_time = 0;
    anim.current_cell = 0;

    // Skip ahead 500ms - should advance 5 frames (each 100ms)
    anim.proceedAnimation(499);
    ASSERT_EQ(5, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Timing_VariableDuration) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.next_time = 0;
    anim.current_cell = 0;

    // Set variable durations
    anim.duration_list[0] = 50;
    anim.duration_list[1] = 150;
    anim.duration_list[2] = 100;

    // At t=49: next_time=0 <= 49, so advance. Cell becomes 1, next_time += duration_list[1] = 150
    // Then 150 > 49, exit loop
    anim.proceedAnimation(49);
    ASSERT_EQ(1, anim.current_cell);
    ASSERT_EQ(150, anim.next_time);  // Uses NEW cell's duration after advancing

    // At t=249: 150 <= 249, advance. Cell becomes 2, next_time += duration_list[2] = 250
    // Then 250 > 249, exit loop
    anim.proceedAnimation(249);
    ASSERT_EQ(2, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Timing_ZeroDuration) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.next_time = 0;
    anim.current_cell = 0;

    // Set zero duration on cell 1 (the one we advance TO)
    anim.duration_list[0] = 100;
    anim.duration_list[1] = 0;  // Zero duration here
    anim.duration_list[2] = 100;

    // At t=99: advance to cell 1, next_time += duration_list[1] = 0
    // duration_list[1] <= 0, so break immediately
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);

    return true;
}

//==============================================================================
// Visibility Tests
//==============================================================================

TEST_CASE(AnimInfo_NotVisible) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.visible = false;
    anim.next_time = 0;
    anim.current_cell = 0;

    bool changed = anim.proceedAnimation(100);
    ASSERT_FALSE(changed);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_NotAnimatable) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = false;
    anim.next_time = 0;
    anim.current_cell = 0;

    bool changed = anim.proceedAnimation(100);
    ASSERT_FALSE(changed);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_FullyVisible) {
    MockAnimationInfo anim;
    anim.pos_x = 100;
    anim.pos_y = 100;
    anim.width = 200;
    anim.height = 200;

    ASSERT_TRUE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_OutsideLeft) {
    MockAnimationInfo anim;
    anim.pos_x = -200;
    anim.pos_y = 100;
    anim.width = 100;
    anim.height = 100;

    ASSERT_FALSE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_OutsideRight) {
    MockAnimationInfo anim;
    anim.pos_x = 700;
    anim.pos_y = 100;
    anim.width = 100;
    anim.height = 100;

    ASSERT_FALSE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_OutsideTop) {
    MockAnimationInfo anim;
    anim.pos_x = 100;
    anim.pos_y = -200;
    anim.width = 100;
    anim.height = 100;

    ASSERT_FALSE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_OutsideBottom) {
    MockAnimationInfo anim;
    anim.pos_x = 100;
    anim.pos_y = 500;
    anim.width = 100;
    anim.height = 100;

    ASSERT_FALSE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_PartiallyVisible) {
    MockAnimationInfo anim;
    anim.pos_x = -50;
    anim.pos_y = 100;
    anim.width = 100;
    anim.height = 100;

    ASSERT_TRUE(anim.isVisible(640, 480));

    return true;
}

TEST_CASE(AnimInfo_VisibilityBounds_EdgeCase) {
    MockAnimationInfo anim;

    // Exactly at left edge
    anim.pos_x = 0;
    anim.pos_y = 0;
    anim.width = 100;
    anim.height = 100;
    ASSERT_TRUE(anim.isVisible(640, 480));

    // Just touching right edge
    anim.pos_x = 540;
    ASSERT_TRUE(anim.isVisible(640, 480));

    // Exactly at right edge (not visible)
    anim.pos_x = 640;
    ASSERT_FALSE(anim.isVisible(640, 480));

    return true;
}

//==============================================================================
// Single Cell Tests
//==============================================================================

TEST_CASE(AnimInfo_SingleCell_NoChange) {
    MockAnimationInfo anim;
    anim.setNumOfCells(1);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;

    // With one cell, loop mode should stay at 0
    anim.proceedAnimation(100);
    ASSERT_EQ(0, anim.current_cell);

    anim.proceedAnimation(1000);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_SingleCell_PingPong) {
    MockAnimationInfo anim;
    anim.setNumOfCells(1);
    anim.is_animatable = true;
    anim.loop_mode = 2;
    anim.current_cell = 0;
    anim.next_time = 0;

    // With one cell, ping-pong should stay at 0
    anim.proceedAnimation(100);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

//==============================================================================
// Two Cell Animation Tests
//==============================================================================

TEST_CASE(AnimInfo_TwoCell_Loop) {
    MockAnimationInfo anim;
    anim.setNumOfCells(2);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;

    // 0 -> 1
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);

    // 1 -> 0 (loop)
    anim.proceedAnimation(199);
    ASSERT_EQ(0, anim.current_cell);

    // 0 -> 1
    anim.proceedAnimation(299);
    ASSERT_EQ(1, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_TwoCell_PingPong) {
    MockAnimationInfo anim;
    anim.setNumOfCells(2);
    anim.is_animatable = true;
    anim.loop_mode = 2;
    anim.current_cell = 0;
    anim.direction = 1;
    anim.next_time = 0;

    // 0 -> 1
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);

    // 1 -> 0 (ping-pong reverses)
    anim.proceedAnimation(199);
    ASSERT_EQ(0, anim.current_cell);

    // 0 -> 1 (forward again)
    anim.proceedAnimation(299);
    ASSERT_EQ(1, anim.current_cell);

    return true;
}

//==============================================================================
// Stress Tests
//==============================================================================

TEST_CASE(AnimInfo_ManyFrames) {
    MockAnimationInfo anim;
    anim.setNumOfCells(100);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;

    // Simulate advancing in small increments
    for (int t = 99; t <= 9999; t += 100) {
        anim.proceedAnimation(t);
    }

    // After 100 iterations of 100ms each (t=99 to t=9999), we've done 100 frame advances
    // With 100 cells and loop mode, we should be back at cell 0
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_RapidFrameChanges) {
    MockAnimationInfo anim;
    anim.setNumOfCells(10);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;

    // Very fast animation (1ms per frame)
    for (int i = 0; i < 10; i++) {
        anim.duration_list[i] = 1;
    }

    // Advance 100ms - should process many frames
    anim.proceedAnimation(100);

    // After processing, current_cell should be valid
    ASSERT_GE(anim.current_cell, 0);
    ASSERT_LT(anim.current_cell, 10);

    return true;
}

TEST_CASE(AnimInfo_LongDuration) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;

    // Very long frame duration (10 seconds)
    for (int i = 0; i < 3; i++) {
        anim.duration_list[i] = 10000;
    }

    // 5 seconds should advance one frame (since 0 <= 5000)
    anim.proceedAnimation(5000);
    ASSERT_EQ(1, anim.current_cell);

    // 15 seconds should advance to frame 2 (since 10000 <= 15000)
    anim.proceedAnimation(15000);
    ASSERT_EQ(2, anim.current_cell);

    return true;
}

//==============================================================================
// Position and Transform Tests
//==============================================================================

TEST_CASE(AnimInfo_Position_Default) {
    MockAnimationInfo anim;

    ASSERT_EQ(0, anim.pos_x);
    ASSERT_EQ(0, anim.pos_y);
    ASSERT_EQ(0, anim.orig_pos_x);
    ASSERT_EQ(0, anim.orig_pos_y);

    return true;
}

TEST_CASE(AnimInfo_Position_Set) {
    MockAnimationInfo anim;

    anim.pos_x = 100;
    anim.pos_y = 200;
    anim.orig_pos_x = 50;
    anim.orig_pos_y = 75;

    ASSERT_EQ(100, anim.pos_x);
    ASSERT_EQ(200, anim.pos_y);
    ASSERT_EQ(50, anim.orig_pos_x);
    ASSERT_EQ(75, anim.orig_pos_y);

    return true;
}

TEST_CASE(AnimInfo_Scale_Default) {
    MockAnimationInfo anim;

    ASSERT_EQ(100, anim.scale_x);
    ASSERT_EQ(100, anim.scale_y);

    return true;
}

TEST_CASE(AnimInfo_Scale_Double) {
    MockAnimationInfo anim;

    anim.scale_x = 200;
    anim.scale_y = 200;

    ASSERT_EQ(200, anim.scale_x);
    ASSERT_EQ(200, anim.scale_y);

    return true;
}

TEST_CASE(AnimInfo_Scale_Half) {
    MockAnimationInfo anim;

    anim.scale_x = 50;
    anim.scale_y = 50;

    ASSERT_EQ(50, anim.scale_x);
    ASSERT_EQ(50, anim.scale_y);

    return true;
}

TEST_CASE(AnimInfo_Rotation_Default) {
    MockAnimationInfo anim;

    ASSERT_EQ(0, anim.rot);

    return true;
}

TEST_CASE(AnimInfo_Rotation_Values) {
    MockAnimationInfo anim;

    anim.rot = 90;
    ASSERT_EQ(90, anim.rot);

    anim.rot = 180;
    ASSERT_EQ(180, anim.rot);

    anim.rot = 270;
    ASSERT_EQ(270, anim.rot);

    anim.rot = 360;
    ASSERT_EQ(360, anim.rot);

    anim.rot = -90;
    ASSERT_EQ(-90, anim.rot);

    return true;
}

//==============================================================================
// Direction Tests
//==============================================================================

TEST_CASE(AnimInfo_Direction_Forward) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.current_cell = 0;
    anim.next_time = 0;
    anim.direction = 1;

    // Step forward (use t-1 to avoid double-stepping)
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);

    anim.proceedAnimation(199);
    ASSERT_EQ(2, anim.current_cell);

    anim.proceedAnimation(299);
    ASSERT_EQ(3, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Direction_Backward) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);
    anim.is_animatable = true;
    anim.loop_mode = 2;  // Ping-pong allows backward
    anim.current_cell = 4;
    anim.next_time = 0;
    anim.direction = -1;

    // Step backward (use t-1 to avoid double-stepping)
    anim.proceedAnimation(99);
    ASSERT_EQ(3, anim.current_cell);

    anim.proceedAnimation(199);
    ASSERT_EQ(2, anim.current_cell);

    return true;
}

TEST_CASE(AnimInfo_Direction_ReverseAtEnd) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 2;  // Ping-pong
    anim.current_cell = 2;
    anim.next_time = 0;
    anim.direction = 1;

    // At cell 2 going forward, should reverse
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);
    ASSERT_EQ(-1, anim.direction);

    return true;
}

TEST_CASE(AnimInfo_Direction_ReverseAtStart) {
    MockAnimationInfo anim;
    anim.setNumOfCells(3);
    anim.is_animatable = true;
    anim.loop_mode = 2;  // Ping-pong
    anim.current_cell = 0;
    anim.next_time = 0;
    anim.direction = -1;

    // At cell 0 going backward, should reverse
    anim.proceedAnimation(99);
    ASSERT_EQ(1, anim.current_cell);
    ASSERT_EQ(1, anim.direction);

    return true;
}

//==============================================================================
// Edge Cases
//==============================================================================

TEST_CASE(AnimInfo_Empty_NoCells) {
    MockAnimationInfo anim;
    // Don't call setNumOfCells, leave with default 1 cell
    anim.is_animatable = true;
    anim.next_time = 0;

    // Should not crash - duration_list is null
    bool changed = anim.proceedAnimation(100);
    ASSERT_FALSE(changed);
    ASSERT_EQ(0, anim.current_cell);

    return true;
}

//==============================================================================
// Consistency Tests
//==============================================================================

TEST_CASE(AnimInfo_Consistency_CellBounds) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);
    anim.is_animatable = true;
    anim.loop_mode = 0;
    anim.next_time = 0;

    // Run many iterations
    for (int t = 99; t <= 10000; t += 100) {
        anim.proceedAnimation(t);

        // current_cell should always be valid
        ASSERT_GE(anim.current_cell, 0);
        ASSERT_LT(anim.current_cell, anim.num_of_cells);
    }

    return true;
}

TEST_CASE(AnimInfo_Consistency_PingPongBounds) {
    MockAnimationInfo anim;
    anim.setNumOfCells(5);
    anim.is_animatable = true;
    anim.loop_mode = 2;  // Ping-pong
    anim.next_time = 0;

    // Run many iterations
    for (int t = 99; t <= 10000; t += 100) {
        anim.proceedAnimation(t);

        // current_cell should always be valid
        ASSERT_GE(anim.current_cell, 0);
        ASSERT_LT(anim.current_cell, anim.num_of_cells);
    }

    return true;
}
