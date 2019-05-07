#include <assert.h>
#include <stdlib.h>

#include <SDL_surface.h>
#include <libavcodec/version.h>

#include "kitchensink/kiterror.h"
#include "kitchensink/internal/utils/kitlog.h"
#include "kitchensink/internal/kitlibstate.h"
#include "kitchensink/internal/subtitle/kitsubtitlepacket.h"
#include "kitchensink/internal/subtitle/kitatlas.h"
#include "kitchensink/internal/utils/kithelpers.h"
#include "kitchensink/internal/subtitle/renderers/kitsubass.h"

extern int Kit_FindFreeAtlasSlot(Kit_TextureAtlas *atlas, SDL_Surface *surface, Kit_TextureAtlasItem *item);

typedef struct Kit_ASSSubtitleRenderer {
    ASS_Renderer *renderer;
    ASS_Track *track;
} Kit_ASSSubtitleRenderer;

static void Kit_ProcessAssImage(SDL_Surface *surface, const ASS_Image *img) {
    unsigned char r = ((img->color) >> 24) & 0xFF;
    unsigned char g = ((img->color) >> 16) & 0xFF;
    unsigned char b = ((img->color) >>  8) & 0xFF;
    unsigned char a = 0xFF - ((img->color) & 0xFF);
    unsigned char *src = img->bitmap;
    unsigned char *dst = surface->pixels;
    unsigned int x;
    unsigned int y;
    unsigned int rx;

    for(y = 0; y < img->h; y++) {
        for(x = 0; x < img->w; x++) {
            rx = x * 4;
            dst[rx + 0] = r;
            dst[rx + 1] = g;
            dst[rx + 2] = b;
            dst[rx + 3] = (a * src[x]) >> 8;
        }
        src += img->stride;
        dst += surface->pitch;
    }
}

static void Kit_ProcessAssImageRaw(void *dst_data, int dst_pitch, const ASS_Image *img) {
    unsigned char r = ((img->color) >> 24) & 0xFF;
    unsigned char g = ((img->color) >> 16) & 0xFF;
    unsigned char b = ((img->color) >>  8) & 0xFF;
    unsigned char a = 0xFF - ((img->color) & 0xFF);
    unsigned char *src = img->bitmap;
    unsigned char *dst = dst_data;
    unsigned int x;
    unsigned int y;
    unsigned int rx;

    for(y = 0; y < img->h; y++) {
        for(x = 0; x < img->w; x++) {
            rx = x * 4;
            dst[rx + 0] = r;
            dst[rx + 1] = g;
            dst[rx + 2] = b;
            dst[rx + 3] = (a * src[x]) >> 8;
        }
        src += img->stride;
        dst += dst_pitch;
    }
}

static void ren_render_ass_cb(Kit_SubtitleRenderer *ren, void *src, double pts, double start, double end) {
    assert(ren != NULL);
    assert(src != NULL);

    Kit_ASSSubtitleRenderer *ass_ren = ren->userdata;
    AVSubtitle *sub = src;

    // Read incoming subtitle packets to libASS
    long long start_ms = (start + pts) * 1000;
    long long end_ms = end * 1000;
    if(Kit_LockDecoderOutput(ren->dec) == 0) {
        for(int r = 0; r < sub->num_rects; r++) {
            if(sub->rects[r]->ass == NULL)
                continue;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,25,100)
            ass_process_data(
                ass_ren->track,
                sub->rects[r]->ass,
                strlen(sub->rects[r]->ass));
#else
            // This requires the sub_text_format codec_opt set for ffmpeg
            ass_process_chunk(
                ass_ren->track,
                sub->rects[r]->ass,
                strlen(sub->rects[r]->ass),
                start_ms,
                end_ms);
#endif
        }
        Kit_UnlockDecoderOutput(ren->dec);
    }
}

static void ren_close_ass_cb(Kit_SubtitleRenderer *ren) {
    if(ren == NULL) return;

    Kit_ASSSubtitleRenderer *ass_ren = ren->userdata;
    ass_free_track(ass_ren->track);
    ass_renderer_done(ass_ren->renderer);
    free(ass_ren);
}

static int ren_get_ass_data_cb(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, SDL_Texture *texture, double current_pts) {
    Kit_ASSSubtitleRenderer *ass_ren = ren->userdata;
    SDL_Surface *dst = NULL;
    ASS_Image *src = NULL;
    int change = 0;
    long long now = current_pts * 1000;

    if(Kit_LockDecoderOutput(ren->dec) == 0) {
        // Tell ASS to render some images
        src = ass_render_frame(ass_ren->renderer, ass_ren->track, now, &change);

        // If there was no change, stop here
        if(change == 0) {
            Kit_UnlockDecoderOutput(ren->dec);
            return 0;
        }

        // There was some change, process images and add them to atlas
        Kit_ClearAtlasContent(atlas);
        Kit_CheckAtlasTextureSize(atlas, texture);
        for(; src; src = src->next) {
            if(src->w == 0 || src->h == 0)
                continue;
            dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, SDL_PIXELFORMAT_RGBA32);
            Kit_ProcessAssImage(dst, src);
            SDL_Rect target;
            target.x = src->dst_x;
            target.y = src->dst_y;
            target.w = dst->w;
            target.h = dst->h;
            Kit_AddAtlasItem(atlas, texture, dst, &target);
            SDL_FreeSurface(dst);
        }

        Kit_UnlockDecoderOutput(ren->dec);
    }

    ren->dec->clock_pos = current_pts;
    return 0;
}

