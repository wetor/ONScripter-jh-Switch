/* -*- C++ -*-
 *
 *  main.cpp - Main entry point for ONScripter-jh Nintendo Switch
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
 *            (C) 2022-2023 yurisizuku <https://github.com/YuriSizuku>
 *            (C) 2019-2025 ONScripter-jh-Switch contributors
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

#include <switch.h>

#ifdef DEBUG
#include <twili.h>
#endif

#include "main.h"
#include "Utils.h"
#include "Common.h"
#include "version.h"
#include "GameBrowser.h"

#include <string>
#include <dirent.h>

// Global path variables for file-based logging (from OnscripterYuri)
std::string g_stdoutpath = "sdmc:/onsemu/stdout.txt";
std::string g_stderrpath = "sdmc:/onsemu/stderr.txt";

// Global variables
void *mouse_png = nullptr;
int mouse_png_size = 0;
int english = 0;
// Global gamepad state
PadState pad;

// Constants
static constexpr const char* MOUSE_CURSOR_PATH = "romfs:/cursor/mouse.png";
static constexpr int MOUSE_PNG_EXPECTED_SIZE = 1699;

// Forward declarations
static bool initializeSystem();
static void cleanupSystem();
static bool loadMouseCursor();
static void parseCommandLineArgs(int argc, char* argv[], char* path, int* fullmode, int* outline);

/**
 * Parse command line arguments
 *
 * Args format:
 * 0 : self NRO program path
 * 1 : ONScripter game folder
 * 2 : mode: 0 default; mode&1 fullscreen stretch; mode&2 outline; mode&4 english
 */
static void parseCommandLineArgs(int argc, char* argv[], char* path, int* fullmode, int* outline)
{
    if (!envHasArgv() || argc <= 1) {
        utils::printWarning("No command line arguments provided\n");
        return;
    }

    // Copy game path
    if (argv[1]) {
        utils::strncpy_safe(path, argv[1], 256);
        utils::printInfo("Game path: %s\n", path);
    }

    // Parse mode flags
    if (argc > 2 && argv[2]) {
        int setting = atoi(argv[2]);
        if (setting & 1) {
            *fullmode = 1;
            utils::printDebug("Fullscreen mode enabled\n");
        }
        if (setting & 2) {
            *outline = 1;
            utils::printDebug("Font outline enabled\n");
        }
        if (setting & 4) {
            english = 1;
            utils::printDebug("English mode enabled\n");
        }
    }
}

/**
 * Initialize Switch system services
 */
static bool initializeSystem()
{
    // Initialize random seed first
    srand(static_cast<unsigned int>(time(nullptr)));

#ifdef DEBUG
    if (R_FAILED(twiliInitialize())) {
        // Debug init failed, but continue anyway
    }
    twiliBindStdio();
#endif

    // Initialize romfs early (before logging)
    Result rc = romfsInit();
    if (R_FAILED(rc)) {
        // Can't log yet, just return false
        return false;
    }

    // Try to redirect stdout/stderr to files for debugging
    // Don't fail if this doesn't work - continue with console output
    freopen(g_stdoutpath.c_str(), "w", stdout);
    freopen(g_stderrpath.c_str(), "w", stderr);

    // Disable buffering so logs flush immediately
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Enable debug logging
    utils::setLogLevel(utils::LogLevel::DEBUG);

    utils::printInfo("=== ONScripter System Initialization ===\n");
    utils::printInfo("ROM filesystem initialized\n");

    return true;
}

/**
 * Clean up system resources
 */
static void cleanupSystem()
{
    // Free mouse cursor memory
    if (mouse_png) {
        free(mouse_png);
        mouse_png = nullptr;
        mouse_png_size = 0;
    }

    // Exit romfs
    romfsExit();

#ifdef DEBUG
    twiliExit();
#endif
}

/**
 * Load mouse cursor image from romfs
 */
static bool loadMouseCursor()
{
    mouse_png_size = MOUSE_PNG_EXPECTED_SIZE;
    mouse_png = malloc(mouse_png_size);

    if (!mouse_png) {
        utils::printError("Failed to allocate memory for mouse cursor\n");
        return false;
    }

    FILE* f = fopen(MOUSE_CURSOR_PATH, "rb");
    if (!f) {
        utils::printError("Failed to open mouse cursor: %s\n", MOUSE_CURSOR_PATH);
        free(mouse_png);
        mouse_png = nullptr;
        mouse_png_size = 0;
        return false;
    }

    size_t bytesRead = fread(mouse_png, 1, mouse_png_size, f);
    fclose(f);

    if (bytesRead != static_cast<size_t>(mouse_png_size)) {
        utils::printWarning("Mouse cursor file size mismatch: expected %d, got %zu\n",
                           mouse_png_size, bytesRead);
        // Continue anyway, might still work
    }

    utils::printDebug("Mouse cursor loaded successfully\n");
    return true;
}

/**
 * Exit handler - returns to HBMenu
 */
