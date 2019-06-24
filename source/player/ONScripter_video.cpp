#include "ONScripter.h"
#include "Utils.h"
#include <kitchensink/kitchensink.h>
#include <stdbool.h>
/*
* Note! This example does not do proper error handling etc.
* It is for example use only!
*/

#define AUDIOBUFFER_SIZE (1024 * 64)
#define ATLAS_WIDTH 4096
#define ATLAS_HEIGHT 4096
#define ATLAS_MAX 1024
enum JoyKey {
	JKEY_A,
	JKEY_B,
	JKEY_X,
	JKEY_Y
};
int ONScripter::PlayVideo(SDL_RWops *file_rw, char* filename, bool debug) {
	
	

	printf("-S-------------------------------%d\n", SDL_GetTicks());
	Mix_CloseAudio();
	int err = 0, ret = 0;
	bool run = true;
	Kit_Source *src = NULL;
	Kit_Player *player = NULL;
	SDL_AudioSpec wanted_spec, audio_spec;
	SDL_AudioDeviceID audio_dev;


	// Initialize Kitchensink with network and libass support.
	err = Kit_Init(KIT_INIT_NETWORK | KIT_INIT_ASS);
	if (err != 0) {
		utils::printInfo("Unable to initialize Kitchensink: %s", Kit_GetError());
		return 1;
	}
	// Allow Kit to use more threads
	Kit_SetHint(KIT_HINT_THREAD_COUNT, 4);
	//utils::printInfo("SDL_GetCPUCount %d\n", SDL_GetCPUCount());
	// Lots of buffers for smooth playback (will eat up more memory, too).
	Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 5);
	Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 192);
	// Open up the sourcefile.
	// This can be a local file, network url, ...
	if (filename != NULL) {
		src = Kit_CreateSourceFromUrl(filename);
	}
	else {
		src = Kit_CreateSourceFromRW(file_rw);
	}
	if (src == NULL) {
		utils::printInfo("Unable to load file '%s': %s\n", filename, Kit_GetError());
		return 1;
	}

	// Create the player. Pick best video, audio and subtitle streams, and set subtitle
	// rendering resolution to screen resolution.
	player = Kit_CreatePlayer(
		src,
		Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
		Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
		Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
		1920, 1080);
	if (player == NULL) {
		utils::printInfo("Unable to create player: %s\n", Kit_GetError());
		return 1;
	}
	// Print some information
	Kit_PlayerInfo pinfo;
	Kit_GetPlayerInfo(player, &pinfo);

	// Make sure there is video in the file to play first.
	if (Kit_GetPlayerVideoStream(player) == -1) {
		utils::printInfo("File contains no video!\n");
		return 1;
	}
	/*utils::printInfo("Media information:\n");
	if (Kit_GetPlayerAudioStream(player) >= 0) {
		utils::printInfo(" * Audio: %s (%s), threads=%d, %dHz, %dch, %db, %s\n",
			pinfo.audio.codec.name,
			pinfo.audio.codec.description,
			pinfo.video.codec.threads,
			pinfo.audio.output.samplerate,
			pinfo.audio.output.channels,
			pinfo.audio.output.bytes,
			pinfo.audio.output.is_signed ? "signed" : "unsigned");
	}
	if (Kit_GetPlayerVideoStream(player) >= 0) {
		utils::printInfo(" * Video: %s (%s), threads=%d, %dx%d\n",
			pinfo.video.codec.name,
			pinfo.video.codec.description,
			pinfo.video.codec.threads,
			pinfo.video.output.width,
			pinfo.video.output.height);
	}
	if (Kit_GetPlayerSubtitleStream(player) >= 0) {
		utils::printInfo(" * Subtitle: %s (%s), threads=%d\n",
			pinfo.subtitle.codec.name,
			pinfo.subtitle.codec.description,
			pinfo.video.codec.threads);
	}
	utils::printInfo("Duration: %f seconds\n", Kit_GetPlayerDuration(player));
	*/
	// Init audio
	SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
	wanted_spec.freq = pinfo.audio.output.samplerate;
	wanted_spec.format = pinfo.audio.output.format;
	wanted_spec.channels = pinfo.audio.output.channels;
	audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &audio_spec, 0);
	if (audio_dev == 0) {
		printf("Failed to open audio: %s\n", SDL_GetError());
	}
	SDL_PauseAudioDevice(audio_dev, 0);
	// Initialize video texture. This will probably end up as YV12 most of the time.

	SDL_Texture *video_tex = SDL_CreateTexture(
		renderer,
		pinfo.video.output.format,
		SDL_TEXTUREACCESS_STATIC,
		pinfo.video.output.width,
		pinfo.video.output.height);
	if (video_tex == NULL) {
		utils::printInfo("Error while attempting to create a video texture\n");
		return 1;
	}
	
	// This is the subtitle texture atlas. This contains all the subtitle image fragments.
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // Always nearest for atlas operations
	SDL_Texture * subtitle_tex = SDL_CreateTexture(renderer, pinfo.subtitle.output.format, SDL_TEXTUREACCESS_STATIC, ATLAS_WIDTH, ATLAS_HEIGHT);
	if (subtitle_tex == NULL) {
		utils::printInfo("Error while attempting to create a subtitle texture atlas\n");
		return 1;
	}

	// Make sure subtitle texture is in correct blendmode
	SDL_SetTextureBlendMode(subtitle_tex, SDL_BLENDMODE_BLEND);

	printf("-E-------------------------------%d\n", SDL_GetTicks());

	// Start playback
	Kit_PlayerPlay(player);

	// Playback temporary data buffers
	char audiobuf[AUDIOBUFFER_SIZE];
	SDL_Rect sources[ATLAS_MAX];
	SDL_Rect targets[ATLAS_MAX];
	
	SDL_mutexP(mutex);
	// Get movie area size
	SDL_RenderSetLogicalSize(renderer, pinfo.video.output.width, pinfo.video.output.height);

	while (run) {
		if (Kit_GetPlayerState(player) == KIT_STOPPED) {
			run = false;
			continue;
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				goto end;
			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.button == JKEY_X || event.jbutton.button == JKEY_Y) {//Ìø¹ý
					goto end;
				}
				/*if (event.jbutton.button == JKEY_B ) {
					double aaa = Kit_GetPlayerPosition(player);
					printf("%lf / %lf\n", aaa, Kit_GetPlayerDuration(player));
					Kit_PlayerSeek(player,  aaa+ 30);
				}*/
				break;
			}
		}


		// Refresh audio
		int queued = SDL_GetQueuedAudioSize(audio_dev);
		if (queued < AUDIOBUFFER_SIZE) {
			int need = AUDIOBUFFER_SIZE - queued;

			while (need > 0) {
				ret = Kit_GetPlayerAudioData(
					player,
					(unsigned char*)audiobuf,
					AUDIOBUFFER_SIZE);
				//utils::printInfo("%d\n", ret);
				need -= ret;
				if (ret > 0) {
					SDL_QueueAudio(audio_dev, audiobuf, ret);
				}
				else {
					break;
				}
			}

			// If we now have data, start playback (again)
			if (SDL_GetQueuedAudioSize(audio_dev) > 0) {
				SDL_PauseAudioDevice(audio_dev, 0);
			}
		}
		// Refresh videotexture and render it
		Kit_GetPlayerVideoData(player, video_tex);
		SDL_RenderCopy(renderer, video_tex, NULL, NULL);

		// Refresh subtitle texture atlas and render subtitle frames from it
		// For subtitles, use screen size instead of video size for best quality
		int got = Kit_GetPlayerSubtitleData(player, subtitle_tex, sources, targets, ATLAS_MAX);
		for (int i = 0; i < got; i++) {
			SDL_RenderCopy(renderer, subtitle_tex, &sources[i], &targets[i]);
		}

		// Render to screen + wait for vsync
		SDL_RenderPresent(renderer);
	}
end:
	Kit_ClosePlayer(player);
	Kit_CloseSource(src);
	Kit_Quit();

	SDL_DestroyTexture(subtitle_tex);
	SDL_DestroyTexture(video_tex);
	SDL_CloseAudioDevice(audio_dev);


	openAudio();
	SDL_RenderSetLogicalSize(renderer, device_width / scale_ratio, device_height / scale_ratio);
	SDL_mutexV(mutex);
	SDL_CondSignal(cond);

	return 0;
}
