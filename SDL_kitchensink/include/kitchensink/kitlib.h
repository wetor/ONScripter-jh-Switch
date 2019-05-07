#ifndef KITLIB_H
#define KITLIB_H

/**
 * @brief Library initialization and deinitialization functionality
 * 
 * @file kitlib.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#include "kitchensink/kitconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Font hinting options. Used as values for Kit_SetHint(KIT_HINT_FONT_HINTING, ...).
 */
enum {
    KIT_FONT_HINTING_NONE = 0,  ///< No hinting. This is recommended option
    KIT_FONT_HINTING_LIGHT,  ///< Light hinting. Use this if you need hinting
    KIT_FONT_HINTING_NORMAL,  ///< Not recommended, please see libass docs for details
    KIT_FONT_HINTING_NATIVE,  ///< Not recommended, please see libass docs for details
    KIT_FONT_HINTING_COUNT
};

/**
 * @brief SDL_kitchensink library version container
 */
typedef struct Kit_Version {
    unsigned char major;  ///< Major version number, raising this signifies API breakage
    unsigned char minor;  ///< Minor version number, small/internal changes
    unsigned char patch;  ///< Patch version number, bugfixes etc.
} Kit_Version;

/**
 * @brief Library hint types. Used as keys for Kit_SetHint().
 * 
 * Note that all of these must be set *before* player initialization for them to take effect!
 */
typedef enum Kit_HintType {
    KIT_HINT_FONT_HINTING, ///< Set font hinting mode (currently used for libass)
    KIT_HINT_THREAD_COUNT, ///< Set thread count for ffmpeg (1 by default)
    KIT_HINT_VIDEO_BUFFER_FRAMES, ///< Video output buffer frames (3 by default)
    KIT_HINT_AUDIO_BUFFER_FRAMES, ///< Audio output buffers (64 by default)
    KIT_HINT_SUBTITLE_BUFFER_FRAMES ///< Subtitle output buffers (64 by default, used by image subtitles)
} Kit_HintType;

/**
 * @brief Library initialization options, please see Kit_Init()
 * 
 */
enum {
    KIT_INIT_NETWORK = 0x1, ///< Initialise ffmpeg network support
    KIT_INIT_ASS = 0x2 ///< Initialize libass support (library must be linked statically or loadable dynamically)
};

/**
 * @brief Initialize SDL_kitchensink library.
 * 
 * This MUST be run before doing anything. After you are done using the library, you should use Kit_Quit() to
 * deinitialize everything. Otherwise there might be resource leaks.
 * 
 * Following flags can be used to initialize subsystems:
 * - `KIT_INIT_NETWORK` for ffmpeg network support (playback from the internet, for example)
 * - `KIT_INIT_ASS` for libass subtitles (text and ass/ssa subtitle support)
 * 
 * Note that if this function fails, the failure reason should be available via Kit_GetError().
 * 
 * For example:
 * ```
 * if(Kit_Init(KIT_INIT_NETWORK|KIT_INIT_ASS) != 0) {
 *     fprintf(stderr, "Error: %s\n", Kit_GetError());
 *     return 1;
 * }
 * ```
 * 
 * @param flags Library initialization flags
 * @return Returns 0 on success, 1 on failure.
 */
KIT_API int Kit_Init(unsigned int flags);

/**
 * @brief Deinitializes SDL_kitchensink
 * 
 * Note that any calls to library functions after this will cause undefined behaviour!
 */
KIT_API void Kit_Quit();

/**
 * @brief Sets a librarywide hint
 * 
 * This can be used to set hints on how the library should behave. See Kit_HintType
 * for all the options.
 * 
 * @param type Hint type (refer to Kit_HintType for options)
 * @param value Value for the hint
 */
KIT_API void Kit_SetHint(Kit_HintType type, int value);

/**
 * @brief Gets a previously set or default hint value
 * 
 * @param type Hint type (refer to Kit_HintType for options)
 * @return Hint value
 */
KIT_API int Kit_GetHint(Kit_HintType type);

/**
 * @brief Can be used to fetch the version of the linked SDL_kitchensink library
 * 
 * @param version Allocated Kit_Version
 */
KIT_API void Kit_GetVersion(Kit_Version *version);

#ifdef __cplusplus
}
#endif

#endif // KITLIB_H
