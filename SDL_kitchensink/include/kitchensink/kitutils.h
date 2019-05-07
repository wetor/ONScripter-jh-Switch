#ifndef KITUTILS_H
#define KITUTILS_H

/**
 * @brief Helpful utilities
 * 
 * @file kitutils.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#include "kitchensink/kitconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns a descriptive string for SDL audio format types
 * 
 * @param type SDL_AudioFormat
 * @return Format string, eg. "AUDIO_S8".
 */
KIT_API const char* Kit_GetSDLAudioFormatString(unsigned int type);

/**
 * @brief Returns a descriptive string for SDL pixel format types
 * 
 * @param type SDL_PixelFormat
 * @return Format string, eg. "SDL_PIXELFORMAT_YV12"
 */
KIT_API const char* Kit_GetSDLPixelFormatString(unsigned int type);

/**
 * @brief Returns a descriptibe string for Kitchensink stream types
 * 
 * @param type Kit_StreamType
 * @return Format string, eg. "KIT_STREAMTYPE_VIDEO"
 */
KIT_API const char* Kit_GetKitStreamTypeString(unsigned int type);

#ifdef __cplusplus
}
#endif

#endif // KITUTILS_H
