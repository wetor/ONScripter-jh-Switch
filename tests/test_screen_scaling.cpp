#include "test_framework.h"
#include "screen_logic.h"

using namespace ScreenLogic;

void test_calculateScaleRatio_same_aspect() {
    TEST("calculateScaleRatio: same aspect ratio");
    float ratio = calculateScaleRatio(640, 480, 1280, 960);
    ASSERT_EQ_FLOAT(2.0f, ratio, 0.01f);
    TEST_PASS();
}

void test_calculateScaleRatio_wider_device() {
    TEST("calculateScaleRatio: wider device (pillarbox)");
    float ratio = calculateScaleRatio(640, 480, 1920, 720);
    ASSERT_EQ_FLOAT(1.5f, ratio, 0.01f);
    TEST_PASS();
}

void test_calculateScaleRatio_taller_device() {
    TEST("calculateScaleRatio: taller device (letterbox)");
    float ratio = calculateScaleRatio(640, 480, 640, 720);
    ASSERT_EQ_FLOAT(1.0f, ratio, 0.01f);
    TEST_PASS();
}

void test_calculateScaleRatio_switch_native() {
    TEST("calculateScaleRatio: Switch 1280x720 with 640x480 game");
    float ratio = calculateScaleRatio(640, 480, 1280, 720);
    ASSERT_EQ_FLOAT(1.5f, ratio, 0.01f);
    TEST_PASS();
}

void test_calculateRenderRect_stretch() {
    TEST("calculateRenderRect: stretch mode fills screen");
    Rect rect = calculateRenderRect(640, 480, 1280, 720, true);
    ASSERT_EQ(0, rect.x);
    ASSERT_EQ(0, rect.y);
    ASSERT_EQ(1280, rect.w);
    ASSERT_EQ(720, rect.h);
    TEST_PASS();
}

void test_calculateRenderRect_letterbox() {
    TEST("calculateRenderRect: letterbox (game is wider than device)");
    // Game 16:9 (1.78), Device 4:3 (1.33) - game is wider, so letterbox (bars on top/bottom)
    Rect rect = calculateRenderRect(1280, 720, 1024, 768, false);
    ASSERT_TRUE(rect.y > 0);  // Black bars on top/bottom
    ASSERT_EQ(0, rect.x);     // No horizontal offset
    ASSERT_EQ(1024, rect.w);  // Full width
    TEST_PASS();
}

void test_calculateRenderRect_pillarbox() {
    TEST("calculateRenderRect: pillarbox (game is taller)");
    Rect rect = calculateRenderRect(640, 480, 1280, 720, false);
    ASSERT_TRUE(rect.x > 0);
    ASSERT_EQ(0, rect.y);
    ASSERT_EQ(720, rect.h);
    TEST_PASS();
}

void test_deviceToScreenX_center() {
    TEST("deviceToScreenX: center of render area");
    int x = deviceToScreenX(640, 640, 160, 960);
    ASSERT_EQ(320, x);
    TEST_PASS();
}

void test_deviceToScreenX_left_edge() {
    TEST("deviceToScreenX: left of render area clamps to 0");
    int x = deviceToScreenX(100, 640, 160, 960);
    ASSERT_EQ(0, x);
    TEST_PASS();
}

void test_deviceToScreenX_right_edge() {
    TEST("deviceToScreenX: right of render area clamps to max");
    int x = deviceToScreenX(1200, 640, 160, 960);
    ASSERT_EQ(639, x);
    TEST_PASS();
}

void test_deviceToScreenY_center() {
    TEST("deviceToScreenY: center of render area");
    int y = deviceToScreenY(360, 480, 0, 720);
    ASSERT_EQ(240, y);
    TEST_PASS();
}

void test_screenToDeviceX() {
    TEST("screenToDeviceX: converts correctly");
    int x = screenToDeviceX(320, 640, 160, 960);
    ASSERT_EQ(640, x);
    TEST_PASS();
}

void test_screenToDeviceY() {
    TEST("screenToDeviceY: converts correctly");
    int y = screenToDeviceY(240, 480, 0, 720);
    ASSERT_EQ(360, y);
    TEST_PASS();
}

void test_clampMouseX() {
    TEST("clampMouseX: clamps correctly");
    ASSERT_EQ(0, clampMouseX(-10, 640));
    ASSERT_EQ(320, clampMouseX(320, 640));
    ASSERT_EQ(639, clampMouseX(700, 640));
    TEST_PASS();
}

void test_clampMouseY() {
    TEST("clampMouseY: clamps correctly");
    ASSERT_EQ(0, clampMouseY(-10, 480));
    ASSERT_EQ(240, clampMouseY(240, 480));
    ASSERT_EQ(479, clampMouseY(500, 480));
    TEST_PASS();
}

void test_switchDefaults() {
    TEST("Switch default resolution");
    ASSERT_EQ(1280, getDefaultSwitchWidth());
    ASSERT_EQ(720, getDefaultSwitchHeight());
    TEST_PASS();
}

void run_scale_ratio_tests() {
    TEST_SUITE_BEGIN("Scale Ratio Tests");
    test_calculateScaleRatio_same_aspect();
    test_calculateScaleRatio_wider_device();
    test_calculateScaleRatio_taller_device();
    test_calculateScaleRatio_switch_native();
    TEST_SUITE_END();
}

void run_render_rect_tests() {
    TEST_SUITE_BEGIN("Render Rectangle Tests");
    test_calculateRenderRect_stretch();
    test_calculateRenderRect_letterbox();
    test_calculateRenderRect_pillarbox();
    TEST_SUITE_END();
}

void run_coordinate_tests() {
    TEST_SUITE_BEGIN("Coordinate Conversion Tests");
    test_deviceToScreenX_center();
    test_deviceToScreenX_left_edge();
    test_deviceToScreenX_right_edge();
    test_deviceToScreenY_center();
    test_screenToDeviceX();
    test_screenToDeviceY();
    test_clampMouseX();
    test_clampMouseY();
    TEST_SUITE_END();
}

void run_defaults_tests() {
    TEST_SUITE_BEGIN("Switch Defaults Tests");
    test_switchDefaults();
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Screen Scaling Unit Tests\n");
    printf("========================================\n");
    
    run_scale_ratio_tests();
    run_render_rect_tests();
    run_coordinate_tests();
    run_defaults_tests();
    
    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");
    
    return get_test_result();
}
