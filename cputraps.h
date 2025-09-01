#pragma once

#include <stdint.h>


typedef struct {
    uint16_t trap_addr;
    void (*trap_func)(Z80 *z80);
    uint8_t rom_no;
} cpu_trap_t;


void _trap_SA_SPACE(Z80 *z80);
void _trap_LD_BYTES(Z80 *z80);
void _trap_SA_BYTES(Z80 *z80);


// cputraps.h