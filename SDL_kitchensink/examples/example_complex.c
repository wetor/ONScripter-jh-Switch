#include <kitchensink/kitchensink.h>
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

/*
* Note! This example does not do proper error handling etc.
* It is for example use only!
*/

#define AUDIOBUFFER_SIZE (1024 * 64)
#define ATLAS_WIDTH 4096
#define ATLAS_HEIGHT 4096
#define ATLAS_MAX 1024


void render_gui(SDL_Renderer *renderer, double percent) {
    // Get window size
    int size_w, size_h;
    SDL_RenderGetLogicalSize(renderer, &size_w, &size_h);

    // Render progress bar
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect progress_border;
    progress_border.x = 28;
    progress_border.y = size_h - 61;
    progress_border.w = size_w - 57;
    progress_border.h = 22;
    SDL_RenderFillRect(renderer, &progress_border);

    SDL_SetRenderDrawColor(renderer, 155, 155, 155, 255);
    SDL_Rect progress_bottom;
    progress_bottom.x = 30;
    progress_bottom.y = size_h - 60;
    progress_bottom.w = size_w - 60;
    progress_bottom.h = 20;
    SDL_RenderFillRect(renderer, &progress_bottom);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect progress_top;
    progress_top.x = 30;
    progress_top.y = size_h - 60;
    progress_top.w = (size_w - 60) * percent;
    progress_top.h = 20;
    SDL_RenderFillRect(renderer, &progress_top);
}

void find_viewport_size(int sw, int sh, int vw, int vh, int *rw, int *rh) {
    float r_x = (float)sw / (float)vw;
    float r_y = (float)sh / (float)vh;
    float r_t = r_x < r_y ? r_x : r_y;
    *rw = vw * r_t;
    *rh = vh * r_t;
}

