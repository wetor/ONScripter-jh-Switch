#ifndef SWITCH_INPUT_LOGIC_H
#define SWITCH_INPUT_LOGIC_H

#include "mock_sdl.h"
#include <cstdlib>

namespace SwitchInput {

static int transJoystickAxis_old_axis = -1;

inline void resetAxisState() {
    transJoystickAxis_old_axis = -1;
}

inline SDL_KeyboardEvent transJoystickAxis(SDL_JoyAxisEvent jaxis, bool left_is_mouse = false)
{
    SDL_KeyboardEvent event;
    event.type = 0;
    event.keysym.sym = SDLK_UNKNOWN;

    ONS_Key axis_map[] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN};
    int axis = -1;
    
    if (jaxis.axis < 2) {
        if (left_is_mouse) {
            event.keysym.sym = SDLK_UNKNOWN;
            return event;
        }
    }
    else {
        jaxis.axis -= 2;
    }

    if (jaxis.axis < 2) {
        axis = ((3200 > jaxis.value) && (jaxis.value > -3200) ? -1 : (jaxis.axis * 2 + (jaxis.value > 0 ? 1 : 0)));
    }

    if (axis != transJoystickAxis_old_axis) {
        if (axis == -1) {
            event.type = SDL_KEYUP;
            event.keysym.sym = axis_map[transJoystickAxis_old_axis];
        }
        else {
            event.type = SDL_KEYDOWN;
            event.keysym.sym = axis_map[axis];
        }
        transJoystickAxis_old_axis = axis;
    }
    else {
        event.keysym.sym = SDLK_UNKNOWN;
    }

    return event;
}

inline SDL_Keycode transJoystickButton(Uint8 button)
{
    SDL_Keycode button_map[] = {
        SDLK_RETURN,  /* A */
        SDLK_RCTRL,   /* B */
        SDLK_a,       /* X */
        SDLK_ESCAPE,  /* Y */
        SDLK_F2,      /* LSTICK */
        SDLK_UNKNOWN, /* RSTICK */
        SDLK_o,       /* L */
        SDLK_s,       /* R */
        SDLK_UNKNOWN, /* ZL */
        SDLK_UNKNOWN, /* ZR */
        SDLK_SPACE,   /* + START */
        SDLK_0,       /* - SELECT */
        SDLK_LEFT,    /* LEFT */
        SDLK_UP,      /* UP */
        SDLK_RIGHT,   /* RIGHT */
        SDLK_DOWN,    /* DOWN */
        SDLK_UNKNOWN, /* L LEFT */
        SDLK_UNKNOWN, /* L UP */
        SDLK_UNKNOWN, /* L RIGHT */
        SDLK_UNKNOWN, /* L DOWN */
        SDLK_UNKNOWN, /* R LEFT */
        SDLK_UNKNOWN, /* R UP */
        SDLK_UNKNOWN, /* R RIGHT */
        SDLK_UNKNOWN, /* R DOWN */
        SDLK_UNKNOWN, /* SL_LEFT */
        SDLK_UNKNOWN, /* SR_LEFT */
        SDLK_UNKNOWN, /* SL_RIGHT */
        SDLK_UNKNOWN  /* SR_RIGHT */
    };
    if (button < sizeof(button_map) / sizeof(button_map[0]))
        return button_map[button];
    return SDLK_UNKNOWN;
}

struct MouseMoveResult {
    float x;
    float y;
    bool moved;
};

inline MouseMoveResult calculateMouseMove(
    int current_x, int current_y,
    int screen_width, int screen_height,
    int screen_device_width, int screen_device_height,
    Sint16 axis_value, Uint8 axis_id)
{
    MouseMoveResult result;
    result.x = (float)current_x * screen_device_width / screen_width;
    result.y = (float)current_y * screen_device_height / screen_height;
    result.moved = false;
    
    if (axis_id >= 2) {
        return result;
    }

    float level = axis_value >> 12;
    if (level >= 0)
        level++;

    if (level > 1 || level < -1) {
        if (axis_id == 0) {
            result.x += 0.2f * abs((int)level) * level + 2.0;
            result.moved = true;
            if (level < 0) result.x -= 0.5;
        }
        if (axis_id == 1) {
            result.y += 0.2f * abs((int)level) * level + 2;
            result.moved = true;
            if (level < 0) result.y -= 0.5;
        }
    }
    
    return result;
}

}

#endif
