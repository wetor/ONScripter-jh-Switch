#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>

#include "kitchensink/kitsource.h"
#include "kitchensink/kiterror.h"
#include "kitchensink/internal/utils/kitlog.h"

#define AVIO_BUF_SIZE 32768

static int _ScanSource(AVFormatContext *format_ctx) {
    av_opt_set_int(format_ctx, "probesize", INT_MAX, 0);
    av_opt_set_int(format_ctx, "analyzeduration", INT_MAX, 0);
    if(avformat_find_stream_info(format_ctx, NULL) < 0) {
        Kit_SetError("Unable to fetch source information");
        return 1;
    }
    return 0;
}

Kit_Source* Kit_CreateSourceFromUrl(const char *url) {
    assert(url != NULL);

    Kit_Source *src = calloc(1, sizeof(Kit_Source));
    if(src == NULL) {
        Kit_SetError("Unable to allocate source");
        return NULL;
    }

    // Attempt to open source
    int res = avformat_open_input((AVFormatContext **)&src->format_ctx, url, NULL, NULL);
    if(res < 0) {
        Kit_SetError(av_err2str(res));
        goto exit_0;
    }

    // Scan source information (may seek forwards)
    if(_ScanSource(src->format_ctx)) {
        goto exit_1;
    }

    return src;

exit_1:
    avformat_close_input((AVFormatContext **)&src->format_ctx);
exit_0:
    free(src);
    return NULL;
}

Kit_Source* Kit_CreateSourceFromCustom(Kit_ReadCallback read_cb, Kit_SeekCallback seek_cb, void *userdata) {
    assert(read_cb != NULL);

    Kit_Source *src = calloc(1, sizeof(Kit_Source));
    if(src == NULL) {
        Kit_SetError("Unable to allocate source");
        return NULL;
    }

    uint8_t *avio_buf = av_malloc(AVIO_BUF_SIZE);
    if(avio_buf == NULL) {
        Kit_SetError("Unable to allocate avio buffer");
        goto exit_0;
    }

    AVFormatContext *format_ctx = avformat_alloc_context();
    if(format_ctx == NULL) {
        Kit_SetError("Unable to allocate format context");
        goto exit_1;
    }

    AVIOContext *avio_ctx = avio_alloc_context(
        avio_buf, AVIO_BUF_SIZE, 0, userdata, read_cb, 0, seek_cb);
    if(avio_ctx == NULL) {
        Kit_SetError("Unable to allocate avio context");
        goto exit_2;
    }

    // Set the format as AVIO format
    format_ctx->pb = avio_ctx;

    // Attempt to open source
    if(avformat_open_input(&format_ctx, "", NULL, NULL) < 0) {
        Kit_SetError("Unable to open custom source");
        goto exit_3;
    }

    // Scan source information (may seek forwards)
    if(_ScanSource(format_ctx)) {
        goto exit_4;
    }

    // Set internals
    src->format_ctx = format_ctx;
    src->avio_ctx = avio_ctx;
    return src;

exit_4:
    avformat_close_input(&format_ctx);
exit_3:
    av_freep(&avio_ctx);
exit_2:
    avformat_free_context(format_ctx);
exit_1:
    av_freep(&avio_buf);
exit_0:
    free(src);
    return NULL;
}

static int _RWReadCallback(void *userdata, uint8_t *buf, int size) {
    return SDL_RWread((SDL_RWops*)userdata, buf, 1, size);
}

static int64_t _RWGetSize(SDL_RWops *rw_ops) {
    int64_t current_pos;
    int64_t max_pos;
    
    // First, see if tell works at all, and fail with -1 if it doesn't.
    current_pos = SDL_RWtell(rw_ops);
    if(current_pos < 0) {
        return -1;
    }

    // Seek to end, get pos (this is the size), then return.
    if(SDL_RWseek(rw_ops, 0, RW_SEEK_END) < 0) {
        return -1; // Seek failed, never mind then
    }
    max_pos = SDL_RWtell(rw_ops);
    SDL_RWseek(rw_ops, current_pos, RW_SEEK_SET);
    return max_pos;
}

