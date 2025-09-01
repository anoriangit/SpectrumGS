#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "spectrum.h"

enum MEM_MAPPING_TYPE { M_READ_WRITE, M_READ_ONLY};

// memory
#define MEM_BANK_SIZE   16384
#define MEM_PAGE_SIZE   (MEM_BANK_SIZE/2)
#define MEM_NUM_BANKS   64                  // 64 banks = 1024k (more is not possible without changing bios banking)
#define MEM_NUM_PAGES   (MEM_NUM_BANKS*2)

// Note: when changing this scheme (adding more rom banks for example)
// make sure that ROM_0_BANK remains the first non ram bank
#define ROM_3_BANK (MEM_NUM_BANKS-1)	// the interface 1 rom
#define ROM_2_BANK (MEM_NUM_BANKS-2)	// the original 48k rom
#define ROM_1_BANK (MEM_NUM_BANKS-3)	// 128k rom1 (basically a 48k rom)
#define ROM_0_BANK (MEM_NUM_BANKS-4)	// 128k rom0

#define RAM_0_BANK 0
#define RAM_1_BANK 1
#define RAM_2_BANK 2
#define RAM_3_BANK 3
#define RAM_4_BANK 4
#define RAM_5_BANK 5
#define RAM_6_BANK 6
#define RAM_7_BANK 7

#define ULAX_0_BANK 8	// extended ULA: bank 1 for display mode 2
#define ULAX_1_BANK 9	// extended ULA: bank 2 for display mode 2
#define ULAX_2_BANK 10	// extended ULA: bank 3 for display mode 2


typedef struct {
	int index;
	enum MEM_MAPPING_TYPE mapping_type;
} z80_mmu_mapping_t;

// we manage memory in banks of 16k and pages of 8k
// the 16k bank is mostly required for spectrum 128k compatibility
typedef struct {

	// overall memory pool
	uint8_t memory[MEM_BANK_SIZE*MEM_NUM_BANKS];	// raw memory

	uint8_t *banks[MEM_NUM_BANKS];	// tables of bank and page physical start adresses
	uint8_t *pages[MEM_NUM_PAGES];

	// memory actually "visible" to the Z80 (mapped from the overall pool)
	z80_mmu_mapping_t visible_banks[4];
	z80_mmu_mapping_t visible_pages[8];

	int current_rom;
	bool enable_128k_banking;

} z80_mmu_t;


void z80_mmu_Init(z80_mmu_t *mmu, zx_type_t system_type);
void z80_mmu_Reset(z80_mmu_t *mmu, zx_type_t system_type);

#if 0
// convert a virtual address into an actual physical address in the host machines memory address space
void* z80_mmu_GetPhysicalAddress(z80_mmu_t *mmu, uint16_t virtual_address);
// these read & write from virtual addresses inside the emulated Z80's 64k address space
void z80_mmu_PutByte(z80_mmu_t *mmu, uint8_t byte, uint16_t virtual_address);
uint8_t z80_mmu_GetByte(z80_mmu_t *mmu, uint16_t virtual_address);
uint16_t z80_mmu_GetWord(z80_mmu_t *mmu, uint16_t virtual_address);
void z80_mm_PutWord(z80_mmu_t *mmu, uint16_t word, uint16_t virtual_address);
#endif


// ----------------------------------------------------------------------------
// 
// Virtual Memory access functions
// any adress used by the emulated Z80 is a virtual address from 0-ffff
// but due to the ability of the mmu to change mappings of memory banks/pages 
// we need to translate each virtual address va into a physical address prior to access

static inline void *z80_mmu_GetPhysicalAddress(z80_mmu_t *mmu, uint16_t virtual_address) {
	
	// determine visible (bank) slot and offset from va
	int bank = virtual_address / MEM_BANK_SIZE;
	int offset = virtual_address % MEM_BANK_SIZE;

	uint8_t *base_address = mmu->banks[mmu->visible_banks[bank].index];
	return (void*)(base_address + offset);
}

static inline uint8_t z80_mmu_GetByte(z80_mmu_t *mmu, uint16_t virtual_address) {
	uint8_t *addr = (uint8_t*)z80_mmu_GetPhysicalAddress(mmu, virtual_address);
	return *addr;
}

static inline uint16_t z80_mmu_GetWord(z80_mmu_t *mmu, uint16_t virtual_address) {
	uint8_t *addr0 = (uint8_t *)z80_mmu_GetPhysicalAddress(mmu, virtual_address);
	uint8_t *addr1 = (uint8_t *)z80_mmu_GetPhysicalAddress(mmu, virtual_address+1);
	return (*addr0) + *(addr1) * 256;
}

// Copy a block of memory from emulation virtual memory space to host memory space
// WARNING: only ever retrieve blocks of memory that don't cross page boundaries,
// otherwise StrangerThings will very likely occur!
// NOTE: obviously this limits the largest retrievable block to MEM_PAGE_SIZE
// RETURN: dest_addr (god knows why)
/*
void *z80_mmu_GetMultiByte(z80_mmu_t *mmu, uint16_t virtual_address, pointer_t dest_addr, uint16_t size) {
	pointer_t src_addr = zx_MMU_GetPhysicalAddress(mmu, virtual_address);
	memcpy((void*)dest_addr, (void*)src_addr, (size_t) size);
	return dest_addr;
} */

static inline void z80_mmu_PutByte(z80_mmu_t *mmu, uint8_t byte, uint16_t virtual_address) {
	int bank = virtual_address / MEM_BANK_SIZE;

	// prevent writing to ROM (RAM under ROM not yet implemented)
	if (mmu->visible_banks[bank].mapping_type == M_READ_WRITE) {
		uint8_t *addr = (uint8_t*)z80_mmu_GetPhysicalAddress(mmu, virtual_address);
		*addr = byte;
	}
}

// as usual: low byte then high byte
static inline void z80_mmu_PutWord(z80_mmu_t *mmu, uint16_t word, uint16_t virtual_address) {
	int bank = virtual_address / MEM_BANK_SIZE;

	// prevent writing to ROM (RAM under ROM not yet implemented)
	if (mmu->visible_banks[bank].mapping_type == M_READ_WRITE) {
		uint8_t *addr = (uint8_t*)z80_mmu_GetPhysicalAddress(mmu, virtual_address);
		*addr = (word & 0x00ff);
		*(addr+1) = ((word & 0xff00)>>8);
	}
}


// NOTE: this one is fast but potentially dangerous (see source code comments)
//extern pointer_t zx_MMU_GetMultiByte(zx_mmu_t *mmu, uint16_t virtual_address, pointer_t dest_addr, uint16_t size);

// 128k style BANK mapping
//void _zx_MMU_update_memory_map_zx128(zx_mmu_t *mmu, uint8_t data);

// use with care
//extern int zx_MMU_MemMap(zx_mmu_t *mmu, int slot, int bank_no, enum MEM_MAPPING_TYPE mapping_type);

#ifdef __cplusplus
} // extern "C"
#endif

// z80mmu.h