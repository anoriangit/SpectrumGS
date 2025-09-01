#pragma once

typedef struct {

	// keyboard: 1 = key not pressed, 0 = key pressed
	// http://www.breakintoprogram.co.uk/hardware/computers/zx-spectrum/keyboard
	uint8_t key_matrix[8];		// keys encoded in bits 0-4

} zx_ula_t;

uint8_t zx_ULA_get_key_row(zx_ula_t *ula, uint8_t row);
void zx_ULA_key_up(zx_ula_t *ula, uint8_t keysym);
void zx_ULA_key_down(zx_ula_t *ula, uint8_t keysym);

// spectrum_ula.h