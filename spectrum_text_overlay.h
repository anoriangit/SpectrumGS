#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void sto_putchar(int c);
void sto_puts(char *string);
int sto_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

// spectrum_text_overlay.h