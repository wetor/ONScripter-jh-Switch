/* Windows TermIO
 *
 * copyright (C) 2003 Sascha Sommer
 *
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <io.h>
#include <pthread.h>
#include <assert.h>
#include "common/common.h"
#include "input/keycodes.h"
#include "input/input.h"
#include "terminal.h"
#include "osdep/io.h"
#include "osdep/threads.h"
#include "osdep/w32_keyboard.h"

#define hSTDOUT GetStdHandle(STD_OUTPUT_HANDLE)
#define hSTDERR GetStdHandle(STD_ERROR_HANDLE)

#define FOREGROUND_ALL (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

static short stdoutAttrs = 0;
static const unsigned char ansi2win32[8] = {
    0,
    FOREGROUND_RED,
    FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_BLUE,
    FOREGROUND_BLUE  | FOREGROUND_RED,
    FOREGROUND_BLUE  | FOREGROUND_GREEN,
    FOREGROUND_BLUE  | FOREGROUND_GREEN | FOREGROUND_RED,
};

static bool running;
static HANDLE death;
static pthread_t input_thread;
static struct input_ctx *input_ctx;

void terminal_get_size(int *w, int *h)
{
    CONSOLE_SCREEN_BUFFER_INFO cinfo;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cinfo)) {
        *w = cinfo.dwMaximumWindowSize.X - 1;
        *h = cinfo.dwMaximumWindowSize.Y;
    }
}

static bool has_input_events(HANDLE h)
{
    DWORD num_events;
    if (!GetNumberOfConsoleInputEvents(h, &num_events))
        return false;
    return !!num_events;
}

static void read_input(HANDLE in)
{
    // Process any input events in the buffer
    while (has_input_events(in)) {
        INPUT_RECORD event;
        if (!ReadConsoleInputW(in, &event, 1, &(DWORD){0}))
            break;

        // Only key-down events are interesting to us
        if (event.EventType != KEY_EVENT)
            continue;
        KEY_EVENT_RECORD *record = &event.Event.KeyEvent;
        if (!record->bKeyDown)
            continue;

        UINT vkey = record->wVirtualKeyCode;
        bool ext = record->dwControlKeyState & ENHANCED_KEY;

        int mods = 0;
        if (record->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
            mods |= MP_KEY_MODIFIER_ALT;
        if (record->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
            mods |= MP_KEY_MODIFIER_CTRL;
        if (record->dwControlKeyState & SHIFT_PRESSED)
            mods |= MP_KEY_MODIFIER_SHIFT;

        int mpkey = mp_w32_vkey_to_mpkey(vkey, ext);
        if (mpkey) {
            mp_input_put_key(input_ctx, mpkey | mods);
        } else {
            // Only characters should be remaining
            int c = record->uChar.UnicodeChar;
            // The ctrl key always produces control characters in the console.
            // Shift them back up to regular characters.
            if (c > 0 && c < 0x20 && (mods & MP_KEY_MODIFIER_CTRL))
                c += (mods & MP_KEY_MODIFIER_SHIFT) ? 0x40 : 0x60;
            if (c >= 0x20)
                mp_input_put_key(input_ctx, c | mods);
        }
    }
}

static void *input_thread_fn(void *ptr)
{
    mpthread_set_name("terminal");
    HANDLE in = ptr;
    HANDLE stuff[2] = {in, death};
    while (1) {
        DWORD r = WaitForMultipleObjects(2, stuff, FALSE, INFINITE);
        if (r != WAIT_OBJECT_0)
            break;
        read_input(in);
    }
    return NULL;
}

void terminal_setup_getch(struct input_ctx *ictx)
{
    if (running)
        return;

    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (GetNumberOfConsoleInputEvents(in, &(DWORD){0})) {
        input_ctx = ictx;
        death = CreateEventW(NULL, TRUE, FALSE, NULL);
        if (!death)
            return;
        if (pthread_create(&input_thread, NULL, input_thread_fn, in)) {
            CloseHandle(death);
            return;
        }
        running = true;
    }
}

void terminal_uninit(void)
{
    if (running) {
        SetEvent(death);
        pthread_join(input_thread, NULL);
        input_ctx = NULL;
        running = false;
    }
}

bool terminal_in_background(void)
{
    return false;
}

static void write_console_text(HANDLE wstream, char *buf)
{
    wchar_t *out = mp_from_utf8(NULL, buf);
    size_t out_len = wcslen(out);
    WriteConsoleW(wstream, out, out_len, NULL, NULL);
    talloc_free(out);
}

// Mutates the input argument (buf), because we're evil.
void mp_write_console_ansi(HANDLE wstream, char *buf)
{
    while (*buf) {
        char *next = strchr(buf, '\033');
        if (!next) {
            write_console_text(wstream, buf);
            break;
        }
        next[0] = '\0'; // mutate input for fun and profit
        write_console_text(wstream, buf);
        if (next[1] != '[') {
            write_console_text(wstream, "\033");
            buf = next;
            continue;
        }
        next += 2;
        // ANSI codes generally follow this syntax:
        //    "\033[" [ <i> (';' <i> )* ] <c>
        // where <i> are integers, and <c> a single char command code.
        // Also see: http://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes
        int params[2] = {-1, -1}; // 'm' might be unlimited; ignore that
        int num_params = 0;
        while (num_params < 2) {
            char *end = next;
            long p = strtol(next, &end, 10);
            if (end == next)
                break;
            next = end;
            params[num_params++] = p;
            if (next[0] != ';' || !next[0])
                break;
            next += 1;
        }
        char code = next[0];
        if (code)
            next += 1;
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(wstream, &info);
        switch (code) {
        case 'K': {     // erase to end of line
            COORD at = info.dwCursorPosition;
            int len = info.dwSize.X - at.X;
            FillConsoleOutputCharacterW(wstream, ' ', len, at, &(DWORD){0});
            SetConsoleCursorPosition(wstream, at);
            break;
        }
        case 'A': {     // cursor up
            info.dwCursorPosition.Y -= 1;
            SetConsoleCursorPosition(wstream, info.dwCursorPosition);
            break;
        }
        case 'm': {     // "SGR"
            for (int n = 0; n < num_params; n++) {
                int p = params[n];
                if (p == 0) {
                    info.wAttributes = stdoutAttrs;
                    SetConsoleTextAttribute(wstream, info.wAttributes);
                } else if (p == 1) {
                    info.wAttributes |= FOREGROUND_INTENSITY;
                    SetConsoleTextAttribute(wstream, info.wAttributes);
                } else if (p >= 30 && p < 38) {
                    info.wAttributes &= ~FOREGROUND_ALL;
                    info.wAttributes |= ansi2win32[p - 30];
                    SetConsoleTextAttribute(wstream, info.wAttributes);
                }
            }
            break;
        }
        }
        buf = next;
    }
}

static bool is_a_console(HANDLE h)
{
    return GetConsoleMode(h, &(DWORD){0});
}

static void reopen_console_handle(DWORD std, int fd, FILE *stream)
{
    if (is_a_console(GetStdHandle(std))) {
        freopen("CONOUT$", "wt", stream);
        dup2(fileno(stream), fd);
        setvbuf(stream, NULL, _IONBF, 0);
    }
}

bool terminal_try_attach(void)
{
    // mpv.exe is a flagged as a GUI application, but it acts as a console
    // application when started from the console wrapper (see
    // osdep/win32-console-wrapper.c). The console wrapper sets
    // _started_from_console=yes, so check that variable before trying to
    // attach to the console.
    wchar_t console_env[4] = { 0 };
    if (!GetEnvironmentVariableW(L"_started_from_console", console_env, 4))
        return false;
    if (wcsncmp(console_env, L"yes", 4))
        return false;
    SetEnvironmentVariableW(L"_started_from_console", NULL);

    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        return false;

    // We have a console window. Redirect output streams to that console's
    // low-level handles, so things that use printf directly work later on.
    reopen_console_handle(STD_OUTPUT_HANDLE, STDOUT_FILENO, stdout);
    reopen_console_handle(STD_ERROR_HANDLE, STDERR_FILENO, stderr);

    return true;
}

void terminal_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO cinfo;
    DWORD cmode = 0;
    GetConsoleMode(hSTDOUT, &cmode);
    cmode |= (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    SetConsoleMode(hSTDOUT, cmode);
    SetConsoleMode(hSTDERR, cmode);
    GetConsoleScreenBufferInfo(hSTDOUT, &cinfo);
    stdoutAttrs = cinfo.wAttributes;
}
