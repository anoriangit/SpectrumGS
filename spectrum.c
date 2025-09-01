
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>		// memset()

#include "spectrum.h"
#include "spectrum_keyboard.h"
#include "spectrum_palettes.h"

#include "gw03.h"		// gosh wonderful rom

/**----------------------------------------------------------------------------
 *	DATA
 */

//uint dma_display;
//uint32_t dma_color = 0;

zx_spectrum_t ZXSPECTRUM;




/**----------------------------------------------------------------------------
 *	INITIALIZATION
 */

void init_spectrum() {

   /*
	* ZX Spectrum display memory layout:
	* - 3 blocks of 64 pixel lines (8 character rows) [0-63][64-127][128-191] each 2k in size
	* - inside each block lines are stored in an interleaved pattern
	*		- the first 32 bytes (256 pixels) are block line 0 
	*		- the next 32 bytes are block line 8
	*		- then block line 16 etc up to block line 56
	*		- now the pattern repeats with block line 1 until the block is filled up
	*		- to get from one pixel line to the next in memory we thus need to add 8*32=256
	* - display ram size is 3 * 2048 = 6144 which is immediately followed by 768 attributes bytes
	* - attributes are stored as a straight row major array 24 rows * 32 columns 
	*/

	// clear to all 0 first
	memset(&ZXSPECTRUM, 0, sizeof(ZXSPECTRUM));

	// init the mmu: for now we are always a 48k
	z80_mmu_Init(&ZXSPECTRUM.mmu, ZX_TYPE_48K);

	// init palette 
	memcpy(ZXSPECTRUM.spectrum_palette, _palette_argb32, 16 * sizeof(uint32_t));

	// build the source line pointers array: this is helpful for linearizing access to
	// the awfully confusing zx spectrum display memory segmented/interleaved structure

	uint8_t segment_mask = 0xc0;
	int segment, cell, line, offset;
	uint8_t y;

	for (y = 0; y < SCREENH; y++) {
		segment = (y & segment_mask) >> 6;
		cell = (y - segment * 64) / 8;
		line = (y - segment * 64) % 8;
		// segment offset is 2048, cell offset inside segments is 32 and lines within cells 256
		offset = segment * 2048 + cell * 32 + line * 256;
		ZXSPECTRUM.linep[y] = offset;
	}

	// copy rom for 48k mode (using the "gosh wonderful" rom)
	memcpy(ZXSPECTRUM.mmu.banks[ROM_2_BANK], gw03, SPECTRUM_ROM_SIZE);

	// init keyboard
	//init_spectrum_keyboard();

	// init ZXSPECTRUM object
	memset(ZXSPECTRUM.ula.key_matrix, 0xff, sizeof(ZXSPECTRUM.ula.key_matrix));
	ZXSPECTRUM.cpu = &Z80CPU;
	ZXSPECTRUM.power_state = 0;
	ZXSPECTRUM.border = 7;
	z80cpu_init(&ZXSPECTRUM);
}

void spectrum_power(int on) {
	if(on) {
		z80cpu_power(true);
		ZXSPECTRUM.power_state = 1;
	} else {
		z80cpu_power(false);
		ZXSPECTRUM.power_state = 0;
	}
}

/**----------------------------------------------------------------------------
 *	DISPLAY
 **/

static bool flash = 0;
static uint32_t spectrum_framecount = 0;

static void _memset_16(uint16_t *dest, uint16_t value, size_t len) {
	while(len--) {
		*dest++ = value;
	}
}

void render_spectrum_scanline(int scanline, uint32_t *LINEBUF) {

	if(ZXSPECTRUM.power_state == 0)
		return;

	//scanline = (scanline+1) % SCREENH;

	if(scanline == 0) {
		spectrum_framecount++;
		// at 60 Hz this is one full cycle (on off on) every 0.64s
		if(!(spectrum_framecount%19))		
			flash = !flash;
	}

	uint32_t *dp = LINEBUF;

	// hard coded display file always assumed to be in bank 5
	uint8_t *attributes = ZXSPECTRUM.mmu.banks[RAM_5_BANK] + SPECTRUM_ATTRIBUTES_OFFSET;

	uint8_t pixels = 0;
	uint8_t attrib = 0;
	uint8_t bright = 0;

	if(scanline < 24 || scanline >= 216) {
		// upper & lower border
		memset(dp, ZXSPECTRUM.spectrum_palette[ZXSPECTRUM.border], FRAME_WIDTH*sizeof(uint32_t));
	} else {

		int spectrum_scanline = scanline - 24;
		
		// hard coded display file always assumed to be in bank 5
		uint8_t *l_sp = ZXSPECTRUM.mmu.banks[RAM_5_BANK] + ZXSPECTRUM.linep[spectrum_scanline];
		int attrib_row_index = (spectrum_scanline >> 3) << 5;	// (scanline / 8) * 32

		// left border
		memset(dp, ZXSPECTRUM.spectrum_palette[ZXSPECTRUM.border], 64*sizeof(uint32_t));
		dp += 64;

		for (int x = 0; x < 32; x++) {
			pixels = *l_sp++;
			attrib = attributes[attrib_row_index + x];	// get attribute

			bright = (attrib & 0x40)>>3;		// bright bit shifted to provide a +8 offset
			uint32_t ink = ZXSPECTRUM.spectrum_palette[(attrib & 0x7) + bright];
			uint32_t paper = ZXSPECTRUM.spectrum_palette[((attrib & 0x38) >> 3) + bright];

			if((attrib&0b10000000) && flash) {
				uint16_t t = ink;
				ink = paper;
				paper = t;
			}

			// unrolled inner loop 
			// 0 
			if (pixels & 0b10000000) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 1
			if (pixels & 0b01000000) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 2 
			if (pixels & 0b00100000) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 3
			if (pixels & 0b00010000) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 4
			if (pixels & 0b00001000) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 5
			if (pixels & 0b00000100) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 6 
			if (pixels & 0b00000010) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
			// 7
			if (pixels & 0b00000001) {
				*dp++ = ink;
				*dp++ = ink;
			}
			else {
				*dp++ = paper;
				*dp++ = paper;
			}
		}

		// right border
		memset(dp, ZXSPECTRUM.spectrum_palette[ZXSPECTRUM.border], 64*sizeof(uint32_t));

	}
}


// spectrum.c