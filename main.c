

#include "spectrum.h"
#include "text_box_l.h"
#include "sdlut.h"
#include "sdlevent.h"

int main(int argc, char *argv[]) {

	// SDL setup and init: using trusty old sdlut.c
	SDLDATA.v_width = FRAME_WIDTH;		// virtual dimensions as required by the application
	SDLDATA.v_height = FRAME_HEIGHT;		
	SDLDATA.width = FRAME_WIDTH*2;		// actual display dimensions: the display will be scaled up to this size
	SDLDATA.height = FRAME_HEIGHT*2;	
	SDLDATA.v_depth = 32;		
	InitSDL();

	// always do these after calling InitSDL()
	SDLDATA.event_cb = sdl_event_callback;
	USetWindowTitle(SPECTRUM_EMU_VERSION_STRING);

	// initialize the spectrum emulation
	init_spectrum();
	init_spectrum_keyboard();
	ltb_init();
	spectrum_power(1);

	ltb_printf( "%s - %s\n", SPECTRUM_EMU_VERSION_STRING, __DATE__);

	// Drive the display and the emulator
	uint32_t LINEBUF[FRAME_WIDTH];
	uint32_t* screen = SDLDATA.display_surface->pixels;

	while (SDLDATA.runloop) {
		BeginSDLFrame();

		for(int scanline = 0; scanline < DISPLAY_HEIGHT; scanline++) {

			// drive the zx spectrum display: render a single scanline
			render_spectrum_scanline(scanline, LINEBUF); 

			// scanline doubler
			int frame_scanline = scanline * 2;
			memcpy(screen+FRAME_WIDTH*frame_scanline, (void*)LINEBUF, sizeof(LINEBUF));
			memcpy(screen+FRAME_WIDTH*(frame_scanline+1), (void*)LINEBUF, sizeof(LINEBUF));

			// drive the z80 cpu
			// currently this happens at scanline granularity
			if(scanline == 0) {	
				// scanline just flipped back over to 0: frame complete
				// ==> we need to vblank
				z80_int(ZXSPECTRUM.cpu, 1);
				z80cpu_step(32);
				z80_int(ZXSPECTRUM.cpu, 0);
				z80cpu_step(SPECTRUM_SCANLINE_TSTATES - 32);
			}
			else {
				z80cpu_step(SPECTRUM_SCANLINE_TSTATES);
			}
		}

		ltb_render_overlay(screen);

		EndSDLFrame();
	}

	// unreachable 
	return 0;
}

// main.c