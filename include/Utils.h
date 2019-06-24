/* -*- C++ -*-
 *
 *  Utils.h
 *
 *  Copyright (C) 2014 jh10001 <jh10001@live.cn>
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

#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdio.h>
#include <stdarg.h>

namespace utils{
	inline void printInfo(const char *format, ...){
		va_list ap;
		va_start(ap, format);
#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_VERBOSE, "Info", format, ap);
#elif defined(WINRT)
		char *buf = new char[256];
		vsprintf(buf, format, ap);
		LPWSTR wstr = new WCHAR[128];
		MByteToWChar(buf, wstr, 256);
		OutputDebugString(wstr);
#else
		vprintf(format, ap);
#endif
		va_end(ap);
	}

	inline void printError(const char *format, ...){
		va_list ap;
		va_start(ap, format);
#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, "ERR", format, ap);
#elif defined(WINRT)
		char *buf = new char[256];
		vsprintf(buf,format,ap);
		LPWSTR wstr = new WCHAR[128];
		MByteToWChar(buf, wstr, 256);
		OutputDebugString(wstr);
#else
		vfprintf(stderr, format, ap);
#endif
		va_end(ap);
	}
}

#endif
