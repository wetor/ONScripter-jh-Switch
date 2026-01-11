#include "test_framework.h"
#include "switch_input_logic.h"

void test_transJoystickAxis_left_stick_left() {
    TEST("transJoystickAxis: left stick pushed left");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = -20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_LEFT, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_left_stick_right() {
    TEST("transJoystickAxis: left stick pushed right");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_RIGHT, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_left_stick_up() {
    TEST("transJoystickAxis: left stick pushed up");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 1;
    jaxis.value = -20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_UP, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_left_stick_down() {
    TEST("transJoystickAxis: left stick pushed down");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 1;
    jaxis.value = 20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_DOWN, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_deadzone() {
    TEST("transJoystickAxis: value in deadzone returns UNKNOWN");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 1000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDLK_UNKNOWN, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_left_is_mouse_mode() {
    TEST("transJoystickAxis: left stick as mouse returns UNKNOWN");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, true);
    
    ASSERT_EQ(SDLK_UNKNOWN, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_right_stick_works_in_mouse_mode() {
    TEST("transJoystickAxis: right stick works even in mouse mode");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 2;
    jaxis.value = 20000;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, true);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_RIGHT, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_keyup_on_release() {
    TEST("transJoystickAxis: key up when stick returns to center");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 20000;
    SwitchInput::transJoystickAxis(jaxis, false);
    
    jaxis.value = 0;
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYUP, result.type);
    ASSERT_EQ(SDLK_RIGHT, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickButton_A() {
    TEST("transJoystickButton: A button maps to RETURN");
    ASSERT_EQ(SDLK_RETURN, SwitchInput::transJoystickButton(0));
    TEST_PASS();
}

void test_transJoystickButton_B() {
    TEST("transJoystickButton: B button maps to RCTRL");
    ASSERT_EQ(SDLK_RCTRL, SwitchInput::transJoystickButton(1));
    TEST_PASS();
}

void test_transJoystickButton_X() {
    TEST("transJoystickButton: X button maps to 'a'");
    ASSERT_EQ(SDLK_a, SwitchInput::transJoystickButton(2));
    TEST_PASS();
}

void test_transJoystickButton_Y() {
    TEST("transJoystickButton: Y button maps to ESCAPE");
    ASSERT_EQ(SDLK_ESCAPE, SwitchInput::transJoystickButton(3));
    TEST_PASS();
}

void test_transJoystickButton_LSTICK() {
    TEST("transJoystickButton: L-stick press maps to F2");
    ASSERT_EQ(SDLK_F2, SwitchInput::transJoystickButton(4));
    TEST_PASS();
}

void test_transJoystickButton_L() {
    TEST("transJoystickButton: L button maps to 'o'");
    ASSERT_EQ(SDLK_o, SwitchInput::transJoystickButton(6));
    TEST_PASS();
}

void test_transJoystickButton_R() {
    TEST("transJoystickButton: R button maps to 's'");
    ASSERT_EQ(SDLK_s, SwitchInput::transJoystickButton(7));
    TEST_PASS();
}

void test_transJoystickButton_PLUS() {
    TEST("transJoystickButton: + button maps to SPACE");
    ASSERT_EQ(SDLK_SPACE, SwitchInput::transJoystickButton(10));
    TEST_PASS();
}

void test_transJoystickButton_MINUS() {
    TEST("transJoystickButton: - button maps to '0'");
    ASSERT_EQ(SDLK_0, SwitchInput::transJoystickButton(11));
    TEST_PASS();
}

void test_transJoystickButton_DPAD() {
    TEST("transJoystickButton: D-pad buttons map correctly");
    ASSERT_EQ(SDLK_LEFT, SwitchInput::transJoystickButton(12));
    ASSERT_EQ(SDLK_UP, SwitchInput::transJoystickButton(13));
    ASSERT_EQ(SDLK_RIGHT, SwitchInput::transJoystickButton(14));
    ASSERT_EQ(SDLK_DOWN, SwitchInput::transJoystickButton(15));
    TEST_PASS();
}

void test_transJoystickButton_out_of_range() {
    TEST("transJoystickButton: out of range returns UNKNOWN");
    ASSERT_EQ(SDLK_UNKNOWN, SwitchInput::transJoystickButton(100));
    TEST_PASS();
}

void test_calculateMouseMove_no_movement_in_deadzone() {
    TEST("calculateMouseMove: no movement when in deadzone");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 1280, 720, 1000, 0);
    
    ASSERT_FALSE(result.moved);
    TEST_PASS();
}

void test_calculateMouseMove_x_positive() {
    TEST("calculateMouseMove: positive X axis movement");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 640, 480, 20000, 0);
    
    ASSERT_TRUE(result.moved);
    ASSERT_TRUE(result.x > 100.0f);
    TEST_PASS();
}

void test_calculateMouseMove_x_negative() {
    TEST("calculateMouseMove: negative X axis movement");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 640, 480, -20000, 0);
    
    ASSERT_TRUE(result.moved);
    ASSERT_TRUE(result.x < 100.0f);
    TEST_PASS();
}

void test_calculateMouseMove_y_positive() {
    TEST("calculateMouseMove: positive Y axis movement");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 640, 480, 20000, 1);
    
    ASSERT_TRUE(result.moved);
    ASSERT_TRUE(result.y > 100.0f);
    TEST_PASS();
}

void test_calculateMouseMove_y_negative() {
    TEST("calculateMouseMove: negative Y axis movement");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 640, 480, -20000, 1);
    
    ASSERT_TRUE(result.moved);
    ASSERT_TRUE(result.y < 100.0f);
    TEST_PASS();
}

void test_calculateMouseMove_right_stick_ignored() {
    TEST("calculateMouseMove: right stick axis ignored");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        100, 100, 640, 480, 640, 480, 20000, 2);
    
    ASSERT_FALSE(result.moved);
    TEST_PASS();
}

void test_calculateMouseMove_screen_scaling() {
    TEST("calculateMouseMove: screen scaling applied correctly");
    
    SwitchInput::MouseMoveResult result = SwitchInput::calculateMouseMove(
        320, 240, 640, 480, 1280, 960, 0, 0);
    
    ASSERT_EQ_FLOAT(640.0f, result.x, 0.1f);
    ASSERT_EQ_FLOAT(480.0f, result.y, 0.1f);
    TEST_PASS();
}

namespace SwitchConstants {
    const int DEADZONE_THRESHOLD = 3200;
    const int SWITCH_BUTTON_A = 0;
    const int SWITCH_BUTTON_B = 1;
    const int SWITCH_BUTTON_RSTICK = 5;
    const int SWITCH_BUTTON_ZL = 8;
    const int SWITCH_BUTTON_ZR = 9;
    const int SWITCH_BUTTON_PLUS = 10;
    const int SWITCH_BUTTON_MINUS = 11;
    const int SWITCH_AXIS_LEFT_X = 0;
    const int SWITCH_AXIS_LEFT_Y = 1;
    const int SWITCH_AXIS_RIGHT_X = 2;
    const int SWITCH_AXIS_RIGHT_Y = 3;
}

void test_button_constants_defined() {
    TEST("Switch button constants are properly defined");
    ASSERT_EQ(0, SwitchConstants::SWITCH_BUTTON_A);
    ASSERT_EQ(1, SwitchConstants::SWITCH_BUTTON_B);
    ASSERT_EQ(10, SwitchConstants::SWITCH_BUTTON_PLUS);
    ASSERT_EQ(11, SwitchConstants::SWITCH_BUTTON_MINUS);
    TEST_PASS();
}

void test_axis_constants_defined() {
    TEST("Switch axis constants are properly defined");
    ASSERT_EQ(0, SwitchConstants::SWITCH_AXIS_LEFT_X);
    ASSERT_EQ(1, SwitchConstants::SWITCH_AXIS_LEFT_Y);
    ASSERT_EQ(2, SwitchConstants::SWITCH_AXIS_RIGHT_X);
    ASSERT_EQ(3, SwitchConstants::SWITCH_AXIS_RIGHT_Y);
    TEST_PASS();
}

void test_deadzone_threshold() {
    TEST("Deadzone threshold is 3200");
    ASSERT_EQ(3200, SwitchConstants::DEADZONE_THRESHOLD);
    TEST_PASS();
}

void test_rstick_button_unmapped() {
    TEST("R-stick button is unmapped (UNKNOWN)");
    ASSERT_EQ(SDLK_UNKNOWN, SwitchInput::transJoystickButton(SwitchConstants::SWITCH_BUTTON_RSTICK));
    TEST_PASS();
}

void test_zl_zr_buttons_unmapped() {
    TEST("ZL and ZR buttons are unmapped");
    ASSERT_EQ(SDLK_UNKNOWN, SwitchInput::transJoystickButton(SwitchConstants::SWITCH_BUTTON_ZL));
    ASSERT_EQ(SDLK_UNKNOWN, SwitchInput::transJoystickButton(SwitchConstants::SWITCH_BUTTON_ZR));
    TEST_PASS();
}

void test_transJoystickAxis_boundary_value() {
    TEST("transJoystickAxis: boundary value just outside deadzone");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 3201;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDL_KEYDOWN, result.type);
    ASSERT_EQ(SDLK_RIGHT, result.keysym.sym);
    TEST_PASS();
}

