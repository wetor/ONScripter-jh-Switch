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
#include <ctime>

GameBrowser::GameBrowser()
    : window_(nullptr)
    , renderer_(nullptr)
    , font_large_(nullptr)
    , font_small_(nullptr)
    , selected_index_(0)
    , scroll_offset_(0)
    , battery_level_(53)
    , last_time_update_(0)
    , screen_width_(1280)
    , screen_height_(720)
    , items_per_page_(8)
    , running_(false)
    , show_help_(false)
{
    time_str_[0] = '\0';
    color_background_ = {230, 230, 230, 255};
    color_text_ = {31, 41, 55, 255};
    color_selected_ = {45, 130, 220, 255};
    color_disabled_ = {107, 114, 128, 255};
    color_highlight_ = {16, 185, 129, 255};
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
    drawRect(0, 0, screen_width_, 60, {230, 230, 230, 255}, true);

    drawText("ONScripter-Jh for Nintendo Switch", 40, 18, font_large_, {31, 41, 55, 255});
    drawText("版本:1.0", 430, 18, font_large_, {31, 41, 55, 255});

    Uint32 now = SDL_GetTicks();
    if (now - last_time_update_ > 1000 || time_str_[0] == '\0') {
        std::time_t t = std::time(nullptr);
        std::tm* tm_info = std::localtime(&t);
        if (tm_info) {
            std::snprintf(time_str_, sizeof(time_str_), "%02d:%02d", tm_info->tm_hour, tm_info->tm_min);
        } else {
            std::snprintf(time_str_, sizeof(time_str_), "00:00");
        }
        last_time_update_ = now;
    }

    int right_x = screen_width_ - 220;
    drawText(time_str_, right_x, 18, font_large_, {31, 41, 55, 255});

    char battery_text[8];
    std::snprintf(battery_text, sizeof(battery_text), "%d%%", battery_level_);
    drawText(battery_text, right_x + 70, 18, font_large_, {31, 41, 55, 255});
    drawBattery(right_x + 120, 16, battery_level_);

    drawRect(0, 60, screen_width_, 2, {209, 213, 219, 255}, true);

    drawText("ONS GameBrowser created by wetor (http://www.wetor.top)",
             screen_width_ / 2, 70, font_small_, {107, 114, 128, 255}, true);
}

void GameBrowser::renderGameList()
{
    int center_y = screen_height_ / 2 - 60;
    int center_x = screen_width_ / 2;

    for (int i = 0; i < static_cast<int>(games_.size()); i++) {
        int card_w = 210;
        int card_h = 210;
        int card_spacing = 40;
        float offset = (i - static_cast<float>(scroll_offset_)) * (card_w + card_spacing);
        int x_pos = center_x + static_cast<int>(offset) - card_w / 2;
        int y_pos = center_y - card_h / 2;
        float distance = std::fabs(static_cast<float>(i) - static_cast<float>(scroll_offset_));
        bool is_active = (i == selected_index_);

        int scaled_w = card_w;
        int scaled_h = card_h;
        if (distance < 2.0f) {
            float scale = 1.1f - distance * 0.1f;
            if (scale < 0.85f) {
                scale = 0.85f;
            }
            scaled_w = static_cast<int>(card_w * scale);
            scaled_h = static_cast<int>(card_h * scale);
            x_pos = center_x + static_cast<int>(offset) - scaled_w / 2;
            y_pos = center_y - scaled_h / 2;
        }

        SDL_Color shadow = {0, 0, 0, 30};
        drawRect(x_pos + 4, y_pos + 4, scaled_w, scaled_h, shadow, true);

        SDL_Color bg = {255, 255, 255, 255};
        drawRect(x_pos, y_pos, scaled_w, scaled_h, bg, true);

        SDL_Color border_color;
        if (is_active) {
            border_color = color_highlight_;
        } else {
            border_color = {209, 213, 219, 255};
        }
        drawRect(x_pos, y_pos, scaled_w, scaled_h, border_color, false);

        SDL_Rect inner = {x_pos + 12, y_pos + 12, scaled_w - 24, scaled_h - 24};
        SDL_Color theme = {96, 165, 250, 255};
        SDL_SetRenderDrawColor(renderer_, theme.r, theme.g, theme.b, theme.a);
        SDL_RenderFillRect(renderer_, &inner);

        SDL_Color text_color;
        if (is_active) {
            text_color = {31, 41, 55, 255};
        } else {
            text_color = {107, 114, 128, 255};
        }
        drawText(games_[i].name.c_str(), x_pos + scaled_w / 2, y_pos + scaled_h + 18, font_large_, text_color, true);
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

void GameBrowser::drawText(const char* text, int x, int y, TTF_Font* font, SDL_Color color, bool centered)
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

    if (centered) {
        dest.x -= dest.w / 2;
    }

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

void GameBrowser::drawBattery(int x, int y, int level)
{
    SDL_Color border = {75, 85, 99, 255};
    SDL_Color fill = {34, 197, 94, 255};
    if (level < 30) {
        fill = {239, 68, 68, 255};
    }

    SDL_Rect body = {x, y, 42, 18};
    SDL_SetRenderDrawColor(renderer_, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer_, &body);

    SDL_Rect cap = {x + 42, y + 4, 4, 10};
    SDL_RenderFillRect(renderer_, &cap);

    int fill_width = (42 - 4) * level / 100;
    SDL_Rect fill_rect = {x + 2, y + 2, fill_width, 14};
    SDL_SetRenderDrawColor(renderer_, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer_, &fill_rect);
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
