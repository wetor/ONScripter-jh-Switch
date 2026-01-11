/* -*- C++ -*-
 *
 *  ONScripter_video.cpp - Video Player for ONScripter Nintendo Switch
 *
 *  Copyright (C) 2019-2025 wetor <makisehoshimi@163.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ONScripter.h"
#include "Utils.h"
#include <kitchensink/kitchensink.h>
#include <stdbool.h>

// Buffer and atlas constants
namespace {
    constexpr int AUDIOBUFFER_SIZE = 1024 * 64;
    constexpr int ATLAS_WIDTH = 4096;
    constexpr int ATLAS_HEIGHT = 4096;
    constexpr int ATLAS_MAX = 1024;
    constexpr int DEFAULT_VIDEO_WIDTH = 1920;
    constexpr int DEFAULT_VIDEO_HEIGHT = 1080;
    constexpr int THREAD_COUNT = 4;
    constexpr int VIDEO_BUFFER_FRAMES = 5;
    constexpr int AUDIO_BUFFER_FRAMES = 192;

    // Joy-Con button mappings
    enum class JoyButton : int {
        A = 0,
        B = 1,
        X = 2,
        Y = 3
    };
}

/**
 * Video player result codes
 */
enum class VideoResult {
    Success = 0,
    InitError = 1,
    SourceError = 2,
    PlayerError = 3,
    NoVideoStream = 4,
    TextureError = 5,
    AudioError = 6
};

/**
 * RAII wrapper for Kit_Source
 */
class KitSourceGuard {
public:
    explicit KitSourceGuard(Kit_Source* src) : source(src) {}
    ~KitSourceGuard() {
        if (source) {
            Kit_CloseSource(source);
        }
    }
    Kit_Source* get() const { return source; }
    Kit_Source* release() {
        Kit_Source* tmp = source;
        source = nullptr;
        return tmp;
    }
private:
    Kit_Source* source;
};

/**
 * RAII wrapper for Kit_Player
 */
class KitPlayerGuard {
public:
    explicit KitPlayerGuard(Kit_Player* p) : player(p) {}
    ~KitPlayerGuard() {
        if (player) {
            Kit_ClosePlayer(player);
        }
    }
    Kit_Player* get() const { return player; }
private:
    Kit_Player* player;
};

/**
 * RAII wrapper for SDL_Texture
 */
class TextureGuard {
public:
    explicit TextureGuard(SDL_Texture* tex) : texture(tex) {}
    ~TextureGuard() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    SDL_Texture* get() const { return texture; }
private:
    SDL_Texture* texture;
};

/**
 * Play video file using SDL_kitchensink library
 *
 * @param file_rw SDL_RWops for reading from memory/file
 * @param filename Path to video file (optional, used if file_rw is null)
 * @param debug Enable debug output
 * @return 0 on success, non-zero on error
 */
