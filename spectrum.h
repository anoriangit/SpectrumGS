#pragma once

#include <stdint.h>


#include "z80cpu.h"

#include "spectrum_ula.h"
#include "spectrum_text_overlay.h"
#include "spectrum_keyboard.h"

#define SPECTRUM_EMU_VERSION_STRING "SpectrumGS 1.0.0"

#define SPECTRUM_SCANLINE_TSTATES   224

// zx spectrum mode (2) display dimensions
#define SCREENH 192
#define SCREENW 256

// output display dimensions
#define DISPLAY_WIDTH   320     // width of the virtual screen
#define DISPLAY_HEIGHT  240     // height of the virtual screen

// actual display resolution
#define FRAME_WIDTH     640     // width of the physical screen
#define FRAME_HEIGHT    480     // height of the physical screen

// ROM
#define SPECTRUM_ROM_SIZE   16384

// offset of attributes storage from start of display memory
#define SPECTRUM_DISPLAY_OFFSET     16384   // from start of memory
#define SPECTRUM_ATTRIBUTES_OFFSET  6144    // from start of display ram

// maximum length of file/directory paths for disk operations
#define SPECTRUM_MAX_FILE_DIR_LEN   256

// banked memory
// each of these banks can be mapped for write access at 0x0000
#define PICOLO_BANK_SIZE        16348       // 
#define PICOLO_BANK_COUNT       5           // 81920 (need min. 76800 to be able to access all display ram)
#define PICOLO_SCREEN_0_BANK    1

//extern uint8_t *BANKED_MEM_BANKS[PICOLO_BANK_COUNT];
//extern uint8_t spectrum_memory[];

#ifdef __cplusplus
extern "C" {
#endif

// ZX Spectrum models
typedef enum {
    ZX_TYPE_48K,
    ZX_TYPE_128K,
    ZX_TYPE_ZXX,
} zx_type_t;

#include "z80mmu.h"

typedef struct zx_spectrum {

    zx_type_t zx_type;
    Z80 *cpu;
    z80_mmu_t mmu;
    zx_ula_t ula;
    int power_state;

    uint8_t border;
    uint32_t spectrum_palette[16];
    int linep[SCREENH];

} zx_spectrum_t;

extern zx_spectrum_t ZXSPECTRUM;

void init_spectrum();
void spectrum_power(int on);
void render_spectrum_scanline(int scanline, uint32_t *LINEBUF);
uint8_t *spectrum_get_current_bank_ptr();
uint8_t *spectrum_get_screen_0_ptr();


#ifdef __cplusplus
}
#endif

// spectrum.h