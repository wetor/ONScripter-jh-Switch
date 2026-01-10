# Makefile for ONScripter-jh-Switch Unit Tests
#
# Copyright (C) 2025 ONScripter-jh-Switch contributors
#
# Usage:
#   make           - Build tests for host platform
#   make run       - Build and run tests
#   make clean     - Clean build artifacts
#   make verbose   - Build and run with verbose output

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++14 -Wall -Wextra -g -O0
CXXFLAGS += -I../include

# Test source files
TEST_MAIN := test_main.cpp
TEST_FRAMEWORK := test_framework.h
TEST_SOURCES := test_encoding.cpp \
                test_lua_animation.cpp \
                test_coordinates.cpp \
                test_rendering.cpp \
                test_dirty_rect.cpp \
                test_image_processing.cpp \
                test_animation_info.cpp \
                test_font_layout.cpp \
                test_utf_conversion.cpp

# Output
BUILD_DIR := build
TARGET := $(BUILD_DIR)/run_tests

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS
    CXXFLAGS += -stdlib=libc++
endif
ifeq ($(UNAME_S),Linux)
    # Linux
    LDFLAGS += -lm
endif

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build test executable
$(TARGET): $(TEST_MAIN) $(TEST_SOURCES) $(TEST_FRAMEWORK) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_MAIN) $(LDFLAGS)

# Run tests
run: $(TARGET)
	@echo ""
	@echo "Running tests..."
	@echo ""
	@./$(TARGET)

# Run tests with filter
# Usage: make run-filter FILTER=Encoding
run-filter: $(TARGET)
	@./$(TARGET) $(FILTER)

# Run specific test categories
test-encoding: $(TARGET)
	@./$(TARGET) UTF8
	@./$(TARGET) SJIS
	@./$(TARGET) GBK
	@./$(TARGET) Lua

test-animation: $(TARGET)
	@./$(TARGET) LuaAnimation

test-coordinates: $(TARGET)
	@./$(TARGET) Button
	@./$(TARGET) Touch

test-rendering: $(TARGET)
	@./$(TARGET) Sharpness
	@./$(TARGET) CAS
	@./$(TARGET) Render
	@./$(TARGET) Cursor

test-dirty-rect: $(TARGET)
	@./$(TARGET) DirtyRect

test-image: $(TARGET)
	@./$(TARGET) Image

test-anim-info: $(TARGET)
	@./$(TARGET) AnimInfo

test-font: $(TARGET)
	@./$(TARGET) Font

test-utf: $(TARGET)
	@./$(TARGET) UTF16
	@./$(TARGET) UTF8To

# Verbose build
verbose: CXXFLAGS += -DTEST_VERBOSE
verbose: clean $(TARGET)
	@./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Check for compilation errors without running
check: $(TARGET)
	@echo "Build successful!"

# Count tests
count: $(TARGET)
	@./$(TARGET) 2>&1 | grep -E "^Running:" | wc -l | xargs echo "Total tests:"

# Help target
help:
	@echo "ONScripter-jh-Switch Test Suite"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build tests (default)"
	@echo "  run              - Build and run all tests"
	@echo "  run-filter       - Run tests matching FILTER (e.g., make run-filter FILTER=UTF8)"
	@echo "  test-encoding    - Run encoding tests"
	@echo "  test-animation   - Run Lua animation tests"
	@echo "  test-coordinates - Run button/touch coordinate tests"
	@echo "  test-rendering   - Run rendering/sharpness tests"
	@echo "  test-dirty-rect  - Run dirty rectangle tests"
	@echo "  test-image       - Run image processing tests"
	@echo "  test-anim-info   - Run AnimationInfo tests"
	@echo "  test-font        - Run font layout tests"
	@echo "  test-utf         - Run UTF conversion tests"
	@echo "  clean            - Remove build artifacts"
	@echo "  check            - Verify build without running"
	@echo "  count            - Count total number of tests"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make run"
	@echo "  make run-filter FILTER=UTF8"
	@echo "  make test-encoding"
	@echo "  make test-dirty-rect"

.PHONY: all run run-filter verbose clean check count help
.PHONY: test-encoding test-animation test-coordinates test-rendering
.PHONY: test-dirty-rect test-image test-anim-info test-font test-utf
