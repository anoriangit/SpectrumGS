
#include <stdint.h>

#include "spectrum_ula.h"
#include "spectrum_keyboard.h"
#include "spectrum_text_overlay.h"


// The ULA handles, among other things, the keyboard
// we store keys in 8 rows of 5 key entries matching the original 48k keyboard
// using the same weird logic of 0 meaning key pressed and 1 meaning not pressed
// The ordering follows the (weird) original logic too: bottom left 5 keys (left to right)
// are row 0, from here we move up for the next three rows and then walk down the right side 
// keys (right to left order now, of course! :D) 
// 
// 0: CAPS - V
// 1: A - G
// 2: Q - T
// ...
// 6: ENTER - H
// 7: SPACE - B
// 
void zx_ULA_key_down(zx_ula_t *ula, uint8_t keysym) {

    //sto_printf("keydown:%c ", keysym);

    switch (keysym) {

    // keyboard left side
    case SPECTRUM_KEY_CAPS_SHIFT: ula->key_matrix[0] &= ~1; break;      // caps-shift
    case 'z': ula->key_matrix[0] &= ~2; break;
    case 'x': ula->key_matrix[0] &= ~4; break;
    case 'c': ula->key_matrix[0] &= ~8; break;
    case 'v': ula->key_matrix[0] &= ~16; break;

    case 'a': ula->key_matrix[1] &= ~1; break;
    case 's': ula->key_matrix[1] &= ~2; break;
    case 'd': ula->key_matrix[1] &= ~4; break;
    case 'f': ula->key_matrix[1] &= ~8; break;
    case 'g': ula->key_matrix[1] &= ~16; break;

    case 'q': ula->key_matrix[2] &= ~1; break;
    case 'w': ula->key_matrix[2] &= ~2; break;
    case 'e': ula->key_matrix[2] &= ~4; break;
    case 'r': ula->key_matrix[2] &= ~8; break;
    case 't': ula->key_matrix[2] &= ~16; break;

    case '1': ula->key_matrix[3] &= ~1; break;
    case '2': ula->key_matrix[3] &= ~2; break;
    case '3': ula->key_matrix[3] &= ~4; break;
    case '4': ula->key_matrix[3] &= ~8; break;
    case '5': ula->key_matrix[3] &= ~16; break;

    // keyboard right side
    case '0': ula->key_matrix[4] &= ~1; break;
    case '9': ula->key_matrix[4] &= ~2; break;
    case '8': ula->key_matrix[4] &= ~4; break;
    case '7': ula->key_matrix[4] &= ~8; break;
    case '6': ula->key_matrix[4] &= ~16; break;

    case 'p': ula->key_matrix[5] &= ~1; break;
    case 'o': ula->key_matrix[5] &= ~2; break;
    case 'i': ula->key_matrix[5] &= ~4; break;
    case 'u': ula->key_matrix[5] &= ~8; break;
    case 'y': ula->key_matrix[5] &= ~16; break;

    case 13:  ula->key_matrix[6] &= ~1; break;      // "ENTER"
    case 'l': ula->key_matrix[6] &= ~2; break;
    case 'k': ula->key_matrix[6] &= ~4; break;
    case 'j': ula->key_matrix[6] &= ~8; break;
    case 'h': ula->key_matrix[6] &= ~16; break;

    case ' ': ula->key_matrix[7] &= ~1; break;
    case SPECTRUM_KEY_SYMBOL_SHIFT: ula->key_matrix[7] &= ~2; break;  // symbol-shift
    case 'm': ula->key_matrix[7] &= ~4; break;
    case 'n': ula->key_matrix[7] &= ~8; break;
    case 'b': ula->key_matrix[7] &= ~16; break;
    }

}


