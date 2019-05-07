#include <kitchensink/kitchensink.h>
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

/*
* Note! This example does not do proper error handling etc.
* It is for example use only!
*/

#define AUDIOBUFFER_SIZE (32768)

const char *stream_types[] = {
    "KIT_STREAMTYPE_UNKNOWN",
    "KIT_STREAMTYPE_VIDEO",
    "KIT_STREAMTYPE_AUDIO",
    "KIT_STREAMTYPE_DATA",
    "KIT_STREAMTYPE_SUBTITLE",
    "KIT_STREAMTYPE_ATTACHMENT"
};

int main(int argc, char *argv[]) {
    int err = 0, ret = 0;
    const char* filename = NULL;

    // Events
    bool run = true;
    
    // Kitchensink
    Kit_Source *src = NULL;
    Kit_Player *player = NULL;

    // Audio playback
    SDL_AudioSpec wanted_spec, audio_spec;
    SDL_AudioDeviceID audio_dev;
    char audiobuf[AUDIOBUFFER_SIZE];

    // Get filename to open
    if(argc != 2) {
        fprintf(stderr, "Usage: audio <filename>\n");
        return 0;
    }
    filename = argv[1];

    // Init SDL
    err = SDL_Init(SDL_INIT_AUDIO);
    if(err != 0) {
        fprintf(stderr, "Unable to initialize SDL!\n");
        return 1;
    }

    err = Kit_Init(KIT_INIT_NETWORK);
    if(err != 0) {
        fprintf(stderr, "Unable to initialize Kitchensink: %s", Kit_GetError());
        return 1;
    }

    // Open up the sourcefile.
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
        fprintf(stderr, " * Stream #%d: %s\n", i, stream_types[sinfo.type]);
    }

    // Create the player. No video, pick best audio stream, no subtitles, no screen
    player = Kit_CreatePlayer(
        src,
        -1,
        Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
        -1,
        0, 0);
    if(player == NULL) {
        fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
        return 1;
    }

    // Print some information
    Kit_PlayerInfo pinfo;
    Kit_GetPlayerInfo(player, &pinfo);

    // Make sure there is audio in the file to play first.
    if(Kit_GetPlayerAudioStream(player) == -1) {
        fprintf(stderr, "File contains no audio!\n");
        return 1;
    }

    fprintf(stderr, "Media information:\n");
    fprintf(stderr, " * Audio: %s (%s), %dHz, %dch, %db, %s\n",
        pinfo.audio.codec.name,
        pinfo.audio.codec.description,
        pinfo.audio.output.samplerate,
        pinfo.audio.output.channels,
        pinfo.audio.output.bytes,
        pinfo.audio.output.is_signed ? "signed" : "unsigned");

    // Init audio
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
    wanted_spec.freq = pinfo.audio.output.samplerate;
    wanted_spec.format = pinfo.audio.output.format;
    wanted_spec.channels = pinfo.audio.output.channels;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &audio_spec, 0);
    SDL_PauseAudioDevice(audio_dev, 0);

    // Flush output just in case
    fflush(stderr);

    // Start playback
    Kit_PlayerPlay(player);

    while(run) {
        if(Kit_GetPlayerState(player) == KIT_STOPPED) {
            run = false;
            continue;
        }

        // Refresh audio
        int queued = SDL_GetQueuedAudioSize(audio_dev);
        if(queued < AUDIOBUFFER_SIZE) {
            ret = Kit_GetPlayerAudioData(player, (unsigned char*)audiobuf, AUDIOBUFFER_SIZE - queued);
            if(ret > 0) {
                SDL_QueueAudio(audio_dev, audiobuf, ret);
            }
        }

        SDL_Delay(1);
    }

    Kit_ClosePlayer(player);
    Kit_CloseSource(src);
    Kit_Quit();

    SDL_CloseAudioDevice(audio_dev);
    SDL_Quit();
    return 0;
}
