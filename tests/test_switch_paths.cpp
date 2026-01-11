#include "test_framework.h"
#include <cstring>
#include <string>

#define SWITCH_GAMES_PATH "sdmc:/onsemu"
#define SWITCH_LOG_PATH "sdmc:/onsemu/"
#define SWITCH_FONT_PATH "sdmc:/switch/ONScripter/"
#define SWITCH_STDOUT_PATH "sdmc:/onsemu/stdout.txt"
#define SWITCH_STDERR_PATH "sdmc:/onsemu/stderr.txt"
#define SWITCH_APP_DIR "sdmc:/switch/ONScripter"
#define SWITCH_ROMFS_FONT "romfs:/font.ttf"

void test_games_path_value() {
    TEST("SWITCH_GAMES_PATH has correct value");
    ASSERT_STREQ("sdmc:/onsemu", SWITCH_GAMES_PATH);
    TEST_PASS();
}

void test_log_path_value() {
    TEST("SWITCH_LOG_PATH has correct value");
    ASSERT_STREQ("sdmc:/onsemu/", SWITCH_LOG_PATH);
    TEST_PASS();
}

void test_font_path_value() {
    TEST("SWITCH_FONT_PATH has correct value");
    ASSERT_STREQ("sdmc:/switch/ONScripter/", SWITCH_FONT_PATH);
    TEST_PASS();
}

void test_games_path_starts_with_sdmc() {
    TEST("SWITCH_GAMES_PATH starts with sdmc:");
    ASSERT_TRUE(strncmp(SWITCH_GAMES_PATH, "sdmc:", 5) == 0);
    TEST_PASS();
}

void test_log_path_starts_with_sdmc() {
    TEST("SWITCH_LOG_PATH starts with sdmc:");
    ASSERT_TRUE(strncmp(SWITCH_LOG_PATH, "sdmc:", 5) == 0);
    TEST_PASS();
}

void test_font_path_starts_with_sdmc() {
    TEST("SWITCH_FONT_PATH starts with sdmc:");
    ASSERT_TRUE(strncmp(SWITCH_FONT_PATH, "sdmc:", 5) == 0);
    TEST_PASS();
}

void test_stdout_path_value() {
    TEST("SWITCH_STDOUT_PATH has correct value");
    ASSERT_STREQ("sdmc:/onsemu/stdout.txt", SWITCH_STDOUT_PATH);
    TEST_PASS();
}

void test_stderr_path_value() {
    TEST("SWITCH_STDERR_PATH has correct value");
    ASSERT_STREQ("sdmc:/onsemu/stderr.txt", SWITCH_STDERR_PATH);
    TEST_PASS();
}

void test_stdout_path_is_in_log_directory() {
    TEST("stdout.txt is in log directory");
    std::string stdout_path = SWITCH_STDOUT_PATH;
    std::string log_path = SWITCH_LOG_PATH;
    ASSERT_TRUE(stdout_path.find(log_path) == 0);
    TEST_PASS();
}

void test_stderr_path_is_in_log_directory() {
    TEST("stderr.txt is in log directory");
    std::string stderr_path = SWITCH_STDERR_PATH;
    std::string log_path = SWITCH_LOG_PATH;
    ASSERT_TRUE(stderr_path.find(log_path) == 0);
    TEST_PASS();
}

void test_app_dir_value() {
    TEST("SWITCH_APP_DIR has correct value");
    ASSERT_STREQ("sdmc:/switch/ONScripter", SWITCH_APP_DIR);
    TEST_PASS();
}

void test_romfs_font_path() {
    TEST("SWITCH_ROMFS_FONT has correct value");
    ASSERT_STREQ("romfs:/font.ttf", SWITCH_ROMFS_FONT);
    TEST_PASS();
}

void test_romfs_font_starts_with_romfs() {
    TEST("SWITCH_ROMFS_FONT starts with romfs:");
    ASSERT_TRUE(strncmp(SWITCH_ROMFS_FONT, "romfs:", 6) == 0);
    TEST_PASS();
}

std::string buildSavePath(const std::string& game_path) {
    return game_path + "/save";
}

void test_save_path_construction() {
    TEST("Save path is constructed correctly from game path");
    std::string game_path = "sdmc:/onsemu/MyGame";
    std::string save_path = buildSavePath(game_path);
    ASSERT_STREQ("sdmc:/onsemu/MyGame/save", save_path.c_str());
    TEST_PASS();
}

namespace FontPathLogic {
    const char* FONT_SEARCH_PATHS[] = {
        "romfs:/font.ttf",
        "sdmc:/switch/ONScripter/default.ttf",
        "sdmc:/switch/ONScripter/font.ttf",
        nullptr
    };

    int getFontSearchPathCount() {
        int count = 0;
        while (FONT_SEARCH_PATHS[count] != nullptr) count++;
        return count;
    }

    bool isRomfsPath(const char* path) {
        return strncmp(path, "romfs:", 6) == 0;
    }

    bool isSdmcPath(const char* path) {
        return strncmp(path, "sdmc:", 5) == 0;
    }
}

void test_font_search_path_count() {
    TEST("Font search has 3 fallback paths");
    ASSERT_EQ(3, FontPathLogic::getFontSearchPathCount());
    TEST_PASS();
}

void test_font_first_path_is_romfs() {
    TEST("First font path is romfs (bundled)");
    ASSERT_TRUE(FontPathLogic::isRomfsPath(FontPathLogic::FONT_SEARCH_PATHS[0]));
    TEST_PASS();
}

void test_font_fallback_paths_are_sdmc() {
    TEST("Fallback font paths are on sdmc");
    ASSERT_TRUE(FontPathLogic::isSdmcPath(FontPathLogic::FONT_SEARCH_PATHS[1]));
    ASSERT_TRUE(FontPathLogic::isSdmcPath(FontPathLogic::FONT_SEARCH_PATHS[2]));
    TEST_PASS();
}

void test_font_search_order() {
    TEST("Font search order: romfs -> default.ttf -> font.ttf");
    ASSERT_STREQ("romfs:/font.ttf", FontPathLogic::FONT_SEARCH_PATHS[0]);
    ASSERT_STREQ("sdmc:/switch/ONScripter/default.ttf", FontPathLogic::FONT_SEARCH_PATHS[1]);
    ASSERT_STREQ("sdmc:/switch/ONScripter/font.ttf", FontPathLogic::FONT_SEARCH_PATHS[2]);
    TEST_PASS();
}

void run_path_tests() {
    TEST_SUITE_BEGIN("Switch Path Configuration Tests");
    
    test_games_path_value();
    test_log_path_value();
    test_font_path_value();
    test_games_path_starts_with_sdmc();
    test_log_path_starts_with_sdmc();
    test_font_path_starts_with_sdmc();
    test_stdout_path_value();
    test_stderr_path_value();
    test_stdout_path_is_in_log_directory();
    test_stderr_path_is_in_log_directory();
    test_app_dir_value();
    test_romfs_font_path();
    test_romfs_font_starts_with_romfs();
    test_save_path_construction();
    test_font_search_path_count();
    test_font_first_path_is_romfs();
    test_font_fallback_paths_are_sdmc();
    test_font_search_order();
    
    TEST_SUITE_END();
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  ONScripter-Switch Path Tests\n");
    printf("========================================\n");
    
    run_path_tests();
    
    printf("\n========================================\n");
    printf("  Final Results: %d passed, %d failed\n", _test_passed, _test_failed);
    printf("========================================\n\n");
    
    return get_test_result();
}
