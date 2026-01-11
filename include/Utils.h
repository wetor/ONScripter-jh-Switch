/* -*- C++ -*-
 *
 *  Utils.h - Utility functions and logging system
 *
 *  Copyright (C) 2014 jh10001 <jh10001@live.cn>
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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <utility>
#include <string>

#ifdef ANDROID
#include <android/log.h>
#endif

#ifdef WINRT
#include <windows.h>
#endif

// Global paths for file-based logging (from OnscripterYuri)
extern std::string g_stdoutpath;
extern std::string g_stderrpath;

namespace utils {

// Log levels
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    NONE = 4
};

// Current log level (can be changed at runtime)
inline LogLevel& getLogLevel() {
    static LogLevel level = LogLevel::INFO;
    return level;
}

inline void setLogLevel(LogLevel level) {
    getLogLevel() = level;
}

// Get current timestamp string
inline const char* getTimestamp() {
    static char buffer[32];
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    if (tm_info) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    } else {
        buffer[0] = '\0';
    }
    return buffer;
}

// Get log level string
inline const char* getLogLevelStr(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

// Core logging function
inline void logMessage(LogLevel level, const char* format, va_list ap) {
    if (level < getLogLevel()) {
        return;
    }

#ifdef ANDROID
    int androidLevel;
    switch (level) {
        case LogLevel::DEBUG:   androidLevel = ANDROID_LOG_DEBUG; break;
        case LogLevel::INFO:    androidLevel = ANDROID_LOG_INFO; break;
        case LogLevel::WARNING: androidLevel = ANDROID_LOG_WARN; break;
        case LogLevel::ERROR:   androidLevel = ANDROID_LOG_ERROR; break;
        default:                androidLevel = ANDROID_LOG_VERBOSE; break;
    }
    __android_log_vprint(androidLevel, "ONScripter", format, ap);

#elif defined(WINRT)
    char buf[512];
    vsnprintf(buf, sizeof(buf), format, ap);
    WCHAR wstr[256];
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wstr, 256);
    OutputDebugStringW(wstr);

#else
    // Console output with timestamp and level
    FILE* out = (level >= LogLevel::WARNING) ? stderr : stdout;
    fprintf(out, "[%s][%s] ", getTimestamp(), getLogLevelStr(level));
    vfprintf(out, format, ap);
    fflush(out);
#endif
}

// Debug level logging
inline void printDebug(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    logMessage(LogLevel::DEBUG, format, ap);
    va_end(ap);
}

// Info level logging (compatible with old printInfo)
inline void printInfo(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    logMessage(LogLevel::INFO, format, ap);
    va_end(ap);
}

// Warning level logging
inline void printWarning(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    logMessage(LogLevel::WARNING, format, ap);
    va_end(ap);
}

// Error level logging (compatible with old printError)
inline void printError(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    logMessage(LogLevel::ERROR, format, ap);
    va_end(ap);
}

// String utilities
inline bool startsWith(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    size_t strLen = strlen(str);
    size_t prefixLen = strlen(prefix);
    if (prefixLen > strLen) return false;
    return strncmp(str, prefix, prefixLen) == 0;
}

inline bool endsWith(const char* str, const char* suffix) {
    if (!str || !suffix) return false;
    size_t strLen = strlen(str);
    size_t suffixLen = strlen(suffix);
    if (suffixLen > strLen) return false;
    return strcmp(str + strLen - suffixLen, suffix) == 0;
}

// Case-insensitive string comparison
inline int strcasecmp_safe(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
#ifdef _WIN32
    return _stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

// Safe string copy with null termination
inline void strncpy_safe(char* dest, const char* src, size_t destSize) {
    if (!dest || destSize == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

// Memory utilities
template<typename T>
inline T clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

template<typename T>
inline T min(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
inline T max(T a, T b) {
    return (a > b) ? a : b;
}

// File utilities
inline bool fileExists(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

// Get file extension (returns pointer to the dot, or empty string if none)
inline const char* getFileExtension(const char* filename) {
    if (!filename) return "";
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot;
}

// Auto cast utility (from OnscripterYuri)
// Allows implicit conversion between types with explicit cast semantics
template <typename From>
class auto_cast {
public:
    explicit constexpr auto_cast(From const& t) noexcept
        : val { t }
    {}

    template <typename To>
    constexpr operator To() const noexcept(noexcept(static_cast<To>(std::declval<From>()))) {
        return static_cast<To>(val);
    }

private:
    From const& val;
};

} // namespace utils

#endif // __UTILS_H__
