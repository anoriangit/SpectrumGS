#include <string.h>         // memset()
#include <stdio.h>


#include "spectrum.h"
#include "text_box_l.h"
#include "z80cpu.h"
#include "cputraps.h"


// original documentation:
// https://zxe.io/software/Z80/documentation/latest/


Z80 Z80CPU;


uint32_t tStatesPerMilliSecond();

/**----------------------------------------------------------------------------
 *	CPU traps definitions
 **/


static cpu_trap_t TRAPS[] = {
	{ 0x04c2, _trap_SA_BYTES, ROM_2_BANK },		// Spectrum 48k ROM tape save routine
	{ 0x0556, _trap_LD_BYTES, ROM_2_BANK },		// load header&data blocks
	// 0x0621 traps SA-SPACE: at that point the rom has moved the filename to the calc. stack
	{ 0x0621, _trap_SA_SPACE, ROM_2_BANK },		// save, verify, load, merge main entry point original 48k rom 

};

static int NUM_TRAPS = sizeof(TRAPS) / sizeof(cpu_trap_t);



uint32_t z80cpu_step(uint32_t tstates) {  

    const uint32_t k = z80_run(&Z80CPU, tstates);
    return k;
}
  
void z80cpu_power(bool state) {
    z80_power(&Z80CPU, true);
  }
  
void z80cpu_reset() {
    z80_instant_reset(&Z80CPU);
}

static  uint8_t _fetch_opcode(void *context, uint16_t address) {
    zx_spectrum_t *zx = (zx_spectrum_t*)context;

    for(int i = 0; i < NUM_TRAPS; i++) {
        if(TRAPS[i].trap_addr == address) {
            //ltb_printf("cpu trap hit!\n");
            TRAPS[i].trap_func(0);
        }
    }
    return z80_mmu_GetByte(&zx->mmu, address);
}

static  uint8_t _read_memory(void *context, uint16_t address) {
    zx_spectrum_t *zx = (zx_spectrum_t*)context;
    return z80_mmu_GetByte(&zx->mmu, address);
}

static void _write_memory(void *context, uint16_t address, uint8_t value) {
    zx_spectrum_t *zx = (zx_spectrum_t*)context;
    z80_mmu_PutByte(&zx->mmu, value, address);
}

static  uint8_t _read_port(void *context, uint16_t address) {
    zx_spectrum_t *zx = (zx_spectrum_t*)context;

    if(!(address & 1)) {
        //sto_printf("addr=$%04x ", address);
        // all even ports: ULA read
  		uint16_t row_mask = (~(address >> 8)) & 0x00FF;
		uint8_t data = zx_ULA_get_key_row(&ZXSPECTRUM.ula, (uint8_t)row_mask) & 0xBF;
		return data;
    }
    return 255;    
}

static void _write_port(void *context, uint16_t address, uint8_t value) {
    return;
 }

#if 0
static uint8_t _int_ack(void * context, uint16_t address) {
    z80_int(&Z80CPU, false);
    return 0xff;
}
#endif

void z80cpu_init(void *ctx) {

    memset(&Z80CPU, 0, sizeof(Z80));

	// initialize the processor module callbacks
	Z80CPU.options = Z80_MODEL_ZILOG_NMOS;

 	Z80CPU.fetch_opcode = _fetch_opcode;
    
 	Z80CPU.fetch =
    Z80CPU.nop =
    Z80CPU.read = _read_memory;
    Z80CPU.write = _write_memory;
  	
    Z80CPU.in = _read_port;
    Z80CPU.out = _write_port;
    
    Z80CPU.context = ctx;
    // Z80CPU.inta = _int_ack;
}

// z80cpu.c