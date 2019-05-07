#ifndef KITLIBSTATE_H
#define KITLIBSTATE_H

#include "kitchensink/internal/libass.h"
#include "kitchensink/kitconfig.h"

typedef struct Kit_LibraryState {
    unsigned int init_flags;
    unsigned int thread_count;
    unsigned int font_hinting;
    unsigned int video_buf_frames;
    unsigned int audio_buf_frames;
    unsigned int subtitle_buf_frames;
    ASS_Library *libass_handle;
    void *ass_so_handle;
#ifdef __PPLAY__
    char subtitle_font_path[512];
#endif
} Kit_LibraryState;

KIT_LOCAL Kit_LibraryState* Kit_GetLibraryState();

#endif // KITLIBSTATE_H
