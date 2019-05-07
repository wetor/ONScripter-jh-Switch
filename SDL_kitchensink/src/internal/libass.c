#ifdef USE_DYNAMIC_LIBASS

#include <SDL_loadso.h>
#include "kitchensink/internal/libass.h"

int load_libass(void *handle) {
    ass_library_init = SDL_LoadFunction(handle, "ass_library_init");
    ass_library_done = SDL_LoadFunction(handle, "ass_library_done");
    ass_set_message_cb = SDL_LoadFunction(handle, "ass_set_message_cb");
    ass_renderer_init = SDL_LoadFunction(handle, "ass_renderer_init");
    ass_renderer_done = SDL_LoadFunction(handle, "ass_renderer_done");
    ass_set_frame_size = SDL_LoadFunction(handle, "ass_set_frame_size");
    ass_set_hinting = SDL_LoadFunction(handle, "ass_set_hinting");
    ass_set_fonts = SDL_LoadFunction(handle, "ass_set_fonts");
    ass_render_frame = SDL_LoadFunction(handle, "ass_render_frame");
    ass_new_track = SDL_LoadFunction(handle, "ass_new_track");
    ass_free_track = SDL_LoadFunction(handle, "ass_free_track");
    ass_process_data = SDL_LoadFunction(handle, "ass_process_data");
    ass_add_font = SDL_LoadFunction(handle, "ass_add_font");
    ass_process_codec_private = SDL_LoadFunction(handle, "ass_process_codec_private");
    ass_process_chunk = SDL_LoadFunction(handle, "ass_process_chunk");
    ass_set_storage_size = SDL_LoadFunction(handle, "ass_set_storage_size");
    return 0;
}

#endif // USE_DYNAMIC_LIBASS
