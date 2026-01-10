/* -*- C++ -*-
 *
 *  test_framework.h - Minimal test framework for ONScripter-jh-Switch
 *
 *  Copyright (C) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_FRAMEWORK_H__
#define __TEST_FRAMEWORK_H__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <functional>

namespace TestFramework {

// Test result structure
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

// Global test registry
static std::vector<std::pair<std::string, std::function<bool()>>> g_tests;
static std::vector<TestResult> g_results;
static int g_assertions_passed = 0;
static int g_assertions_failed = 0;

// Colors for terminal output (disabled on Switch)
#if defined(SWITCH)
    #define COLOR_RED ""
    #define COLOR_GREEN ""
    #define COLOR_YELLOW ""
    #define COLOR_RESET ""
#else
    #define COLOR_RED "\033[31m"
    #define COLOR_GREEN "\033[32m"
    #define COLOR_YELLOW "\033[33m"
    #define COLOR_RESET "\033[0m"
#endif

// Test registration macro
#define TEST_CASE(name) \
    static bool test_##name(); \
    static bool _test_##name##_registered = TestFramework::registerTest(#name, test_##name); \
    static bool test_##name()

// Assertion macros
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_TRUE(%s)\n", __FILE__, __LINE__, #expr); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_FALSE(expr) \
    do { \
        if (expr) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_FALSE(%s)\n", __FILE__, __LINE__, #expr); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        auto _expected = (expected); \
        auto _actual = (actual); \
        if (_expected != _actual) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_EQ(%s, %s)\n", __FILE__, __LINE__, #expected, #actual); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        auto _expected = (expected); \
        auto _actual = (actual); \
        if (_expected == _actual) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_NE(%s, %s)\n", __FILE__, __LINE__, #expected, #actual); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_LT(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a < _b)) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_LT(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_LE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a <= _b)) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_LE(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_GT(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a > _b)) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_GT(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_GE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a >= _b)) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_GE(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_STREQ(expected, actual) \
    do { \
        const char* _expected = (expected); \
        const char* _actual = (actual); \
        if (_expected == nullptr || _actual == nullptr || strcmp(_expected, _actual) != 0) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_STREQ(\"%s\", \"%s\")\n", \
                   __FILE__, __LINE__, _expected ? _expected : "NULL", _actual ? _actual : "NULL"); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

#define ASSERT_NEAR(expected, actual, epsilon) \
    do { \
        auto _expected = (expected); \
        auto _actual = (actual); \
        auto _epsilon = (epsilon); \
        auto _diff = (_expected > _actual) ? (_expected - _actual) : (_actual - _expected); \
        if (_diff > _epsilon) { \
            printf("  " COLOR_RED "FAIL" COLOR_RESET ": %s:%d: ASSERT_NEAR(%s, %s, %s) diff=%f\n", \
                   __FILE__, __LINE__, #expected, #actual, #epsilon, (double)_diff); \
            TestFramework::g_assertions_failed++; \
            return false; \
        } \
        TestFramework::g_assertions_passed++; \
    } while(0)

// Register a test function
inline bool registerTest(const std::string& name, std::function<bool()> func) {
    g_tests.push_back({name, func});
    return true;
}

// Run all registered tests
inline int runAllTests() {
    printf("\n" COLOR_YELLOW "========================================" COLOR_RESET "\n");
    printf(COLOR_YELLOW "  ONScripter-jh-Switch Test Suite" COLOR_RESET "\n");
    printf(COLOR_YELLOW "========================================" COLOR_RESET "\n\n");

    int passed = 0;
    int failed = 0;

    for (const auto& test : g_tests) {
        printf("Running: %s... ", test.first.c_str());
        fflush(stdout);

        bool result = false;
        try {
            result = test.second();
        } catch (...) {
            printf(COLOR_RED "EXCEPTION" COLOR_RESET "\n");
            failed++;
            g_results.push_back({test.first, false, "Exception thrown"});
            continue;
        }

        if (result) {
            printf(COLOR_GREEN "PASSED" COLOR_RESET "\n");
            passed++;
            g_results.push_back({test.first, true, ""});
        } else {
            printf(COLOR_RED "FAILED" COLOR_RESET "\n");
            failed++;
            g_results.push_back({test.first, false, "Assertion failed"});
        }
    }

    printf("\n" COLOR_YELLOW "========================================" COLOR_RESET "\n");
    printf("  Results: ");
    if (failed == 0) {
        printf(COLOR_GREEN "%d/%d tests passed" COLOR_RESET "\n", passed, passed + failed);
    } else {
        printf(COLOR_RED "%d/%d tests passed" COLOR_RESET "\n", passed, passed + failed);
    }
    printf("  Assertions: %d passed, %d failed\n", g_assertions_passed, g_assertions_failed);
    printf(COLOR_YELLOW "========================================" COLOR_RESET "\n\n");

    return failed;
}

// Run tests matching a filter
inline int runTests(const char* filter) {
    if (filter == nullptr || strlen(filter) == 0) {
        return runAllTests();
    }

    printf("\n" COLOR_YELLOW "Running tests matching: %s" COLOR_RESET "\n\n", filter);

    int passed = 0;
    int failed = 0;

    for (const auto& test : g_tests) {
        if (strstr(test.first.c_str(), filter) == nullptr) {
            continue;
        }

        printf("Running: %s... ", test.first.c_str());
        fflush(stdout);

        bool result = false;
        try {
            result = test.second();
        } catch (...) {
            printf(COLOR_RED "EXCEPTION" COLOR_RESET "\n");
            failed++;
            continue;
        }

        if (result) {
            printf(COLOR_GREEN "PASSED" COLOR_RESET "\n");
            passed++;
        } else {
            printf(COLOR_RED "FAILED" COLOR_RESET "\n");
            failed++;
        }
    }

    printf("\nResults: %d/%d tests passed\n", passed, passed + failed);
    return failed;
}

} // namespace TestFramework

#endif // __TEST_FRAMEWORK_H__