void zx_ULA_key_up(zx_ula_t *ula, uint8_t keysym) {

    //sto_printf("keyup:%c ", keysym);

    switch (keysym) {

    // keyboard left side
    case SPECTRUM_KEY_CAPS_SHIFT: ula->key_matrix[0] |= 1; break;
    case 'z': ula->key_matrix[0] |= 2; break;
    case 'x': ula->key_matrix[0] |= 4; break;
    case 'c': ula->key_matrix[0] |= 8; break;
    case 'v': ula->key_matrix[0] |= 16; break;

    case 'a': ula->key_matrix[1] |= 1; break;
    case 's': ula->key_matrix[1] |= 2; break;
    case 'd': ula->key_matrix[1] |= 4; break;
    case 'f': ula->key_matrix[1] |= 8; break;
    case 'g': ula->key_matrix[1] |= 16; break;

    case 'q': ula->key_matrix[2] |= 1; break;
    case 'w': ula->key_matrix[2] |= 2; break;
    case 'e': ula->key_matrix[2] |= 4; break;
    case 'r': ula->key_matrix[2] |= 8; break;
    case 't': ula->key_matrix[2] |= 16; break;

    case '1': ula->key_matrix[3] |= 1; break;
    case '2': ula->key_matrix[3] |= 2; break;
    case '3': ula->key_matrix[3] |= 4; break;
    case '4': ula->key_matrix[3] |= 8; break;
    case '5': ula->key_matrix[3] |= 16; break;

    // keyboard right side
    case '0': ula->key_matrix[4] |= 1; break;
    case '9': ula->key_matrix[4] |= 2; break;
    case '8': ula->key_matrix[4] |= 4; break;
    case '7': ula->key_matrix[4] |= 8; break;
    case '6': ula->key_matrix[4] |= 16; break;

    case 'p': ula->key_matrix[5] |= 1; break;
    case 'o': ula->key_matrix[5] |= 2; break;
    case 'i': ula->key_matrix[5] |= 4; break;
    case 'u': ula->key_matrix[5] |= 8; break;
    case 'y': ula->key_matrix[5] |= 16; break;

    case 13:  ula->key_matrix[6] |= 1; break;
    case 'l': ula->key_matrix[6] |= 2; break;
    case 'k': ula->key_matrix[6] |= 4; break;
    case 'j': ula->key_matrix[6] |= 8; break;
    case 'h': ula->key_matrix[6] |= 16; break;

    case ' ': ula->key_matrix[7] |= 1; break;
    case SPECTRUM_KEY_SYMBOL_SHIFT: ula->key_matrix[7] |= 2; break;
    case 'm': ula->key_matrix[7] |= 4; break;
    case 'n': ula->key_matrix[7] |= 8; break;
    case 'b': ula->key_matrix[7] |= 16; break;
    }

}

// this uses the row encoding as employed by the ROM when polling the keyboard
// FIXME: this might need further review (so far it seems to work correctly)
uint8_t zx_ULA_get_key_row(zx_ula_t *ula, uint8_t row) {

    //sto_printf("row:0x%x ",row);

#if 1
    uint8_t data = 0xff;
    if ((row & 0x01) == 0x01) data &= ula->key_matrix[0];    // caps - V
    if ((row & 0x02) == 0x02) data &= ula->key_matrix[1];    // A - G
    if ((row & 0x04) == 0x04) data &= ula->key_matrix[2];    // Q - T
    if ((row & 0x08) == 0x08) data &= ula->key_matrix[3];    // 1 - 5
    if ((row & 0x10) == 0x10) data &= ula->key_matrix[4];    // 0 - 6
    if ((row & 0x20) == 0x20) data &= ula->key_matrix[5];    // P - Y
    if ((row & 0x40) == 0x40) data &= ula->key_matrix[6];    // return - G
    if ((row & 0x80) == 0x80) data &= ula->key_matrix[7];    // space - B

    //if(data != 255) sto_printf("kdata=$%02x ", data);
    return data;
#else
    
    uint8 keys = 0xff;  // 0xbf 1011 1111 

    switch (row) {
    case 0x01: keys = ula->key_matrix[0]; break;    // caps - V
    case 0x02: keys = ula->key_matrix[1]; break;    // A - G
    case 0x04: keys = ula->key_matrix[2]; break;    // Q - T
    case 0x08: keys = ula->key_matrix[3]; break;    // 1 - 5
    case 0x10: keys = ula->key_matrix[4]; break;    // 0 - 6
    case 0x20: keys = ula->key_matrix[5]; break;    // P - Y
    case 0x40: keys = ula->key_matrix[6]; break;    // return - G
    case 0x80: keys = ula->key_matrix[7]; break;    // space - B
    }
    return keys;    

#endif

}


// spectrum_ula.c