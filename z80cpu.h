
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <Z80.h>


#ifdef __cplusplus
extern "C" {
#endif


extern Z80 Z80CPU;

void z80cpu_init(void *ctx);
uint32_t z80cpu_step(uint32_t tstates);
void z80cpu_power(bool state);
void z80cpu_reset();

#ifdef __cplusplus
}
#endif

// z80cpu.h