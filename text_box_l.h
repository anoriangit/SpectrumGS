#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void ltb_init();
void ltb_toggle_overlay();
void ltb_render_overlay(uint32_t *framebuffer);

void ltb_putchar(int c);
void ltb_puts(char *string);
int ltb_printf(const char *format, ...);


#ifdef __cplusplus
}
#endif