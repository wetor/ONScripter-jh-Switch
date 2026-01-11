#include "test_framework.h"
#include "game_browser_logic.h"

using namespace GameBrowserLogic;

void test_isValidScriptFile_0txt() {
    TEST("isValidScriptFile: 0.txt is valid");
    ASSERT_TRUE(isValidScriptFile("0.txt"));
    TEST_PASS();
}

void test_isValidScriptFile_00txt() {
    TEST("isValidScriptFile: 00.txt is valid");
    ASSERT_TRUE(isValidScriptFile("00.txt"));
    TEST_PASS();
}

void test_isValidScriptFile_nscript_dat() {
    TEST("isValidScriptFile: nscript.dat is valid");
    ASSERT_TRUE(isValidScriptFile("nscript.dat"));
    TEST_PASS();
}

void test_isValidScriptFile_nscript_underscore() {
    TEST("isValidScriptFile: nscript.___ is valid");
    ASSERT_TRUE(isValidScriptFile("nscript.___"));
    TEST_PASS();
}

void test_isValidScriptFile_nscr_sec_dat() {
    TEST("isValidScriptFile: nscr_sec.dat is valid");
    ASSERT_TRUE(isValidScriptFile("nscr_sec.dat"));
    TEST_PASS();
}

void test_isValidScriptFile_invalid() {
    TEST("isValidScriptFile: random.txt is invalid");
    ASSERT_FALSE(isValidScriptFile("random.txt"));
    TEST_PASS();
}

void test_isValidScriptFile_empty() {
    TEST("isValidScriptFile: empty string is invalid");
    ASSERT_FALSE(isValidScriptFile(""));
    TEST_PASS();
}

void test_sortGamesAlphabetically() {
    TEST("sortGamesAlphabetically: sorts correctly");
    std::vector<GameInfo> games;
    GameInfo g1, g2, g3;
    g1.name = "Zeta";
    g2.name = "Alpha";
    g3.name = "Beta";
    games.push_back(g1);
    games.push_back(g2);
    games.push_back(g3);
    
    sortGamesAlphabetically(games);
    
    ASSERT_STREQ("Alpha", games[0].name.c_str());
    ASSERT_STREQ("Beta", games[1].name.c_str());
    ASSERT_STREQ("Zeta", games[2].name.c_str());
    TEST_PASS();
}

void test_sortGamesAlphabetically_empty() {
    TEST("sortGamesAlphabetically: handles empty list");
    std::vector<GameInfo> games;
    sortGamesAlphabetically(games);
    ASSERT_EQ(0, (int)games.size());
    TEST_PASS();
}

void test_calculateScrollOffset_fits_on_screen() {
    TEST("calculateScrollOffset: no scroll when items fit");
    int offset = calculateScrollOffset(3, 8, 5);
    ASSERT_EQ(0, offset);
    TEST_PASS();
}

void test_calculateScrollOffset_at_start() {
    TEST("calculateScrollOffset: start position");
    int offset = calculateScrollOffset(0, 8, 20);
    ASSERT_EQ(0, offset);
    TEST_PASS();
}

void test_calculateScrollOffset_at_end() {
    TEST("calculateScrollOffset: end position");
    int offset = calculateScrollOffset(19, 8, 20);
    ASSERT_EQ(12, offset);
    TEST_PASS();
}

void test_calculateScrollOffset_middle() {
    TEST("calculateScrollOffset: middle position");
    int offset = calculateScrollOffset(10, 8, 20);
    ASSERT_EQ(6, offset);
    TEST_PASS();
}

void test_clampSelection_empty_list() {
    TEST("clampSelection: empty list returns -1");
    ASSERT_EQ(-1, clampSelection(0, 0));
    TEST_PASS();
}

void test_clampSelection_negative() {
    TEST("clampSelection: negative index clamps to 0");
    ASSERT_EQ(0, clampSelection(-5, 10));
    TEST_PASS();
}

void test_clampSelection_too_large() {
    TEST("clampSelection: too large clamps to last");
    ASSERT_EQ(9, clampSelection(15, 10));
    TEST_PASS();
}

void test_clampSelection_valid() {
    TEST("clampSelection: valid index unchanged");
    ASSERT_EQ(5, clampSelection(5, 10));
    TEST_PASS();
}

void test_moveSelectionWithWrap_down() {
    TEST("moveSelectionWithWrap: move down");
    ASSERT_EQ(3, moveSelectionWithWrap(2, 1, 10));
    TEST_PASS();
}

void test_moveSelectionWithWrap_up() {
    TEST("moveSelectionWithWrap: move up");
    ASSERT_EQ(1, moveSelectionWithWrap(2, -1, 10));
    TEST_PASS();
}

