/**----------------------------------------------------------------------------
 *	spectrum_keyboard.c
 *  interfacing the host keyboard support to the emulated ULA
 *  circle version
 **/


#include <stdint.h>
#include <string.h>

#include "spectrum.h"
#include "spectrum_keyboard.h"
#include "spectrum_text_overlay.h"
#include "text_box_l.h"

static key_repl_t key_replacements[128];


bool KEYMOD(uint8_t source_mod, uint16_t mod_mask) {
	if ((source_mod & mod_mask) != 0)
		return true;
	else if (mod_mask == 0 && source_mod == 0)
		return true;
	else
        return false;
}

static uint8_t b_modded_key[128] = {

	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
};

// need to re-encode certain circle keyboard character codes
static uint8_t circle_key_repl[128] = {

	0x20,0x00,0x08,0x00, 0x0d,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,		// 0 -15 
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,

};

// Spectrum symbol/caps-shift'ed keys as replacements 
// for pc keyboard keys that don't exist on the speccy
// example: simply pressing the '.' key on a pc keyboard
// needs to be translated into two keypresses for the spectrum
// simulating symbol-shift + M being pressed at the same time
static void _process_key_replacements(uint8_t *c, uint8_t key_mod, bool b_keydown) {

	if(*c >= 128) 
		return;

	struct _key_repl *rp = &key_replacements[*c].primary;
	struct _key_repl *rp2 = &key_replacements[*c].secondary;

	if (rp->src_sym == *c) {
		// there is a primary replacement defined for this key
		if (b_keydown) {
			// KEY DOWN
			if (KEYMOD(key_mod, rp->src_mod)) {
				zx_ULA_key_down(&ZXSPECTRUM.ula, rp->shift);	// inject appropriate shift key
				*c = rp->rep_sym;	                // and replace the original key
				b_modded_key[rp->src_sym] |= 1;		// mark the original as modded so we can properly process KEY UP for it
			}
		}
		else {
			// KEY UP
			if ((b_modded_key[rp->src_sym] & 1) == 1) {
				zx_ULA_key_up(&ZXSPECTRUM.ula, rp->shift);
				*c = rp->rep_sym;
				b_modded_key[rp->src_sym] &= ~1;
			}
		}
	} 
	// check secondary too
	if (rp2->src_sym == *c) {
		// there is a replacement defined for this key
		if (b_keydown) {
			// KEY DOWN
			if (KEYMOD(key_mod, rp2->src_mod)) {
				zx_ULA_key_down(&ZXSPECTRUM.ula, rp2->shift);
				*c = rp2->rep_sym;
				b_modded_key[rp2->src_sym] |= 2;			
			}
		}
		else {
			// KEY UP
			if ((b_modded_key[rp2->src_sym] & 2) == 2) {
				zx_ULA_key_up(&ZXSPECTRUM.ula, rp->shift);
				*c = rp2->rep_sym;
				b_modded_key[rp2->src_sym] &= ~2;
			}
		}
	}
}

void spectrum_process_key(uint8_t c, uint8_t key_mod, bool b_keydown) {

	if(c >= 128) 
		return;

	//if(circle_key_repl[c])
	//	c = circle_key_repl[c];

	uint8_t c_original = c;

    // will potentially modify c to a different character and also
    // inject additional ULA key presses as required (symbol-shift etc.)
    _process_key_replacements(&c, key_mod, b_keydown);

	// debug

    if(b_keydown) {
		ltb_printf("\nD code:%u mod:%u repl:%u ('%c')", c_original, key_mod, c, (c != 13) ? c : 0);
        zx_ULA_key_down(&ZXSPECTRUM.ula, c);
    } else {
		ltb_printf("\nU code:%u mod:%u repl:%u ('%c')", c_original, key_mod, c, (c != 13) ? c : 0);
        zx_ULA_key_up(&ZXSPECTRUM.ula, c);
    }

}

// define a key replacement for code i (with mod m) to be replaced with r and the spectrum shift key to inject
static void _define_key_replacement(uint8_t i, uint8_t m, uint8_t r, uint8_t shift_inject) {
	
	key_replacements[i].primary.src_sym = i;
	key_replacements[i].primary.src_mod = m;
	key_replacements[i].primary.rep_sym = r;
	key_replacements[i].primary.shift = shift_inject;

}

