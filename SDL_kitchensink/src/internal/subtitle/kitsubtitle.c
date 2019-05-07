#include <assert.h>

#include <SDL.h>
#include <libavformat/avformat.h>

#include "kitchensink/internal/utils/kitlog.h"

#include "kitchensink/kiterror.h"
#include "kitchensink/kitlib.h"
#include "kitchensink/internal/utils/kitlog.h"
#include "kitchensink/internal/kitlibstate.h"
#include "kitchensink/internal/subtitle/kitsubtitlepacket.h"
#include "kitchensink/internal/subtitle/kitsubtitle.h"
#include "kitchensink/internal/subtitle/kitatlas.h"
#include "kitchensink/internal/subtitle/renderers/kitsubimage.h"
#include "kitchensink/internal/subtitle/renderers/kitsubass.h"
#include "kitchensink/internal/subtitle/renderers/kitsubrenderer.h"
#include "kitchensink/internal/utils/kithelpers.h"


typedef struct Kit_SubtitleDecoder {
    Kit_SubtitleRenderer *renderer;
    AVSubtitle scratch_frame;
    Kit_TextureAtlas *atlas;
} Kit_SubtitleDecoder;


static void free_out_subtitle_packet_cb(void *packet) {
    Kit_FreeSubtitlePacket((Kit_SubtitlePacket*)packet);
}

static int dec_decode_subtitle_cb(Kit_Decoder *dec, AVPacket *in_packet) {
    assert(dec != NULL);

    if(in_packet == NULL) {
        return 0;
    }

    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    double pts;
    double start;
    double end;
    int frame_finished;
    int len;

    if(in_packet->size > 0) {
        len = avcodec_decode_subtitle2(dec->codec_ctx, &subtitle_dec->scratch_frame, &frame_finished, in_packet);
        if(len < 0) {
            return 0;
        }

        if(frame_finished) {
            // Start and end presentation timestamps for subtitle frame
            pts = 0;
            if(in_packet->pts != AV_NOPTS_VALUE) {
                pts = in_packet->pts;
                pts *= av_q2d(dec->format_ctx->streams[dec->stream_index]->time_base);
            }

            // If subtitle has no ending time, we set some safety value.
            if(subtitle_dec->scratch_frame.end_display_time == UINT_MAX) {
                subtitle_dec->scratch_frame.end_display_time = 30000;
            }

            start = subtitle_dec->scratch_frame.start_display_time / 1000.0F;
            end = subtitle_dec->scratch_frame.end_display_time / 1000.0F;

            // Create a packet. This should be filled by renderer.
            Kit_RunSubtitleRenderer(
                subtitle_dec->renderer, &subtitle_dec->scratch_frame, pts, start, end);

            // Free subtitle since it has now been handled
            avsubtitle_free(&subtitle_dec->scratch_frame);
        }
    }
    return 0;
}

static void dec_close_subtitle_cb(Kit_Decoder *dec) {
    if(dec == NULL) return;
    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    Kit_FreeAtlas(subtitle_dec->atlas);
    Kit_CloseSubtitleRenderer(subtitle_dec->renderer);
    free(subtitle_dec);
}

Kit_Decoder* Kit_CreateSubtitleDecoder(const Kit_Source *src, int stream_index, int video_w, int video_h, int screen_w, int screen_h) {
    assert(src != NULL);
    assert(video_w >= 0);
    assert(video_h >= 0);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    if(stream_index < 0) {
        return NULL;
    }

    Kit_LibraryState *state = Kit_GetLibraryState();

    // First the generic decoder component
    Kit_Decoder *dec = Kit_CreateDecoder(
        src,
        stream_index,
        state->subtitle_buf_frames,
        free_out_subtitle_packet_cb,
        state->thread_count);
    if(dec == NULL) {
        Kit_SetError("Unable to allocate subtitle decoder");
        goto exit_0;
    }

    // ... then allocate the subtitle decoder
    Kit_SubtitleDecoder *subtitle_dec = calloc(1, sizeof(Kit_SubtitleDecoder));
    if(subtitle_dec == NULL) {
        Kit_SetError("Unable to allocate subtitle decoder");
        goto exit_1;
    }

    // Set format. Note that is_enabled may be changed below ...
    Kit_OutputFormat output;
    memset(&output, 0, sizeof(Kit_OutputFormat));
    output.format = SDL_PIXELFORMAT_RGBA32; // Always this

    // For subtitles, we need a renderer for the stream. Pick one based on codec ID.
    switch(dec->codec_ctx->codec_id) {
        case AV_CODEC_ID_TEXT:
        case AV_CODEC_ID_HDMV_TEXT_SUBTITLE:
        case AV_CODEC_ID_SRT:
        case AV_CODEC_ID_SUBRIP:
        case AV_CODEC_ID_SSA:
        case AV_CODEC_ID_ASS:
            if(state->init_flags & KIT_INIT_ASS) {
                subtitle_dec->renderer = Kit_CreateASSSubtitleRenderer(dec, video_w, video_h, screen_w, screen_h);
            }
            break;
        case AV_CODEC_ID_DVD_SUBTITLE:
        case AV_CODEC_ID_DVB_SUBTITLE:
        case AV_CODEC_ID_HDMV_PGS_SUBTITLE:
        case AV_CODEC_ID_XSUB:
            subtitle_dec->renderer = Kit_CreateImageSubtitleRenderer(dec, video_w, video_h, screen_w, screen_h);
            break;
        default:
            Kit_SetError("Unrecognized subtitle format");
            break;
    }
    if(subtitle_dec->renderer == NULL) {
        goto exit_2;
    }

    // Allocate texture atlas for subtitle rectangles
    subtitle_dec->atlas = Kit_CreateAtlas();
    if(subtitle_dec->atlas == NULL) {
        Kit_SetError("Unable to allocate subtitle texture atlas");
        goto exit_3;
    }

    // Set callbacks and userdata, and we're go
    dec->dec_decode = dec_decode_subtitle_cb;
    dec->dec_close = dec_close_subtitle_cb;
    dec->userdata = subtitle_dec;
    dec->output = output;
    return dec;

exit_3:
    Kit_CloseSubtitleRenderer(subtitle_dec->renderer);
exit_2:
    free(subtitle_dec);
exit_1:
    Kit_CloseDecoder(dec);
exit_0:
    return NULL;
}

void Kit_SetSubtitleDecoderSize(Kit_Decoder *dec, int screen_w, int screen_h) {
    assert(dec != NULL);
    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    Kit_SetSubtitleRendererSize(subtitle_dec->renderer, screen_w, screen_h);
}

void Kit_GetSubtitleDecoderTexture(Kit_Decoder *dec, SDL_Texture *texture, double sync_ts) {
    assert(dec != NULL);
    assert(texture != NULL);

    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    Kit_GetSubtitleRendererData(subtitle_dec->renderer, subtitle_dec->atlas, texture, sync_ts);
}

void Kit_GetSubtitleDecoderTextureRaw(Kit_Decoder *dec, void *data, double sync_ts) {
    assert(dec != NULL);
    assert(data != NULL);

    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    Kit_GetSubtitleRendererDataRaw(subtitle_dec->renderer, subtitle_dec->atlas, data, sync_ts);
}

int Kit_GetSubtitleDecoderInfo(Kit_Decoder *dec, SDL_Rect *sources, SDL_Rect *targets, int limit) {
    Kit_SubtitleDecoder *subtitle_dec = dec->userdata;
    return Kit_GetAtlasItems(subtitle_dec->atlas, sources, targets, limit);
}
