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
#include <cctype>
#include <ctime>

GameBrowser::GameBrowser()
    : window_(nullptr)
    , renderer_(nullptr)
    , font_large_(nullptr)
    , font_medium_(nullptr)
    , font_small_(nullptr)
    , font_icon_(nullptr)
    , default_icon_texture_(nullptr)
    , selected_index_(0)
    , scroll_offset_(0)
    , carousel_scroll_(0)
    , screen_width_(1280)
    , screen_height_(720)
    , items_per_page_(8)
    , running_(false)
    , show_help_(false)
    , show_info_(false)
    , info_scroll_(0)
{
    color_background_ = {230, 230, 230, 255};
    color_text_ = {31, 41, 55, 255};
    color_accent1_ = {16, 185, 129, 255};
    color_accent2_ = {156, 163, 175, 150};
    color_shadow_ = {0, 0, 0, 40};
    color_selected_border_ = {16, 185, 129, 255};
    color_disabled_ = {107, 114, 128, 255};
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

    loadButtonIcons();

    // Initialize gamepad
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad_);

    printf("GameBrowser: Initialized successfully\n");
    return true;
}

void GameBrowser::loadButtonIcons()
{
    const struct {
        const char* key;
        const char* file;
    } icons[] = {
        {"A", "A.png"},
        {"B", "B.png"},
        {"X", "X.png"},
        {"Y", "Y.png"},
        {"L", "L.png"},
        {"R", "R.png"},
        {"LEFT", "LEFT.png"},
        {"RIGHT", "RIGHT.png"},
        {"+", "PLUS.png"},
        {"-", "MINUS.png"},
        {nullptr, nullptr}
    };

    for (int i = 0; icons[i].key != nullptr; i++) {
        char path[256];
        std::snprintf(path, sizeof(path), "romfs:/image/%s", icons[i].file);
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            continue;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            continue;
        }

        button_textures_[icons[i].key] = texture;
    }

    SDL_Surface* default_surface = IMG_Load("romfs:/default_icon.png");
    if (default_surface) {
        default_icon_texture_ = SDL_CreateTextureFromSurface(renderer_, default_surface);
        SDL_FreeSurface(default_surface);
    }
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
            font_medium_ = TTF_OpenFont(font_paths[i], 24);
            if (font_medium_) {
                font_small_ = TTF_OpenFont(font_paths[i], 20);
                if (font_small_) {
                    font_icon_ = TTF_OpenFont(font_paths[i], 18);
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
    game.has_cover = false;
    game.cover_file_path.clear();

    for (int i = 0; i <= 9; i++) {
        char icon_name[32];
        if (i == 0) {
            snprintf(icon_name, sizeof(icon_name), "icon.png");
        } else {
            snprintf(icon_name, sizeof(icon_name), "icon%d.png", i);
        }

        char icon_path[512];
        snprintf(icon_path, sizeof(icon_path), "%s/%s", game.path.c_str(), icon_name);
        struct stat st;
        if (stat(icon_path, &st) == 0 && S_ISREG(st.st_mode)) {
            game.cover_file_path = icon_path;
            game.has_cover = true;
            printf("GameBrowser: Found cover file: %s\n", icon_path);
            return true;
        }
    }

    const char* cover_names[] = {
        "icon.jpg",
        "icon.png",
        "logo.png",
        "logo.jpg",
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

    if (!game.name.empty()) {
        const char* name_exts[] = {".png", ".jpg", ".jpeg", nullptr};
        for (int i = 0; name_exts[i] != nullptr; i++) {
            char cover_path[512];
            snprintf(cover_path, sizeof(cover_path), "%s/%s%s", game.path.c_str(), game.name.c_str(), name_exts[i]);
            struct stat st;
            if (stat(cover_path, &st) == 0 && S_ISREG(st.st_mode)) {
                game.cover_file_path = cover_path;
                game.has_cover = true;
                printf("GameBrowser: Found cover file: %s\n", cover_path);
                return true;
            }
        }
    }

    DIR* dir = opendir(game.path.c_str());
    if (dir) {
        std::string fallback_path;
        std::string name_lower = game.name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            std::string file_name = entry->d_name;
            std::string file_lower = file_name;
            std::transform(file_lower.begin(), file_lower.end(), file_lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (file_lower.size() < 4) {
                continue;
            }

            bool is_image = false;
            if (file_lower.size() >= 4 && file_lower.compare(file_lower.size() - 4, 4, ".png") == 0) {
                is_image = true;
            } else if (file_lower.size() >= 4 && file_lower.compare(file_lower.size() - 4, 4, ".jpg") == 0) {
                is_image = true;
            } else if (file_lower.size() >= 5 && file_lower.compare(file_lower.size() - 5, 5, ".jpeg") == 0) {
                is_image = true;
            }

            if (!is_image) {
                continue;
            }

            if (file_lower.find("icon") != std::string::npos || file_lower.find("cover") != std::string::npos ||
                (!name_lower.empty() && file_lower.find(name_lower) != std::string::npos)) {
                char cover_path[512];
                snprintf(cover_path, sizeof(cover_path), "%s/%s", game.path.c_str(), entry->d_name);
                fallback_path = cover_path;
                break;
            }

            if (fallback_path.empty()) {
                char cover_path[512];
                snprintf(cover_path, sizeof(cover_path), "%s/%s", game.path.c_str(), entry->d_name);
                fallback_path = cover_path;
            }
        }

        closedir(dir);

        if (!fallback_path.empty()) {
            game.cover_file_path = fallback_path;
            game.has_cover = true;
            printf("GameBrowser: Found cover file: %s\n", game.cover_file_path.c_str());
            return true;
        }
    }

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
            if (info.has_cover) {
                loadCoverTexture(info);
            }
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

    if (show_info_) {
        if (kDown & (HidNpadButton_B | HidNpadButton_A | HidNpadButton_Plus | HidNpadButton_Minus | HidNpadButton_L)) {
            show_info_ = false;
            return;
        }
        if (kDown & (HidNpadButton_X | HidNpadButton_Y)) {
            show_info_ = false;
            return;
        }
        if (kDown & (HidNpadButton_Up | HidNpadButton_StickLUp)) {
            if (info_scroll_ > 0) {
                info_scroll_--;
            }
            return;
        }
        if (kDown & (HidNpadButton_Down | HidNpadButton_StickLDown)) {
            if (info_scroll_ + 1 < static_cast<int>(info_lines_.size())) {
                info_scroll_++;
            }
            return;
        }
        return;
    }

    // Toggle help overlay (L button)
    if (kDown & HidNpadButton_L) {
        show_help_ = !show_help_;
        show_info_ = false;
        return;
    }

    // If help is showing, any other button closes it
    if (show_help_) {
        if (kDown) {
            show_help_ = false;
        }
        return;
    }

    // Exit browser (MINUS button)
    if (kDown & HidNpadButton_Minus) {
        printf("GameBrowser: Reload requested\n");
        running_ = false;
        selected_index_ = -1;
        return;
    }

    // Settings placeholder (PLUS button)
    if (kDown & HidNpadButton_Plus) {
        info_text_ = "设置功能尚未实现";
        updateInfoLines();
        info_scroll_ = 0;
        show_info_ = true;
        show_help_ = false;
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

    if (kDown & HidNpadButton_Y) {
        info_text_ = buildInfoText(games_[selected_index_]);
        updateInfoLines();
        info_scroll_ = 0;
        show_info_ = true;
        show_help_ = false;
        return;
    }

    if (kDown & HidNpadButton_X) {
        info_text_ = buildResourceText(games_[selected_index_]);
        updateInfoLines();
        info_scroll_ = 0;
        show_info_ = true;
        show_help_ = false;
        return;
    }

    // Horizontal navigation for carousel
    if (kDown & HidNpadButton_Left || kDown & HidNpadButton_StickLLeft) {
        moveSelection(-1);
    }
    else if (kDown & HidNpadButton_Right || kDown & HidNpadButton_StickLRight) {
        moveSelection(1);
    }

    // Fast scroll with R
    if (kDown & HidNpadButton_R) {
        moveSelection(3);
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

    const char* subtitle = "ONS GameBrowser created by wetor (http://www.wetor.top)";
    int subtitle_width;
    TTF_SizeText(font_small_, subtitle, &subtitle_width, nullptr);
    drawText(subtitle, screen_width_ / 2 - subtitle_width / 2, STATUS_BAR_HEIGHT + 8, font_small_, color_disabled_);

    if (games_.empty()) {
        renderNoGames();
    } else {
        renderCarousel();
    }

    renderBottomBar();

    if (show_help_) {
        renderHelpOverlay();
    } else if (show_info_) {
        renderInfoOverlay();
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    SDL_RenderPresent(renderer_);
}

void GameBrowser::renderStatusBar()
{
    drawRect(0, 0, screen_width_, STATUS_BAR_HEIGHT, color_background_, true);
    drawRect(0, STATUS_BAR_HEIGHT - 1, screen_width_, 1, {200, 200, 200, 255}, true);

    int title_x = 20;
    int title_y = 8;
    drawText("ONScripter-Jh for Nintendo Switch", title_x, title_y, font_medium_, color_text_);

    int title_width = 0;
    TTF_SizeText(font_medium_, "ONScripter-Jh for Nintendo Switch", &title_width, nullptr);
    int version_x = title_x + title_width + 16;
    drawText("版本:1.0", version_x, title_y, font_medium_, color_text_);

    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);

    int time_width = 0;
    TTF_SizeText(font_medium_, time_str, &time_width, nullptr);

    int battery_percent = 53;
    char battery_text[16];
    snprintf(battery_text, sizeof(battery_text), "%d%%", battery_percent);
    int battery_text_width = 0;
    TTF_SizeText(font_medium_, battery_text, &battery_text_width, nullptr);

    int battery_icon_x = screen_width_ - 8 - 42;
    int battery_text_x = battery_icon_x - 8 - battery_text_width;
    int time_x = battery_text_x - 16 - time_width;

    drawText(time_str, time_x, title_y, font_medium_, color_text_);
    drawText(battery_text, battery_text_x, title_y, font_medium_, color_text_);
    drawBatteryIcon(battery_icon_x, title_y, battery_percent);
}

void GameBrowser::renderCarousel()
{
    if (games_.empty()) return;

    int center_x = screen_width_ / 2;
    int base_y = CAROUSEL_START_Y;

    int visible_left = std::max(0, selected_index_ - 3);
    int visible_right = std::min((int)games_.size() - 1, selected_index_ + 3);

    float unselected_scale = 0.8f;
    float unselected_height = CARD_HEIGHT * unselected_scale;
    int bar_height = (int)(unselected_height + 10.0f);
    int bar_center = (int)(base_y - unselected_height / 2.0f);
    int bar_y = bar_center - bar_height / 2;
    drawRect(0, bar_y, screen_width_, bar_height, {220, 220, 220, 160}, true);

    for (int i = visible_left; i <= visible_right; i++) {
        if (i == selected_index_) {
            continue;
        }

        int offset = i - selected_index_;
        float x_pos = center_x + offset * (CARD_WIDTH + CARD_SPACING);
        float scale = unselected_scale;
        float alpha = 0.9f;

        renderGameCard(i, x_pos, base_y, CARD_WIDTH, CARD_HEIGHT, scale, alpha);

        SDL_Color label_color = color_disabled_;
        int label_width;
        TTF_SizeText(font_medium_, games_[i].name.c_str(), &label_width, nullptr);
        drawText(games_[i].name.c_str(),
                 (int)(x_pos - label_width / 2),
                 (int)(base_y - CARD_HEIGHT * unselected_scale - 22),
                 font_medium_, label_color);
    }

    if (selected_index_ >= 0 && selected_index_ < (int)games_.size()) {
        float x_pos = center_x;
        float scale = 1.2f;
        float alpha = 1.0f;

        renderGameCard(selected_index_, x_pos, base_y, CARD_WIDTH, CARD_HEIGHT, scale, alpha);

        int label_width;
        TTF_SizeText(font_large_, games_[selected_index_].name.c_str(), &label_width, nullptr);
        drawText(games_[selected_index_].name.c_str(),
                 center_x - label_width / 2,
                 (int)(base_y - CARD_HEIGHT * 1.2f - 30),
                 font_large_, color_text_);
    }

    int button_y = screen_height_ / 2 - BUTTON_HEIGHT / 2;
    drawButton(5, button_y, BUTTON_HEIGHT, true, selected_index_ > 0);
    drawButton(screen_width_ - BUTTON_HEIGHT - 5, button_y, BUTTON_HEIGHT, false,
               selected_index_ < (int)games_.size() - 1);
}

void GameBrowser::renderGameCard(int index, float center_x, float base_y, float width, float height, float scale, float alpha)
{
    float scaled_width = width * scale;
    float scaled_height = height * scale;
    float scaled_x = center_x - scaled_width / 2;
    float scaled_y = base_y - scaled_height;

    bool is_selected = (index == selected_index_);

    SDL_Color card_color = {255, 255, 255, (Uint8)(255 * alpha)};
    if (is_selected) {
        drawShadow(scaled_x + 2, scaled_y + 2, scaled_width, scaled_height, 4, {0, 0, 0, (Uint8)(20 * alpha)});
        drawRect((int)scaled_x, (int)scaled_y, (int)scaled_width, (int)scaled_height, color_selected_border_, false);
    } else {
        drawShadow(scaled_x + 2, scaled_y + 2, scaled_width, scaled_height, 3, {0, 0, 0, (Uint8)(18 * alpha)});
    }

    drawRect((int)scaled_x, (int)scaled_y, (int)scaled_width, (int)scaled_height, card_color, true);

    int content_padding = 6;
    SDL_Rect content_rect = {
        (int)(scaled_x + content_padding),
        (int)(scaled_y + content_padding),
        (int)(scaled_width - content_padding * 2),
        (int)(scaled_height - content_padding * 2)
    };

    if (index >= 0 && index < (int)games_.size()) {
        GameInfo& game = games_[index];

        if (game.has_cover && !game.texture_loaded) {
            loadCoverTexture(game);
        }

        if (game.has_cover && game.texture_loaded && game.cover_texture_) {
            SDL_SetTextureAlphaMod(game.cover_texture_, (Uint8)(255 * alpha));
            SDL_RenderCopy(renderer_, game.cover_texture_, nullptr, &content_rect);
        } else if (default_icon_texture_) {
            SDL_SetTextureAlphaMod(default_icon_texture_, (Uint8)(255 * alpha));
            SDL_RenderCopy(renderer_, default_icon_texture_, nullptr, &content_rect);
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

            int text_width;
            TTF_SizeText(font_medium_, "No Cover", &text_width, nullptr);
            drawText("No Cover",
                     (int)(scaled_x + scaled_width / 2 - text_width / 2),
                     (int)(scaled_y + scaled_height / 2),
                     font_medium_, (SDL_Color){120, 120, 130, (Uint8)(220 * alpha)});
        }

    }
}

void GameBrowser::drawButton(int x, int y, int size, bool is_left, bool is_enabled)
{
    if (!is_enabled) return;

    const char* key = is_left ? "LEFT" : "RIGHT";
    auto icon_it = button_textures_.find(key);
    if (icon_it != button_textures_.end()) {
        drawRect(x, y, size, size, {255, 255, 255, 128}, true);
        SDL_Rect dst = {x, y, size, size};
        SDL_RenderCopy(renderer_, icon_it->second, nullptr, &dst);
        return;
    }

    SDL_Color button_color = {30, 30, 30, 200};
    drawCircle(x + size / 2, y + size / 2, size / 2, button_color, true);
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

    drawRect(0, bar_y, screen_width_, BOTTOM_BAR_HEIGHT, {255, 255, 255, 200}, true);

    int left_size = 8;
    int button_height = 32;
    int button_width = 120;
    int y = screen_height_ - button_height - 4;
    int x = left_size;

    drawControlKey("L", "游戏帮助", x, y);
    x += button_width + button_height + left_size * 2;
    drawControlKey("A", "确认/开始", x, y);
    x += button_width + button_height + left_size * 2;
    drawControlKey("B", "返回/取消", x, y);
    x += button_width + button_height + left_size * 2;
    drawControlKey("Y", "详细信息", x, y);
    x += button_width + button_height + left_size * 2;
    drawControlKey("X", "资源查看", x, y);
    x += button_width + button_height + left_size * 2;
    drawControlKey("R", "播放器", x, y);

    int right_button_width = button_width / 2;
    x = screen_width_ - button_height - left_size - right_button_width;
    drawControlKey("+", "设置", x, y);
    x -= right_button_width + button_height + left_size * 2;
    drawControlKey("-", "重载", x, y);
}

void GameBrowser::drawControlKey(const char* key, const char* text, int x, int y)
{
    SDL_Color text_color = {17, 24, 39, 255};
    int icon_w = 32;
    int icon_h = 32;
    int icon_y = y;

    SDL_Texture* icon_texture = nullptr;
    auto icon_it = button_textures_.find(key);
    if (icon_it != button_textures_.end()) {
        icon_texture = icon_it->second;
    }

    if (icon_texture) {
        SDL_Rect dst = {x, icon_y, icon_w, icon_h};
        SDL_RenderCopy(renderer_, icon_texture, nullptr, &dst);
        drawText(text, x + icon_w + 8, y + 6, font_small_, text_color);
        return;
    }

    drawText(key, x, y + 4, font_medium_, text_color);
    drawText(text, x + icon_w + 8, y + 6, font_small_, text_color);
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
        drawText(help_lines[i], box_x + 50, line_y, font_small_, {235, 235, 235, 255});
        line_y += 35;
    }

    drawText("按任意键关闭", box_x + 280, box_y + box_h - 40, font_small_, {200, 200, 200, 255});
}

void GameBrowser::renderInfoOverlay()
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    drawRect(0, 0, screen_width_, screen_height_, {0, 0, 0, 200}, true);

    int box_w = 560;
    int box_h = 300;
    int box_x = (screen_width_ - box_w) / 2;
    int box_y = (screen_height_ - box_h) / 2;

    drawRect(box_x, box_y, box_w, box_h, {30, 35, 50, 255}, true);
    drawRect(box_x, box_y, box_w, 60, color_accent1_, true);

    drawText("提示", box_x + 230, box_y + 15, font_large_, {255, 255, 255, 255});

    int line_gap = 26;
    int content_top = box_y + 90;
    int content_bottom = box_y + box_h - 60;
    int max_lines = (content_bottom - content_top) / line_gap;

    for (int i = 0; i < max_lines; i++) {
        int index = info_scroll_ + i;
        if (index >= static_cast<int>(info_lines_.size())) {
            break;
        }
        drawText(info_lines_[index].c_str(), box_x + 40, content_top + i * line_gap,
                 font_small_, {235, 235, 235, 255});
    }

    drawText("上/下滚动  B关闭", box_x + 180, box_y + box_h - 40, font_small_, {200, 200, 200, 255});
}

void GameBrowser::updateInfoLines()
{
    info_lines_.clear();
    size_t start = 0;
    while (start < info_text_.size()) {
        size_t end = info_text_.find('\n', start);
        std::string line = (end == std::string::npos) ? info_text_.substr(start)
                                                      : info_text_.substr(start, end - start);
        info_lines_.push_back(line);
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
}

std::string GameBrowser::buildInfoText(const GameInfo& game) const
{
    std::string text = "游戏: " + game.name;
    text += "\n脚本: " + game.script_file;
    text += "\n路径: " + game.path;
    if (game.has_cover) {
        text += "\n封面: " + game.cover_file_path;
    } else {
        text += "\n封面: 默认";
    }
    return text;
}

std::string GameBrowser::buildResourceText(const GameInfo& game) const
{
    std::string text = "资源列表:";

    DIR* dir = opendir(game.path.c_str());
    if (!dir) {
        text += "\n(无法打开目录)";
        return text;
    }

    struct dirent* entry;
    int count = 0;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        text += "\n";
        text += entry->d_name;
        count++;
        if (count >= 8) {
            break;
        }
    }

    closedir(dir);
    if (count == 0) {
        text += "\n(空)";
    }

    return text;
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

void GameBrowser::drawCircle(int x, int y, int radius, SDL_Color color, bool filled)
{
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            int dist = w * w + h * h;
            if (filled) {
                if (dist <= radius * radius) {
                    SDL_RenderDrawPoint(renderer_, x + w, y + h);
                }
            } else {
                if (dist >= (radius - 1) * (radius - 1) && dist <= radius * radius) {
                    SDL_RenderDrawPoint(renderer_, x + w, y + h);
                }
            }
        }
    }
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
    int width = 42;
    int height = 24;
    int tip_width = 6;
    int tip_height = 8;

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

    if (default_icon_texture_) {
        SDL_DestroyTexture(default_icon_texture_);
        default_icon_texture_ = nullptr;
    }

    for (auto& entry : button_textures_) {
        if (entry.second) {
            SDL_DestroyTexture(entry.second);
        }
    }
    button_textures_.clear();

    for (auto& game : games_) {
        if (game.cover_texture_) {
            SDL_DestroyTexture(game.cover_texture_);
            game.cover_texture_ = nullptr;
        }
        game.texture_loaded = false;
    }

    games_.clear();
}

#endif // SWITCH
