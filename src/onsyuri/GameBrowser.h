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
    bool has_script;
    bool has_font;

    GameInfo() : has_script(false), has_font(false) {}
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

    // Rendering
    void render();
    void renderTitle();
    void renderGameList();
    void renderGameItem(int index, int y_pos);
    void renderHelp();
    void renderNoGames();
    void renderHelpOverlay();

    // Drawing helpers
    void drawText(const char* text, int x, int y, TTF_Font* font, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color, bool filled);

    // Font loading
    bool loadFonts();

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_large_;
    TTF_Font* font_small_;

    std::vector<GameInfo> games_;
    int selected_index_;
    int scroll_offset_;

    int screen_width_;
    int screen_height_;
    int items_per_page_;

    bool running_;
    bool show_help_;

    // Colors
    SDL_Color color_background_;
    SDL_Color color_text_;
    SDL_Color color_selected_;
    SDL_Color color_disabled_;
    SDL_Color color_highlight_;

    // Gamepad state
    PadState pad_;
};

#endif // SWITCH

#endif // __GAME_BROWSER_H__
