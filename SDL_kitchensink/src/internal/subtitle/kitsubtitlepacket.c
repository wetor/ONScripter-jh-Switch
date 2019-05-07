#include "kitchensink/internal/subtitle/kitsubtitlepacket.h"


Kit_SubtitlePacket* Kit_CreateSubtitlePacket(
        bool clear, double pts_start, double pts_end, int pos_x, int pos_y, SDL_Surface *surface)
{
    Kit_SubtitlePacket *p = calloc(1, sizeof(Kit_SubtitlePacket));
    p->pts_start = pts_start;
    p->pts_end = pts_end;
    p->x = pos_x;
    p->y = pos_y;
    p->surface = surface;
    if(p->surface != NULL) {
        p->surface->refcount++; // We dont want to needlessly copy; instead increase refcount.
    }
    p->clear = clear;
    return p;
}

void Kit_FreeSubtitlePacket(Kit_SubtitlePacket *p) {
    SDL_FreeSurface(p->surface);
    free(p);
}
