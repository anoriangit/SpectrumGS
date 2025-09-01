
#include <stdbool.h>
#include "sdlut.h"
#include "spectrum.h"
#include "text_box_l.h"


void sdl_event_callback(SDL_Event e) {

    if (e.type == SDL_KEYDOWN) {
        if(e.key.keysym.scancode == SDL_SCANCODE_F12) {
            ltb_toggle_overlay();
        } else {
            spectrum_process_key(e.key.keysym.sym, e.key.keysym.mod, true);
        }
    } else if (e.type == SDL_KEYUP) {
        spectrum_process_key(e.key.keysym.sym, e.key.keysym.mod, false);
    }

}

// sdlevent.c