#ifndef KITCODEC_H
#define KITCODEC_H

/**
 * @brief Codec type
 * 
 * @file kitcodec.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#ifdef __cplusplus
extern "C" {
#endif

#define KIT_CODEC_NAME_MAX 8
#define KIT_CODEC_DESC_MAX 48

/**
 * @brief Contains information about the used codec for playback
 */
typedef struct Kit_Codec {
    unsigned int threads; ///< Currently enabled threads (For all decoders)
    char name[KIT_CODEC_NAME_MAX]; ///< Codec short name, eg. "ogg" or "webm"
    char description[KIT_CODEC_DESC_MAX]; ///< Codec longer, more descriptive name
} Kit_Codec;

#ifdef __cplusplus
}
#endif

#endif // KITCODEC_H
