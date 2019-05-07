#ifndef KITSUBTITLEPACKET_H
#define KITSUBTITLEPACKET_H

#include <stdbool.h>
#include <SDL_surface.h>

#include "kitchensink/kitconfig.h"

typedef struct Kit_SubtitlePacket {
    double pts_start;
    double pts_end;
    int x;
    int y;
    bool clear;
    SDL_Surface *surface;
} Kit_SubtitlePacket;

KIT_LOCAL Kit_SubtitlePacket* Kit_CreateSubtitlePacket(
    bool clear, double pts_start, double pts_end, int pos_x, int pos_y, SDL_Surface *surface);
KIT_LOCAL void Kit_FreeSubtitlePacket(Kit_SubtitlePacket *packet);

#endif // KITSUBTITLEPACKET_H