int ONScripter::PlayVideo(SDL_RWops* file_rw, char* filename, bool debug)
{
    utils::printInfo("PlayVideo: Starting video playback\n");
    Uint32 startTime = SDL_GetTicks();

    // Close audio mixer temporarily for video playback
    Mix_CloseAudio();

    // Initialize Kitchensink with network and libass support
    int err = Kit_Init(KIT_INIT_NETWORK | KIT_INIT_ASS);
    if (err != 0) {
        utils::printError("PlayVideo: Failed to initialize Kitchensink: %s\n", Kit_GetError());
        openAudio();
        return static_cast<int>(VideoResult::InitError);
    }

    // Configure Kitchensink hints for optimal performance
    Kit_SetHint(KIT_HINT_THREAD_COUNT, THREAD_COUNT);
    Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, VIDEO_BUFFER_FRAMES);
    Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, AUDIO_BUFFER_FRAMES);

    // Open source file
    Kit_Source* src = nullptr;
    if (filename != nullptr) {
        utils::printInfo("PlayVideo: Opening file: %s\n", filename);
        src = Kit_CreateSourceFromUrl(filename);
    } else if (file_rw != nullptr) {
        utils::printInfo("PlayVideo: Opening from RWops\n");
        src = Kit_CreateSourceFromRW(file_rw);
    } else {
        utils::printError("PlayVideo: No source specified\n");
        Kit_Quit();
        openAudio();
        return static_cast<int>(VideoResult::SourceError);
    }

    if (src == nullptr) {
        utils::printError("PlayVideo: Failed to open source: %s\n", Kit_GetError());
        Kit_Quit();
        openAudio();
        return static_cast<int>(VideoResult::SourceError);
    }

    KitSourceGuard sourceGuard(src);

    // Create player with best available streams
    Kit_Player* player = Kit_CreatePlayer(
        src,
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
        DEFAULT_VIDEO_WIDTH,
        DEFAULT_VIDEO_HEIGHT
    );

    if (player == nullptr) {
        utils::printError("PlayVideo: Failed to create player: %s\n", Kit_GetError());
        Kit_Quit();
        openAudio();
        return static_cast<int>(VideoResult::PlayerError);
    }

    KitPlayerGuard playerGuard(player);

    // Get player information
    Kit_PlayerInfo pinfo;
    Kit_GetPlayerInfo(player, &pinfo);

    // Verify video stream exists
    if (Kit_GetPlayerVideoStream(player) == -1) {
        utils::printError("PlayVideo: No video stream found\n");
        Kit_Quit();
        openAudio();
        return static_cast<int>(VideoResult::NoVideoStream);
    }

    // Log media information in debug mode
    if (debug) {
        utils::printInfo("PlayVideo: Media information:\n");
        if (Kit_GetPlayerAudioStream(player) >= 0) {
            utils::printInfo("  Audio: %s (%s), %dHz, %dch\n",
                pinfo.audio.codec.name,
                pinfo.audio.codec.description,
                pinfo.audio.output.samplerate,
                pinfo.audio.output.channels);
        }
        if (Kit_GetPlayerVideoStream(player) >= 0) {
            utils::printInfo("  Video: %s (%s), %dx%d\n",
                pinfo.video.codec.name,
                pinfo.video.codec.description,
                pinfo.video.output.width,
                pinfo.video.output.height);
        }
        if (Kit_GetPlayerSubtitleStream(player) >= 0) {
            utils::printInfo("  Subtitle: %s (%s)\n",
                pinfo.subtitle.codec.name,
                pinfo.subtitle.codec.description);
        }
        utils::printInfo("  Duration: %.2f seconds\n", Kit_GetPlayerDuration(player));
    }

    // Initialize audio device
    SDL_AudioSpec wanted_spec, audio_spec;
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
    wanted_spec.freq = pinfo.audio.output.samplerate;
    wanted_spec.format = pinfo.audio.output.format;
    wanted_spec.channels = pinfo.audio.output.channels;

    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &audio_spec, 0);
    if (audio_dev == 0) {
        utils::printWarning("PlayVideo: Failed to open audio device: %s\n", SDL_GetError());
        // Continue without audio
    } else {
        SDL_PauseAudioDevice(audio_dev, 0);
    }

    // Create video texture
    SDL_Texture* video_tex = SDL_CreateTexture(
        renderer,
        pinfo.video.output.format,
        SDL_TEXTUREACCESS_STATIC,
        pinfo.video.output.width,
        pinfo.video.output.height
    );

    if (video_tex == nullptr) {
        utils::printError("PlayVideo: Failed to create video texture: %s\n", SDL_GetError());
        if (audio_dev != 0) SDL_CloseAudioDevice(audio_dev);
        Kit_Quit();
        openAudio();
        return static_cast<int>(VideoResult::TextureError);
    }

    TextureGuard videoTexGuard(video_tex);

    // Create subtitle texture atlas
    SDL_Texture* subtitle_tex = SDL_CreateTexture(
        renderer,
        pinfo.subtitle.output.format,
        SDL_TEXTUREACCESS_STATIC,
        ATLAS_WIDTH,
        ATLAS_HEIGHT
    );

    if (subtitle_tex == nullptr) {
        utils::printWarning("PlayVideo: Failed to create subtitle texture\n");
        // Continue without subtitles
    } else {
        SDL_SetTextureBlendMode(subtitle_tex, SDL_BLENDMODE_BLEND);
    }

    TextureGuard subtitleTexGuard(subtitle_tex);

    utils::printInfo("PlayVideo: Initialization took %u ms\n", SDL_GetTicks() - startTime);

    // Start playback
    Kit_PlayerPlay(player);

    // Lock mutex for rendering
    SDL_mutexP(mutex);

    // Set logical size for video rendering
    SDL_RenderSetLogicalSize(renderer, pinfo.video.output.width, pinfo.video.output.height);

    // Playback buffers
    char audiobuf[AUDIOBUFFER_SIZE];
    SDL_Rect sources[ATLAS_MAX];
    SDL_Rect targets[ATLAS_MAX];

    bool running = true;
    bool userSkipped = false;

    // Main playback loop
    while (running) {
        // Check if playback has stopped naturally
        if (Kit_GetPlayerState(player) == KIT_STOPPED) {
            running = false;
            continue;
        }

        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    userSkipped = true;
                    break;

                case SDL_JOYBUTTONDOWN:
                    // Allow skip with X or Y button
                    if (event.jbutton.button == static_cast<int>(JoyButton::X) ||
                        event.jbutton.button == static_cast<int>(JoyButton::Y)) {
                        running = false;
                        userSkipped = true;
                    }
                    break;

                case SDL_KEYDOWN:
                    // Allow skip with Escape or Space
                    if (event.key.keysym.sym == SDLK_ESCAPE ||
                        event.key.keysym.sym == SDLK_SPACE) {
                        running = false;
                        userSkipped = true;
                    }
                    break;

                default:
                    break;
            }
        }

        if (!running) break;

        // Refresh audio buffer
        if (audio_dev != 0) {
            int queued = SDL_GetQueuedAudioSize(audio_dev);
            if (queued < AUDIOBUFFER_SIZE) {
                int need = AUDIOBUFFER_SIZE - queued;

                while (need > 0) {
                    int ret = Kit_GetPlayerAudioData(
                        player,
                        reinterpret_cast<unsigned char*>(audiobuf),
                        AUDIOBUFFER_SIZE
                    );

                    if (ret > 0) {
                        SDL_QueueAudio(audio_dev, audiobuf, ret);
                        need -= ret;
                    } else {
                        break;
                    }
                }

                // Resume audio if we have data
                if (SDL_GetQueuedAudioSize(audio_dev) > 0) {
                    SDL_PauseAudioDevice(audio_dev, 0);
                }
            }
        }

        // Refresh and render video
        Kit_GetPlayerVideoData(player, video_tex);
        SDL_RenderCopy(renderer, video_tex, nullptr, nullptr);

        // Refresh and render subtitles
        if (subtitle_tex != nullptr) {
            int got = Kit_GetPlayerSubtitleData(player, subtitle_tex, sources, targets, ATLAS_MAX);
            for (int i = 0; i < got; i++) {
                SDL_RenderCopy(renderer, subtitle_tex, &sources[i], &targets[i]);
            }
        }

        // Present frame
        SDL_RenderPresent(renderer);
    }

    if (userSkipped) {
        utils::printInfo("PlayVideo: Playback skipped by user\n");
    } else {
        utils::printInfo("PlayVideo: Playback completed\n");
    }

    // Cleanup
    Kit_Quit();

    if (audio_dev != 0) {
        SDL_CloseAudioDevice(audio_dev);
    }

    // Restore audio and rendering state
    openAudio();
    SDL_RenderSetLogicalSize(renderer, device_width / scale_ratio, device_height / scale_ratio);

    SDL_mutexV(mutex);
    SDL_CondSignal(cond);

    utils::printInfo("PlayVideo: Total playback time: %u ms\n", SDL_GetTicks() - startTime);

    return static_cast<int>(VideoResult::Success);
}
