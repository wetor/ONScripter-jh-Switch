/* -*- C++ -*-
 *
 *  Common.h - Common definitions and includes for ONScripter-jh Switch
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

#ifndef __COMMON_H__
#define __COMMON_H__

// Standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

// Archive readers
#include "SarReader.h"
#include "NsaReader.h"

// External errno declaration
extern int errno;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NSA archive decryptor main function
 * Used for decrypting encrypted NSA archives
 *
 * @param file Path to the NSA file to decrypt
 * @return 0 on success, non-zero on error
 */
int nsadec_main(char* file);

#ifdef __cplusplus
}
#endif

// Platform-specific definitions
#if defined(SWITCH)
    // Nintendo Switch specific settings
    #define ONS_PLATFORM_NAME "Nintendo Switch"
    #define ONS_DEFAULT_SAVE_DIR "sdmc:/onsemu/"
    #define ONS_PATH_SEPARATOR "/"
#elif defined(_WIN32) || defined(_WIN64)
    #define ONS_PLATFORM_NAME "Windows"
    #define ONS_DEFAULT_SAVE_DIR "./"
    #define ONS_PATH_SEPARATOR "\\"
#elif defined(__APPLE__)
    #define ONS_PLATFORM_NAME "macOS"
    #define ONS_DEFAULT_SAVE_DIR "./"
    #define ONS_PATH_SEPARATOR "/"
#elif defined(__linux__)
    #define ONS_PLATFORM_NAME "Linux"
    #define ONS_DEFAULT_SAVE_DIR "./"
    #define ONS_PATH_SEPARATOR "/"
#elif defined(ANDROID)
    #define ONS_PLATFORM_NAME "Android"
    #define ONS_DEFAULT_SAVE_DIR "/sdcard/ons/"
    #define ONS_PATH_SEPARATOR "/"
#elif defined(__vita__)
    #define ONS_PLATFORM_NAME "PS Vita"
    #define ONS_DEFAULT_SAVE_DIR "ux0:data/ons/"
    #define ONS_PATH_SEPARATOR "/"
#else
    #define ONS_PLATFORM_NAME "Unknown"
    #define ONS_DEFAULT_SAVE_DIR "./"
    #define ONS_PATH_SEPARATOR "/"
#endif

// Common macros
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) do { if (p) { delete (p); (p) = nullptr; } } while(0)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) do { if (p) { delete[] (p); (p) = nullptr; } } while(0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p) do { if (p) { free(p); (p) = nullptr; } } while(0)
#endif

// Array size helper
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// Unused parameter helper (to suppress warnings)
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#endif // __COMMON_H__
