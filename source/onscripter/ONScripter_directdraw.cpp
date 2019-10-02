/* -*- C++ -*-
* 
*  ONScripter_directdraw.cpp
*
*  Copyright (C) 2015-2016 jh10001 <jh10001@live.cn>
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

#include "ONScripter.h"
#include <string.h>
#include <assert.h>

extern ONScripter ons;

void DirectDraw::loadTexture(int no, const char *filename)
{
    assert(no >= 0 && no < MAX_TEXTURE_NUM);
    if (!texture_info[no])
    {
        SDL_DestroyTexture(texture_info[no]);
        texture_info[no] = NULL;
    }
    char s[256];
    strncpy(s, filename, sizeof(s));
    bool has_alpha;
    int location;
    SDL_Surface *surface = ons.createSurfaceFromFile(s, &has_alpha, &location);
    if (surface) texture_info[no] = SDL_CreateTextureFromSurface(ons.renderer, surface);
}

void DirectDraw::deleteTexture(int no)
{
    assert(no >= 0 && no < MAX_TEXTURE_NUM);
    if (!texture_info[no]) return;
    SDL_DestroyTexture(texture_info[no]);
    texture_info[no] = NULL;
}

void DirectDraw::draw(int no, int dx, int dy, int w, int h, int sx, int sy, int alpha, bool add)
{
    assert(no >= 0 && no < MAX_TEXTURE_NUM);
    if (!texture_info[no]) return;
    SDL_SetTextureBlendMode(texture_info[no], add ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture_info[no], alpha);
    SDL_Rect src_rect = {sx, sy, w, h}, dst_rect = {dx, dy, w, h};
    ons.setScreenDirty(true);
    SDL_RenderCopy(ons.renderer, texture_info[no], &src_rect, &dst_rect);
}

void DirectDraw::draw2(int no, int dcx, int dcy, int sx, int sy, int w, int h, float xs, float ys, float rot, int alpha, bool add)
{
    assert(no >= 0 && no < MAX_TEXTURE_NUM);
    if (!texture_info[no]) return;
    SDL_SetTextureBlendMode(texture_info[no], add ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture_info[no], alpha);
    int dx = dcx-w/2*xs, dy = dcy-h/2*ys;
    SDL_Rect src_rect = {sx, sy, w, h}, dst_rect = {dx, dy, (int)(w*xs), (int)(h*ys)};
    ons.setScreenDirty(true);
    SDL_RenderCopyEx(ons.renderer, texture_info[no], &src_rect, &dst_rect, rot, NULL, SDL_FLIP_NONE);
}

void DirectDraw::fill(int lx, int ly, int rx, int ry, int r, int g, int b)
{
    SDL_SetRenderDrawColor(ons.renderer, r, g, b , SDL_ALPHA_OPAQUE);
    SDL_Rect rect;
    rect.x = lx;
    rect.y = ly;
    rect.w = rx - lx;
    rect.h = ry - ly;
    SDL_RenderFillRect(ons.renderer, &rect);
}

void DirectDraw::getTextureSize(int no, int &w, int &h)
{
    assert(no >= 0 && no < MAX_TEXTURE_NUM);
    if (!texture_info[no])
    {
        w = 0;
        h = 0;
        return;
    }
    SDL_QueryTexture(texture_info[no], NULL, NULL, &w, &h);
}


void DirectDraw::present() 
{
    SDL_RenderPresent(ons.renderer);
}

void DirectDraw::clear() 
{
    SDL_SetRenderDrawColor(ons.renderer, 0, 0, 0 , SDL_ALPHA_OPAQUE);
    SDL_RenderClear(ons.renderer);
}
