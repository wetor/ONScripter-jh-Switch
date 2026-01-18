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
#include <SDL2/SDL_image.h>
#include <switch.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <ctime>

GameBrowser::GameBrowser()
    : window_(nullptr)
    , renderer_(nullptr)
    , font_large_(nullptr)
    , font_medium_(nullptr)
    , font_small_(nullptr)
    , font_icon_(nullptr)
    , selected_index_(0)
    , scroll_offset_(0)
    , carousel_scroll_(0)
    , screen_width_(1280)
    , screen_height_(720)
    , items_per_page_(8)
    , running_(false)
    , show_help_(false)
{
    color_background_ = {245, 245, 245, 255};
    color_text_ = {40, 40, 40, 255};
    color_accent1_ = {50, 200, 100, 255};
    color_accent2_ = {100, 100, 100, 150};
    color_shadow_ = {0, 0, 0, 40};
    color_selected_border_ = {50, 200, 100, 255};
    color_disabled_ = {150, 150, 150, 255};
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

    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("GameBrowser: Failed to initialize SDL_image: %s\n", IMG_GetError());
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
        font_large_ = TTF_OpenFont(font_paths[i], 24);
        if (font_large_) {
            font_medium_ = TTF_OpenFont(font_paths[i], 20);
            if (font_medium_) {
                font_small_ = TTF_OpenFont(font_paths[i], 16);
                if (font_small_) {
                    font_icon_ = TTF_OpenFont(font_paths[i], 14);
                    if (font_icon_) {
                        printf("GameBrowser: Loaded font from: %s\n", font_paths[i]);
                        return true;
                    } else {
                        TTF_CloseFont(font_large_);
                        TTF_CloseFont(font_medium_);
                        TTF_CloseFont(font_small_);
                        font_large_ = nullptr;
                        font_medium_ = nullptr;
                        font_small_ = nullptr;
                    }
                } else {
                    TTF_CloseFont(font_large_);
                    TTF_CloseFont(font_medium_);
                    font_large_ = nullptr;
                    font_medium_ = nullptr;
                }
            } else {
                TTF_CloseFont(font_large_);
                font_large_ = nullptr;
            }
        }
    }

    printf("GameBrowser: No valid font found!\n");
    return false;
}

bool GameBrowser::loadGameCover(GameInfo& game)
{
    const char* cover_names[] = {
        "icon.jpg",
        "icon.png",
        "cover.png",
        "cover.jpg",
        "cover.jpeg",
        "thumbnail.png",
        "thumbnail.jpg",
        "img.jpg",
        "img.png",
        "preview.png",
        "preview.jpg",
        nullptr
    };

    for (int i = 0; cover_names[i] != nullptr; i++) {
        char cover_path[512];
        snprintf(cover_path, sizeof(cover_path), "%s/%s", game.path.c_str(), cover_names[i]);

        struct stat st;
        if (stat(cover_path, &st) == 0 && S_ISREG(st.st_mode)) {
            game.cover_file_path = cover_path;
            game.has_cover = true;
            printf("GameBrowser: Found cover file: %s\n", cover_path);
            return true;
        }
    }

    game.has_cover = false;
    return false;
}

