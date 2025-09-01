// ----------------------------------------------------------------------------
// Minimal SDL2 Utility Wrapper
// v 1.2.1 Sep 2025
// ----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdlut.h"


struct SDLData SDLDATA;
static Uint32 _last_t;

// events.c
extern void DefaultSDLEventCallback(SDL_Event e);


void PrintSDLError() {
	char *msg = (char*)SDL_GetError();
	printf("SDL Error: %s\n", msg);
}

// ----------------------------------------------------------------------------
// AUDIO

// SDL Audio is not actually being used 
extern void AudioCallback(void *userdata, Uint8 *stream, int len);	// implemented in audio.c

void InitSDLAudio(int samplerate) {

	SDL_AudioSpec want, have;

	SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
	want.freq = SDLUT_DEFAULT_AUDIO_SAMPLE_RATE;
	want.format = AUDIO_F32;
	want.channels = 1;
	want.samples = 2048;			// 512 to 8192 (needs some trial&error)
	want.callback = NULL; //AudioCallback;	// set to NULL for using push method via SDL_QueueAudio()

	SDLDATA.audio_device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (SDLDATA.audio_device_id == 0) {
		printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		return;
	}
	else {
		if (have.format != want.format) {	/* we let this one thing change. */
			SDL_Log("We didn't get Float32 audio format.");
		}
		SDL_PauseAudioDevice(SDLDATA.audio_device_id, 0);		/* start audio playing. */
	}

	/*
	SDL_AudioSpec soundspec;
	soundspec.freq = samplerate;
	soundspec.channels = 1;
	soundspec.format = AUDIO_S16;
	soundspec.samples = 16384;
	soundspec.userdata = NULL;
	soundspec.callback = play;
	if (SDL_OpenAudio(&soundspec, NULL) < 0) {
		printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		return;
	} 
	SDL_PauseAudio(0);  // start sdl playback
	*/
}


void DefaultSDLEventCallback(SDL_Event e) {
}

int InitSDL() {

	// init the SDLDATA structure with some defaults

	SDLDATA.bg_r = 0;
	SDLDATA.bg_g = 0;
	SDLDATA.bg_b = 0;
	SDLDATA.bg_a = 255;
	SDLDATA.event_cb = DefaultSDLEventCallback;
	SDLDATA.runloop = true;

	//printf("SDL_INIT\n");

	// init the video driver
	int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (ret != 0) {
		PrintSDLError();
		return 1;
	}

	Uint32 sdlflags = 0;

	// create our display window
	//printf("SDL_CreateWindow\n");

	SDLDATA.screen = SDL_CreateWindow(((SDLDATA.app_name[0] != 0) ? SDLDATA.app_name : "SDL-WINDOW"),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SDLDATA.width, SDLDATA.height,
		sdlflags);

	if (SDLDATA.screen == NULL) {
		PrintSDLError();
		return 2;
	}

	// Auto size the window
	SDL_DisplayMode current;
	ret = SDL_GetCurrentDisplayMode(0, &current);
	if (ret != 0) {
		PrintSDLError();
		return ret;
	}

	// adapt our window resolution (and thus dimensions) to the 
	// maximum size that will fit the users desktop display
	// NOTE: autosizing giving me problems on WSL right now
#if 0
	int x = current.w / SDLDATA.v_width;	// how often "does it fit in"
	int y = current.h / SDLDATA.v_height;
	int scale = (x < y) ? x : y;
	x = SDLDATA.v_width * scale;
	y = SDLDATA.v_height * scale;
	SDLDATA.width = x;				// get actual dimensions
	SDLDATA.height = y;
	SDL_SetWindowSize(SDLDATA.screen, x, y);
	SDL_SetWindowPosition(SDLDATA.screen, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_SetWindowResizable(SDLDATA.screen, true);
#endif

	// 2d SDL renderer
	//printf("SDL_CreateRenderer\n");
	// NOTE: we enforce VSYNC here and that means (these days) 60Hz 
	SDLDATA.renderer = SDL_CreateRenderer(SDLDATA.screen, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	if (SDLDATA.renderer == NULL) {
		PrintSDLError();
		return 3;
	}

	// create display surface (this is normally where the application renders to)
	// and display texture 
	SDLDATA.display_surface = SDL_CreateRGBSurface(0, SDLDATA.v_width, SDLDATA.v_height, SDLDATA.v_depth, 0, 0, 0, 0);
	if (!SDLDATA.display_surface) {
		PrintSDLError();
		return 5;
	}
	SDLDATA.display_texture = SDL_CreateTextureFromSurface(SDLDATA.renderer, SDLDATA.display_surface);
	if (!SDLDATA.display_texture) {
		PrintSDLError();
		return 6;
	}

	// enable SDL textinput events
	SDL_StartTextInput();

	// Clean up on exit, exit on window close and interrupt
	// atexit(SDL_Quit);
	
	// initialize frame timer
	_last_t = SDL_GetTicks();

	// init audio
	// NOT using SDL audio at this point (using SOKOL instead)
	// InitSDLAudio(P_DEFAULT_AUDIO_SAMPLE_RATE);

	//printf("InitSDL() done\n");

	return 0;
}

void USetWindowTitle(const char* title) {
	SDL_SetWindowTitle(SDLDATA.screen, title);
}


void BeginSDLFrame() {
	const Uint8 *keys;

	// update frame timer
	Uint32 now_t = SDL_GetTicks();
	SDLDATA.last_frame_t =  now_t - _last_t;
	_last_t = now_t;

	// Poll & Process SDL events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_MOUSEBUTTONDOWN:
#if SDLUT_ESC_LEFT_CLICK_TERMINATES
			// [ESC]+left click terminates
			keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_ESCAPE] == SDL_PRESSED) {
				SDLDATA.runloop = false;

				SDL_Event user_event;
				user_event.type = SDL_USEREVENT;
				SDL_PushEvent(&user_event);
			}
#endif
			if (SDLDATA.event_cb)
				SDLDATA.event_cb(event);
			break;

		case SDL_USEREVENT:
			if (SDLDATA.event_cb)
				SDLDATA.event_cb(event);
			break;

		// pre processed text input
		case SDL_TEXTINPUT:
			if (SDLDATA.event_cb)
				SDLDATA.event_cb(event);
			break;

		// key pressed
		case SDL_KEYDOWN:
			if (SDLDATA.event_cb)
				SDLDATA.event_cb(event);
			break;

			// key released
		case SDL_KEYUP:
			if (SDLDATA.event_cb)
				SDLDATA.event_cb(event);
			break;

		case SDL_QUIT:
			SDLDATA.runloop = false;
			break;
		}	// end switch

		//return SDL_GetTicks();
	}	// end while poll event 


	// call application begin frame code
	if (SDLDATA.begin_frame_cb)
		SDLDATA.begin_frame_cb();

}