void ons_exit(int flag)
{
    cleanupSystem();
    exit(flag == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

/**
 * Main entry point
 */
int main(int argc, char* argv[])
{
    // Print early boot message (before system init)
    printf("ONScripter-jh for Nintendo Switch starting...\n");

    // Initialize system
    if (!initializeSystem()) {
        printf("FATAL: System initialization failed\n");
        return 1;
    }

    // Print version info
    utils::printInfo("===========================================\n");
    utils::printInfo("ONScripter-jh for Nintendo Switch\n");
    utils::printInfo("Version: %s (JH: %s, ONS: %s)\n",
                     ONS_NX_VERSION, ONS_JH_VERSION, ONS_VERSION);
    utils::printInfo("NSC Version: %d.%02d\n", NSC_VERSION / 100, NSC_VERSION % 100);
    utils::printInfo("Build Date: %s %s\n", __DATE__, __TIME__);
    utils::printInfo("===========================================\n\n");

    // Parse arguments
    char path[256] = {0};
    int fullmode = 0;
    int outline = 0;

    parseCommandLineArgs(argc, argv, path, &fullmode, &outline);

    // If no game path provided, show game browser
    if (!envHasArgv() || argc <= 1 || strlen(path) == 0) {
        utils::printInfo("No command line arguments - launching game browser\n");

        // Initialize SDL for browser
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
            utils::printError("Failed to initialize SDL: %s\n", SDL_GetError());
            ons_exit(0);
            return 1;
        }

        // Initialize gamepad
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&pad);

        // Create window for browser
        SDL_Window* browser_window = SDL_CreateWindow(
            "ONScripter Game Browser",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
        );

        if (!browser_window) {
            utils::printError("Failed to create browser window: %s\n", SDL_GetError());
            SDL_Quit();
            ons_exit(0);
            return 1;
        }

        SDL_Renderer* browser_renderer = SDL_CreateRenderer(
            browser_window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

        if (!browser_renderer) {
            utils::printError("Failed to create browser renderer: %s\n", SDL_GetError());
            SDL_DestroyWindow(browser_window);
            SDL_Quit();
            ons_exit(0);
            return 1;
        }

        // Create and run browser
        GameBrowser browser;
        if (!browser.init(browser_window, browser_renderer)) {
            utils::printError("Failed to initialize game browser\n");
            SDL_DestroyRenderer(browser_renderer);
            SDL_DestroyWindow(browser_window);
            SDL_Quit();
            ons_exit(0);
            return 1;
        }

        // Scan for games
        int game_count = browser.scanGames("sdmc:/onsemu");
        if (game_count == 0) {
            utils::printWarning("No games found in sdmc:/onsemu\n");
            utils::printInfo("Please create game folders with 0.txt or 00.txt\n");

            browser.cleanup();
            SDL_DestroyRenderer(browser_renderer);
            SDL_DestroyWindow(browser_window);
            SDL_Quit();
            ons_exit(0);
            return 0;
        }

        // Run browser and get selected game
        int selected = browser.run();

        if (selected < 0) {
            utils::printInfo("Browser cancelled by user\n");
            browser.cleanup();
            SDL_DestroyRenderer(browser_renderer);
            SDL_DestroyWindow(browser_window);
            SDL_Quit();
            ons_exit(0);
            return 0;
        }

        // Get selected game info
        const GameInfo* game_info = browser.getGameInfo(selected);
        if (game_info) {
            utils::printInfo("Selected game: %s\n", game_info->name.c_str());
            utils::strncpy_safe(path, game_info->path.c_str(), sizeof(path));
        }

        // Cleanup browser resources
        browser.cleanup();
        SDL_DestroyRenderer(browser_renderer);
        SDL_DestroyWindow(browser_window);
        // Don't quit SDL yet - ONScripter will re-initialize it
    }

    utils::printInfo("Command line arguments received: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        utils::printDebug("  argv[%d] = %s\n", i, argv[i] ? argv[i] : "(null)");
    }

    utils::printInfo("Game path: %s\n", path);
    utils::printInfo("Settings - Fullscreen: %d, Outline: %d, English: %d\n", fullmode, outline, english);

    // Build argument list for ONScripter
    // Maximum 16 arguments should be enough
    const int MAX_ARGS = 16;
    char* ons_argv[MAX_ARGS];
    int ons_argc = 0;

    // argv[0] is program name (unused but required)
    ons_argv[ons_argc++] = argv[0];

    // --root <path>
    ons_argv[ons_argc++] = const_cast<char*>("--root");
    ons_argv[ons_argc++] = path;

    // --compatible
    ons_argv[ons_argc++] = const_cast<char*>("--compatible");

    // --fontcache
    ons_argv[ons_argc++] = const_cast<char*>("--fontcache");

    // Window mode
    if (fullmode) {
        ons_argv[ons_argc++] = const_cast<char*>("--fullscreen");
    } else {
        ons_argv[ons_argc++] = const_cast<char*>("--window");
    }

    // Font outline
    if (outline) {
        ons_argv[ons_argc++] = const_cast<char*>("--render-font-outline");
    }

    // English mode (SJIS encoding)
    if (english) {
        ons_argv[ons_argc++] = const_cast<char*>("--enc:sjis");
    }

    // Debug mode (uncomment if needed)
    // ons_argv[ons_argc++] = const_cast<char*>("--debug:1");

    utils::printInfo("=== Preparing to launch ONScripter engine ===\n");
    utils::printInfo("Total arguments: %d\n", ons_argc);
    for (int i = 0; i < ons_argc; i++) {
        utils::printInfo("  [%d] %s\n", i, ons_argv[i]);
    }

    // Load mouse cursor
    utils::printInfo("Loading mouse cursor...\n");
    if (!loadMouseCursor()) {
        utils::printWarning("Mouse cursor not loaded, will use default\n");
    } else {
        utils::printInfo("Mouse cursor loaded successfully\n");
    }

    utils::printInfo("\n=== Starting ONScripter Engine ===\n");

    // Run ONScripter main
    int result = OnsMain(ons_argc, ons_argv);

    utils::printInfo("\n=== ONScripter Engine Stopped ===\n");
    utils::printInfo("Exit code: %d\n", result);

    if (result != 0) {
        utils::printError("ONScripter returned error code: %d\n", result);
    }

    // Clean exit
    ons_exit(EXIT_SUCCESS);

    return 0;
}