void test_transJoystickAxis_boundary_value_inside_deadzone() {
    TEST("transJoystickAxis: boundary value just inside deadzone");
    SwitchInput::resetAxisState();
    
    SDL_JoyAxisEvent jaxis;
    jaxis.axis = 0;
    jaxis.value = 3199;
    
    SDL_KeyboardEvent result = SwitchInput::transJoystickAxis(jaxis, false);
    
    ASSERT_EQ(SDLK_UNKNOWN, result.keysym.sym);
    TEST_PASS();
}

void run_axis_tests() {
    TEST_SUITE_BEGIN("Joystick Axis Tests");
    
    test_transJoystickAxis_left_stick_left();
    test_transJoystickAxis_left_stick_right();
    test_transJoystickAxis_left_stick_up();
    test_transJoystickAxis_left_stick_down();
    test_transJoystickAxis_deadzone();
    test_transJoystickAxis_left_is_mouse_mode();
    test_transJoystickAxis_right_stick_works_in_mouse_mode();
    test_transJoystickAxis_keyup_on_release();
    
    TEST_SUITE_END();
}

void run_button_tests() {
    TEST_SUITE_BEGIN("Joystick Button Tests");
    
    test_transJoystickButton_A();
    test_transJoystickButton_B();
    test_transJoystickButton_X();
    test_transJoystickButton_Y();
    test_transJoystickButton_LSTICK();
    test_transJoystickButton_L();
    test_transJoystickButton_R();
    test_transJoystickButton_PLUS();
    test_transJoystickButton_MINUS();
    test_transJoystickButton_DPAD();
    test_transJoystickButton_out_of_range();
    
    TEST_SUITE_END();
}

void run_mouse_move_tests() {
    TEST_SUITE_BEGIN("Mouse Move Calculation Tests");
    
    test_calculateMouseMove_no_movement_in_deadzone();
    test_calculateMouseMove_x_positive();
    test_calculateMouseMove_x_negative();
    test_calculateMouseMove_y_positive();
    test_calculateMouseMove_y_negative();
    test_calculateMouseMove_right_stick_ignored();
    test_calculateMouseMove_screen_scaling();
    
    TEST_SUITE_END();
}

void run_constants_tests() {
    TEST_SUITE_BEGIN("Switch Constants Tests");
    
    test_button_constants_defined();
    test_axis_constants_defined();
    test_deadzone_threshold();
    test_rstick_button_unmapped();
    test_zl_zr_buttons_unmapped();
    test_transJoystickAxis_boundary_value();
    test_transJoystickAxis_boundary_value_inside_deadzone();
    
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  ONScripter-Switch Unit Tests\n");
    printf("========================================\n");
    
    run_axis_tests();
    run_button_tests();
    run_mouse_move_tests();
    run_constants_tests();
    
    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");
    
    return get_test_result();
}
