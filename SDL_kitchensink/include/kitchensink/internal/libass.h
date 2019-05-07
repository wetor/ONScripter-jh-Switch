#ifndef KITLIBASS_H
#define KITLIBASS_H

#ifndef USE_DYNAMIC_LIBASS

#include <ass/ass.h>

#else // USE_DYNAMIC_LIBASS

#include <stdint.h>
#include <stdarg.h>

#include "kitchensink/kitconfig.h"

typedef struct ass_library ASS_Library;
typedef struct ass_renderer ASS_Renderer;
typedef struct ass_track ASS_Track;

typedef struct ass_image {
    int w, h;
    int stride;
    unsigned char *bitmap;
    uint32_t color;
    int dst_x, dst_y;
    struct ass_image *next;
    enum {
        IMAGE_TYPE_CHARACTER,
        IMAGE_TYPE_OUTLINE,
        IMAGE_TYPE_SHADOW
    } type;
} ASS_Image;

typedef enum {
    ASS_HINTING_NONE = 0,
    ASS_HINTING_LIGHT,
    ASS_HINTING_NORMAL,
    ASS_HINTING_NATIVE
} ASS_Hinting;

KIT_LOCAL ASS_Library* (*ass_library_init)(void);
KIT_LOCAL void (*ass_library_done)(ASS_Library *priv);
KIT_LOCAL void (*ass_process_codec_private)(ASS_Track *track, char *data, int size);
KIT_LOCAL void (*ass_set_message_cb)(ASS_Library *priv, void (*msg_cb)(int level, const char *fmt, va_list args, void *data), void *data);
KIT_LOCAL ASS_Renderer* (*ass_renderer_init)(ASS_Library *);
KIT_LOCAL void (*ass_renderer_done)(ASS_Renderer *priv);
KIT_LOCAL void (*ass_set_frame_size)(ASS_Renderer *priv, int w, int h);
KIT_LOCAL void (*ass_set_hinting)(ASS_Renderer *priv, ASS_Hinting ht);
KIT_LOCAL void (*ass_set_fonts)(ASS_Renderer *priv, const char *default_font, const char *default_family, int dfp, const char *config, int update);
KIT_LOCAL ASS_Image* (*ass_render_frame)(ASS_Renderer *priv, ASS_Track *track, long long now, int *detect_change);
KIT_LOCAL ASS_Track* (*ass_new_track)(ASS_Library *);
KIT_LOCAL void (*ass_free_track)(ASS_Track *track);
KIT_LOCAL void (*ass_process_data)(ASS_Track *track, char *data, int size);
KIT_LOCAL void (*ass_process_chunk)(ASS_Track *track, char *data, int size, long long timecode, long long duration);
KIT_LOCAL void (*ass_add_font)(ASS_Library *library, char *name, char *data, int data_size);
KIT_LOCAL void (*ass_set_storage_size)(ASS_Renderer *priv, int w, int h);

KIT_LOCAL int load_libass(void *handle);

#endif // USE_DYNAMIC_LIBASS

// For compatibility
#ifndef ASS_FONTPROVIDER_AUTODETECT
#define ASS_FONTPROVIDER_AUTODETECT 1
#endif

#endif // KITLIBASS_H