bool GameBrowser::loadCoverTexture(GameInfo& game)
{
    if (game.texture_loaded || game.cover_file_path.empty()) {
        return game.texture_loaded;
    }

    SDL_Surface* surface = IMG_Load(game.cover_file_path.c_str());
    if (surface) {
        printf("GameBrowser: Loading texture for: %s (size: %dx%d)\n",
               game.name.c_str(), surface->w, surface->h);
        game.cover_texture_ = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);

        if (game.cover_texture_) {
            game.texture_loaded = true;
            printf("GameBrowser: Texture loaded successfully for: %s\n", game.name.c_str());
            return true;
        } else {
            printf("GameBrowser: Failed to create texture for: %s - %s\n",
                   game.name.c_str(), SDL_GetError());
        }
    } else {
        printf("GameBrowser: Failed to load surface: %s - %s\n",
               game.cover_file_path.c_str(), IMG_GetError());
    }

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
    bool scan_complete = false;
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

        GameInfo info;
        if (isValidGameFolder(full_path, info)) {
            info.path = full_path;
            info.name = entry->d_name;
            loadGameCover(info);
            games_.push_back(info);
            printf("GameBrowser: Found game: %s (%s)\n", info.name.c_str(), info.script_file.c_str());
        }
    }
    scan_complete = true;

    closedir(dir);
    if (!scan_complete) {
        printf("GameBrowser: Scan was interrupted\n");
        return 0;
    }

    // Sort games alphabetically
    std::sort(games_.begin(), games_.end(),
        [](const GameInfo& a, const GameInfo& b) {
            return a.name < b.name;
        });

    printf("GameBrowser: Found %d game(s)\n", (int)games_.size());

    for (size_t i = 0; i < games_.size(); i++) {
        if (games_[i].has_cover && games_[i].cover_texture_) {
            int w, h;
            SDL_QueryTexture(games_[i].cover_texture_, nullptr, nullptr, &w, &h);
            printf("GameBrowser: Game[%zu] %s - has_cover=%d, texture=%p, size=%dx%d\n",
                   i, games_[i].name.c_str(), games_[i].has_cover, games_[i].cover_texture_, w, h);
        } else {
            printf("GameBrowser: Game[%zu] %s - has_cover=%d, texture=%p\n",
                   i, games_[i].name.c_str(), games_[i].has_cover, games_[i].cover_texture_);
        }
    }

    return games_.size();
}