void EndSDLFrame() {

	// call application end frame code
	if (SDLDATA.end_frame_cb)
		SDLDATA.end_frame_cb();

	// clear the back buffer in order to prepare for rendering this frame
	// ready the SDL 2D renderer for this frame
	// NOT needed when copying full size textures over the entire display below
	//SDL_SetRenderDrawColor(SDLDATA.renderer, SDLDATA.bg_r, SDLDATA.bg_g, SDLDATA.bg_b, SDLDATA.bg_a);
	//SDL_RenderClear(SDLDATA.renderer);

	// blit the display surface/texture to the main display: this involves updating the texture 
	// from the display surface and then rendering the texture to the actual display
	int err = SDL_UpdateTexture(SDLDATA.display_texture, NULL, SDLDATA.display_surface->pixels, SDLDATA.display_surface->pitch);
	if (err != 0) PrintSDLError();

	// this automatically takes care of any scaling if the actual SDL window used is larger than our display
	err = SDL_RenderCopy(SDLDATA.renderer, SDLDATA.display_texture, NULL, NULL);
	if (err != 0) PrintSDLError();

	//SDLDATA.display_frame_count++;

	SDL_RenderPresent(SDLDATA.renderer);
}

#if SDLUT_PROVIDE_WORKER_THREAD

// Optional worker thread for conveniently running event&display processing 
// in the background automatically 

// need to provide this if IMachine is to be used
extern void RenderDisplay();

// SDLUT worker thread
// NOTE that its vital that the same thread calls InitSDL() and Begin/End SDLFrame()!
int _IMachineRun(void *userdata) {

	InitSDL();

	while (SDLDATA.runloop) {
		BeginSDLFrame();
		RenderDisplay();
		EndSDLFrame();
	}

	return 0;
}


SDL_Thread *CreateSDLBackgroundThread(const char *name) {

	// clear out the SDLDATA structure, this is mandatory
	memset(&SDLDATA, 0, sizeof(struct SDLData));

	SDLDATA.v_width = FRAME_WIDTH;	// sdl display must match settings in display.h
	SDLDATA.v_height = FRAME_HEIGHT;
	SDLDATA.v_depth = 32;

	strcpy(SDLDATA.app_name, name);

	// create the SDLUT worker thread
	SDLDATA.ITHREAD = SDL_CreateThread(_IMachineRun, name, (void *)NULL);
	return SDLDATA.ITHREAD;
}
#endif

// sdlut.c