void test_moveSelectionWithWrap_clamp_top() {
    TEST("moveSelectionWithWrap: clamp at top");
    ASSERT_EQ(0, moveSelectionWithWrap(0, -1, 10));
    TEST_PASS();
}

void test_moveSelectionWithWrap_clamp_bottom() {
    TEST("moveSelectionWithWrap: clamp at bottom");
    ASSERT_EQ(9, moveSelectionWithWrap(9, 1, 10));
    TEST_PASS();
}

void test_moveSelectionWithWrap_fast_scroll() {
    TEST("moveSelectionWithWrap: fast scroll");
    ASSERT_EQ(7, moveSelectionWithWrap(2, 5, 10));
    TEST_PASS();
}

void test_calculateTouchedIndex_valid_touch() {
    TEST("calculateTouchedIndex: valid touch");
    int index = calculateTouchedIndex(200, 130, 70, 0, 10);
    ASSERT_EQ(1, index);
    TEST_PASS();
}

void test_calculateTouchedIndex_with_scroll() {
    TEST("calculateTouchedIndex: with scroll offset");
    int index = calculateTouchedIndex(200, 130, 70, 5, 20);
    ASSERT_EQ(6, index);
    TEST_PASS();
}

void test_calculateTouchedIndex_above_list() {
    TEST("calculateTouchedIndex: touch above list");
    int index = calculateTouchedIndex(50, 130, 70, 0, 10);
    ASSERT_EQ(-1, index);
    TEST_PASS();
}

void test_calculateTouchedIndex_out_of_range() {
    TEST("calculateTouchedIndex: touch beyond items");
    int index = calculateTouchedIndex(700, 130, 70, 0, 3);
    ASSERT_EQ(-1, index);
    TEST_PASS();
}

void test_defaultColors() {
    TEST("Default colors have correct values");
    Color bg = getDefaultBackgroundColor();
    ASSERT_EQ(25, bg.r);
    ASSERT_EQ(30, bg.g);
    ASSERT_EQ(40, bg.b);
    ASSERT_EQ(255, bg.a);
    
    Color text = getDefaultTextColor();
    ASSERT_EQ(230, text.r);
    
    Color selected = getDefaultSelectedColor();
    ASSERT_EQ(45, selected.r);
    ASSERT_EQ(130, selected.g);
    ASSERT_EQ(220, selected.b);
    
    Color highlight = getDefaultHighlightColor();
    ASSERT_EQ(255, highlight.r);
    ASSERT_EQ(180, highlight.g);
    ASSERT_EQ(50, highlight.b);
    TEST_PASS();
}

void run_script_file_tests() {
    TEST_SUITE_BEGIN("Script File Validation Tests");
    test_isValidScriptFile_0txt();
    test_isValidScriptFile_00txt();
    test_isValidScriptFile_nscript_dat();
    test_isValidScriptFile_nscript_underscore();
    test_isValidScriptFile_nscr_sec_dat();
    test_isValidScriptFile_invalid();
    test_isValidScriptFile_empty();
    TEST_SUITE_END();
}

void run_sorting_tests() {
    TEST_SUITE_BEGIN("Game Sorting Tests");
    test_sortGamesAlphabetically();
    test_sortGamesAlphabetically_empty();
    TEST_SUITE_END();
}

void run_scroll_tests() {
    TEST_SUITE_BEGIN("Scroll Offset Tests");
    test_calculateScrollOffset_fits_on_screen();
    test_calculateScrollOffset_at_start();
    test_calculateScrollOffset_at_end();
    test_calculateScrollOffset_middle();
    TEST_SUITE_END();
}

void run_selection_tests() {
    TEST_SUITE_BEGIN("Selection Logic Tests");
    test_clampSelection_empty_list();
    test_clampSelection_negative();
    test_clampSelection_too_large();
    test_clampSelection_valid();
    test_moveSelectionWithWrap_down();
    test_moveSelectionWithWrap_up();
    test_moveSelectionWithWrap_clamp_top();
    test_moveSelectionWithWrap_clamp_bottom();
    test_moveSelectionWithWrap_fast_scroll();
    TEST_SUITE_END();
}

void run_touch_tests() {
    TEST_SUITE_BEGIN("Touch Input Tests");
    test_calculateTouchedIndex_valid_touch();
    test_calculateTouchedIndex_with_scroll();
    test_calculateTouchedIndex_above_list();
    test_calculateTouchedIndex_out_of_range();
    TEST_SUITE_END();
}

void run_color_tests() {
    TEST_SUITE_BEGIN("Color Configuration Tests");
    test_defaultColors();
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  GameBrowser Unit Tests\n");
    printf("========================================\n");
    
    run_script_file_tests();
    run_sorting_tests();
    run_scroll_tests();
    run_selection_tests();
    run_touch_tests();
    run_color_tests();
    
    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");
    
    return get_test_result();
}
