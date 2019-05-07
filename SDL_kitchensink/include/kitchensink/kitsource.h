#ifndef KITSOURCE_H
#define KITSOURCE_H

/**
 * @brief Video/Audio source file handling
 * 
 * @file kitsource.h
 * @author Tuomas Virtanen
 * @date 2018-06-27
 */

#include <inttypes.h>
#include <SDL_rwops.h>
#include "kitchensink/kitconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of the stream.
 * 
 * This is used by Kit_SourceStreamInfo and Kit_GetSourceStreamInfo().
 */
typedef enum Kit_StreamType {
    KIT_STREAMTYPE_UNKNOWN, ///< Unknown stream type
    KIT_STREAMTYPE_VIDEO, ///< Video stream
    KIT_STREAMTYPE_AUDIO, ///< Audio stream
    KIT_STREAMTYPE_DATA, ///< Data stream
    KIT_STREAMTYPE_SUBTITLE, ///< Subtitle streawm
    KIT_STREAMTYPE_ATTACHMENT ///< Attachment stream (images, etc)
} Kit_StreamType;

/**
 * @brief Audio/video source.
 * 
 * Should be created using Kit_CreateSourceFromUrl() or Kit_CreateSourceFromCustom(), and 
 * closed with Kit_CloseSource().
 * 
 * Source must exist for the whole duration of using a player. You must take care of closing the source
 * yourself after you are done with it!
 */
typedef struct Kit_Source {
    void *format_ctx; ///< FFmpeg: Videostream format context
    void *avio_ctx;   ///< FFmpeg: AVIO context
} Kit_Source;

/**
 * @brief Information for a source stream.
 * 
 * Fetch information by using Kit_GetSourceStreamInfo().
 */
typedef struct Kit_SourceStreamInfo {
    int index; ///< Stream index
    Kit_StreamType type; ///< Stream type
} Kit_SourceStreamInfo;

/**
 * @brief Callback function type for reading data stream
 * 
 * Used by Kit_CreateSourceFromCustom() for reading data from user defined source.
 * 
 * A custom reader function must accept three arguments:
 * - userdata, this is the same data as set as last argument for Kit_CreateSourceFromCustom
 * - buf, a buffer the data must be copied into
 * - size, how much data you are expected to provide at maximum.
 * 
 * The function must return the amount of bytes copied to the buffer or <0 on error.
 * 
 * Note that this callback is passed directly to ffmpeg avio, so please refer to ffmpeg documentation
 * for any further details.
 */
typedef int (*Kit_ReadCallback)(void *userdata, uint8_t *buf, int size);

/**
 * @brief Callback function type for seeking data strema
 * 
 * Used by Kit_CreateSourceFromCustom() for seeking a user defined source.
 * 
 * A custom seeking function must accept three arguments:
 * - userdata, this is the same data as set as last argument for Kit_CreateSourceFromCustom
 * - offset, an seeking offset in bytes
 * - whence, reference position for the offset.
 * 
 * Whence parameter can be one of the standard fseek values or optionally AVSEEK_SIZE.
 * - SEEK_SET: Reference position is beginning of file
 * - SEEK_CUR: Reference position is the current position of the file pointer
 * - SEEK_END: Reference position is the end of the file
 * - AVSEEK_SIZE: Optional. Does not seek, instead finds the size of the source file.
 * - AVSEEK_FORCE: Optional. Suggests that seeking should be done at any cost. May be passed alongside
 *   any of the SEEK_* flags, eg. SEEK_SET|AVSEEK_FORCE.
 * 
 * The function must return the position (in bytes) we seeked to or <0 on error or on unsupported operation.
 * 
 * Note that this callback is passed directly to ffmpeg avio, so please refer to ffmpeg documentation
 * for any further details.
 */
typedef int64_t (*Kit_SeekCallback)(void *userdata, int64_t offset, int whence);

/**
 * @brief Create a new source from a given url
 * 
 * This can be used to load video/audio from a file or network resource. If you wish to 
 * use network resources, make sure the library has been initialized using KIT_INIT_NETWORK flag. 
 * 
 * This function will return an initialized Kit_Source on success. Note that you need to manually
 * free the source when it's no longer needed by calling Kit_CloseSource().
 * 
 * On failure, this function will return NULL, and further error data is available via Kit_GetError().
 * 
 * For example:
 * ```
 * Kit_Source *src = Kit_CreateSourceFromUrl(filename);
 * if(src == NULL) {
 *     fprintf(stderr, "Error: %s\n", Kit_GetError());
 *     return 1;
 * }
 * ```
 * 
 * @param url File path or URL to a video/audio resource 
 * @return Returns an initialized Kit_Source* on success or NULL on failure
 */
