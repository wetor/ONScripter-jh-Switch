#ifndef KITPLAYER_H
#define KITPLAYER_H

/**
 * @brief Video/audio player functions
 * 
 * @file kitplayer.h
 * @author Tuomas Virtanen
 * @date 2018-06-27
 */

#include "kitchensink/kitsource.h"
#include "kitchensink/kitconfig.h"
#include "kitchensink/kitformat.h"
#include "kitchensink/kitcodec.h"

#include <SDL_render.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Playback states
 */
typedef enum Kit_PlayerState {
    KIT_STOPPED = 0, ///< Playback stopped or has not started yet.
    KIT_PLAYING,     ///< Playback started & player is actively decoding.
    KIT_PAUSED,      ///< Playback paused; player is actively decoding but no new data is given out.
    KIT_CLOSED,      ///< Playback is stopped and player is closing.
} Kit_PlayerState;

/**
 * @brief Player state container
 */
typedef struct Kit_Player {
    Kit_PlayerState state;   ///< Playback state
    void *decoders[3];       ///< Decoder contexts
    void *dec_thread;        ///< Decoder thread
    void *dec_lock;          ///< Decoder lock
    const Kit_Source *src;   ///< Reference to Audio/Video source
    double pause_started;    ///< Temporary flag for handling pauses
} Kit_Player;

/**
 * @brief Contains data about a stream selected for playback
 */
typedef struct Kit_PlayerStreamInfo {
    Kit_Codec codec; ///< Decoder codec information
    Kit_OutputFormat output; ///< Information about the output format
} Kit_PlayerStreamInfo;

/**
 * @brief Contains information about the streams selected for playback
 * 
 */
typedef struct Kit_PlayerInfo {
    Kit_PlayerStreamInfo video; ///< Video stream data
    Kit_PlayerStreamInfo audio; ///< Audio stream data
    Kit_PlayerStreamInfo subtitle; ///< Subtitle stream data
} Kit_PlayerInfo;

/**
 * @brief Creates a new player from a source.
 * 
 * Creates a new player from the given source. The source must be previously succesfully
 * initialized by calling either Kit_CreateSourceFromUrl() or Kit_CreateSourceFromCustom(), 
 * and it must not be used by any other player. Source must stay valid during the whole 
 * playback (as in, don't close it while stuff is playing).
 * 
 * Screen width and height are used for subtitle positioning, scaling and rendering resolution.
 * Ideally this should be precisely the size of your screen surface (in pixels).
 * Higher resolution leads to higher resolution text rendering. This MUST be set precisely
 * if you plan to use font hinting! If you don't care or don't have subtitles at all,
 * set both to video surface size or 0.
 * 
 * For streams, either video and/or audio stream MUST be set! Either set the stream indexes manually,
 * or pick them automatically by using Kit_GetBestSourceStream().
 * 
 * On success, this will return an initialized Kit_Player which can later be freed by Kit_ClosePlayer().
 * On error, NULL is returned and a more detailed error is availably via Kit_GetError().
 * 
 * For example:
 * ```
 * Kit_Player *player = Kit_CreatePlayer(
 *     src,
 *     Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
 *     Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
 *     Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
 *     1280, 720);
 * if(player == NULL) {
 *     fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
 *     return 1;
 * }
 * ```
 * 
 * @param src Valid video/audio source
 * @param video_stream_index Video stream index or -1 if not wanted
 * @param audio_stream_index Audio stream index or -1 if not wanted
 * @param subtitle_stream_index Subtitle stream index or -1 if not wanted
 * @param screen_w Screen width in pixels
 * @param screen_h Screen height in pixels
 * @return Ínitialized Kit_Player or NULL
 */
KIT_API Kit_Player* Kit_CreatePlayer(const Kit_Source *src,
                                     int video_stream_index,
                                     int audio_stream_index,
                                     int subtitle_stream_index,
                                     int screen_w,
                                     int screen_h);

/**
 * @brief Close previously initialized player
 * 
 * Closes a previously initialized Kit_Player instance. Note that this does NOT free
 * the linked Kit_Source -- you must free it manually.
 * 
 * @param player Player instance
 */
KIT_API void Kit_ClosePlayer(Kit_Player *player);

