#pragma once

// these are rgb332 colors matching the original 
// spectrum colors as closely as possible
uint8_t _palette[16] = {
    0x00, // Black (toned down)
    0x02, // Blue (toned down)
    0x80, // Red (toned down)
    0x82, // Magenta (toned down)
    0x10, // Green (toned down)
    0x12, // Cyan (toned down)
    0x90, // Yellow (toned down)
    0x92, // White (toned down)
    0x00, // Bright Black
    0x03, // Bright Blue
    0xE0, // Bright Red
    0xE3, // Bright Magenta
    0x1C, // Bright Green
    0x1F, // Bright Cyan
    0xFC, // Bright Yellow
    0xFF  // Bright White
};

uint16_t _palette_16[16] = {
    0x0000, // Black (toned down): R=0x0, G=0x0, B=0x0 -> (0 << 11) | (0 << 5) | 0 = 0x0000
    0x0018, // Blue (toned down): R=0x0, G=0x0, B=0x2 -> (0 << 11) | (0 << 5) | (2 << 3) = 0x0018
    0xA000, // Red (toned down): R=0x4, G=0x0, B=0x0 -> (4 << 11) | (0 << 5) | 0 = 0x8000
    0xA018, // Magenta (toned down): R=0x4, G=0x0, B=0x2 -> (4 << 11) | (0 << 5) | (2 << 3) = 0x8018
    0x0600, // Green (toned down): R=0x0, G=0x2, B=0x0 -> (0 << 11) | (2 << 5) | 0 = 0x0040
    0x0618, // Cyan (toned down): R=0x0, G=0x2, B=0x2 -> (0 << 11) | (2 << 5) | (2 << 3) = 0x0058
    0xA600, // Yellow (toned down): R=0x4, G=0x2, B=0x0 -> (4 << 11) | (2 << 5) | 0 = 0x8040
    0xA618, // White (toned down): R=0x4, G=0x2, B=0x2 -> (4 << 11) | (2 << 5) | (2 << 3) = 0x8058
    0x0000, // Bright Black: R=0x0, G=0x0, B=0x0 -> (0 << 11) | (0 << 5) | 0 = 0x0000
    0x001F, // Bright Blue: R=0x0, G=0x0, B=0x3 -> (0 << 11) | (0 << 5) | (3 << 3) = 0x0018
    0xF800, // Bright Red: R=0x7, G=0x0, B=0x0 -> (7 << 11) | (0 << 5) | 0 = 0xE000
    0xF81F, // Bright Magenta: R=0x7, G=0x0, B=0x3 -> (7 << 11) | (0 << 5) | (3 << 3) = 0xE018
    0x0FC0, // Bright Green: R=0x0, G=0x7, B=0x0 -> (0 << 11) | (7 << 5) | 0 = 0x01C0
    0x0FDF, // Bright Cyan: R=0x0, G=0x7, B=0x3 -> (0 << 11) | (7 << 5) | (3 << 3) = 0x01D8
    0xFFC0, // Bright Yellow: R=0x7, G=0x7, B=0x0 -> (7 << 11) | (7 << 5) | 0 = 0xE1C0
    0xFFFF  // Bright White: R=0x7, G=0x7, B=0x3 -> (7 << 11) | (7 << 5) | (3 << 3) = 0xE1D8
};


uint16_t _palette_rgb565[16] = {
	0x0000,  // black
	0x0010,  // blue
	0x8000,  // red
	0x8010,  // magenta
	0x0400,  // green
	0x0410,  // cyan
	0x8400,  // yellow
	0x8410,  // white
	0x0000,  // black
	0x001F,  // bright blue
	0xF800,  // bright red
	0xF81F,  // bright magenta
	0x07E0,  // bright green
	0x07FF,  // bright cyan
	0xFFE0,  // bright yellow
	0xFFFF   // bright white
};

uint32_t _palette_argb32[16] = {
    0xFF000000, // black
    0xFF000080, // blue
    0xFF800000, // red
    0xFF800080, // magenta
    0xFF008000, // green
    0xFF008080, // cyan
    0xFF808000, // yellow
    0xFF808080, // white
    0xFF000000, // black
    0xFF0000FF, // bright blue
    0xFFFF0000, // bright red
    0xFFFF00FF, // bright magenta
    0xFF00FF00, // bright green
    0xFF00FFFF, // bright cyan
    0xFFFFFF00, // bright yellow
    0xFFFFFFFF  // bright white
};

uint32_t _palette_abgr32[16] = {
    0xFF000000, // black
    0xFF800000, // blue
    0xFF000080, // red
    0xFF800080, // magenta
    0xFF008000, // green
    0xFF808000, // cyan
    0xFF008080, // yellow
    0xFF808080, // white
    0xFF000000, // black
    0xFFFF0000, // bright blue
    0xFF0000FF, // bright red
    0xFFFF00FF, // bright magenta
    0xFF00FF00, // bright green
    0xFFFFFF00, // bright cyan
    0xFF00FFFF, // bright yellow
    0xFFFFFFFF  // bright white
};

// spectrum_palettes.h