int main(int argc, char *argv[]) {
    int err = 0, ret = 0;
    const char* filename = NULL;

    // Video
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    // Events
    SDL_Event event;
    bool run = true;
    
    // Kitchensink
    Kit_Source *src = NULL;
    Kit_Player *player = NULL;

    // Audio playback
    SDL_AudioSpec wanted_spec, audio_spec;
    SDL_AudioDeviceID audio_dev;

    // Get filename to open
    if(argc != 2) {
        fprintf(stderr, "Usage: complex <filename>\n");
        return 0;
    }
    filename = argv[1];

    // Init SDL
    err = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    if(err != 0) {
        fprintf(stderr, "Unable to initialize SDL2!\n");
        return 1;
    }

    // Create a resizable window.
    window = SDL_CreateWindow(filename, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE);
    if(window == NULL) {
        fprintf(stderr, "Unable to create a new window!\n");
        return 1;
    }

    // Create an accelerated renderer. Enable vsync, so we don't need to play around with SDL_Delay.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(window == NULL) {
        fprintf(stderr, "Unable to create a renderer!\n");
        return 1;
    }

    // Initialize Kitchensink with network and libass support.
    err = Kit_Init(KIT_INIT_NETWORK|KIT_INIT_ASS);
    if(err != 0) {
        fprintf(stderr, "Unable to initialize Kitchensink: %s", Kit_GetError());
        return 1;
    }

    // Allow Kit to use more threads
    Kit_SetHint(KIT_HINT_THREAD_COUNT, SDL_GetCPUCount() <= 4 ? SDL_GetCPUCount() : 4);

    // Lots of buffers for smooth playback (will eat up more memory, too).
    Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 5);
    Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 192);

    // Open up the sourcefile.
    // This can be a local file, network url, ...
    src = Kit_CreateSourceFromUrl(filename);
    if(src == NULL) {
        fprintf(stderr, "Unable to load file '%s': %s\n", filename, Kit_GetError());
        return 1;
    }

    // Print stream information
    Kit_SourceStreamInfo sinfo;
    fprintf(stderr, "Source streams:\n");
    for(int i = 0; i < Kit_GetSourceStreamCount(src); i++) {
        err = Kit_GetSourceStreamInfo(src, &sinfo, i);
        if(err) {
            fprintf(stderr, "Unable to fetch stream #%d information: %s.\n", i, Kit_GetError());
            return 1;
        }
        fprintf(stderr, " * Stream #%d: %s\n", i, Kit_GetKitStreamTypeString(sinfo.type));
    }

    // Create the player. Pick best video, audio and subtitle streams, and set subtitle
    // rendering resolution to screen resolution.
    player = Kit_CreatePlayer(
        src,
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
        1280, 720);
    if(player == NULL) {
        fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
        return 1;
    }

    // Print some information
    Kit_PlayerInfo pinfo;
    Kit_GetPlayerInfo(player, &pinfo);

    // Make sure there is video in the file to play first.
    if(Kit_GetPlayerVideoStream(player) == -1) {
        fprintf(stderr, "File contains no video!\n");
        return 1;
    }

    fprintf(stderr, "Media information:\n");
    if(Kit_GetPlayerAudioStream(player) >= 0) {
        fprintf(stderr, " * Audio: %s (%s), threads=%d, %dHz, %dch, %db, %s\n",
            pinfo.audio.codec.name,
            pinfo.audio.codec.description,
            pinfo.video.codec.threads,
            pinfo.audio.output.samplerate,
            pinfo.audio.output.channels,
            pinfo.audio.output.bytes,
            pinfo.audio.output.is_signed ? "signed" : "unsigned");
    }
    if(Kit_GetPlayerVideoStream(player) >= 0) {
        fprintf(stderr, " * Video: %s (%s), threads=%d, %dx%d\n",
            pinfo.video.codec.name,
            pinfo.video.codec.description,
            pinfo.video.codec.threads,
            pinfo.video.output.width,
            pinfo.video.output.height);
    }
    if(Kit_GetPlayerSubtitleStream(player) >= 0) {
        fprintf(stderr, " * Subtitle: %s (%s), threads=%d\n",
            pinfo.subtitle.codec.name,
            pinfo.subtitle.codec.description,
            pinfo.video.codec.threads);
    }
    fprintf(stderr, "Duration: %f seconds\n", Kit_GetPlayerDuration(player));

    // Init audio
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
    wanted_spec.freq = pinfo.audio.output.samplerate;
    wanted_spec.format = pinfo.audio.output.format;
    wanted_spec.channels = pinfo.audio.output.channels;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &audio_spec, 0);
    SDL_PauseAudioDevice(audio_dev, 0);

    // Print some format info
    fprintf(stderr, "Texture type: %s\n", Kit_GetSDLPixelFormatString(pinfo.video.output.format));
    fprintf(stderr, "Audio format: %s\n", Kit_GetSDLAudioFormatString(pinfo.audio.output.format));
    fprintf(stderr, "Subtitle format: %s\n", Kit_GetSDLPixelFormatString(pinfo.subtitle.output.format));
    fflush(stderr);

    // Initialize video texture. This will probably end up as YV12 most of the time.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Texture *video_tex = SDL_CreateTexture(
        renderer,
        pinfo.video.output.format,
        SDL_TEXTUREACCESS_STATIC,
        pinfo.video.output.width,
        pinfo.video.output.height);
    if(video_tex == NULL) {
        fprintf(stderr, "Error while attempting to create a video texture\n");
        return 1;
    }

    // This is the subtitle texture atlas. This contains all the subtitle image fragments.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // Always nearest for atlas operations
    SDL_Texture *subtitle_tex = SDL_CreateTexture(
        renderer,
        pinfo.subtitle.output.format,
        SDL_TEXTUREACCESS_STATIC,
        ATLAS_WIDTH, ATLAS_HEIGHT);
    if(subtitle_tex == NULL) {
        fprintf(stderr, "Error while attempting to create a subtitle texture atlas\n");
        return 1;
    }

    // Make sure subtitle texture is in correct blendmode
    SDL_SetTextureBlendMode(subtitle_tex, SDL_BLENDMODE_BLEND);

    // Clear screen with black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Start playback
    Kit_PlayerPlay(player);

    // Playback temporary data buffers
    char audiobuf[AUDIOBUFFER_SIZE];
    SDL_Rect sources[ATLAS_MAX];
    SDL_Rect targets[ATLAS_MAX];
    int mouse_x = 0;
    int mouse_y = 0;
    int size_w = 0;
    int size_h = 0;
    int screen_w = 0;
    int screen_h = 0;
    bool fullscreen = false;

    // Get movie area size
    SDL_GetWindowSize(window, &screen_w, &screen_h);
    find_viewport_size(screen_w, screen_h, pinfo.video.output.width, pinfo.video.output.height, &size_w, &size_h);
    SDL_RenderSetLogicalSize(renderer, size_w, size_h);
    Kit_SetPlayerScreenSize(player, size_w, size_h);
    
    // Run until playback is stopped
    while(run) {
        if(Kit_GetPlayerState(player) == KIT_STOPPED) {
            run = false;
            continue;
        }

        // Check for events
        const Uint8 *state;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_ESCAPE) {
                        run = false;
                    }
                    break;

                case SDL_KEYDOWN:
                    // Find alt+enter
                    state = SDL_GetKeyboardState(NULL);
                    if(state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_LALT]) {
                        if(!fullscreen) {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        } else {
                            SDL_SetWindowFullscreen(window, 0);
                        }
                        fullscreen = !fullscreen;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    break;

                case SDL_WINDOWEVENT:
                    switch(event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            SDL_GetWindowSize(window, &screen_w, &screen_h);
                            find_viewport_size(
                                screen_w, screen_h, pinfo.video.output.width, pinfo.video.output.height, &size_w, &size_h);
                            SDL_RenderSetLogicalSize(renderer, size_w, size_h);
                            Kit_SetPlayerScreenSize(player, size_w, size_h);
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    // Handle user clicking the progress bar
                    if(mouse_x >= 30 && mouse_x <= size_w-30 && mouse_y >= size_h - 60 && mouse_y <= size_h - 40) {
                        double pos = ((double)mouse_x - 30) / ((double)size_w - 60);
                        double m_time = Kit_GetPlayerDuration(player) * pos;
                        if(Kit_PlayerSeek(player, m_time) != 0) {
                            fprintf(stderr, "%s\n", Kit_GetError());
                        }
                        SDL_ClearQueuedAudio(audio_dev);
                    } else {
                        // Handle pause
                        if(Kit_GetPlayerState(player) == KIT_PAUSED) {
                            Kit_PlayerPlay(player);
                        } else {
                            Kit_PlayerPause(player);
                        }
                    }
                    break;

                case SDL_QUIT:
                    run = false;
                    break;
            }
        }

        // Refresh audio
        int queued = SDL_GetQueuedAudioSize(audio_dev);
        if(queued < AUDIOBUFFER_SIZE) {
            int need = AUDIOBUFFER_SIZE - queued;

            while(need > 0) {
                ret = Kit_GetPlayerAudioData(
                    player,
                    (unsigned char*)audiobuf,
                    AUDIOBUFFER_SIZE);
                need -= ret;
                if(ret > 0) {
                    SDL_QueueAudio(audio_dev, audiobuf, ret);
                } else {
                    break;
                }
            }
            // If we now have data, start playback (again)
            if(SDL_GetQueuedAudioSize(audio_dev) > 0) {
                SDL_PauseAudioDevice(audio_dev, 0);
            }
        }

        // Refresh videotexture and render it
        Kit_GetPlayerVideoData(player, video_tex);
        SDL_RenderCopy(renderer, video_tex, NULL, NULL);

        // Refresh subtitle texture atlas and render subtitle frames from it
        // For subtitles, use screen size instead of video size for best quality
        int got = Kit_GetPlayerSubtitleData(player, subtitle_tex, sources, targets, ATLAS_MAX);
        for(int i = 0; i < got; i++) {
            SDL_RenderCopy(renderer, subtitle_tex, &sources[i], &targets[i]);
        }

        // Enable GUI if mouse is hovering over the bottom third of the screen
        if(mouse_y >= ((size_h / 3) * 2)) {
            double percent = Kit_GetPlayerPosition(player) / Kit_GetPlayerDuration(player);
            render_gui(renderer, percent);
        }

        // Render to screen + wait for vsync
        SDL_RenderPresent(renderer);
    }

    Kit_ClosePlayer(player);
    Kit_CloseSource(src);
    Kit_Quit();

    SDL_DestroyTexture(subtitle_tex);
    SDL_DestroyTexture(video_tex);
    SDL_CloseAudioDevice(audio_dev);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