static int64_t _RWSeekCallback(void *userdata, int64_t offset, int whence) {
    int rw_whence = 0;
    if(whence & AVSEEK_SIZE)
        return _RWGetSize(userdata);

    if((whence & ~AVSEEK_FORCE) == SEEK_CUR)
        rw_whence = RW_SEEK_CUR;
    else if((whence & ~AVSEEK_FORCE) == SEEK_SET)
        rw_whence = RW_SEEK_SET;
    else if((whence & ~AVSEEK_FORCE) == SEEK_END)
        rw_whence = RW_SEEK_END;

    return SDL_RWseek((SDL_RWops*)userdata, offset, rw_whence);
}


Kit_Source* Kit_CreateSourceFromRW(SDL_RWops *rw_ops) {
    return Kit_CreateSourceFromCustom(_RWReadCallback, _RWSeekCallback, rw_ops);
}

void Kit_CloseSource(Kit_Source *src) {
    assert(src != NULL);
    AVFormatContext *format_ctx = src->format_ctx;
    AVIOContext *avio_ctx = src->avio_ctx;
    avformat_close_input(&format_ctx);
    if(avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    free(src);
}

static Kit_StreamType _GetKitStreamType(const enum AVMediaType type) {
    switch(type) {
        case AVMEDIA_TYPE_DATA: return KIT_STREAMTYPE_DATA;
        case AVMEDIA_TYPE_VIDEO: return KIT_STREAMTYPE_VIDEO;
        case AVMEDIA_TYPE_AUDIO: return KIT_STREAMTYPE_AUDIO;
        case AVMEDIA_TYPE_SUBTITLE: return KIT_STREAMTYPE_SUBTITLE;
        case AVMEDIA_TYPE_ATTACHMENT: return KIT_STREAMTYPE_ATTACHMENT;
        default:
            return KIT_STREAMTYPE_UNKNOWN;
    }
}

int Kit_GetSourceStreamInfo(const Kit_Source *src, Kit_SourceStreamInfo *info, int index) {
    assert(src != NULL);
    assert(info != NULL);

    AVFormatContext *format_ctx = (AVFormatContext *)src->format_ctx;
    if(index < 0 || index >= format_ctx->nb_streams) {
        Kit_SetError("Invalid stream index");
        return 1;
    }

    AVStream *stream = format_ctx->streams[index];
    enum AVMediaType codec_type;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    codec_type = stream->codec->codec_type;
#else
    codec_type = stream->codecpar->codec_type;
#endif
    info->type = _GetKitStreamType(codec_type);
    info->index = index;
    return 0;
}

int Kit_GetSourceStreamList(const Kit_Source *src, const Kit_StreamType type, int *list, int size) {
    int p = 0;
    AVFormatContext *format_ctx = (AVFormatContext *)src->format_ctx;
    for(int n = 0; n < Kit_GetSourceStreamCount(src); n++) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
        if(_GetKitStreamType(format_ctx->streams[n]->codec->codec_type) == type) {
#else
        if(_GetKitStreamType(format_ctx->streams[n]->codecpar->codec_type) == type) {
#endif
            if(p >= size) {
                return p;
            }
            list[p++] = n;
        }
    }
    return p;
}

int Kit_GetBestSourceStream(const Kit_Source *src, const Kit_StreamType type) {
    assert(src != NULL);
    int avmedia_type = 0;
    switch(type) {
        case KIT_STREAMTYPE_VIDEO: avmedia_type = AVMEDIA_TYPE_VIDEO; break;
        case KIT_STREAMTYPE_AUDIO: avmedia_type = AVMEDIA_TYPE_AUDIO; break;
        case KIT_STREAMTYPE_SUBTITLE: avmedia_type = AVMEDIA_TYPE_SUBTITLE; break;
        default: return -1;
    }
    int ret = av_find_best_stream((AVFormatContext *)src->format_ctx, avmedia_type, -1, -1, NULL, 0);
    if(ret == AVERROR_STREAM_NOT_FOUND) {
        return -1;
    }
    if(ret == AVERROR_DECODER_NOT_FOUND) {
        Kit_SetError("Unable to find a decoder for the stream");
        return 1;
    }
    return ret;
}

int Kit_GetSourceStreamCount(const Kit_Source *src) {
    assert(src != NULL);
    return ((AVFormatContext *)src->format_ctx)->nb_streams;
}