/**
 * @brief Sets the current screen size in pixels
 * 
 * Call this to change the subtitle font rendering resolution if eg. your
 * video window size changes.
 * 
 * This does nothing if subtitles are not in use or if subtitles are bitmaps.
 * 
 * @param player Player instance
 * @param w New width in pixels
 * @param h New height in pixels
 */
KIT_API void Kit_SetPlayerScreenSize(Kit_Player *player, int w, int h);

/**
 * @brief Gets the current video stream index
 * 
 * Returns the current video stream index or -1 if one is not selected.
 * 
 * @param player Player instance
 * @return Video stream index or -1
 */
KIT_API int Kit_GetPlayerVideoStream(const Kit_Player *player);

/**
 * @brief Gets the current audio stream index
 * 
 * Returns the current audio stream index or -1 if one is not selected.
 * 
 * @param player Player instance
 * @return Audio stream index or -1
 */
KIT_API int Kit_GetPlayerAudioStream(const Kit_Player *player);

/**
 * @brief Gets the current subtitle stream index
 * 
 * Returns the current subtitle stream index or -1 if one is not selected.
 * 
 * @param player Player instance
 * @return Subtitle stream index or -1
 */
KIT_API int Kit_GetPlayerSubtitleStream(const Kit_Player *player);

/**
 * @brief Fetches a new video frame from the player
 * 
 * Note that the output texture must be previously allocated and valid. 
 * 
 * It is important to select the correct texture format and size. If you pick a different
 * texture format or size from what the decoder outputs, then the decoder will attempt to convert
 * the frames to fit the texture. This will slow down the decoder a *lot* however, so if possible,
 * pick the texture format from what Kit_GetPlayerInfo() outputs.
 * 
 * Access flag for the texture *MUST* always be SDL_TEXTUREACCESS_STATIC! Anything else will lead to
 * undefined behaviour.
 * 
 * This function will do nothing if player playback has not been started.
 * 
 * @param player Player instance
 * @param texture A previously allocated texture
 * @return 0 on success, 1 on error
 */
KIT_API int Kit_GetPlayerVideoData(Kit_Player *player, SDL_Texture *texture);

KIT_API int Kit_GetPlayerVideoDataRaw(Kit_Player *player, void *data);

/**
 * @brief Fetches subtitle data from the player
 * 
 * Output texture will be used as a texture atlas for the subtitle fragments.
 * 
 * Note that the output texture must be previously allocated and valid. Make sure to have large
 * enough a texture for the rendering resolution you picked! If your rendering resolution if 4k,
 * then make sure to have texture sized 4096x4096 etc. This gives the texture room to handle the
 * worst case subtitle textures. If your resolutions is too small, this function will return
 * value -1. At that point you can replace your current texture with a bigger one on the fly.
 * 
 * Note that the texture format for the atlas texture *MUST* be SDL_PIXELFORMAT_RGBA32 and
 * the access flag *MUST* be set to SDL_TEXTUREACCESS_STATIC for correct rendering.
 * Using any other format will lead to undefined behaviour. Also, make sure to set scaling quality
 * to 0 or "nearest" before creating the texture -- otherwise you get artifacts
 * (see SDL_HINT_RENDER_SCALE_QUALITY).
 * 
 * This function will do nothing if player playback has not been started.
 * 
 * For example:
 * ```
 * SDL_Rect sources[256];
 * SDL_Rect targets[256];
 * int got = Kit_GetPlayerSubtitleData(player, subtitle_tex, sources, targets, 256);
 * for(int i = 0; i < got; i++) {
 *     SDL_RenderCopy(renderer, subtitle_tex, &sources[i], &targets[i]);
 * }
 * ```
 * 
 * @param player Player instance
 * @param texture A previously allocated texture
 * @param sources List of source rectangles to copy fropm
 * @param targets List of target rectangles to render
 * @param limit Defines the maximum size of your rectangle lists
 * @return Number of sources or <0 on error
 */
KIT_API int Kit_GetPlayerSubtitleData(Kit_Player *player,
                                      SDL_Texture *texture,
                                      SDL_Rect *sources,
                                      SDL_Rect *targets,
                                      int limit);

KIT_API int Kit_GetPlayerSubtitleDataRaw(Kit_Player *player,
                                      void *data,
                                      SDL_Rect *sources,
                                      SDL_Rect *targets,
                                      int limit);

