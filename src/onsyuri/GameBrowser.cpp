/* -*- C++ -*-
 *
 *  GameBrowser.cpp - Game selection browser for ONScripter Switch
 *
 *  Copyright (c) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifdef SWITCH

#include "GameBrowser.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>

GameBrowser::GameBrowser()
    : window_(nullptr)
    , renderer_(nullptr)
    , font_large_(nullptr)
    , font_small_(nullptr)
    , selected_index_(0)
    , scroll_offset_(0)
    , screen_width_(1280)
    , screen_height_(720)
    , items_per_page_(8)
    , running_(false)
    , show_help_(false)
{
    // Initialize colors
    color_background_ = {25, 30, 40, 255};       // Dark blue-gray
    color_text_ = {230, 230, 230, 255};          // Light gray
    color_selected_ = {45, 130, 220, 255};       // Bright blue
    color_disabled_ = {120, 120, 120, 255};      // Gray
    color_highlight_ = {255, 180, 50, 255};      // Orange-gold
}

GameBrowser::~GameBrowser()
{
    cleanup();
}

bool GameBrowser::init(SDL_Window* window, SDL_Renderer* renderer)
{
    printf("GameBrowser: Initializing...\n");

    window_ = window;
    renderer_ = renderer;

    if (!window_ || !renderer_) {
        printf("GameBrowser: Invalid SDL window or renderer\n");
        return false;
    }

    // Get window size
    SDL_GetWindowSize(window_, &screen_width_, &screen_height_);
    printf("GameBrowser: Screen size: %dx%d\n", screen_width_, screen_height_);

    // Initialize SDL_ttf if not already done
    if (!TTF_WasInit()) {
        if (TTF_Init() != 0) {
            printf("GameBrowser: Failed to initialize SDL_ttf: %s\n", TTF_GetError());
            return false;
        }
    }

    // Load fonts
    if (!loadFonts()) {
        printf("GameBrowser: Failed to load fonts\n");
        return false;
    }

    // Initialize gamepad
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad_);

    printf("GameBrowser: Initialized successfully\n");
    return true;
}

bool GameBrowser::loadFonts()
{
    const char* font_paths[] = {
        "romfs:/font.ttf",
        "sdmc:/switch/ONScripter/default.ttf",
        "sdmc:/switch/ONScripter/font.ttf",
        nullptr
    };

    for (int i = 0; font_paths[i] != nullptr; i++) {
        printf("GameBrowser: Trying font: %s\n", font_paths[i]);
        font_large_ = TTF_OpenFont(font_paths[i], 28);
        if (font_large_) {
            font_small_ = TTF_OpenFont(font_paths[i], 20);
            if (font_small_) {
                printf("GameBrowser: Loaded font from: %s\n", font_paths[i]);
                return true;
            } else {
                TTF_CloseFont(font_large_);
                font_large_ = nullptr;
            }
        }
    }

    printf("GameBrowser: No valid font found!\n");
    return false;
}

int GameBrowser::scanGames(const char* base_path)
{
    printf("GameBrowser: Scanning for games in: %s\n", base_path);

    games_.clear();

    DIR* dir = opendir(base_path);
    if (!dir) {
        printf("GameBrowser: Failed to open directory: %s\n", base_path);
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip special directories
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build full path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        // Check if it's a directory
        struct stat st;
        if (stat(full_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
            continue;
        }

        // Check if it's a valid game folder
        GameInfo info;
        if (isValidGameFolder(full_path, info)) {
            info.path = full_path;
            info.name = entry->d_name;
            games_.push_back(info);
            printf("GameBrowser: Found game: %s (%s)\n", info.name.c_str(), info.script_file.c_str());
        }
    }

    closedir(dir);

    // Sort games alphabetically
    std::sort(games_.begin(), games_.end(),
        [](const GameInfo& a, const GameInfo& b) {
            return a.name < b.name;
        });

    printf("GameBrowser: Found %d game(s)\n", (int)games_.size());
    return games_.size();
}

bool GameBrowser::isValidGameFolder(const char* path, GameInfo& info)
{
    // Check for valid script files
    const char* script_files[] = {
        "0.txt",
        "00.txt",
        "nscript.dat",
        "nscript.___",
        "nscr_sec.dat",
        nullptr
    };

    info.has_script = false;
    info.has_font = false;

    for (int i = 0; script_files[i] != nullptr; i++) {
        char script_path[512];
        snprintf(script_path, sizeof(script_path), "%s/%s", path, script_files[i]);

        struct stat st;
        if (stat(script_path, &st) == 0 && S_ISREG(st.st_mode)) {
            info.has_script = true;
            info.script_file = script_files[i];
            break;
        }
    }

    // Check for font file (optional, we have romfs font)
    char font_path[512];
    snprintf(font_path, sizeof(font_path), "%s/default.ttf", path);
    struct stat st;
    if (stat(font_path, &st) == 0 && S_ISREG(st.st_mode)) {
        info.has_font = true;
    }

    return info.has_script;
}

int GameBrowser::run()
{
    if (games_.empty()) {
        printf("GameBrowser: No games found, showing empty screen\n");
        // Still show the UI so user knows what to do
    }

    selected_index_ = 0;
    scroll_offset_ = 0;
    running_ = true;

    printf("GameBrowser: Starting browser loop...\n");

    // Main loop
    while (running_ && appletMainLoop()) {
        handleInput();
        render();

        // Small delay to prevent 100% CPU usage
        SDL_Delay(16); // ~60 FPS
    }

    if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
        printf("GameBrowser: Selected game: %s\n", games_[selected_index_].path.c_str());
        return selected_index_;
    }

    return -1;
}

void GameBrowser::handleInput()
{
    // Scan for input
    padUpdate(&pad_);
    u64 kDown = padGetButtonsDown(&pad_);

    // Toggle help overlay (X button)
    if (kDown & HidNpadButton_X) {
        show_help_ = !show_help_;
        return;
    }

    // If help is showing, any other button closes it
    if (show_help_) {
        if (kDown & (HidNpadButton_A | HidNpadButton_B | HidNpadButton_Plus)) {
            show_help_ = false;
        }
        return;
    }

    // Exit browser (B button or PLUS)
    if (kDown & HidNpadButton_B || kDown & HidNpadButton_Plus) {
        printf("GameBrowser: Cancelled by user\n");
        running_ = false;
        selected_index_ = -1;
        return;
    }

    if (games_.empty()) {
        return;
    }

    // Select game (A button)
    if (kDown & HidNpadButton_A) {
        if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
            printf("GameBrowser: Game selected: %s\n", games_[selected_index_].name.c_str());
            running_ = false;
            return;
        }
    }

    // Navigation
    if (kDown & HidNpadButton_Down || kDown & HidNpadButton_StickLDown) {
        moveSelection(1);
    }
    else if (kDown & HidNpadButton_Up || kDown & HidNpadButton_StickLUp) {
        moveSelection(-1);
    }

    // Fast scroll
    if (kDown & HidNpadButton_R) {
        moveSelection(5);
    }
    else if (kDown & HidNpadButton_L) {
        moveSelection(-5);
    }

    // Page up/down
    if (kDown & HidNpadButton_ZR) {
        moveSelection(items_per_page_);
    }
    else if (kDown & HidNpadButton_ZL) {
        moveSelection(-items_per_page_);
    }

    // Touch screen support
    HidTouchScreenState touch_state;
    memset(&touch_state, 0, sizeof(touch_state));
    if (hidGetTouchScreenStates(&touch_state, 1) > 0) {
        if (touch_state.count > 0) {
            int touch_y = touch_state.touches[0].y;

            // Simple touch detection for game list
            int list_start_y = 130;
            int item_height = 70;

            if (touch_y >= list_start_y && touch_y < list_start_y + items_per_page_ * item_height) {
                int touched_index = (touch_y - list_start_y) / item_height + scroll_offset_;
                if (touched_index >= 0 && touched_index < (int)games_.size()) {
                    selected_index_ = touched_index;
                    // Double tap to select
                    static Uint32 last_touch_time = 0;
                    Uint32 current_time = SDL_GetTicks();
                    if (current_time - last_touch_time < 300) {
                        printf("GameBrowser: Touch selected game: %s\n", games_[selected_index_].name.c_str());
                        running_ = false;
                    }
                    last_touch_time = current_time;
                }
            }
        }
    }
}

void GameBrowser::moveSelection(int delta)
{
    if (games_.empty()) return;

    selected_index_ += delta;

    // Clamp to valid range
    if (selected_index_ < 0) {
        selected_index_ = 0;
    }
    if (selected_index_ >= (int)games_.size()) {
        selected_index_ = games_.size() - 1;
    }

    // Adjust scroll offset
    if (selected_index_ < scroll_offset_) {
        scroll_offset_ = selected_index_;
    }
    else if (selected_index_ >= scroll_offset_ + items_per_page_) {
        scroll_offset_ = selected_index_ - items_per_page_ + 1;
    }
}

void GameBrowser::render()
{
    // Clear screen
    SDL_SetRenderDrawColor(renderer_,
        color_background_.r, color_background_.g,
        color_background_.b, color_background_.a);
    SDL_RenderClear(renderer_);

    // Render components
    renderTitle();

    if (games_.empty()) {
        renderNoGames();
    } else {
        renderGameList();
    }

    renderHelp();

    if (show_help_) {
        renderHelpOverlay();
    }

    // Present
    SDL_RenderPresent(renderer_);
}

void GameBrowser::renderTitle()
{
    // Title bar background
    drawRect(0, 0, screen_width_, 110, {15, 20, 30, 255}, true);

    // Title text
    drawText("ONScripter Yuri - 游戏浏览器", 50, 15, font_large_, color_highlight_);

    // Game count
    char count_text[64];
    snprintf(count_text, sizeof(count_text), "在 sdmc:/onsemu/ 中找到 %d 个游戏", (int)games_.size());
    drawText(count_text, 50, 60, font_small_, color_text_);

    // Separator line
    drawRect(0, 110, screen_width_, 3, color_highlight_, true);
}

void GameBrowser::renderGameList()
{
    int list_start_y = 130;
    int item_height = 70;

    // Calculate visible range
    int visible_start = scroll_offset_;
    int visible_end = std::min(scroll_offset_ + items_per_page_, (int)games_.size());

    // Render visible items
    for (int i = visible_start; i < visible_end; i++) {
        int y_pos = list_start_y + (i - scroll_offset_) * item_height;
        renderGameItem(i, y_pos);
    }

    // Scroll indicator
    if ((int)games_.size() > items_per_page_) {
        int track_height = items_per_page_ * item_height;
        int indicator_height = (items_per_page_ * track_height) / games_.size();
        if (indicator_height < 30) indicator_height = 30;
        int indicator_y = list_start_y + (scroll_offset_ * (track_height - indicator_height)) / (games_.size() - items_per_page_);

        // Scroll track background
        drawRect(screen_width_ - 20, list_start_y, 10, track_height, {60, 60, 60, 255}, true);
        // Scroll indicator
        drawRect(screen_width_ - 20, indicator_y, 10, indicator_height, color_highlight_, true);
    }
}

void GameBrowser::renderGameItem(int index, int y_pos)
{
    const GameInfo& game = games_[index];
    bool is_selected = (index == selected_index_);

    int item_height = 65;
    int padding = 30;

    // Background
    if (is_selected) {
        // Highlight background
        drawRect(padding, y_pos, screen_width_ - padding * 2 - 30, item_height,
                 color_selected_, true);
        // Accent border
        drawRect(padding, y_pos, 5, item_height,
                 color_highlight_, true);
    }

    // Game name
    SDL_Color text_color = is_selected ? SDL_Color{255, 255, 255, 255} : color_text_;
    drawText(game.name.c_str(), padding + 20, y_pos + 8, font_large_, text_color);

    // Game info
    char info_text[256];
    const char* font_status = game.has_font ? "Yes" : "No (using system)";
    snprintf(info_text, sizeof(info_text), "Script: %s  |  Local Font: %s",
             game.script_file.c_str(), font_status);

    SDL_Color info_color = is_selected ? SDL_Color{200, 220, 240, 255} : color_disabled_;
    drawText(info_text, padding + 20, y_pos + 40, font_small_, info_color);

    // Index number on the right
    char index_text[32];
    snprintf(index_text, sizeof(index_text), "%d/%d", index + 1, (int)games_.size());
    drawText(index_text, screen_width_ - 120, y_pos + 20, font_small_,
             is_selected ? color_highlight_ : color_disabled_);

    // Separator line (only for non-selected items)
    if (!is_selected) {
        drawRect(padding + 15, y_pos + item_height + 2,
                 screen_width_ - padding * 2 - 60, 1,
                 {50, 55, 65, 255}, true);
    }
}

void GameBrowser::renderNoGames()
{
    int center_y = screen_height_ / 2 - 60;

    drawText("未找到游戏！", screen_width_ / 2 - 100, center_y, font_large_, color_highlight_);

    const char* help_lines[] = {
        "请将游戏文件夹放在：",
        "sdmc:/onsemu/",
        "",
        "每个游戏文件夹需包含：",
        "  - 0.txt、00.txt 或 nscript.dat",
        "  - （可选）default.ttf 字体文件"
    };

    for (int i = 0; i < 6; i++) {
        drawText(help_lines[i], 100, center_y + 50 + i * 30, font_small_, color_text_);
    }
}

void GameBrowser::renderHelp()
{
    int help_y = screen_height_ - 80;

    // Help background
    drawRect(0, help_y - 10, screen_width_, 90, {15, 20, 30, 255}, true);
    drawRect(0, help_y - 10, screen_width_, 3, color_highlight_, true);

    // Help text
    drawText("A:选择 | B/+:退出 | X:帮助 | 上/下:导航 | L/R:快速滚动 | ZL/ZR:翻页",
             50, help_y + 10, font_small_, color_text_);
    drawText("触屏：点击选择，双击启动",
             50, help_y + 40, font_small_, color_disabled_);
}

void GameBrowser::renderHelpOverlay()
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    drawRect(0, 0, screen_width_, screen_height_, {0, 0, 0, 200}, true);

    int box_w = 700;
    int box_h = 480;
    int box_x = (screen_width_ - box_w) / 2;
    int box_y = (screen_height_ - box_h) / 2;

    drawRect(box_x, box_y, box_w, box_h, {30, 35, 50, 255}, true);
    drawRect(box_x, box_y, box_w, 60, color_highlight_, true);

    drawText("游戏中按键说明", box_x + 250, box_y + 15, font_large_, {255, 255, 255, 255});

    const char* help_lines[] = {
        "A          确认 / 前进对话",
        "B          取消 / 返回 / 右键菜单",
        "X          跳过文字",
        "Y          自动模式",
        "+          菜单",
        "-          隐藏文字框",
        "L          回看历史",
        "R          快进",
        "L3         切换鼠标模式",
        "左摇杆     移动光标",
        "触屏       点击操作"
    };

    int line_y = box_y + 80;
    for (int i = 0; i < 11; i++) {
        drawText(help_lines[i], box_x + 50, line_y, font_small_, color_text_);
        line_y += 35;
    }

    drawText("按任意键关闭", box_x + 280, box_y + box_h - 40, font_small_, color_disabled_);
}

void GameBrowser::drawText(const char* text, int x, int y, TTF_Font* font, SDL_Color color)
{
    if (!font || !text || strlen(text) == 0) return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        return;
    }

    SDL_Rect dest = {x, y, 0, 0};
    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);

    SDL_RenderCopy(renderer_, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
}

void GameBrowser::drawRect(int x, int y, int w, int h, SDL_Color color, bool filled)
{
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    if (filled) {
        SDL_RenderFillRect(renderer_, &rect);
    } else {
        SDL_RenderDrawRect(renderer_, &rect);
    }
}

const GameInfo* GameBrowser::getGameInfo(int index) const
{
    if (index < 0 || index >= (int)games_.size()) {
        return nullptr;
    }
    return &games_[index];
}

const char* GameBrowser::getSelectedPath() const
{
    if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
        return games_[selected_index_].path.c_str();
    }
    return nullptr;
}

void GameBrowser::cleanup()
{
    if (font_large_) {
        TTF_CloseFont(font_large_);
        font_large_ = nullptr;
    }

    if (font_small_) {
        TTF_CloseFont(font_small_);
        font_small_ = nullptr;
    }

    games_.clear();
}

#endif // SWITCH
