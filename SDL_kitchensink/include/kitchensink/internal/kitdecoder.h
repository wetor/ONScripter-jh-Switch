#ifndef KITDECODER_H
#define KITDECODER_H

#include <stdbool.h>

#include <SDL_mutex.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "kitchensink/kitformat.h"
#include "kitchensink/kitcodec.h"
#include "kitchensink/kitconfig.h"
#include "kitchensink/kitsource.h"
#include "kitchensink/internal/utils/kitbuffer.h"

enum {
    KIT_DEC_BUF_IN = 0,
    KIT_DEC_BUF_OUT,
    KIT_DEC_BUF_COUNT
};

typedef struct Kit_Decoder Kit_Decoder;

typedef int (*dec_decode_cb)(Kit_Decoder *dec, AVPacket *in_packet);

typedef void (*dec_close_cb)(Kit_Decoder *dec);

typedef void (*dec_free_packet_cb)(void *packet);

KIT_LOCAL struct Kit_Decoder {
    int stream_index;            ///< Source stream index for the current stream
    double clock_sync;           ///< Sync source for current stream
    double clock_pos;            ///< Current pts for the stream
    Kit_OutputFormat output;     ///< Output format for the decoder

    AVCodecContext *codec_ctx;   ///< FFMpeg internal: Codec context
    AVFormatContext *format_ctx; ///< FFMpeg internal: Format context (owner: Kit_Source)

    SDL_mutex *output_lock;      ///< Threading lock for output buffer
    Kit_Buffer *buffer[2];       ///< Buffers for incoming and decoded packets

    void *userdata;              ///< Decoder specific information (Audio, video, subtitle context)
    dec_decode_cb dec_decode;    ///< Decoder decoding function callback
    dec_close_cb dec_close;      ///< Decoder close function callback
};

KIT_LOCAL Kit_Decoder *Kit_CreateDecoder(const Kit_Source *src, int stream_index,
                                         int out_b_size, dec_free_packet_cb free_out_cb,
                                         int thread_count);

KIT_LOCAL int Kit_ReInitDecoder(Kit_Decoder *dec, int stream_index);

KIT_LOCAL void Kit_CloseDecoder(Kit_Decoder *dec);

KIT_LOCAL int Kit_GetDecoderStreamIndex(const Kit_Decoder *dec);

KIT_LOCAL int Kit_GetDecoderCodecInfo(const Kit_Decoder *dec, Kit_Codec *codec);

KIT_LOCAL int Kit_GetDecoderOutputFormat(const Kit_Decoder *dec, Kit_OutputFormat *output);

KIT_LOCAL void Kit_SetDecoderClockSync(Kit_Decoder *dec, double sync);

KIT_LOCAL void Kit_ChangeDecoderClockSync(Kit_Decoder *dec, double sync);

KIT_LOCAL int Kit_RunDecoder(Kit_Decoder *dec);

KIT_LOCAL void Kit_ClearDecoderBuffers(Kit_Decoder *dec);

KIT_LOCAL bool Kit_CanWriteDecoderInput(Kit_Decoder *dec);

KIT_LOCAL int Kit_WriteDecoderInput(Kit_Decoder *dec, AVPacket *packet);

KIT_LOCAL AVPacket *Kit_ReadDecoderInput(Kit_Decoder *dec);

KIT_LOCAL void Kit_ClearDecoderInput(Kit_Decoder *dec);

KIT_LOCAL AVPacket *Kit_PeekDecoderInput(Kit_Decoder *dec);

KIT_LOCAL void Kit_AdvanceDecoderInput(Kit_Decoder *dec);

KIT_LOCAL int Kit_WriteDecoderOutput(Kit_Decoder *dec, void *packet);

KIT_LOCAL bool Kit_CanWriteDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void *Kit_PeekDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void *Kit_ReadDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void Kit_AdvanceDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void Kit_ForEachDecoderOutput(Kit_Decoder *dec, Kit_ForEachItemCallback foreach_cb, void *userdata);

KIT_LOCAL int Kit_LockDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void Kit_UnlockDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL void Kit_ClearDecoderOutput(Kit_Decoder *dec);

KIT_LOCAL unsigned int Kit_GetDecoderOutputLength(Kit_Decoder *dec);

#ifdef __cplusplus
}
#endif

#endif // KITDECODER_H
