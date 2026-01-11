/**
 * Minimal Test Framework for ONScripter-Switch
 * A lightweight header-only test framework
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test result tracking
static int _test_total = 0;
static int _test_passed = 0;
static int _test_failed = 0;
static const char* _current_test_name = NULL;

// Colors for terminal output
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"

// Test macros
#define TEST_SUITE_BEGIN(name) \
    printf("\n" COLOR_YELLOW "=== Test Suite: %s ===" COLOR_RESET "\n", name)

#define TEST_SUITE_END() \
    printf("\n" COLOR_YELLOW "=== Results: %d/%d passed, %d failed ===" COLOR_RESET "\n", \
           _test_passed, _test_total, _test_failed)

#define TEST(name) \
    _current_test_name = name; \
    _test_total++; \
    printf("  Running: %s ... ", name);

#define TEST_PASS() \
    do { \
        _test_passed++; \
        printf(COLOR_GREEN "PASS" COLOR_RESET "\n"); \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        _test_failed++; \
        printf(COLOR_RED "FAIL" COLOR_RESET "\n"); \
        printf("    " COLOR_RED "Error: %s" COLOR_RESET "\n", msg); \
        printf("    at %s:%d\n", __FILE__, __LINE__); \
    } while(0)

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            TEST_FAIL(#cond " is false"); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            TEST_FAIL(#cond " is true"); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            char _msg[256]; \
            snprintf(_msg, sizeof(_msg), "Expected %d, got %d", (int)(expected), (int)(actual)); \
            TEST_FAIL(_msg); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ_FLOAT(expected, actual, epsilon) \
    do { \
        float _diff = (expected) - (actual); \
        if (_diff < 0) _diff = -_diff; \
        if (_diff > (epsilon)) { \
            char _msg[256]; \
            snprintf(_msg, sizeof(_msg), "Expected %f, got %f (diff: %f)", \
                    (float)(expected), (float)(actual), _diff); \
            TEST_FAIL(_msg); \
            return; \
        } \
    } while(0)

#define ASSERT_STREQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            char _msg[256]; \
            snprintf(_msg, sizeof(_msg), "Expected \"%s\", got \"%s\"", (expected), (actual)); \
            TEST_FAIL(_msg); \
            return; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            TEST_FAIL(#ptr " is NULL"); \
            return; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            TEST_FAIL(#ptr " is not NULL"); \
            return; \
        } \
    } while(0)

// Get test results
static inline int get_test_result() {
    return _test_failed == 0 ? 0 : 1;
}

#endif // TEST_FRAMEWORK_H
