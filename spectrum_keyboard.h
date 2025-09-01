#pragma once

/**----------------------------------------------------------------------------
 *	spectrum_keyboard.c
 *  interfacing the host keyboard support to the emulated ULA
 **/


#include <stdbool.h>

// KEY MODS (matching SDL)
#define KEY_MOD_NONE 	0
#define KEY_MOD_LSHIFT 	1
#define KEY_MOD_RSHIFT 	2
#define KEY_MOD_LCTRL 	64
#define KEY_MOD_RCTRL 	128


#define SPECTRUM_KEY_SHIFT_NONE   	0
#define SPECTRUM_KEY_SYMBOL_SHIFT   1
#define SPECTRUM_KEY_CAPS_SHIFT     2


struct _key_repl {
	uint8_t src_sym;
	uint8_t src_mod;
	uint8_t rep_sym;
	uint8_t shift;		// which shift key to inject
};

typedef struct {
	struct _key_repl primary;
	struct _key_repl secondary;
} key_repl_t;

#ifdef __cplusplus
extern "C" {
#endif

void spectrum_process_key(uint8_t hid_key, uint8_t key_mod, bool b_keydown);
void init_spectrum_keyboard();

#ifdef __cplusplus
}
#endif

// spectrum_keyboard.h