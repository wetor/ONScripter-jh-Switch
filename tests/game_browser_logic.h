#ifndef GAME_BROWSER_LOGIC_H
#define GAME_BROWSER_LOGIC_H

#include <string>
#include <vector>
#include <algorithm>

namespace GameBrowserLogic {

struct GameInfo {
    std::string path;
    std::string name;
    std::string script_file;
    bool has_script;
    bool has_font;

    GameInfo() : has_script(false), has_font(false) {}
};

inline void sortGamesAlphabetically(std::vector<GameInfo>& games) {
    std::sort(games.begin(), games.end(),
        [](const GameInfo& a, const GameInfo& b) {
            return a.name < b.name;
        });
}

inline bool isValidScriptFile(const std::string& filename) {
    const char* valid_scripts[] = {
        "0.txt",
        "00.txt",
        "nscript.dat",
        "nscript.___",
        "nscr_sec.dat",
        nullptr
    };
    
    for (int i = 0; valid_scripts[i] != nullptr; i++) {
        if (filename == valid_scripts[i]) {
            return true;
        }
    }
    return false;
}

inline int calculateScrollOffset(int selected_index, int items_per_page, int total_items) {
    if (total_items <= items_per_page) {
        return 0;
    }
    
    int offset = selected_index - items_per_page / 2;
    if (offset < 0) {
        offset = 0;
    }
    if (offset > total_items - items_per_page) {
        offset = total_items - items_per_page;
    }
    return offset;
}

inline int clampSelection(int index, int total_items) {
    if (total_items == 0) return -1;
    if (index < 0) return 0;
    if (index >= total_items) return total_items - 1;
    return index;
}

inline int moveSelectionWithWrap(int current, int delta, int total_items) {
    if (total_items == 0) return -1;
    int new_index = current + delta;
    return clampSelection(new_index, total_items);
}

inline int calculateTouchedIndex(int touch_y, int list_start_y, int item_height, int scroll_offset, int total_items) {
    if (touch_y < list_start_y) return -1;
    
    int relative_y = touch_y - list_start_y;
    int index = relative_y / item_height + scroll_offset;
    
    if (index < 0 || index >= total_items) {
        return -1;
    }
    return index;
}

struct Color {
    unsigned char r, g, b, a;
};

inline Color getDefaultBackgroundColor() {
    return {25, 30, 40, 255};
}

inline Color getDefaultTextColor() {
    return {230, 230, 230, 255};
}

inline Color getDefaultSelectedColor() {
    return {45, 130, 220, 255};
}

inline Color getDefaultHighlightColor() {
    return {255, 180, 50, 255};
}

}

#endif
