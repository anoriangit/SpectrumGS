#pragma once
#include <stdbool.h>

#include <SDL.h>
#include <SDL_audio.h>

// ----------------------------------------------------------------------------
// Minimal SDL2 Utility Wrapper
// v 1.2.1 Sep 2025
// ----------------------------------------------------------------------------

#define SDLUT_AUDIO_BUFFER_SIZE			0xffff
#define SDLUT_DEFAULT_AUDIO_SAMPLE_RATE 44100
#define SDLUT_APPNAME_MAX_SIZE			128

// We encode scancode + modifier + event type as a single event
// SDL scan codes range from 0-512 and thus can be encoded in 9 bits (we use 12) 
// SDL key modifiers are encoded using 16 bits
// four bits are used for type (none, keydown, keyup, timer)

typedef uint32_t key_event_t;

#define MAX_KEY_EVENTS	64

#define KEY_CODE_MASK	0x0000fff0
#define KEY_CODE_SHIFT	4
#define KEY_MOD_MASK	0xffff0000
#define KEY_MOD_SHIFT	16
#define KEY_TYPE_MASK	0x0000000f

#define KEY_MOD(e)  ((e&KEY_MOD_MASK)>>KEY_MOD_SHIFT)
#define KEY_CODE(e) ((e&KEY_CODE_MASK)>>KEY_CODE_SHIFT)
#define KEY_TYPE(e) (e&KEY_TYPE_MASK)

// scancode, mod, type
#define KEY_EVENT(k, m, t) ((m<<KEY_MOD_SHIFT)|((k<<KEY_CODE_SHIFT)&KEY_CODE_MASK)|(t&KEY_TYPE_MASK))

#define KEY_TYPE_NONE		0
#define KEY_TYPE_KEYUP		1
#define KEY_TYPE_KEYDOWN	2
#define KEY_TYPE_TIMER		3


#define KMOD_NONE	0x0000
#define	KMOD_LSHIFT 0x0001
#define	KMOD_RSHIFT 0x0002
#define	KMOD_LCTRL	0x0040
#define	KMOD_RCTRL	0x0080
#define KMOD_LALT	0x0100
#define KMOD_RALT	0x0200
#define KMOD_LGUI	0x0400
#define KMOD_RGUI	0x0800
#define KMOD_NUM	0x1000
#define KMOD_CAPS	0x2000
#define KMOD_MODE	0x4000
#define KMOD_RESERVED 0x8000



struct SDLData {
	SDL_Window *screen;
	SDL_Renderer *renderer;

	SDL_Surface *display_surface;
	SDL_Texture *display_texture;

	int v_depth;				// virtual display depth, also actual depth of created display_surface
	int v_width, v_height;		// virtual dimensions as required by the application
	int width, height;			// actual dimensions: the display will be scaled up to this size

	SDL_AudioDeviceID audio_device_id;

	Uint8 bg_r, bg_g, bg_b, bg_a;
	bool runloop;
	bool abort;

	Uint32 last_frame_t;	// ms since last BeginFrame()

	SDL_Thread *ITHREAD;

	void(*event_cb) (SDL_Event);
	void(*begin_frame_cb) ();
	void(*end_frame_cb) ();

	char app_name[SDLUT_APPNAME_MAX_SIZE+1];

};

extern struct SDLData SDLDATA;

extern void PrintSDLError();
extern int InitSDL();
extern void BeginSDLFrame();
extern void EndSDLFrame();
extern void USetWindowTitle(const char* text);
void InitSDLAudio(int samplerate);

SDL_Thread *CreateSDLBackgroundThread(const char *name);

key_event_t GetKeyboardInputEvent();

// sdlut.h