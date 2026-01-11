/**
 * Mock SDL types for testing Switch-specific functionality
 * without requiring actual SDL2 installation
 */

#ifndef MOCK_SDL_H
#define MOCK_SDL_H

#include <stdint.h>

typedef int32_t Sint32;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef uint8_t Uint8;
typedef int32_t SDL_Keycode;

#define SDLK_UNKNOWN    0
#define SDLK_RETURN     13
#define SDLK_ESCAPE     27
#define SDLK_SPACE      32
#define SDLK_0          48
#define SDLK_a          97
#define SDLK_o          111
#define SDLK_s          115
#define SDLK_LEFT       1073741904
#define SDLK_RIGHT      1073741903
#define SDLK_UP         1073741906
#define SDLK_DOWN       1073741905
#define SDLK_RCTRL      1073742052
#define SDLK_F2         1073741883

#define SDL_KEYDOWN     0x300
#define SDL_KEYUP       0x301

typedef struct {
    SDL_Keycode sym;
} SDL_Keysym;

typedef struct {
    Uint32 type;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct {
    Uint8 axis;
    Sint16 value;
} SDL_JoyAxisEvent;

typedef SDL_Keycode ONS_Key;

#endif
