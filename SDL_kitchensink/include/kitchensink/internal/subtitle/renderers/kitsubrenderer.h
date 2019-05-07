#ifndef KITSUBRENDERER_H
#define KITSUBRENDERER_H

#include <SDL_render.h>

#include "kitchensink/kitsource.h"

typedef struct Kit_SubtitleRenderer Kit_SubtitleRenderer;
typedef struct Kit_TextureAtlas Kit_TextureAtlas;
typedef struct Kit_Decoder Kit_Decoder;

typedef void (*ren_render_cb)(Kit_SubtitleRenderer *ren, void *src, double pts, double start, double end);
typedef int (*ren_get_data_cb)(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, SDL_Texture *texture, double current_pts);
typedef int (*ren_get_data_raw_cb)(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, void *data, double current_pts);
typedef void (*ren_set_size_cb)(Kit_SubtitleRenderer *ren, int w, int h);
typedef void (*ren_close_cb)(Kit_SubtitleRenderer *ren);

struct Kit_SubtitleRenderer {
    Kit_Decoder *dec;
    void *userdata;
    ren_render_cb ren_render; ///< Subtitle rendering function callback
    ren_get_data_cb ren_get_data; ///< Subtitle data getter function callback
    ren_get_data_raw_cb ren_get_data_raw; ///< Subtitle raw data getter function callback
    ren_set_size_cb ren_set_size; ///< Screen size setter function callback
    ren_close_cb ren_close; ///< Subtitle renderer close function callback
};

KIT_LOCAL Kit_SubtitleRenderer* Kit_CreateSubtitleRenderer(Kit_Decoder *dec);
KIT_LOCAL void Kit_RunSubtitleRenderer(Kit_SubtitleRenderer *ren, void *src, double pts, double start, double end);
KIT_LOCAL int Kit_GetSubtitleRendererData(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, SDL_Texture *texture, double current_pts);
KIT_LOCAL int Kit_GetSubtitleRendererDataRaw(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, void *data, double current_pts);
KIT_LOCAL void Kit_SetSubtitleRendererSize(Kit_SubtitleRenderer *ren, int w, int h);
KIT_LOCAL void Kit_CloseSubtitleRenderer(Kit_SubtitleRenderer *ren);

#endif // KITSUBRENDERER_H
