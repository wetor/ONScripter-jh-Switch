#include <assert.h>
#include <stdlib.h>

#include <SDL_surface.h>

#include "kitchensink/kiterror.h"
#include "kitchensink/internal/utils/kitlog.h"
#include "kitchensink/internal/subtitle/kitatlas.h"
#include "kitchensink/internal/subtitle/kitsubtitlepacket.h"
#include "kitchensink/internal/subtitle/renderers/kitsubimage.h"


typedef struct Kit_ImageSubtitleRenderer {
    int video_w;
    int video_h;
    float scale_x;
    float scale_y;
} Kit_ImageSubtitleRenderer;

static void ren_render_image_cb(Kit_SubtitleRenderer *ren, void *sub_src, double pts, double start, double end) {
    assert(ren != NULL);
    assert(sub_src != NULL);

    AVSubtitle *sub = sub_src;
    SDL_Surface *dst = NULL;
    SDL_Surface *src = NULL;
    double start_pts = pts + start;
    double end_pts = pts + end;

    // If this subtitle has no rects, we still need to clear screen from old subs
    if(sub->num_rects == 0) {
        Kit_WriteDecoderOutput(
            ren->dec, Kit_CreateSubtitlePacket(true, start_pts, end_pts, 0, 0, NULL));
        return;
    }

    // Convert subtitle images from paletted to RGBA8888
    for(int n = 0; n < sub->num_rects; n++) {
        AVSubtitleRect *r = sub->rects[n];
        if(r->type != SUBTITLE_BITMAP)
            continue;

        src = SDL_CreateRGBSurfaceWithFormatFrom(
            r->data[0], r->w, r->h, 8, r->linesize[0], SDL_PIXELFORMAT_INDEX8);
        SDL_SetPaletteColors(src->format->palette, (SDL_Color*)r->data[1], 0, 256);
        dst = SDL_CreateRGBSurfaceWithFormat(
            0, r->w, r->h, 32, SDL_PIXELFORMAT_RGBA32);
        
        // Blit source to target and free source surface.
        SDL_BlitSurface(src, NULL, dst, NULL);
        
        // Create a new packet and write it to output buffer
        Kit_WriteDecoderOutput(
            ren->dec, Kit_CreateSubtitlePacket(false, start_pts, end_pts, r->x, r->y, dst));

        // Free surfaces
        SDL_FreeSurface(src);
        SDL_FreeSurface(dst);
    }
}

static int ren_get_img_data_cb(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, SDL_Texture *texture, double current_pts) {
    Kit_ImageSubtitleRenderer *img_ren = ren->userdata;
    Kit_SubtitlePacket *packet = NULL;

    Kit_CheckAtlasTextureSize(atlas, texture);
    while((packet = Kit_PeekDecoderOutput(ren->dec)) != NULL) {
        // Clear dead packets
        if(packet->pts_end < current_pts) {
            Kit_AdvanceDecoderOutput(ren->dec);
            Kit_FreeSubtitlePacket(packet);
            continue;
        }

        // Show visible ones
        if(packet->pts_start < current_pts) {
            if(packet->clear) {
                Kit_ClearAtlasContent(atlas);
            }
            if(packet->surface != NULL) {
                SDL_Rect target;
                target.x = packet->x * img_ren->scale_x;
                target.y = packet->y * img_ren->scale_y;
                target.w = packet->surface->w * img_ren->scale_x;
                target.h = packet->surface->h * img_ren->scale_y;
                Kit_AddAtlasItem(atlas, texture, packet->surface, &target);
            }
            Kit_AdvanceDecoderOutput(ren->dec);
            Kit_FreeSubtitlePacket(packet);
            ren->dec->clock_pos = current_pts;
            continue;
        }
        break;
    }

    return 0;
}

static void ren_set_img_size_cb(Kit_SubtitleRenderer *ren, int w, int h) {
    Kit_ImageSubtitleRenderer *img_ren = ren->userdata;
    img_ren->scale_x = (float)w / (float)img_ren->video_w;
    img_ren->scale_y = (float)h / (float)img_ren->video_h;
}

static void ren_close_ass_cb(Kit_SubtitleRenderer *ren) {
    if(ren == NULL) return;
    free(ren->userdata);
}

Kit_SubtitleRenderer* Kit_CreateImageSubtitleRenderer(Kit_Decoder *dec, int video_w, int video_h, int screen_w, int screen_h) {
    assert(dec != NULL);
    assert(video_w >= 0);
    assert(video_h >= 0);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    // Allocate a new renderer
    Kit_SubtitleRenderer *ren = Kit_CreateSubtitleRenderer(dec);
    if(ren == NULL) {
        goto exit_0;
    }

    // Allocate image renderer internal context
    Kit_ImageSubtitleRenderer *img_ren = calloc(1, sizeof(Kit_ImageSubtitleRenderer));
    if(img_ren == NULL) {
        Kit_SetError("Unable to allocate image subtitle renderer");
        goto exit_1;
    }

    // Only renderer required, no other data.
    img_ren->video_w = video_w;
    img_ren->video_h = video_h;
    img_ren->scale_x = (float)screen_w / (float)video_w;
    img_ren->scale_y = (float)screen_h / (float)video_h;
    ren->ren_render = ren_render_image_cb;
    ren->ren_get_data = ren_get_img_data_cb;
    ren->ren_set_size = ren_set_img_size_cb;
    ren->ren_close = ren_close_ass_cb;
    ren->userdata = img_ren;
    return ren;

exit_1:
    Kit_CloseSubtitleRenderer(ren);
exit_0:
    return NULL;
}
