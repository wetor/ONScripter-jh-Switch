#ifndef SCREEN_LOGIC_H
#define SCREEN_LOGIC_H

namespace ScreenLogic {

struct Rect {
    int x, y, w, h;
};

inline float calculateScaleRatio(int screen_width, int screen_height, 
                                  int device_width, int device_height) {
    float scale_x = (float)device_width / screen_width;
    float scale_y = (float)device_height / screen_height;
    return (scale_x < scale_y) ? scale_x : scale_y;
}

inline Rect calculateRenderRect(int screen_width, int screen_height,
                                 int device_width, int device_height,
                                 bool stretch_mode) {
    Rect rect;
    
    if (stretch_mode) {
        rect.x = 0;
        rect.y = 0;
        rect.w = device_width;
        rect.h = device_height;
    } else {
        float scale = calculateScaleRatio(screen_width, screen_height, 
                                          device_width, device_height);
        rect.w = (int)(screen_width * scale);
        rect.h = (int)(screen_height * scale);
        rect.x = (device_width - rect.w) / 2;
        rect.y = (device_height - rect.h) / 2;
    }
    
    return rect;
}

inline int deviceToScreenX(int device_x, int screen_width, 
                           int render_x, int render_w) {
    if (device_x < render_x) return 0;
    if (device_x >= render_x + render_w) return screen_width - 1;
    return (device_x - render_x) * screen_width / render_w;
}

inline int deviceToScreenY(int device_y, int screen_height,
                           int render_y, int render_h) {
    if (device_y < render_y) return 0;
    if (device_y >= render_y + render_h) return screen_height - 1;
    return (device_y - render_y) * screen_height / render_h;
}

inline int screenToDeviceX(int screen_x, int screen_width,
                           int render_x, int render_w) {
    return render_x + screen_x * render_w / screen_width;
}

inline int screenToDeviceY(int screen_y, int screen_height,
                           int render_y, int render_h) {
    return render_y + screen_y * render_h / screen_height;
}

inline int clampMouseX(int x, int screen_width) {
    if (x < 0) return 0;
    if (x >= screen_width) return screen_width - 1;
    return x;
}

inline int clampMouseY(int y, int screen_height) {
    if (y < 0) return 0;
    if (y >= screen_height) return screen_height - 1;
    return y;
}

inline int getDefaultSwitchWidth() {
    return 1280;
}

inline int getDefaultSwitchHeight() {
    return 720;
}

}

#endif
