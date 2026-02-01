#include "test_framework.h"
#include "screen_logic.h"
#include <climits>

using namespace ScreenLogic;

void test_calculateScaleRatio_zero_screen_width() {
    TEST("calculateScaleRatio: zero screen width returns inf-handled value");
    float ratio = calculateScaleRatio(0, 480, 1280, 720);
    ASSERT_FALSE(ratio <= 0.0f);
    TEST_PASS();
}

void test_calculateScaleRatio_negative_dimensions() {
    TEST("calculateScaleRatio: negative dimensions");
    float ratio = calculateScaleRatio(-640, -480, 1280, 720);
    ASSERT_FALSE(ratio >= 0.0f);
    TEST_PASS();
}

void test_calculateScaleRatio_zero_device() {
    TEST("calculateScaleRatio: zero device dimensions");
    float ratio = calculateScaleRatio(640, 480, 0, 0);
    ASSERT_EQ_FLOAT(0.0f, ratio, 0.01f);
    TEST_PASS();
}

void test_calculateRenderRect_extreme_aspect() {
    TEST("calculateRenderRect: extreme aspect ratio");
    Rect rect = calculateRenderRect(640, 480, 100, 1000, false);
    ASSERT_EQ(0, rect.x);
    ASSERT_TRUE(rect.w <= 100);
    TEST_PASS();
}

void test_deviceToScreenX_overflow() {
    TEST("deviceToScreenX: coordinate overflow");
    int x = deviceToScreenX(INT_MAX, 640, 160, 960);
    ASSERT_GE(x, 0);
    ASSERT_LT(x, 640);
    TEST_PASS();
}

void test_deviceToScreenX_negative() {
    TEST("deviceToScreenX: negative coordinate");
    int x = deviceToScreenX(-100, 640, 160, 960);
    ASSERT_EQ(0, x);
    TEST_PASS();
}

void test_deviceToScreenY_negative() {
    TEST("deviceToScreenY: negative coordinate");
    int y = deviceToScreenY(-50, 480, 0, 720);
    ASSERT_EQ(0, y);
    TEST_PASS();
}

void test_screenToDeviceX_negative_screen() {
    TEST("screenToDeviceX: negative screen coordinate wraps");
    int x = screenToDeviceX(-10, 640, 160, 960);
    ASSERT_EQ(145, x);
    TEST_PASS();
}

void test_screenToDeviceY_negative_screen() {
    TEST("screenToDeviceY: negative screen coordinate");
    int y = screenToDeviceY(-5, 480, 0, 720);
    ASSERT_EQ(-7, y);
    TEST_PASS();
}

void test_clampMouseX_zero_width() {
    TEST("clampMouseX: zero screen width");
    ASSERT_EQ(-1, clampMouseX(100, 0));
    TEST_PASS();
}

void test_clampMouseY_zero_height() {
    TEST("clampMouseY: zero screen height");
    ASSERT_EQ(-1, clampMouseY(100, 0));
    TEST_PASS();
}

void test_clampMouseX_negative_width() {
    TEST("clampMouseX: negative screen width");
    ASSERT_EQ(-101, clampMouseX(100, -100));
    TEST_PASS();
}

void test_calculateScaleRatio_very_large_device() {
    TEST("calculateScaleRatio: very large device resolution");
    float ratio = calculateScaleRatio(640, 480, 7680, 4320);
    ASSERT_GT(ratio, 0.0f);
    TEST_PASS();
}

void run_edge_case_tests() {
    TEST_SUITE_BEGIN("Screen Scaling Edge Cases");
    test_calculateScaleRatio_zero_screen_width();
    test_calculateScaleRatio_negative_dimensions();
    test_calculateScaleRatio_zero_device();
    test_calculateRenderRect_extreme_aspect();
    test_deviceToScreenX_overflow();
    test_deviceToScreenX_negative();
    test_deviceToScreenY_negative();
    test_screenToDeviceX_negative_screen();
    test_screenToDeviceY_negative_screen();
    test_clampMouseX_zero_width();
    test_clampMouseY_zero_height();
    test_clampMouseX_negative_width();
    test_calculateScaleRatio_very_large_device();
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Screen Scaling Edge Cases Tests\n");
    printf("========================================\n");

    run_edge_case_tests();

    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");

    return get_test_result();
}