/**
 * @brief Fetches audio data from the player
 * 
 * Note that the output buffer must be previously allocated.
 * 
 * Outputted audio data will be precisely what is described by the output format struct given
 * by Kit_GetPlayerInfo().
 * 
 * This function will attemt to read the maximum allowed amount of data allowed by the length
 * argument. It is possible however that there is not enough data available, at which point
 * this function will read less and return value may differ from maximum allowed value.
 * Return value 0 should be taken as a hint that there is nothing available.
 * 
 * This function will do nothing if player playback has not been started.
 * 
 * @param player Player instance
 * @param buffer Buffer to read into
 * @param length Maximum length of the buffer
 * @return Amount of data that was read, <0 on error.
 */
KIT_API int Kit_GetPlayerAudioData(Kit_Player *player, unsigned char *buffer, int length);

/**
 * @brief Fetches information about the currently selected streams
 * 
 * This function should be used to fetch codec information and output format data from the player
 * before creating textures and setting up audio outputs.
 * 
 * @param player Player instance
 * @param info A previously allocated Kit_PlayerInfo instance
 */
KIT_API void Kit_GetPlayerInfo(const Kit_Player *player, Kit_PlayerInfo *info);

/**
 * @brief Returns the current state of the player
 * 
 * @param player Player instance
 * @return Current state of the player, see Kit_PlayerState
 */
KIT_API Kit_PlayerState Kit_GetPlayerState(const Kit_Player *player);

/**
 * @brief Starts playback
 * 
 * State shifts:
 * - If player is already playing, will do nothing.
 * - If player is paused, will resume playback.
 * - If player is stopped, will begin playback (and background decoding).
 * 
 * @param player Player instance
 */
#ifdef __PPLAY__
KIT_API int Kit_PlayerPlay(Kit_Player *player);
KIT_API void Kit_SetClockSync(Kit_Player *player);
#else
KIT_API void Kit_PlayerPlay(Kit_Player *player);
#endif

/**
 * @brief Stops playback
 * 
 * State shifts:
 * - If player is already stopped, will do nothing.
 * - If player is paused, will stop playback.
 * - If player is started, will stop playback (and background decoding).
 * 
 * @param player Player instance
 */
KIT_API void Kit_PlayerStop(Kit_Player *player);

/**
 * @brief Pauses playback
 * 
 * State shifts:
 * - If player is already paused, will do nothing.
 * - If player is stopped, will do nothing.
 * - If player is started, will pause playback (and background decoding).
 * 
 * @param player Player instance
 */
KIT_API void Kit_PlayerPause(Kit_Player *player);

/**
 * @brief Seek to timestamp
 * 
 * Rewinds or forwards video/audio playback to the given timestamp (in seconds).
 * 
 * This may not work for network or custom sources!
 * 
 * @param player Player instance
 * @param time Timestamp to seek to in seconds
 * @return 0 on success, 1 on failure.
 */
KIT_API int Kit_PlayerSeek(Kit_Player *player, double time);

/**
 * @brief Selects stream index for specified stream type.
 *
 * This allows switching streams during or outside playback. Handy for eg.
 * switching subtitle track.
 *
 * @param player Player instance
 * @param type Stream to switch
 * @param index Index to use (list can be queried from the source)
 * @return 0 on success, 1 on failure.
 */

KIT_API int Kit_SetPlayerStream(Kit_Player *player, const Kit_StreamType type, int index);
/**
* @brief Returns the current index of the specified stream type
*
* @param player Player instance
* @param type Stream to switch
* @return Stream index or -1 on error or if stream is not set
*/
KIT_API int Kit_GetPlayerStream(const Kit_Player *player, const Kit_StreamType type);

/**
 * @brief Get the duration of the source
 * 
 * Returns the duration of the source in seconds
 * 
 * @param player Player instance
 * @return Duration
 */
KIT_API double Kit_GetPlayerDuration(const Kit_Player *player);

/**
 * @brief Get the current position of the playback
 * 
 * Returns the position of the playback in seconds
 * 
 * @param player Player instance
 * @return Position
 */
KIT_API double Kit_GetPlayerPosition(const Kit_Player *player);

/// PPLAY
int Kit_PlayerSeekStart(Kit_Player *player, double position, double seek_set);
int Kit_PlayerSeekEnd(Kit_Player *player, double position, double seek_set);

#ifdef __cplusplus
}
#endif

#endif // KITPLAYER_H
