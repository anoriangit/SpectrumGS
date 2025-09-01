/*----------------------------------------------------------------------------
 * text_box_l.c: lower text box (ltb)
 * a small 80x6 text box that can be overlayed over the lower border 
 * of the zx spectrum display, useful for debug output etc.
 *
 * Note that all rendering here implies a flat rgb565 framebuffer
 */

 
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "spectrum.h"
#include "text_box_l.h"
#include "besciifont.h"


// for besciifont
#define D_FONT_FIRST_ASCII  32
#define D_FONT_HEIGHT       8
#define D_FONT_WIDTH        8

// standard dimensions: leave bottom row alone
#define LTB_NUM_ROWS    6
#define LTB_NUM_COLS    80
#define LTB_DISPLAY_BASE_ROW    (FRAME_HEIGHT - LTB_NUM_ROWS * D_FONT_HEIGHT)

// cursor
static int CUR_ROW = 0;
static int CUR_COL = 0;


static char CHARBUF[LTB_NUM_COLS*LTB_NUM_ROWS];
static uint8_t b_enabled = 0;



static void _scroll_up() {
	char *dp;
	char *sp = NULL;
	
	//uint8_t *asp = NULL;

	// Note that this implementation relies on the width specifications matching 
	// the buffer width in bytes (i.e one character code stored being one byte too)
	for (int row = 1; row < LTB_NUM_ROWS; row++) {
		dp = &CHARBUF[(row - 1) * LTB_NUM_COLS];
		sp = &CHARBUF[row * LTB_NUM_COLS];
		memcpy(dp, sp, LTB_NUM_COLS);

		// scroll attributes too
		//uint8_t *adp = &attrbuf[(row - 1) * D_CHAR_COLS];
		//asp = &attrbuf[row * D_CHAR_COLS];
		//memcpy(adp, asp, D_CHAR_COLS);
	}
	// clear bottom line
	memset(sp, 32, LTB_NUM_COLS);
	//memset(asp, 0, D_CHAR_COLS);

}

static void _line_feed() {
	CUR_ROW++;
	if (CUR_ROW == LTB_NUM_ROWS) {
		_scroll_up();
		CUR_ROW--;
	}
}

static void _carriage_return() {
	CUR_COL = 0;
}

static int _write_char(int c) {
    
    if(c < D_FONT_FIRST_ASCII || c > 127) {
        return 0;
    }

	char *CPTR = CHARBUF + LTB_NUM_COLS * CUR_ROW + CUR_COL;
	*CPTR = (uint8_t)c;
    return 1;
}


void ltb_putchar(int c) {
    // currently we always imply a lf on cr and vice versa
    if(c == 13 || c == 10) {
        _carriage_return();
        _line_feed();
    } else 
    if(_write_char(c)) {
        CUR_COL++;
        if(CUR_COL == LTB_NUM_COLS) {
            _carriage_return();
            _line_feed();
        }
    }
}

void ltb_puts(char *string) {
    int c = 0;
    while((c = *string++))
        ltb_putchar(c);
}

#define BUFFER_SIZE 1024
static char buffer[BUFFER_SIZE];

int ltb_printf(const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	int err = vsnprintf(buffer, BUFFER_SIZE, format, argp);
	va_end(argp);
	ltb_puts(buffer);
	return err;
}


/** --------------------------------------------------------------------------- 
 *  RENDERING
 **/

// render the glyph for the ascii code c into the framebuffer
// note that this does no bounds checking on c, row or col
static void _render_character(int c, int row, int col, uint32_t *framebuffer) { 

    int base_offset = FRAME_WIDTH * LTB_DISPLAY_BASE_ROW;
    uint32_t *base_pointer = framebuffer + base_offset;

    int scanline = row * D_FONT_HEIGHT;
    int offset_x = col * D_FONT_WIDTH;
    uint32_t *dp = base_pointer + scanline * FRAME_WIDTH + offset_x;

    uint8_t *p_glyph = &bescii[(c - D_FONT_FIRST_ASCII)*8];
    uint8_t mask;

    for(int g_row = 0; g_row < D_FONT_HEIGHT; g_row++) {
        mask = 128;
        uint8_t src_pixels = *p_glyph++;
        for(int pixel = 7; pixel >= 0; pixel--) {
            if(src_pixels & mask) {
                *dp++ = 0xffffffff;
            } else {
                *dp++ = 0x00000000;
            }
            mask = mask >> 1;
        }
        dp += (FRAME_WIDTH-D_FONT_WIDTH);  // move down one scanline
    }
}

void ltb_render_overlay(uint32_t *framebuffer) {

    if(!b_enabled)
        return;

    for(int row = 0; row < LTB_NUM_ROWS; row++) {
        for(int col = 0; col < LTB_NUM_COLS; col++) {
            int c = CHARBUF[row*LTB_NUM_COLS+col];
            if(c >= D_FONT_FIRST_ASCII && c < 128) {
                _render_character(c, row, col, framebuffer);
            }
        }
    }
}

void ltb_toggle_overlay() {
    b_enabled = !b_enabled;
}

void ltb_init() {
    memset(CHARBUF, 32, sizeof(CHARBUF));
}

// text_box_l.c

