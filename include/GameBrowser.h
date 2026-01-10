/* -*- C++ -*-
 *
 *  GameBrowser.h - Game selection browser for ONScripter
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

#ifndef __GAME_BROWSER_H__
#define __GAME_BROWSER_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>

/**
 * Game information structure
 */
struct GameInfo {
    std::string path;           // Full path to game folder
    std::string name;           // Display name (folder name)
    bool has_script;            // Whether game has valid script file
    std::string script_file;    // Script file name (0.txt, 00.txt, etc)

    GameInfo() : has_script(false) {}
    GameInfo(const std::string& p, const std::string& n, bool hs, const std::string& sf)
        : path(p), name(n), has_script(hs), script_file(sf) {}
};

/**
 * Game Browser - SDL2-based game selection interface
 */
class GameBrowser {
public:
    GameBrowser();
    ~GameBrowser();

    /**
     * Initialize browser with SDL window and renderer
     * @param window SDL window
     * @param renderer SDL renderer
     * @return true if successful
     */
    bool init(SDL_Window* window, SDL_Renderer* renderer);

    /**
     * Scan for games in the specified directory
     * @param base_path Base directory to scan (e.g., "sdmc:/onsemu")
     * @return Number of games found
     */
    int scanGames(const char* base_path);

    /**
     * Run the browser loop
     * @return Selected game index, or -1 if cancelled
     */
    int run();

    /**
     * Get game info by index
     * @param index Game index
     * @return Pointer to GameInfo, or nullptr if invalid index
     */
    const GameInfo* getGameInfo(int index) const;

    /**
     * Get number of games found
     */
    int getGameCount() const { return games_.size(); }

    /**
     * Cleanup resources
     */
    void cleanup();

private:
    // SDL resources
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_large_;
    TTF_Font* font_small_;

    // Game list
    std::vector<GameInfo> games_;
    int selected_index_;
    int scroll_offset_;

    // Display settings
    int screen_width_;
    int screen_height_;
    int items_per_page_;

    // Colors
    SDL_Color color_background_;
    SDL_Color color_text_;
    SDL_Color color_selected_;
    SDL_Color color_disabled_;
    SDL_Color color_highlight_;

    // State
    bool running_;

    // Private methods
    bool loadFonts();
    void render();
    void renderTitle();
    void renderGameList();
    void renderHelp();
    void renderGameItem(int index, int y_pos);
    void handleInput();
    void moveSelection(int delta);
    bool isValidGameFolder(const char* path, GameInfo& info);
    SDL_Texture* createTextTexture(const char* text, TTF_Font* font, SDL_Color color);
    void drawText(const char* text, int x, int y, TTF_Font* font, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color, bool filled);
};

#endif // __GAME_BROWSER_H__
