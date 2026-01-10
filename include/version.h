/* -*- C++ -*-
 *
 *  version.h - Version information for ONScripter-jh Switch
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
 *            (C) 2022-2023 yurisizuku <https://github.com/YuriSizuku>
 *            (C) 2019-2025 ONScripter-jh-Switch contributors
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

#ifndef __VERSION_H__
#define __VERSION_H__

// ONScripter-jh Switch version
// Based on OnscripterYuri with Switch-specific enhancements
#define ONS_NX_VERSION "2.2.0"

// Upstream version information
#define ONS_YURI_VERSION "0.7.6"    // OnscripterYuri version we're based on
#define ONS_JH_VERSION "0.8.0"      // ONScripter-jh version (updated from Yuri)
#define ONS_VERSION "20181218"      // Original ONScripter base version
#define NSC_VERSION 296             // NScripter compatibility version

// Version components for programmatic access
#define ONS_NX_VERSION_MAJOR 2
#define ONS_NX_VERSION_MINOR 2
#define ONS_NX_VERSION_PATCH 0

// Feature flags - indicates which OnscripterYuri features are supported
#define ONS_FEATURE_UTF8_SCRIPT     1   // UTF-8 script encoding support
#define ONS_FEATURE_FORCE_RESOLUTION 1  // Arbitrary resolution support
#define ONS_FEATURE_SHARPNESS       1   // GLES sharpness rendering
#define ONS_FEATURE_STRETCH_MODE    1   // Fullscreen with stretch

// Build information
#define ONS_BUILD_PLATFORM "Nintendo Switch"

#endif // __VERSION_H__
