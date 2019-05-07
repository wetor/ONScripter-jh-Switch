#ifndef KITFORMAT_H
#define KITFORMAT_H

/**
 * @brief Audio/video output format type
 * 
 * @file kitformat.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Contains information about the data format coming out from the player
 */
typedef struct Kit_OutputFormat {
    unsigned int format; ///< SDL Format (SDL_PixelFormat if video/subtitle, SDL_AudioFormat if audio)
    int is_signed;       ///< Signedness, 1 = signed, 0 = unsigned (if audio)
    int bytes;           ///< Bytes per sample per channel (if audio)
    int samplerate;      ///< Sampling rate (if audio)
    int channels;        ///< Channels (if audio)
    int width;           ///< Width in pixels (if video)
    int height;          ///< Height in pixels (if video)
} Kit_OutputFormat;

#ifdef __cplusplus
}
#endif

#endif // KITFORMAT_H
