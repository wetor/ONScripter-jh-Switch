#include "test_framework.h"
#include <cstring>

namespace TestUtils {

inline int min(int a, int b) {
    return a < b ? a : b;
}

inline int max(int a, int b) {
    return a > b ? a : b;
}

inline int clamp(int x, int min_val, int max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

}

void test_min_function() {
    TEST("min function returns smaller value");
    ASSERT_EQ(5, TestUtils::min(5, 10));
    ASSERT_EQ(5, TestUtils::min(10, 5));
    ASSERT_EQ(-10, TestUtils::min(-10, -5));
    ASSERT_EQ(5, TestUtils::min(5, 5));
    TEST_PASS();
}

void test_max_function() {
    TEST("max function returns larger value");
    ASSERT_EQ(10, TestUtils::max(5, 10));
    ASSERT_EQ(10, TestUtils::max(10, 5));
    ASSERT_EQ(-5, TestUtils::max(-10, -5));
    ASSERT_EQ(5, TestUtils::max(5, 5));
    TEST_PASS();
}

void test_clamp_below_min() {
    TEST("clamp value below minimum");
    ASSERT_EQ(10, TestUtils::clamp(0, 10, 100));
    ASSERT_EQ(10, TestUtils::clamp(-50, 10, 100));
    TEST_PASS();
}

void test_clamp_above_max() {
    TEST("clamp value above maximum");
    ASSERT_EQ(100, TestUtils::clamp(150, 10, 100));
    ASSERT_EQ(100, TestUtils::clamp(1000, 10, 100));
    TEST_PASS();
}

void test_clamp_within_range() {
    TEST("clamp value within range");
    ASSERT_EQ(50, TestUtils::clamp(50, 10, 100));
    ASSERT_EQ(11, TestUtils::clamp(11, 10, 100));
    ASSERT_EQ(99, TestUtils::clamp(99, 10, 100));
    TEST_PASS();
}

void test_clamp_at_boundaries() {
    TEST("clamp value at boundaries");
    ASSERT_EQ(10, TestUtils::clamp(10, 10, 100));
    ASSERT_EQ(100, TestUtils::clamp(100, 10, 100));
    TEST_PASS();
}

void run_min_max_tests() {
    TEST_SUITE_BEGIN("Min/Max Function Tests");
    test_min_function();
    test_max_function();
    TEST_SUITE_END();
}

void run_clamp_tests() {
    TEST_SUITE_BEGIN("Clamp Function Tests");
    test_clamp_below_min();
    test_clamp_above_max();
    test_clamp_within_range();
    test_clamp_at_boundaries();
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Utils Unit Tests\n");
    printf("========================================\n");

    run_min_max_tests();
    run_clamp_tests();

    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");

    return get_test_result();
}