KIT_API Kit_Source* Kit_CreateSourceFromUrl(const char *url);

/**
 * @brief Create a new source from custom data
 * 
 * This can be used to load data from any resource via the given read and seek functions.
 * 
 * This function will return an initialized Kit_Source on success. Note that you need to manually
 * free the source when it's no longer needed by calling Kit_CloseSource().
 * 
 * On failure, this function will return NULL, and further error data is available via Kit_GetError().
 * 
 * For example:
 * ```
 * Kit_Source *src = Kit_CreateSourceFromCustom(read_fn, seek_fn, fp);
 * if(src == NULL) {
 *     fprintf(stderr, "Error: %s\n", Kit_GetError());
 *     return 1;
 * }
 * ```
 * 
 * @param read_cb Read function callback
 * @param seek_cb Seek function callback
 * @param userdata Any data (or NULL). Will be passed to read_cb and/or seek_cb functions as-is.
 * @return Returns an initialized Kit_Source* on success or NULL on failure
 */
KIT_API Kit_Source* Kit_CreateSourceFromCustom(Kit_ReadCallback read_cb, Kit_SeekCallback seek_cb, void *userdata);

/**
 * @brief Create a new source from SDL RWops struct
 * 
 * Can be used to read data from SDL compatible sources.
 * 
 * This function will return an initialized Kit_Source on success. Note that you need to manually
 * free the source when it's no longer needed by calling Kit_CloseSource().
 * 
 * On failure, this function will return NULL, and further error data is available via Kit_GetError().
 * 
 * Note that the RWops struct must exist during the whole lifetime of the source, and you must take
 * care of freeing the rwops after it's no longer needed.
 * 
 * For example:
 * ```
 * SDL_RWops *rw = SDL_RWFromFile("myvideo.mkv", "rb");
 * Kit_Source *src = Kit_CreateSourceFromRW(rw);
 * if(src == NULL) {
 *     fprintf(stderr, "Error: %s\n", Kit_GetError());
 *     return 1;
 * }
 * ```
 * 
 * @param rw_ops Initialized RWOps
 * @return KIT_API* Kit_CreateSourceFromRW 
 */
KIT_API Kit_Source* Kit_CreateSourceFromRW(SDL_RWops *rw_ops);

/**
 * @brief Closes a previously initialized source
 * 
 * Closes a Kit_Source that was previously created by Kit_CreateSourceFromUrl() or Kit_CreateSourceFromCustom()
 * and frees up all memory and resources used by it. Using the source for anything after this will
 * lead to undefined behaviour.
 * 
 * Passing NULL as argument is valid, and will do nothing.
 * 
 * @param src Previously initialized Kit_Source to close
 */
KIT_API void Kit_CloseSource(Kit_Source *src);

/**
 * @brief Fetches stream information for a given stream index
 * 
 * Sets fields for given Kit_SourceStreamInfo with information about the stream.
 * 
 * For example:
 * ```
 * Kit_SourceStreamInfo stream;
 * if(Kit_GetSourceStreamInfo(source, &stream, 0) == 1) {
 *     fprintf(stderr, "Error: %s\n", Kit_GetError());
 *     return 1;
 * }
 * fprintf(stderr, "Stream type: %s\n", Kit_GetKitStreamTypeString(stream.type))
 * ```
 * 
 * @param src Source to query from
 * @param info A previously allocated Kit_SourceStreamInfo to fill out
 * @param index Stream index (starting from 0)
 * @return 0 on success, 1 on error.
 */
KIT_API int Kit_GetSourceStreamInfo(const Kit_Source *src, Kit_SourceStreamInfo *info, int index);

/**
 * @brief Makes a list of stream indexes with requested type
 *
 * This can be used to get all stream indexes of certain type, eg. all subtitle streams.
 *
 * @param src Source to query from
 * @param type Stream type to search
 * @param list Integer list to insert into
 * @param size Maximum size of the list
 * @return Number of elements found
 */
KIT_API int Kit_GetSourceStreamList(const Kit_Source *src, const Kit_StreamType type, int *list, int size);

/**
 * @brief Gets the amount of streams in source
 * 
 * @param src Source to query from
 * @return Number of streams in the source
 */
KIT_API int Kit_GetSourceStreamCount(const Kit_Source *src);

/**
 * @brief Gets the best stream index for a given stream type.
 * 
 * Find the best stream index for a given stream type, if one exists. If there is no
 * stream for the wanted type, will return -1.
 * 
 * @param src Source to query from
 * @param type Stream type
 * @return Index number on success (>=0), -1 on error.
 */
KIT_API int Kit_GetBestSourceStream(const Kit_Source *src, const Kit_StreamType type);

#ifdef __cplusplus
}
#endif

#endif // KITSOURCE_H
