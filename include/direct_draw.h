/* -*- C++ -*-
*
*  direct_draw.h
*
*  Copyright (C) 2015 jh10001 <jh10001@live.cn>
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
#pragma once
#include <SDL.h>

#define MAX_TEXTURE_NUM 16

class DirectDraw
{
    SDL_Texture *texture_info[MAX_TEXTURE_NUM] = {0};
public:
    void loadTexture(int no, const char *filename);
    void deleteTexture(int no);
    void draw(int no, int dx, int dy, int w, int h, int sx, int sy, int alpha = 255, bool add = false);
    void draw2(int no, int dcx, int dcy, int sx, int sy, int w, int h, float xs, float ys, float rot, int alpha, bool add = false);
    void fill(int lx, int ly, int rx, int ry, int r, int g, int b);
    void getTextureSize(int no, int &w, int &h);
    void present();
    void clear();
};