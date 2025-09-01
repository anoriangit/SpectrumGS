/*----------------------------------------------------------------------------
 * spectrum_text_overlay - STO
 *
 * provides text output to the spectrum display ram
 * useful for debugging purposes etc.
 * 
 */

 
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "spectrum.h"
//#include "nanoprintf.h"

#define IMPLEMENT_BESCII
#include "besciifont.h"


// standard dimensions: leave bottom row alone
#define STO_NUM_ROWS    23
#define STO_NUM_COLS    32

// cursor
static int CUR_ROW = 0;
static int CUR_COL = 0;

#define D_FONT_FIRST_ASCII  32
#define D_FONT_HEIGHT       8


// render the glyph for the ascii code (c) at CUR_ROW,CUR_COL
// into the spectrum display file
static bool _render_character(int c) { 

    if(c < D_FONT_FIRST_ASCII || c > 127)
        return false;

    int scanline_y = CUR_ROW * D_FONT_HEIGHT;
    int offset_x = CUR_COL;

    uint8_t *p_glyph = &bescii[(c - D_FONT_FIRST_ASCII)*8];
    uint8_t mask;
    uint8_t out_byte;

    for(int row = 0; row < 8; row++) {
        mask = 128;
        out_byte = 0;
        uint8_t src_pixels = *p_glyph++;
        for(int pixel = 7; pixel >= 0; pixel--) {
            if(src_pixels & mask) {
                out_byte |= mask;
            } else {
                out_byte &= ~mask;
            } 
            mask = mask >> 1;
        }
        uint8_t *dp = ZXSPECTRUM.linep[scanline_y+row] + offset_x;
        *dp = out_byte;
    }
    return true;
}

// std. scroll: everything from char row 23 upwards
static void _scroll_up() {
    int lines = STO_NUM_ROWS * 8;
    for(int line = 8; line < lines; line++) {
        memcpy(ZXSPECTRUM.linep[line-8], ZXSPECTRUM.linep[line], 32);
    }
    // clear bottom lines
    int start = (STO_NUM_ROWS-1) * 8;
    for(int line = start; line < start+8; line++) {
        memset(ZXSPECTRUM.linep[line], 0, 32);
    }

}

static void _line_feed() {
	CUR_ROW++;
	if (CUR_ROW == STO_NUM_ROWS) {
		_scroll_up();
		CUR_ROW--;
	}
}

static void _carriage_return() {
	CUR_COL = 0;
}

void sto_putchar(int c) {
    if(c == 13 || c == 10) {
        _carriage_return();
        _line_feed();
    } else 
    if(_render_character(c)) {
        CUR_COL++;
        if(CUR_COL == STO_NUM_COLS) {
            _carriage_return();
            _line_feed();
        }
    }
}

void sto_puts(char *string) {
    int c = 0;
    while((c = *string++))
        sto_putchar(c);
}

#define BUFFER_SIZE 512
static char buffer[BUFFER_SIZE];

int sto_printf(const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	int err = vsnprintf(buffer, BUFFER_SIZE, format, argp);
	va_end(argp);
	sto_puts(buffer);
	return err;
}
// spectrum_text_overlay.c

