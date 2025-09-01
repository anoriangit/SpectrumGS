//
// zx_mmu.c
// memory management for the ZXx emulator
/*
	- We allocate one big chunk of memory from platform
	- This then gets subdivided by storing pointers into the memory in PAGES[] and BANKS[] arrays
	- Standard PAGE size is 8k, standard BANK size is 16k
	- The Spectrum 128 style memory management uses BANKS
	- The Spectrum Next style memory management uses PAGES

	Reference: http://www.breakintoprogram.co.uk/hardware/computers/zx-spectrum/memory-map

	ZXX Memory Layout
	32 BANKS of 16k memory = 512k (of which 3 BANKS (48k) are used for ROMs)

	The standard layout at boot matches that of the Spectrum 128k 

	| VISIBLE SLOT  | Boot | possible mappings in 128k mode
	 -----------------------------------------------------------------
	| 0xc000-0xffff | RAM0  [RAM1][RAM2][RAM3][RAM4][RAM5][RAM6][RAM7]
	 -----------------------------------------------------------------
	| 0x8000-0xbfff | RAM2
	 -----------------------------------------------------------
	| 0x4000-0x7fff | RAM5
	 -----------------------------------------------------------
	| 0x0000-0x3fff | ROM1  [ROM0][ROM2]
	 -----------------------------------------------------------

	 ROM2 is the original 48k rom, ROM0 and ROM1 are the 128k roms
*/

#include <stdio.h>

#if defined(__circle__)
#include <circle/util.h>
#else
#include <string.h>		// memcpy/memset
#endif

#include "spectrum.h"
#include "text_box_l.h"

// Maps a bank into slot and returns the previous bank number mapped into slot
int z80_mmu_MemMap(z80_mmu_t *mmu, int slot, int bank_no, enum MEM_MAPPING_TYPE mapping_type) {
	int prev_bank_no = mmu->visible_banks[slot].index;
	mmu->visible_banks[slot].index = bank_no;
	mmu->visible_banks[slot].mapping_type = mapping_type;
	return prev_bank_no;
}

// Called from user code (write to ULA port 0x7FFD) and from the ROM paging routine 
// FIXME: video bank switch not implemented yet
void _zx_MMU_update_memory_map_zx128(z80_mmu_t *mmu, uint8_t data) {
	
	// switching to the 48k rom from 128k mode disables any further banking until reset
	if (mmu->enable_128k_banking == false)
		return;

	// bit 3 defines the video scanout memory bank (5 or 7)
	//sys->display_ram_bank = (data & (1 << 3)) ? 7 : 5;

	// only last memory slot is mappable
	// the mmu physical banks numbering matches the virtual banks numbering
	// NOTE: I believe the logic here is wrong, the three bits should be inverted
	// because right now the rom pages in RAM7 here all the time and afaik it should be RAM0
	z80_mmu_MemMap(mmu, 3, data & 0x7, M_READ_WRITE);

	// ROM0 or ROM1
	if (data & (1 << 4)) {
		// bit 4 set: ROM1
		z80_mmu_MemMap(mmu, 0, ROM_1_BANK, M_READ_ONLY);
		mmu->current_rom = ROM_1_BANK;
	}
	else {
		// bit 4 clear: ROM0
		z80_mmu_MemMap(mmu, 0, ROM_0_BANK, M_READ_ONLY);
		mmu->current_rom = ROM_0_BANK;
	}

	if (data & (1 << 5)) {
		// bit 5 prevents further changes to memory pages
		//	until computer is reset, this is used when switching
		//	to the 48k ROM
		//zx_CON_Printf("MMU: switching to 48k ROM and disabling 128k banking.");
		mmu->enable_128k_banking = false;
	}

}

// ----------------------------------------------------------------------------

static void _setup_boot_mappings(z80_mmu_t *mmu, zx_type_t system_type) {

	// map default 16k banks into the 4 available slots 
	if (system_type == ZX_TYPE_48K) {
		z80_mmu_MemMap(mmu, 0, ROM_2_BANK, M_READ_ONLY);
		mmu->current_rom = ROM_2_BANK;
	}
	else {
		z80_mmu_MemMap(mmu, 0, ROM_0_BANK, M_READ_ONLY);
		mmu->current_rom = ROM_0_BANK;
	}

	z80_mmu_MemMap(mmu, 1, RAM_5_BANK, M_READ_WRITE);
	z80_mmu_MemMap(mmu, 2, RAM_2_BANK, M_READ_WRITE);
	z80_mmu_MemMap(mmu, 3, RAM_0_BANK, M_READ_WRITE);


	/*
	mmu->visible_pages[0].index = 0;
	mmu->visible_pages[0].mapping_type = M_READ_ONLY;
	mmu->visible_pages[1].index = 1;
	mmu->visible_pages[1].mapping_type = M_READ_ONLY;
	mmu->visible_pages[2].index = 2;
	mmu->visible_pages[2].mapping_type = M_READ_WRITE;
	mmu->visible_pages[3].index = 3;
	mmu->visible_pages[3].mapping_type = M_READ_WRITE;
	mmu->visible_pages[4].index = 4;
	mmu->visible_pages[4].mapping_type = M_READ_WRITE;
	mmu->visible_pages[5].index = 5;
	mmu->visible_pages[5].mapping_type = M_READ_WRITE;
	mmu->visible_pages[6].index = 6;
	mmu->visible_pages[6].mapping_type = M_READ_WRITE;
	mmu->visible_pages[7].index = 7;
	mmu->visible_pages[7].mapping_type = M_READ_WRITE;
	*/
}

void z80_mmu_Reset(z80_mmu_t *mmu, zx_type_t system_type) {
	// create the default startup mappings
	_setup_boot_mappings(mmu, system_type);

	// clear ram (note that ROM_0 will always be the first non ram bank)
	for(int bank = 0; bank < ROM_0_BANK; bank++)
		memset((void*)mmu->banks[bank], 0, MEM_BANK_SIZE);

	if (system_type == ZX_TYPE_128K)
		mmu->enable_128k_banking = true;

}

void z80_mmu_Init(z80_mmu_t *mmu, zx_type_t system_type) {

	memset((void*)mmu->memory, 0, MEM_BANK_SIZE*MEM_NUM_BANKS);

	// create mappings for banks and pages
	// we simply map consecutive chunks of the allocated memory into the bank/page slots
	for (int bank = 0; bank < MEM_NUM_BANKS; bank++) {
		mmu->banks[bank] = (uint8_t *)mmu->memory + bank * MEM_BANK_SIZE;
	}
	for (int page = 0; page < MEM_NUM_PAGES; page++) {
		mmu->pages[page] = (uint8_t *)mmu->memory + page * MEM_PAGE_SIZE;
	}

	// create the default startup mappings
	_setup_boot_mappings(mmu, system_type);

	if (system_type == ZX_TYPE_128K)
		mmu->enable_128k_banking = true;
}


// z80mmu.c