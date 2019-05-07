#include <assert.h>
#include <SDL_loadso.h>

#include <libavformat/avformat.h>

#include "kitchensink/internal/utils/kitlog.h"
#include "kitchensink/kitchensink.h"
#include "kitchensink/internal/kitlibstate.h"

static void _libass_msg_callback(int level, const char *fmt, va_list va, void *data) {}

static int max(int a, int b) { return a > b ? a : b; }
static int min(int a, int b) { return a < b ? a : b; }

int Kit_InitASS(Kit_LibraryState *state) {
#ifdef USE_DYNAMIC_LIBASS
    state->ass_so_handle = SDL_LoadObject(DYNAMIC_LIBASS_NAME);
    if(state->ass_so_handle == NULL) {
        Kit_SetError("Unable to load ASS library");
        return 1;
    }
    load_libass(state->ass_so_handle);
#endif
    state->libass_handle = ass_library_init();
    state->thread_count = 1;
    state->font_hinting = KIT_FONT_HINTING_NONE;
    state->video_buf_frames = 3;
    state->audio_buf_frames = 64;
    state->subtitle_buf_frames = 64;
    ass_set_message_cb(state->libass_handle, _libass_msg_callback, NULL);
    return 0;
}

void Kit_CloseASS(Kit_LibraryState *state) {
    ass_library_done(state->libass_handle);
    state->libass_handle = NULL;
#ifdef USE_DYNAMIC_LIBASS
    SDL_UnloadObject(state->ass_so_handle);
    state->ass_so_handle = NULL;
#endif
}

int Kit_Init(unsigned int flags) {
    Kit_LibraryState *state = Kit_GetLibraryState();

    if(state->init_flags != 0) {
        Kit_SetError("SDL_kitchensink is already initialized");
        goto exit_0;
    }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif

    if(flags & KIT_INIT_NETWORK) {
        avformat_network_init();
    }
    if(flags & KIT_INIT_ASS) {
        if(Kit_InitASS(state) != 0) {
            Kit_SetError("Failed to initialize libass");
            goto exit_1;
        }
    }

    state->init_flags = flags;
    return 0;

exit_1:
    avformat_network_deinit();

exit_0:
    return 1;
}

void Kit_Quit() {
    Kit_LibraryState *state = Kit_GetLibraryState();

    if(state->init_flags & KIT_INIT_NETWORK) {
        avformat_network_deinit();
    }
    if(state->init_flags & KIT_INIT_ASS) {
        Kit_CloseASS(state);
    }
    state->init_flags = 0;
}

void Kit_SetHint(Kit_HintType type, int value) {
    Kit_LibraryState *state = Kit_GetLibraryState();
    switch(type) {
        case KIT_HINT_THREAD_COUNT:
            state->thread_count =  max(value, 1);
            break;
        case KIT_HINT_FONT_HINTING:
            state->font_hinting = max(min(value, KIT_FONT_HINTING_COUNT), 0);
            break;
        case KIT_HINT_VIDEO_BUFFER_FRAMES:
            state->video_buf_frames = max(value, 1);
            break;
        case KIT_HINT_AUDIO_BUFFER_FRAMES:
            state->audio_buf_frames = max(value, 1);
            break;
        case KIT_HINT_SUBTITLE_BUFFER_FRAMES:
            state->subtitle_buf_frames = max(value, 1);
            break;
    }
}

int Kit_GetHint(Kit_HintType type) {
    Kit_LibraryState *state = Kit_GetLibraryState();
    switch(type) {
        case KIT_HINT_THREAD_COUNT:
            return state->thread_count;
        case KIT_HINT_FONT_HINTING:
            return state->font_hinting;
        case KIT_HINT_VIDEO_BUFFER_FRAMES:
            return state->video_buf_frames;
        case KIT_HINT_AUDIO_BUFFER_FRAMES:
            return state->audio_buf_frames;
        case KIT_HINT_SUBTITLE_BUFFER_FRAMES:
            return state->subtitle_buf_frames;
        default:
            return 0;
    }
}

void Kit_GetVersion(Kit_Version *version) {
    assert(version != NULL);
    version->major = KIT_VERSION_MAJOR;
    version->minor = KIT_VERSION_MINOR;
    version->patch = KIT_VERSION_PATCH;
}