static int ren_get_ass_data_raw_cb(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, void *data, double current_pts) {
    Kit_ASSSubtitleRenderer *ass_ren = ren->userdata;
    ASS_Image *src = NULL;
    int change = 0;
    long long now = current_pts * 1000;

    if(Kit_LockDecoderOutput(ren->dec) == 0) {
        // Tell ASS to render some images
        src = ass_render_frame(ass_ren->renderer, ass_ren->track, now, &change);

        // If there was no change, stop here
        if(change == 0) {
            Kit_UnlockDecoderOutput(ren->dec);
            return 0;
        }

        // There was some change, process images and add them to atlas
        Kit_ClearAtlasContent(atlas);
        atlas->w = 2048;
        atlas->h = 2048;
        for(; src; src = src->next) {
            if(src->w == 0 || src->h == 0)
                continue;

            SDL_Rect target = {src->dst_x, src->dst_y, src->w, src->h};
            SDL_Surface surface;
            surface.w = src->w;
            surface.h = src->h;

            Kit_TextureAtlasItem *item = Kit_AddAtlasItemRaw(atlas, &surface, &target);
            if(item) {
                unsigned char *dst_data = (unsigned char *) (data + item->source.y * (2048 * 4) + item->source.x * 4);
                Kit_ProcessAssImageRaw(dst_data, 2048 * 4, src);
            }
        }

        Kit_UnlockDecoderOutput(ren->dec);
    }

    ren->dec->clock_pos = current_pts;
    return 0;
}

static void ren_set_ass_size_cb(Kit_SubtitleRenderer *ren, int w, int h) {
    Kit_ASSSubtitleRenderer *ass_ren = ren->userdata;
    ass_set_frame_size(ass_ren->renderer, w, h);
}

Kit_SubtitleRenderer* Kit_CreateASSSubtitleRenderer(Kit_Decoder *dec, int video_w, int video_h, int screen_w, int screen_h) {
    assert(dec != NULL);
    assert(video_w >= 0);
    assert(video_h >= 0);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    // Make sure that libass library has been initialized + get handle
    Kit_LibraryState *state = Kit_GetLibraryState();
    if(state->libass_handle == NULL) {
        Kit_SetError("Libass library has not been initialized");
        return NULL;
    }

    // First allocate the generic decoder component
    Kit_SubtitleRenderer *ren = Kit_CreateSubtitleRenderer(dec);
    if(ren == NULL) {
        goto exit_0;
    }

    // Next, allocate ASS subtitle renderer context.
    Kit_ASSSubtitleRenderer *ass_ren = calloc(1, sizeof(Kit_ASSSubtitleRenderer));
    if(ass_ren == NULL) {
        Kit_SetError("Unable to allocate ass subtitle renderer");
        goto exit_1;
    }

    // Initialize libass renderer
    ASS_Renderer *ass_renderer = ass_renderer_init(state->libass_handle);
    if(ass_renderer == NULL) {
        Kit_SetError("Unable to initialize libass renderer");
        goto exit_2;
    }

    // Read fonts from attachment streams and give them to libass
    for(int j = 0; j < dec->format_ctx->nb_streams; j++) {
        AVStream *st = dec->format_ctx->streams[j];
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
        AVCodecContext *codec = st->codec;
#else
        AVCodecParameters *codec = st->codecpar;
#endif
        if(codec->codec_type == AVMEDIA_TYPE_ATTACHMENT && attachment_is_font(st)) {
            const AVDictionaryEntry *tag = av_dict_get(
                st->metadata,
                "filename",
                NULL,
                AV_DICT_MATCH_CASE);
            if(tag) {
                ass_add_font(
                    state->libass_handle,
                    tag->value,
                    (char*)codec->extradata,
                    codec->extradata_size);
            }
        }
    }

#ifdef __PPLAY__
    if(strlen(state->subtitle_font_path) > 0) {
        ass_set_fonts(
                ass_renderer,
                state->subtitle_font_path, "sans-serif",
                ASS_FONTPROVIDER_NONE, NULL, 0);
    } else {
        ass_set_fonts(
                ass_renderer,
                NULL, "sans-serif",
                ASS_FONTPROVIDER_AUTODETECT, NULL, 1);
    }
#else
    // Init libass fonts and window frame size
    ass_set_fonts(
        ass_renderer,
        NULL, "sans-serif",
        ASS_FONTPROVIDER_AUTODETECT,
        NULL, 1);
#endif
    ass_set_storage_size(ass_renderer, video_w, video_h);
    ass_set_frame_size(ass_renderer, screen_w, screen_h);
    ass_set_hinting(ass_renderer, state->font_hinting);

    // Initialize libass track
    ASS_Track *ass_track = ass_new_track(state->libass_handle);
    if(ass_track == NULL) {
        Kit_SetError("Unable to initialize libass track");
        goto exit_3;
    }

    // Set up libass track headers (ffmpeg provides these)
    if(dec->codec_ctx->subtitle_header) {
        ass_process_codec_private(
            ass_track,
            (char*)dec->codec_ctx->subtitle_header,
            dec->codec_ctx->subtitle_header_size);
    }

    // Set callbacks and userdata, and we're go
    ass_ren->renderer = ass_renderer;
    ass_ren->track = ass_track;
    ren->ren_render = ren_render_ass_cb;
    ren->ren_close = ren_close_ass_cb;
    ren->ren_get_data = ren_get_ass_data_cb;
    ren->ren_get_data_raw = ren_get_ass_data_raw_cb;
    ren->ren_set_size = ren_set_ass_size_cb;
    ren->userdata = ass_ren;
    return ren;

exit_3:
    ass_renderer_done(ass_renderer);
exit_2:
    free(ass_ren);
exit_1:
    Kit_CloseSubtitleRenderer(ren);
exit_0:
    return NULL;
}
