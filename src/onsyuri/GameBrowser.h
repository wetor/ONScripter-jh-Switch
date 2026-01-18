/* -*- C++ -*-
 *
 *  GameBrowser.h - Game selection browser for ONScripter Switch
 *
 *  Copyright (c) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __GAME_BROWSER_H__
#define __GAME_BROWSER_H__

#ifdef SWITCH

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <switch.h>
#include <string>
#include <vector>

struct GameInfo {
    std::string path;
    std::string name;
    std::string script_file;
    std::string cover_file_path;
    SDL_Texture* cover_texture_;
    bool has_script;
    bool has_font;
    bool has_cover;
    bool texture_loaded;

    GameInfo() : cover_texture_(nullptr), has_script(false), has_font(false), has_cover(false), texture_loaded(false) {}
    ~GameInfo() {
        if (cover_texture_) {
            SDL_DestroyTexture(cover_texture_);
            cover_texture_ = nullptr;
        }
    }
};

class GameBrowser {
public:
    GameBrowser();
    ~GameBrowser();

    // Initialize with existing SDL window/renderer
    bool init(SDL_Window* window, SDL_Renderer* renderer);

    // Scan directory for games
    int scanGames(const char* base_path);

    // Run browser loop, returns selected index or -1 if cancelled
    int run();

    // Get game info by index
    const GameInfo* getGameInfo(int index) const;

    // Get selected game path
    const char* getSelectedPath() const;

    // Cleanup resources
    void cleanup();

private:
    // Check if folder contains valid game
    bool isValidGameFolder(const char* path, GameInfo& info);

    // Input handling
    void handleInput();
    void moveSelection(int delta);

    void render();
    void renderStatusBar();
    void renderCarousel();
    void renderBottomBar();
    void renderNoGames();
    void renderHelpOverlay();

    void renderGameCard(int index, float x, float y, float width, float height, float scale, float alpha);
    void drawButton(int x, int y, int radius, bool is_left, bool is_enabled);

    void drawText(const char* text, int x, int y, TTF_Font* font, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color, bool filled);
    void drawRoundedRect(int x, int y, int w, int h, int radius, SDL_Color color);
    void drawShadow(int x, int y, int w, int h, int offset, SDL_Color color);
    void drawBatteryIcon(int x, int y, int percentage);
    void drawControlKey(const char* key, const char* text, int x, int y);

    bool loadFonts();
    bool loadGameCover(GameInfo& game);
    bool loadCoverTexture(GameInfo& game);

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_large_;
    TTF_Font* font_medium_;
    TTF_Font* font_small_;
    TTF_Font* font_icon_;

    std::vector<GameInfo> games_;
    int selected_index_;
    int scroll_offset_;
    int carousel_scroll_;
    int screen_width_;
    int screen_height_;
    int items_per_page_;
    bool running_;
    bool show_help_;
    PadState pad_;

    SDL_Color color_background_;
    SDL_Color color_text_;
    SDL_Color color_accent1_;
    SDL_Color color_accent2_;
    SDL_Color color_shadow_;
    SDL_Color color_selected_border_;
    SDL_Color color_disabled_;

    static constexpr int STATUS_BAR_HEIGHT = 60;
    static constexpr int BOTTOM_BAR_HEIGHT = 80;
    static constexpr int CAROUSEL_START_Y = 480;
    static constexpr int CARD_WIDTH = 200;
    static constexpr int CARD_HEIGHT = 280;
    static constexpr int CARD_SPACING = 30;
};

#endif // SWITCH

#endif // __GAME_BROWSER_H__
