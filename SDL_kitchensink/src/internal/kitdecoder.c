#include <stdlib.h>
#include <assert.h>

#include <libavformat/avformat.h>

#include "kitchensink/internal/kitdecoder.h"
#include "kitchensink/kiterror.h"

#define BUFFER_IN_SIZE 256

static void free_in_video_packet_cb(void *packet) {
    av_packet_free((AVPacket**)&packet);
}

Kit_Decoder* Kit_CreateDecoder(const Kit_Source *src, int stream_index, 
                               int out_b_size, dec_free_packet_cb free_out_cb,
                               int thread_count) {
    assert(src != NULL);
    assert(out_b_size > 0);
    assert(thread_count > 0);

    AVCodecContext *codec_ctx = NULL;
    AVDictionary *codec_opts = NULL;
    AVCodec *codec = NULL;
    AVFormatContext *format_ctx = src->format_ctx;
    int bsizes[2] = {BUFFER_IN_SIZE, out_b_size};
    dec_free_packet_cb free_hooks[2] = {free_in_video_packet_cb, free_out_cb};

    // Make sure index seems correct
    if(stream_index >= (int)format_ctx->nb_streams || stream_index < 0) {
        Kit_SetError("Stream id out of bounds for %d", stream_index);
        goto exit_0;
    }
    
    // Allocate decoder and make sure allocation was a success
    Kit_Decoder *dec = calloc(1, sizeof(Kit_Decoder));
    if(dec == NULL) {
        Kit_SetError("Unable to allocate kit decoder for stream %d", stream_index);
        goto exit_0;
    }

    // Find audio decoder
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    codec = avcodec_find_decoder(format_ctx->streams[stream_index]->codec->codec_id);
#else
    codec = avcodec_find_decoder(format_ctx->streams[stream_index]->codecpar->codec_id);
#endif
    if(codec == NULL) {
        Kit_SetError("No suitable decoder found for stream %d", stream_index);
        goto exit_1;
    }

    // Allocate a context for the codec
    codec_ctx = avcodec_alloc_context3(codec);
    if(codec_ctx == NULL) {
        Kit_SetError("Unable to allocate codec context for stream %d", stream_index);
        goto exit_1;
    }

    // Copy params
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    if(avcodec_copy_context(codec_ctx, format_ctx->streams[stream_index]->codec) != 0)
#else
    if(avcodec_parameters_to_context(codec_ctx, format_ctx->streams[stream_index]->codecpar) < 0)
#endif
    {
        Kit_SetError("Unable to copy codec context for stream %d", stream_index);
        goto exit_2;
    }

    // Required by ffmpeg for now when using the new API.
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
    codec_ctx->pkt_timebase = format_ctx->streams[stream_index]->time_base;
#endif

    // Set thread count
    codec_ctx->thread_count = thread_count;
    codec_ctx->thread_type = FF_THREAD_SLICE|FF_THREAD_FRAME;

    // This is required for ass_process_chunk() support
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 25, 100)
    av_dict_set(&codec_opts, "sub_text_format", "ass", 0);
#endif

    // Open the stream
    if(avcodec_open2(codec_ctx, codec, &codec_opts) < 0) {
        Kit_SetError("Unable to open codec for stream %d", stream_index);
        goto exit_2;
    }

    // Set index and codec
    dec->stream_index = stream_index;
    dec->codec_ctx = codec_ctx;
    dec->format_ctx = format_ctx;

    // Allocate input/output ringbuffers
    for(int i = 0; i < 2; i++) {
        dec->buffer[i] = Kit_CreateBuffer(bsizes[i], free_hooks[i]);
        if(dec->buffer[i] == NULL) {
            Kit_SetError("Unable to allocate buffer for stream %d: %s", stream_index, SDL_GetError());
            goto exit_3;
        }
    }

    // Create a lock for output buffer synchronization
    dec->output_lock = SDL_CreateMutex();
    if(dec->output_lock == NULL) {
        Kit_SetError("Unable to allocate mutex for stream %d: %s", stream_index, SDL_GetError());
        goto exit_3;
    }

    // That's that
    return dec;

exit_3:
    for(int i = 0; i < KIT_DEC_BUF_COUNT; i++) {
        Kit_DestroyBuffer(dec->buffer[i]);
    }
    avcodec_close(codec_ctx);
exit_2:
    av_dict_free(&codec_opts);
    avcodec_free_context(&codec_ctx);
exit_1:
    free(dec);
exit_0:
    return NULL;
}

