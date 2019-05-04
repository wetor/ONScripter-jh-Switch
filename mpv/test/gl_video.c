#include "test_helpers.h"
#include "video/out/gpu/video.h"

static void test_scale_ambient_lux_limits(void **state) {
    float x;
    x = gl_video_scale_ambient_lux(16.0, 64.0, 2.40, 1.961, 16.0);
    assert_float_equal(x, 2.40f);

    x = gl_video_scale_ambient_lux(16.0, 64.0, 2.40, 1.961, 64.0);
    assert_float_equal(x, 1.961f);
}

static void test_scale_ambient_lux_sign(void **state) {
    float x;
    x = gl_video_scale_ambient_lux(16.0, 64.0, 1.961, 2.40, 64.0);
    assert_float_equal(x, 2.40f);
}

static void test_scale_ambient_lux_clamping(void **state) {
    float x;
    x = gl_video_scale_ambient_lux(16.0, 64.0, 2.40, 1.961, 0.0);
    assert_float_equal(x, 2.40f);
}

static void test_scale_ambient_lux_log10_midpoint(void **state) {
    float x;
    // 32 corresponds to the the midpoint after converting lux to the log10 scale
    x = gl_video_scale_ambient_lux(16.0, 64.0, 2.40, 1.961, 32.0);
    float mid_gamma = (2.40 - 1.961) / 2 + 1.961;
    assert_float_equal(x, mid_gamma);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_scale_ambient_lux_limits),
        cmocka_unit_test(test_scale_ambient_lux_sign),
        cmocka_unit_test(test_scale_ambient_lux_clamping),
        cmocka_unit_test(test_scale_ambient_lux_log10_midpoint),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

