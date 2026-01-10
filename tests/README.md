# ONScripter-jh-Switch Unit Tests

This directory contains unit tests for the ONScripter-jh-Switch project, focusing on the bug fixes and features ported from OnscripterYuri.

## Test Categories

### 1. Encoding Tests (`test_encoding.cpp`)

Tests for multi-byte character encoding support:

- **UTF-8 Support**: Tests for the `UTF8_N_BYTE` macro that detects UTF-8 byte sequence lengths (1-4 bytes)
- **Shift-JIS/GBK Detection**: Tests for the `IS_TWO_BYTE` macro that detects double-byte characters
- **Lua Script Processing**: Tests for the backslash escape handling in `NL_dofile` and `loadInitScript`

Key test cases:
- `UTF8_SingleByte_ASCII` - ASCII characters (0x00-0x7F)
- `UTF8_TwoByte_LatinExtended` - Latin extended characters
- `UTF8_ThreeByte_CJK` - Chinese, Japanese, Korean characters
- `UTF8_FourByte_Emoji` - Emoji and special characters
- `LuaEscape_UTF8_*` - Lua script UTF-8 processing
- `LuaEscape_SJIS_*` - Lua script Shift-JIS processing

### 2. Lua Animation Tests (`test_lua_animation.cpp`)

Tests for the optimized Lua animation callback system:

- **Performance Optimization**: Verifies that the optimized version calls callbacks fewer times than the original when catching up
- **Frame Skipping**: Tests the do-while loop that skips multiple frames at once
- **Edge Cases**: Zero duration, negative duration, disabled callbacks

Key test cases:
- `LuaAnimation_Optimized_SkipsMultipleFrames` - Core optimization test
- `LuaAnimation_PerformanceComparison` - Compares original vs optimized
- `LuaAnimation_CatchUpAfterPause` - Tests behavior after long pause
- `LuaAnimation_ZeroDuration` - Edge case handling

### 3. Coordinate Tests (`test_coordinates.cpp`)

Tests for button position and touch coordinate mapping:

- **Button Position (shiftCursorOnButton)**: Tests coordinate transformation with `render_view_rect` offset
- **Touch Input**: Tests touch-to-game coordinate conversion
- **Letterbox/Pillarbox**: Tests coordinate handling with different aspect ratios
- **Bounds Checking**: Tests clamping of out-of-bounds coordinates

Key test cases:
- `Button_WithLetterbox_DockedMode` - 4:3 game on 16:9 screen
- `Button_WithPillarbox_DockedMode` - Portrait game on landscape screen
- `Touch_WithLetterbox_*` - Touch in letterboxed mode
- `Roundtrip_ButtonToTouchAndBack` - Consistency verification

### 4. Rendering Tests (`test_rendering.cpp`)

Tests for rendering parameters and GLES renderer:

- **Sharpness Parameters**: Validates sharpness range (0.0-1.0 or NAN)
- **CAS (Contrast Adaptive Sharpening)**: Tests AMD FidelityFX CAS parameter calculations
- **Render View Rectangle**: Tests viewport calculation for aspect ratio preservation
- **Cursor Double Display Fix**: Tests the `dst_rect.w > dst_rect.h` fix

Key test cases:
- `Sharpness_ValidRange_*` - Parameter validation
- `CAS_Parameter_*` - CAS algorithm tests
- `RenderView_4_3_On_16_9_Letterbox` - Viewport calculation
- `Cursor_WiderThanTall_Fixed` - Double cursor fix

## Building and Running Tests

### Prerequisites

- C++14 compatible compiler (g++, clang++)
- Make

### Build Commands

```bash
# Navigate to tests directory
cd tests

# Build tests
make

# Build and run all tests
make run

# Run tests with filter
make run-filter FILTER=UTF8

# Run specific test categories
make test-encoding
make test-animation
make test-coordinates
make test-rendering

# Clean build artifacts
make clean

# Show help
make help
```

### Running Individual Test Categories

```bash
# Run only encoding tests
./build/run_tests UTF8
./build/run_tests SJIS

# Run only animation tests
./build/run_tests LuaAnimation

# Run only coordinate tests
./build/run_tests Button
./build/run_tests Touch

# Run only rendering tests
./build/run_tests Sharpness
./build/run_tests CAS
```

## Test Framework

The tests use a minimal custom test framework (`test_framework.h`) that provides:

- `TEST_CASE(name)` - Define a test case
- `ASSERT_TRUE(expr)` - Assert expression is true
- `ASSERT_FALSE(expr)` - Assert expression is false
- `ASSERT_EQ(expected, actual)` - Assert equality
- `ASSERT_NE(expected, actual)` - Assert inequality
- `ASSERT_LT(a, b)` - Assert a < b
- `ASSERT_LE(a, b)` - Assert a <= b
- `ASSERT_GT(a, b)` - Assert a > b
- `ASSERT_GE(a, b)` - Assert a >= b
- `ASSERT_NEAR(expected, actual, epsilon)` - Assert approximate equality
- `ASSERT_STREQ(expected, actual)` - Assert string equality

## Adding New Tests

1. Create a new test file (e.g., `test_feature.cpp`)
2. Include the test framework: `#include "test_framework.h"`
3. Write test cases using `TEST_CASE(name) { ... return true; }`
4. Add the file to `TEST_SOURCES` in `Makefile`
5. Include the file in `test_main.cpp`

Example:

```cpp
#include "test_framework.h"

TEST_CASE(MyFeature_BasicTest) {
    int result = myFunction(42);
    ASSERT_EQ(84, result);
    return true;
}
```

## Notes

- Tests are designed to run on the host development machine, not on Switch hardware
- Some tests use mock structures to simulate ONScripter behavior without requiring full initialization
- The test framework auto-registers tests at static initialization time
- Exit code 0 means all tests passed; non-zero indicates failure count

## Related Files

- `source/LUAHandler.cpp` - UTF-8 support in NL_dofile/loadInitScript
- `source/onscripter/ONScripter_animation.cpp` - Lua animation optimization
- `source/onscripter/ONScripter_event.cpp` - Coordinate mapping fixes
- `include/ScriptHandler.h` - UTF8_N_BYTE macro definition