void init_spectrum_keyboard() {

	// In order to enable input of zx spectrum symbol/caps-shifted key presses
	// using their PC equivalents we define a set of key replacements here
	// these are processed in ProcessKeyReplacements() 

	memset(key_replacements, 255, 128 * sizeof(key_repl_t));

	_define_key_replacement(8,  KEY_MOD_NONE, '0', SPECTRUM_KEY_CAPS_SHIFT);					// DELETE <- BACKSPACE
	_define_key_replacement(50, KEY_MOD_LSHIFT|KEY_MOD_RSHIFT, 'p', SPECTRUM_KEY_SYMBOL_SHIFT); // "
	_define_key_replacement(43, KEY_MOD_LSHIFT|KEY_MOD_RSHIFT, 'b', SPECTRUM_KEY_SYMBOL_SHIFT); // *
	_define_key_replacement(46, KEY_MOD_LSHIFT|KEY_MOD_RSHIFT, 'z', SPECTRUM_KEY_SYMBOL_SHIFT); // :
	_define_key_replacement(44, KEY_MOD_LSHIFT|KEY_MOD_RSHIFT, 'o', SPECTRUM_KEY_SYMBOL_SHIFT); // ;
	_define_key_replacement(46, KEY_MOD_NONE, 'm', SPECTRUM_KEY_SYMBOL_SHIFT); 					// .

#if 0

	// " (key code 52 is ' on a us keyboard)
	key_replacements[52].primary.src_sym = 52;
	key_replacements[52].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements[52].primary.rep_sym = 'p';
	// .
	key_replacements['.'].primary.src_sym = '.';
	key_replacements['.'].primary.src_mod = KEY_MOD_NONE;
	key_replacements['.'].primary.rep_sym = 'm';
	// :
	key_replacements['.'].secondary.src_sym = '.';
	key_replacements['.'].secondary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['.'].secondary.rep_sym = 'z';
	// ,
	key_replacements[','].primary.src_sym = ',';
	key_replacements[','].primary.src_mod = KEY_MOD_NONE;
	key_replacements[','].primary.rep_sym = 'n';
	// ;
	key_replacements[','].secondary.src_sym = ',';
	key_replacements[','].secondary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements[','].secondary.rep_sym = 'o';
	// =
	key_replacements['0'].primary.src_sym = '0';
	key_replacements['0'].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['0'].primary.rep_sym = 'l';
	// <
	key_replacements['<'].primary.src_sym = '<';
	key_replacements['<'].primary.src_mod = KEY_MOD_NONE;
	key_replacements['<'].primary.rep_sym = 'r';
	// >
	key_replacements['<'].secondary.src_sym = '<';
	key_replacements['<'].secondary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['<'].secondary.rep_sym = 't';
	// +
	key_replacements['+'].primary.src_sym = '+';
	key_replacements['+'].primary.src_mod = KEY_MOD_NONE;
	key_replacements['+'].primary.rep_sym = 'k';
	// -
	key_replacements['-'].primary.src_sym = '-';
	key_replacements['-'].primary.src_mod = KEY_MOD_NONE;
	key_replacements['-'].primary.rep_sym = 'j';
	// *
	key_replacements['+'].secondary.src_sym = '+';
	key_replacements['+'].secondary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['+'].secondary.rep_sym = 'b';
	// /
	key_replacements['7'].primary.src_sym = '7';
	key_replacements['7'].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['7'].primary.rep_sym = 'v';
	// (
	key_replacements['8'].primary.src_sym = '8';
	key_replacements['8'].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['8'].primary.rep_sym = '8';
	// )
	key_replacements['9'].primary.src_sym = '9';
	key_replacements['9'].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['9'].primary.rep_sym = '9';
	// @
	key_replacements['q'].primary.src_sym = 'q';
	key_replacements['q'].primary.src_mod = KEYBOARD_MODIFIER_LEFTALT;
	key_replacements['q'].primary.rep_sym = '2';
	// #
	key_replacements['#'].primary.src_sym = '#';
	key_replacements['#'].primary.src_mod = KEY_MOD_NONE;
	key_replacements['#'].primary.rep_sym = '3';
	// '
	key_replacements['#'].secondary.src_sym = '#';
	key_replacements['#'].secondary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['#'].secondary.rep_sym = '7';
	// $
	key_replacements['4'].primary.src_sym = '4';
	key_replacements['4'].primary.src_mod = KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT;
	key_replacements['4'].primary.rep_sym = '4';
#endif
};


// spectrum_keyboard.c