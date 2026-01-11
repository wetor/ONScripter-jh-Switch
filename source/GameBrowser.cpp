/* -*- C++ -*-
 *
 *  GameBrowser.cpp - Game selection browser for ONScripter
 *
 *  Copyright (c) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "GameBrowser.h"
#include "Utils.h"
#include <switch.h>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

// External gamepad state
extern PadState pad;

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
    utils::printInfo("Initializing Game Browser...\n");

    window_ = window;
    renderer_ = renderer;

    if (!window_ || !renderer_) {
        utils::printError("Invalid SDL window or renderer\n");
        return false;
    }

    // Get window size
    SDL_GetWindowSize(window_, &screen_width_, &screen_height_);
    utils::printInfo("Screen size: %dx%d\n", screen_width_, screen_height_);

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        utils::printError("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    // Load fonts
    if (!loadFonts()) {
        utils::printError("Failed to load fonts\n");
        return false;
    }

    utils::printInfo("Game Browser initialized successfully\n");
    return true;
}

bool GameBrowser::loadFonts()
{
    // Try to load system font or embedded font
    const char* font_paths[] = {
        "romfs:/font.ttf",
        "/switch/ONScripter/font.ttf",
        "sdmc:/switch/ONScripter/font.ttf",
        nullptr
    };

    for (int i = 0; font_paths[i] != nullptr; i++) {
        font_large_ = TTF_OpenFont(font_paths[i], 32);
        if (font_large_) {
            font_small_ = TTF_OpenFont(font_paths[i], 24);
            if (font_small_) {
                utils::printInfo("Loaded font from: %s\n", font_paths[i]);
                return true;
            } else {
                TTF_CloseFont(font_large_);
                font_large_ = nullptr;
            }
        }
    }

    utils::printError("No valid font found. Please place font.ttf in romfs:/\n");
    return false;
}

int GameBrowser::scanGames(const char* base_path)
{
    utils::printInfo("Scanning for games in: %s\n", base_path);

    games_.clear();

    DIR* dir = opendir(base_path);
    if (!dir) {
        utils::printError("Failed to open directory: %s\n", base_path);
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip special directories
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if it's a directory
        if (entry->d_type != DT_DIR) {
            continue;
        }

        // Build full path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        // Check if it's a valid game folder
        GameInfo info;
        if (isValidGameFolder(full_path, info)) {
            info.path = full_path;
            info.name = entry->d_name;
            games_.push_back(info);
            utils::printInfo("Found game: %s (%s)\n", info.name.c_str(), info.script_file.c_str());
        }
    }

    closedir(dir);

    // Sort games alphabetically
    std::sort(games_.begin(), games_.end(),
        [](const GameInfo& a, const GameInfo& b) {
            return a.name < b.name;
        });

    utils::printInfo("Found %d game(s)\n", (int)games_.size());
    return games_.size();
}

bool GameBrowser::isValidGameFolder(const char* path, GameInfo& info)
{
    // Check for valid script files
    const char* script_files[] = {
        "0.txt",
        "00.txt",
        "nscript.dat",
        "nscr_sec.dat",
        nullptr
    };

    for (int i = 0; script_files[i] != nullptr; i++) {
        char script_path[512];
        snprintf(script_path, sizeof(script_path), "%s/%s", path, script_files[i]);

        struct stat st;
        if (stat(script_path, &st) == 0) {
            info.has_script = true;
            info.script_file = script_files[i];
            return true;
        }
    }

    return false;
}

int GameBrowser::run()
{
    if (games_.empty()) {
        utils::printWarning("No games found\n");
        return -1;
    }

    selected_index_ = 0;
    scroll_offset_ = 0;
    running_ = true;

    utils::printInfo("Starting browser loop...\n");

    // Main loop
    while (running_ && appletMainLoop()) {
        handleInput();
        render();

        // Small delay to prevent 100% CPU usage
        SDL_Delay(16); // ~60 FPS
    }

    return running_ ? -1 : selected_index_;
}

void GameBrowser::handleInput()
{
    // Scan for input
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);

    // Exit browser (B button or PLUS)
    if (kDown & HidNpadButton_B || kDown & HidNpadButton_Plus) {
        utils::printInfo("Browser cancelled by user\n");
        running_ = false;
        selected_index_ = -1;
        return;
    }

    // Select game (A button)
    if (kDown & HidNpadButton_A) {
        if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
            utils::printInfo("Game selected: %s\n", games_[selected_index_].name.c_str());
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
            int item_height = 70;  // Match new item height

            if (touch_y >= list_start_y && touch_y < list_start_y + items_per_page_ * item_height) {
                int touched_index = (touch_y - list_start_y) / item_height + scroll_offset_;
                if (touched_index >= 0 && touched_index < (int)games_.size()) {
                    selected_index_ = touched_index;
                    // Double tap to select
                    static Uint32 last_touch_time = 0;
                    Uint32 current_time = SDL_GetTicks();
                    if (current_time - last_touch_time < 300) {
                        utils::printInfo("触屏选择游戏: %s\n", games_[selected_index_].name.c_str());
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
    renderGameList();
    renderHelp();

    // Present
    SDL_RenderPresent(renderer_);
}

void GameBrowser::renderTitle()
{
    // Title bar background with gradient effect
    drawRect(0, 0, screen_width_, 110, {15, 20, 30, 255}, true);

    // Title text
    drawText("ONScripter 游戏浏览器", 50, 15, font_large_, color_highlight_);

    // Game count
    char count_text[64];
    snprintf(count_text, sizeof(count_text), "共找到 %d 个游戏", (int)games_.size());
    drawText(count_text, 50, 65, font_small_, color_text_);

    // Separator line
    drawRect(0, 110, screen_width_, 3, color_highlight_, true);
}

void GameBrowser::renderGameList()
{
    int list_start_y = 130;
    int item_height = 70;  // Increased height to prevent overlap

    // Calculate visible range
    int visible_start = scroll_offset_;
    int visible_end = std::min(scroll_offset_ + items_per_page_, (int)games_.size());

    // Render visible items
    for (int i = visible_start; i < visible_end; i++) {
        int y_pos = list_start_y + (i - scroll_offset_) * item_height;
        renderGameItem(i, y_pos);
    }

    // Scroll indicator (滚动条)
    if (games_.size() > items_per_page_) {
        int indicator_height = (items_per_page_ * 400) / games_.size();
        int indicator_y = 130 + (scroll_offset_ * 400) / games_.size();
        drawRect(screen_width_ - 15, indicator_y, 8, indicator_height, color_highlight_, true);
        // Scroll track background
        drawRect(screen_width_ - 15, 130, 8, 400, {60, 60, 60, 255}, true);
    }
}

void GameBrowser::renderGameItem(int index, int y_pos)
{
    const GameInfo& game = games_[index];
    bool is_selected = (index == selected_index_);

    int item_height = 65;
    int padding = 20;

    // Background with rounded effect
    if (is_selected) {
        // Highlight background
        drawRect(padding, y_pos - 2, screen_width_ - padding * 2 - 30, item_height,
                 color_selected_, true);
        // Accent border
        drawRect(padding, y_pos - 2, 5, item_height,
                 color_highlight_, true);
    }

    // Game name (larger, more prominent)
    SDL_Color text_color;
    if (is_selected) {
        text_color = {255, 255, 255, 255};
    } else {
        text_color = color_text_;
    }
    drawText(game.name.c_str(), padding + 15, y_pos + 8, font_large_, text_color);

    // Script file info (smaller, below game name with proper spacing)
    char info_text[128];
    snprintf(info_text, sizeof(info_text), "路径: %s | 脚本: %s",
             game.path.c_str(), game.script_file.c_str());
    SDL_Color info_color;
    if (is_selected) {
        info_color = {180, 200, 220, 255};
    } else {
        info_color = color_disabled_;
    }
    drawText(info_text, padding + 15, y_pos + 40, font_small_, info_color);

    // Separator line
    if (!is_selected) {
        drawRect(padding + 10, y_pos + item_height + 2,
                 screen_width_ - padding * 2 - 40, 1,
                 {60, 65, 75, 255}, true);
    }
}

void GameBrowser::renderHelp()
{
    int help_y = screen_height_ - 90;

    // Help background
    drawRect(0, help_y - 10, screen_width_, 100, {15, 20, 30, 255}, true);
    drawRect(0, help_y - 10, screen_width_, 3, color_highlight_, true);

    // Help text (Chinese)
    const char* help_lines[] = {
        "A: 选择游戏  |  B/+: 退出  |  ↑↓: 上下移动  |  L/R: 快速滚动  |  ZL/ZR: 翻页",
        "触屏: 单击选择，双击启动游戏"
    };

    for (int i = 0; i < 2; i++) {
        drawText(help_lines[i], 40, help_y + 5 + i * 30, font_small_, color_text_);
    }
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

    TTF_Quit();

    games_.clear();
}