int Kit_ReInitDecoder(Kit_Decoder *dec, int stream_index) {
    AVCodecContext *codec_ctx = NULL;
    AVCodec *codec = NULL;
    AVFormatContext *format_ctx = dec->format_ctx;
    enum AVMediaType old_type;
    enum AVMediaType new_type;

    // Make sure index seems correct
    if(stream_index >= (int)format_ctx->nb_streams || stream_index < 0) {
        Kit_SetError("Stream id out of bounds for %d", stream_index);
        goto exit_0;
    }
    // New stream type must be the same as the old one
    // We don't want to end up with eg. two audio streams :)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    old_type = format_ctx->streams[dec->stream_index]->codec->codec_type;
    new_type = format_ctx->streams[stream_index]->codec->codec_type;
#else
    old_type = format_ctx->streams[dec->stream_index]->codecpar->codec_type;
    new_type = format_ctx->streams[stream_index]->codecpar->codec_type;
#endif
    if(new_type != old_type) {
        Kit_SetError("Invalid stream type for stream %d", stream_index);
        goto exit_0;
    }
    // Find audio decoder
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    codec = avcodec_find_decoder(format_ctx->streams[stream_index]->codec->codec_id);
#else
    codec = avcodec_find_decoder(format_ctx->streams[stream_index]->codecpar->codec_id);
#endif
    if(!codec) {
        Kit_SetError("No suitable decoder found for stream %d", stream_index);
        goto exit_0;
    }
    // Allocate a context for the codec
    codec_ctx = avcodec_alloc_context3(codec);
    if(codec_ctx == NULL) {
        Kit_SetError("Unable to allocate codec context for stream %d", stream_index);
        goto exit_0;
    }
    // Copy context from stream to target codec context
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    if(avcodec_copy_context(codec_ctx, format_ctx->streams[stream_index]->codec) != 0) {
#else
    if(avcodec_parameters_to_context(codec_ctx, format_ctx->streams[stream_index]->codecpar) < 0) {
#endif
        Kit_SetError("Unable to copy codec context for stream %d", stream_index);
        goto exit_1;
    }
    // Open the stream
    if(avcodec_open2(codec_ctx, codec, NULL) < 0) {
        Kit_SetError("Unable to open codec for stream %d", stream_index);
        goto exit_1;
    }
    // Close current decoder since we have a good new decoder
    avcodec_close(dec->codec_ctx);
    avcodec_free_context(&dec->codec_ctx);
    // Set index and codec
    dec->stream_index = stream_index;
    dec->codec_ctx = codec_ctx;
    return 0;
    exit_1:
    avcodec_free_context(&codec_ctx);
    exit_0:
    return 1;
}

void Kit_CloseDecoder(Kit_Decoder *dec) {
    if(dec == NULL) return;
    if(dec->dec_close) {
        dec->dec_close(dec);
    }
    for(int i = 0; i < KIT_DEC_BUF_COUNT; i++) {
        Kit_DestroyBuffer(dec->buffer[i]);
    }
    SDL_DestroyMutex(dec->output_lock);
    avcodec_close(dec->codec_ctx);
    avcodec_free_context(&dec->codec_ctx);
    free(dec);
}

int Kit_RunDecoder(Kit_Decoder *dec) {
    if(dec == NULL) return 0;

    AVPacket *in_packet;
    int is_output_full = 1;

    // First, check if there is room in output buffer
    if(SDL_LockMutex(dec->output_lock) == 0) {
        is_output_full = Kit_IsBufferFull(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
    if(is_output_full) {
        return 0;
    }

    // Then, see if we have incoming data
    in_packet = Kit_PeekDecoderInput(dec);
    if(in_packet == NULL) {
        return 0;
    }

    // Run decoder with incoming packet
    if(dec->dec_decode(dec, in_packet) == 0) {
        Kit_AdvanceDecoderInput(dec);
        av_packet_free(&in_packet);
        return 1;
    }
    return 0;
}

// ---- Information API ----

int Kit_GetDecoderCodecInfo(const Kit_Decoder *dec, Kit_Codec *codec) {
    if(dec == NULL) {
        memset(codec, 0, sizeof(Kit_Codec));
        return 1;
    }
    codec->threads = dec->codec_ctx->thread_count;
    snprintf(codec->name, KIT_CODEC_NAME_MAX, "%s", dec->codec_ctx->codec->name);
    snprintf(codec->description, KIT_CODEC_DESC_MAX, "%s", dec->codec_ctx->codec->long_name);
    return 0;
}

int Kit_GetDecoderOutputFormat(const Kit_Decoder *dec, Kit_OutputFormat *output) {
    if(dec == NULL) {
        memset(output, 0, sizeof(Kit_OutputFormat));
        return 1;
    }
    memcpy(output, &dec->output, sizeof(Kit_OutputFormat));
    return 0;
}

int Kit_GetDecoderStreamIndex(const Kit_Decoder *dec) {
    if(dec == NULL)
        return -1;
    return dec->stream_index;
}

// ---- Clock handling ----

void Kit_SetDecoderClockSync(Kit_Decoder *dec, double sync) {
    if(dec == NULL)
        return;
    dec->clock_sync = sync;
}

void Kit_ChangeDecoderClockSync(Kit_Decoder *dec, double sync) {
    if(dec == NULL)
        return;
    dec->clock_sync += sync;
}

// ---- Input buffer handling ----

int Kit_WriteDecoderInput(Kit_Decoder *dec, AVPacket *packet) {
    assert(dec != NULL);
    return Kit_WriteBuffer(dec->buffer[KIT_DEC_BUF_IN], packet);
}

bool Kit_CanWriteDecoderInput(Kit_Decoder *dec) {
    assert(dec != NULL);
    return !Kit_IsBufferFull(dec->buffer[KIT_DEC_BUF_IN]);
}

AVPacket* Kit_ReadDecoderInput(Kit_Decoder *dec) {
    assert(dec != NULL);
    return Kit_ReadBuffer(dec->buffer[KIT_DEC_BUF_IN]);
}

AVPacket* Kit_PeekDecoderInput(Kit_Decoder *dec) {
    assert(dec != NULL);
    return Kit_PeekBuffer(dec->buffer[KIT_DEC_BUF_IN]);
}

void Kit_AdvanceDecoderInput(Kit_Decoder *dec) {
    assert(dec != NULL);
    Kit_AdvanceBuffer(dec->buffer[KIT_DEC_BUF_IN]);
}

void Kit_ClearDecoderInput(Kit_Decoder *dec) {
    Kit_ClearBuffer(dec->buffer[KIT_DEC_BUF_IN]);
}

// ---- Output buffer handling ----

int Kit_WriteDecoderOutput(Kit_Decoder *dec, void *packet) {
    assert(dec != NULL);
    int ret = 1;
    if(SDL_LockMutex(dec->output_lock) == 0) {
        ret = Kit_WriteBuffer(dec->buffer[KIT_DEC_BUF_OUT], packet);
        SDL_UnlockMutex(dec->output_lock);
    }
    return ret;
}

void Kit_ClearDecoderOutput(Kit_Decoder *dec) {
    if(SDL_LockMutex(dec->output_lock) == 0) {
        Kit_ClearBuffer(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
}

void* Kit_PeekDecoderOutput(Kit_Decoder *dec) {
    assert(dec != NULL);
    void *ret = NULL;
    if(SDL_LockMutex(dec->output_lock) == 0) {
        ret = Kit_PeekBuffer(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
    return ret;
}

void* Kit_ReadDecoderOutput(Kit_Decoder *dec) {
    assert(dec != NULL);
    void *ret = NULL;
    if(SDL_LockMutex(dec->output_lock) == 0) {
        ret = Kit_ReadBuffer(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
    return ret;
}

bool Kit_CanWriteDecoderOutput(Kit_Decoder *dec) {
    assert(dec != NULL);
    bool ret = false;
    if(SDL_LockMutex(dec->output_lock) == 0) {
        ret = !Kit_IsBufferFull(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
    return ret;
}

void Kit_ForEachDecoderOutput(Kit_Decoder *dec, Kit_ForEachItemCallback cb, void *userdata) {
    assert(dec != NULL);
    if(SDL_LockMutex(dec->output_lock) == 0) {
        Kit_ForEachItemInBuffer(dec->buffer[KIT_DEC_BUF_OUT], cb, userdata);
        SDL_UnlockMutex(dec->output_lock);
    }
}

void Kit_AdvanceDecoderOutput(Kit_Decoder *dec) {
    assert(dec != NULL);
    if(SDL_LockMutex(dec->output_lock) == 0) {
        Kit_AdvanceBuffer(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
}

unsigned int Kit_GetDecoderOutputLength(Kit_Decoder *dec) {
    assert(dec != NULL);
    unsigned int len = 0;
    if(SDL_LockMutex(dec->output_lock) == 0) {
        len = Kit_GetBufferLength(dec->buffer[KIT_DEC_BUF_OUT]);
        SDL_UnlockMutex(dec->output_lock);
    }
    return len;
}

void Kit_ClearDecoderBuffers(Kit_Decoder *dec) {
    if(dec == NULL) return;
    Kit_ClearDecoderInput(dec);
    Kit_ClearDecoderOutput(dec);
    avcodec_flush_buffers(dec->codec_ctx);
}

int Kit_LockDecoderOutput(Kit_Decoder *dec) {
    return SDL_LockMutex(dec->output_lock);
}

void Kit_UnlockDecoderOutput(Kit_Decoder *dec) {
    SDL_UnlockMutex(dec->output_lock);
}