bool GameBrowser::isValidGameFolder(const char* path, GameInfo& info)
{
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

    // Horizontal navigation for carousel
    if (kDown & HidNpadButton_Left || kDown & HidNpadButton_StickLLeft) {
        moveSelection(-1);
    }
    else if (kDown & HidNpadButton_Right || kDown & HidNpadButton_StickLRight) {
        moveSelection(1);
    }

    // Fast scroll with L/R
    if (kDown & HidNpadButton_R) {
        moveSelection(3);
    }
    else if (kDown & HidNpadButton_L) {
        moveSelection(-3);
    }

    // Page up/down with ZL/ZR
    if (kDown & HidNpadButton_ZR) {
        moveSelection(6);
    }
    else if (kDown & HidNpadButton_ZL) {
        moveSelection(-6);
    }

    HidTouchScreenState touch_state;
    memset(&touch_state, 0, sizeof(touch_state));
    if (hidGetTouchScreenStates(&touch_state, 1) > 0) {
        if (touch_state.count > 0) {
            int touch_x = touch_state.touches[0].x;
            int touch_y = touch_state.touches[0].y;

            int center_x = screen_width_ / 2;

            if (touch_y >= CAROUSEL_START_Y - CARD_HEIGHT / 2 && touch_y <= CAROUSEL_START_Y + CARD_HEIGHT / 2) {
                int touch_offset = (touch_x - center_x) / (CARD_WIDTH + CARD_SPACING);

                if (abs(touch_offset) <= 3) {
                    int touched_index = selected_index_ + touch_offset;
                    if (touched_index >= 0 && touched_index < (int)games_.size()) {
                        selected_index_ = touched_index;

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
}

void GameBrowser::moveSelection(int delta)
{
    if (games_.empty()) return;

    selected_index_ += delta;

    if (selected_index_ < 0) {
        selected_index_ = 0;
    }
    if (selected_index_ >= (int)games_.size()) {
        selected_index_ = games_.size() - 1;
    }
}

void GameBrowser::render()
{
    SDL_SetRenderDrawColor(renderer_,
        color_background_.r, color_background_.g,
        color_background_.b, color_background_.a);
    SDL_RenderClear(renderer_);

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    renderStatusBar();

    if (games_.empty()) {
        renderNoGames();
    } else {
        renderCarousel();
    }

    renderBottomBar();

    if (show_help_) {
        renderHelpOverlay();
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    SDL_RenderPresent(renderer_);
}

void GameBrowser::renderStatusBar()
{
    drawRect(0, 0, screen_width_, STATUS_BAR_HEIGHT, {230, 230, 230, 255}, true);
    drawRect(0, STATUS_BAR_HEIGHT - 1, screen_width_, 1, {200, 200, 200, 255}, true);

    drawText("ONScripter-Jh for Nintendo Switch 版本:1.0", 20, 20, font_medium_, color_text_);

    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);

    int time_width;
    TTF_SizeText(font_medium_, time_str, &time_width, nullptr);

    int battery_x = screen_width_ - 200 - time_width;
    char battery_text[16];
    int battery_percent = 53;
    snprintf(battery_text, sizeof(battery_text), "%d%%", battery_percent);
    drawText(battery_text, battery_x, 20, font_medium_, color_text_);

    drawBatteryIcon(screen_width_ - 50, 22, battery_percent);
    drawText(time_str, screen_width_ - 140, 20, font_medium_, color_text_);
}

void GameBrowser::renderCarousel()
{
    if (games_.empty()) return;

    int center_x = screen_width_ / 2;
    int center_y = CAROUSEL_START_Y;

    int visible_left = std::max(0, selected_index_ - 3);
    int visible_right = std::min((int)games_.size() - 1, selected_index_ + 3);

    for (int i = visible_left; i <= visible_right; i++) {
        int offset = i - selected_index_;

        float x_pos = center_x + offset * (CARD_WIDTH + CARD_SPACING) - CARD_WIDTH / 2;
        float y_pos = center_y - CARD_HEIGHT / 2;

        float scale = 1.0f - std::abs(offset) * 0.15f;
        if (scale < 0.6f) scale = 0.6f;

        float alpha = 1.0f - std::abs(offset) * 0.3f;
        if (alpha < 0.4f) alpha = 0.4f;

        renderGameCard(i, x_pos, y_pos, CARD_WIDTH, CARD_HEIGHT, scale, alpha);
    }

    drawButton(center_x - 400, center_y, 25, true, selected_index_ > 0);
    drawButton(center_x + 400, center_y, 25, false, selected_index_ < (int)games_.size() - 1);

    if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
        int title_width;
        TTF_SizeText(font_large_, games_[selected_index_].name.c_str(), &title_width, nullptr);
        drawText(games_[selected_index_].name.c_str(), center_x - title_width / 2, center_y - CARD_HEIGHT / 2 - 50, font_large_, color_text_);
    }
}

void GameBrowser::renderGameCard(int index, float x, float y, float width, float height, float scale, float alpha)
{
    float scaled_width = width * scale;
    float scaled_height = height * scale;
    float scaled_x = x + (width - scaled_width) / 2;
    float scaled_y = y + (height - scaled_height) / 2;

    bool is_selected = (index == selected_index_);

    SDL_Color card_color = {255, 255, 255, (Uint8)(255 * alpha)};
    SDL_Color border_color = is_selected ? color_selected_border_ : (SDL_Color){200, 200, 200, (Uint8)(180 * alpha)};
    int border_width = is_selected ? 4 : 1;

    drawShadow(scaled_x + 5, scaled_y + 5, scaled_width, scaled_height, 8, {0, 0, 0, (Uint8)(30 * alpha)});
    drawRoundedRect(scaled_x, scaled_y, scaled_width, scaled_height, 12, card_color);

    for (int i = 0; i < border_width; i++) {
        drawRoundedRect(scaled_x - i, scaled_y - i, scaled_width + 2 * i, scaled_height + 2 * i, 12 + i, border_color);
    }

    SDL_Rect content_rect = {
        (int)(scaled_x + border_width + 4),
        (int)(scaled_y + border_width + 4),
        (int)(scaled_width - (border_width + 4) * 2),
        (int)(scaled_height - (border_width + 4) * 2)
    };

    if (index >= 0 && index < (int)games_.size()) {
        GameInfo& game = games_[index];

        if (game.has_cover && !game.texture_loaded) {
            loadCoverTexture(game);
        }

        if (game.has_cover && game.texture_loaded && game.cover_texture_) {
            SDL_SetTextureAlphaMod(game.cover_texture_, (Uint8)(255 * alpha));

            SDL_RenderCopy(renderer_, game.cover_texture_, nullptr, &content_rect);
        } else {
            drawRect(
                (int)(content_rect.x + 10),
                (int)(content_rect.y + 10),
                (int)(content_rect.w - 20),
                (int)(content_rect.h - 20),
                {240, 240, 245, (Uint8)(255 * alpha)},
                true
            );

            drawRect(
                (int)(content_rect.x + 10),
                (int)(content_rect.y + 10),
                (int)(content_rect.w - 20),
                (int)(content_rect.h - 20),
                {220, 220, 230, (Uint8)(255 * alpha)},
                false
            );

            SDL_Color icon_color = {100, 180, 200, (Uint8)(200 * alpha)};
            int icon_size = (int)(scaled_width * 0.3f);

            drawRect(
                (int)(scaled_x + scaled_width / 2 - icon_size),
                (int)(scaled_y + scaled_height / 2 - icon_size * 1.5f),
                icon_size * 2,
                icon_size * 2.5f,
                icon_color,
                true
            );

            int text_width;
            TTF_SizeText(font_medium_, "No Cover", &text_width, nullptr);
            drawText("No Cover",
                     (int)(scaled_x + scaled_width / 2 - text_width / 2),
                     (int)(scaled_y + scaled_height / 2 + icon_size),
                     font_medium_, (SDL_Color){120, 120, 130, (Uint8)(220 * alpha)});
        }

        if (!is_selected && alpha > 0.5f) {
            int text_width;
            TTF_SizeText(font_small_, game.name.c_str(), &text_width, nullptr);
            drawText(game.name.c_str(),
                     (int)(scaled_x + scaled_width / 2 - text_width / 2),
                     (int)(scaled_y + scaled_height + 10),
                     font_small_, (SDL_Color){100, 100, 100, (Uint8)(200 * alpha)});
        }
    }
}

void GameBrowser::drawButton(int x, int y, int radius, bool is_left, bool is_enabled)
{
    if (!is_enabled) return;

    SDL_Color button_color = {80, 80, 80, 180};

    drawShadow(x + 3, y + 3, radius * 2, radius * 2, 6, {0, 0, 0, 30});

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, button_color.r, button_color.g, button_color.b, button_color.a);

    for (int angle = 0; angle < 360; angle += 5) {
        float rad = angle * M_PI / 180.0f;
        SDL_RenderDrawPoint(renderer_, x + radius + cos(rad) * (radius - 2), y + radius + sin(rad) * (radius - 2));
    }

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 200);

    for (int i = 0; i < radius - 5; i++) {
        if (is_left) {
            SDL_RenderDrawPoint(renderer_, x + radius - i, y + radius);
            SDL_RenderDrawPoint(renderer_, x + radius + i, y + radius - i + 5);
            SDL_RenderDrawPoint(renderer_, x + radius + i, y + radius + i - 5);
        } else {
            SDL_RenderDrawPoint(renderer_, x + radius + i, y + radius);
            SDL_RenderDrawPoint(renderer_, x + radius - i, y + radius - i + 5);
            SDL_RenderDrawPoint(renderer_, x + radius - i, y + radius + i - 5);
        }
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void GameBrowser::renderNoGames()
{
    int center_y = screen_height_ / 2;

    drawText("未找到游戏", screen_width_ / 2 - 60, center_y - 60, font_large_, color_text_);

    const char* help_lines[] = {
        "请将游戏文件夹放在：",
        "sdmc:/onsemu/",
        "",
        "每个游戏文件夹需包含：",
        "  0.txt、00.txt 或 nscript.dat",
        "  （可选）default.ttf 字体文件"
    };

    for (int i = 0; i < 6; i++) {
        int text_width;
        TTF_SizeText(font_small_, help_lines[i], &text_width, nullptr);
        drawText(help_lines[i], screen_width_ / 2 - text_width / 2, center_y + i * 30, font_small_, color_disabled_);
    }
}

void GameBrowser::renderBottomBar()
{
    int bar_y = screen_height_ - BOTTOM_BAR_HEIGHT;

    drawRect(0, bar_y, screen_width_, BOTTOM_BAR_HEIGHT, {20, 20, 20, 230}, true);

    int key_spacing = 140;
    int start_x = 30;

    drawControlKey("L", "游戏帮助", start_x, bar_y + 25);
    drawControlKey("A", "确认/开始", start_x + key_spacing, bar_y + 25);
    drawControlKey("B", "返回/取消", start_x + key_spacing * 2, bar_y + 25);
    drawControlKey("Y", "详细信息", start_x + key_spacing * 3, bar_y + 25);
    drawControlKey("X", "资源查看", start_x + key_spacing * 4, bar_y + 25);
    drawControlKey("R", "播放器", start_x + key_spacing * 5, bar_y + 25);
    drawControlKey("-", "重载", screen_width_ - 230, bar_y + 25);
    drawControlKey("+", "设置", screen_width_ - 100, bar_y + 25);
}

void GameBrowser::drawControlKey(const char* key, const char* text, int x, int y)
{
    SDL_Color key_bg = {50, 50, 50, 255};
    SDL_Color key_border = {80, 80, 80, 255};

    drawRoundedRect(x, y, 40, 30, 6, key_bg);
    drawRoundedRect(x, y, 40, 30, 6, key_border);

    int key_width;
    TTF_SizeText(font_icon_, key, &key_width, nullptr);
    drawText(key, x + (40 - key_width) / 2, y + 6, font_icon_, {200, 200, 200, 255});

    drawText(text, x + 50, y + 5, font_small_, {180, 180, 180, 255});
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
    drawRect(box_x, box_y, box_w, 60, color_accent1_, true);

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

void GameBrowser::drawRoundedRect(int x, int y, int w, int h, int radius, SDL_Color color)
{
    if (radius * 2 > std::min(w, h)) {
        radius = std::min(w, h) / 2;
    }

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    for (int i = 0; i < radius; i++) {
        float angle = acos(1.0f - (float)i / radius);
        int x_offset = sin(angle) * radius;
        int y_offset = cos(angle) * radius;

        SDL_RenderDrawPoint(renderer_, x + radius - x_offset, y + radius - y_offset);
        SDL_RenderDrawPoint(renderer_, x + w - radius + x_offset - 1, y + radius - y_offset);
        SDL_RenderDrawPoint(renderer_, x + radius - x_offset, y + h - radius + y_offset - 1);
        SDL_RenderDrawPoint(renderer_, x + w - radius + x_offset - 1, y + h - radius + y_offset - 1);
    }

    SDL_Rect rects[4] = {
        {x + radius, y, w - radius * 2, h},
        {x, y + radius, radius, h - radius * 2},
        {x + w - radius, y + radius, radius, h - radius * 2}
    };

    SDL_RenderFillRect(renderer_, &rects[0]);
    SDL_RenderFillRect(renderer_, &rects[1]);
    SDL_RenderFillRect(renderer_, &rects[2]);
}

void GameBrowser::drawShadow(int x, int y, int w, int h, int offset, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < offset; i++) {
        SDL_Rect shadow_rect = {
            x + i,
            y + i,
            w,
            h
        };
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, (Uint8)(color.a * (1.0f - (float)i / offset)));
        SDL_RenderFillRect(renderer_, &shadow_rect);
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void GameBrowser::drawBatteryIcon(int x, int y, int percentage)
{
    int width = 30;
    int height = 16;
    int tip_width = 4;
    int tip_height = 6;

    SDL_Color battery_color = (percentage > 20) ? (SDL_Color){50, 200, 100, 255} : (SDL_Color){255, 100, 100, 255};

    drawRect(x, y + (height - tip_height) / 2, tip_width, tip_height, battery_color, true);
    drawRoundedRect(x + tip_width, y, width - tip_width, height, 3, {220, 220, 220, 255});
    drawRoundedRect(x + tip_width + 2, y + 2, width - tip_width - 6, height - 4, 2, {255, 255, 255, 255});

    int fill_width = (width - tip_width - 6) * percentage / 100;
    if (fill_width > 0) {
        drawRect(x + tip_width + 3, y + 3, fill_width, height - 6, battery_color, true);
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

    if (font_medium_) {
        TTF_CloseFont(font_medium_);
        font_medium_ = nullptr;
    }

    if (font_small_) {
        TTF_CloseFont(font_small_);
        font_small_ = nullptr;
    }

    if (font_icon_) {
        TTF_CloseFont(font_icon_);
        font_icon_ = nullptr;
    }

    games_.clear();
}

#endif // SWITCH
