/* -*- C++ -*-
 *
 *  main.h - Main header file for ONScripter-jh Nintendo Switch
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
 *            (C) 2019-2025 wetor <makisehoshimi@163.com>
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

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Mouse cursor image data (PNG format)
 * Loaded from romfs at startup
 */
extern void *mouse_png;
extern int mouse_png_size;

/**
 * English mode flag
 * 0 = Chinese/Japanese mode (GBK encoding)
 * 1 = English mode (SJIS encoding)
 */
extern int english;



/**
 * ONScripter main entry point
 * Called after system initialization
 *
 * @param argc Number of command line arguments
 * @param argv Command line argument array
 * @return 0 on success, non-zero on error
 */
int OnsMain(int argc, char *argv[]);

/**
 * Exit handler for ONScripter
 * Cleans up resources and returns to HBMenu
 * @param flag Exit status (0 for success, non-zero for error)
 */
void ons_exit(int flag);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
