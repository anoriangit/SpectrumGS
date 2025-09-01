

#include "z80cpu.h"
#include "cputraps.h"
#include "text_box_l.h"

// The 48k rom entry point for save, load, verify, merge
// 0x0621 actually traps SA-SPACE: at that point the rom has moved the filename to the calc. stack
// This trap does not change the ROM execution path: all we do is snatch the filename
// in order to check for automounts (filenames starting with '$' or '#') and, if a prefix
// was found clean up the filename:
//		For '$' we remove the prefix but leave the name as is
//		For '#' we remove the name entirely: this is for loading multi file .taps
void _trap_SA_SPACE(Z80 *z80) {
	ltb_printf("CPU trap: entering 48k ROM routine \"SA-SPACE\"\n");

	// 0x5C65: end of rom calculator stack / workspace (STKEND): this is NOT the machine stack!
	// we replicate STK-FETCH here: get the last 5 byte values from the calculator stack
	// unlike the ROM routine we leave the data there though (we are letting the ROM code run here)
#if 0
	uint16_t stk_end = zx_MMU_GetWord(&zx->mmu, 0x5C65);	
	uint8_t b5 = zx_MMU_GetByte(&zx->mmu, --stk_end);	// register B
	uint8_t b4 = zx_MMU_GetByte(&zx->mmu, --stk_end);	// register C	: name len 
	uint16_t name_len_addr = stk_end;					// save the va of the name len byte
	uint8_t b3 = zx_MMU_GetByte(&zx->mmu, --stk_end);	// register D	: name address hi
	uint8_t b2 = zx_MMU_GetByte(&zx->mmu, --stk_end);	// register E	: name address lo
	uint8_t b1 = zx_MMU_GetByte(&zx->mmu, --stk_end);	// register A	: no idea

	char tapname[15];				// max size is 10 + 4 (".tap") + 0 terminator
	uint16_t va = b3 * 256 + b2;	// name address (virtual)
	for (int i = 0; i < b4; i++) {
		uint8_t *addr = (uint8_t*)zx_MMU_GetPhysicalAddress(&zx->mmu, va++);
		tapname[i] = *addr;
	}
	tapname[b4] = 0;

	if (tapname[0] == '$' || tapname[0] == '#') {
		strcpy(zx->auto_mount_tap.tap_base_name, &tapname[1]);
		strcpy(zx->auto_mount_tap.tap_file_name, zx->auto_mount_tap.tap_base_name);
		strcat(zx->auto_mount_tap.tap_file_name, ".tap");

		zx_CON_Printf("\tprefixed [$#] name found: requesting automount=\"%s\"\n", zx->auto_mount_tap.tap_file_name);

		zx->auto_mount_tap.state = TAP_REQUEST_MOUNT;

		// fixup the name stored in zx memory for the rom: remove the $ prefix
		// or remove name completely (by setting len to 0) if the prefix was #
		uint16_t va = b3 * 256 + b2;		// name address (virtual)
		if(tapname[0] == '$') {
			for (int i = 0; i < b4; i++) {
				uint8_t b = zx_MMU_GetByte(&zx->mmu, va+1);
				zx_MMU_PutByte(&zx->mmu, b, va);
				va++;
			}
			zx_MMU_PutByte(&zx->mmu, ' ', va);				// overwrite the previous last char with a blank
			zx_MMU_PutByte(&zx->mmu, b4-1, name_len_addr);	// fix name len
		}
		else {
			zx_MMU_PutByte(&zx->mmu, 0, name_len_addr);		// set name len to 0
		}
	}
	else {
		zx_CON_Printf("\tname=\"%s\" (no new automount)\n", &tapname[0]);
	}
#endif
}


// 48k Load Bytes Trap
// This is used to actually load blocks of data in both: 48k and 128k modes
void _trap_LD_BYTES(Z80 *z80) {
    
   	//ltb_printf("CPU trap: entering 48k ROM routine \"LD-BYTES\" (carry flag:%d)\n", zx->cpu.f & Z80_CF);
   	ltb_printf("CPU trap: entering 48k ROM routine \"LD-BYTES\"\n");

#if 0
	int err = TAP_OK;

	// coming in here with
	// A block type to load (00 header, ff data)
	// carry set=load, reset=verify
	// DE block len
	// IX target address

	if (zx->auto_mount_tap.state == TAP_REQUEST_MOUNT || zx->auto_mount_tap.state == TAP_MOUNTED) {

		if (zx->cpu.a == 0x00) {
			// header block
			// mount tap if needed
			if (zx->auto_mount_tap.state == TAP_REQUEST_MOUNT) {
				zx->auto_mount_tap.read_index = zx->auto_mount_tap.write_index = 0;
				zx->auto_mount_tap.state = TAP_MOUNTED;
			}

			zx_CON_Printf("\tloading HEADER from automount file: %s... ", zx->auto_mount_tap.tap_file_name);
			
			uint16_t va = zx->cpu.ix;
			err = zx_TAP_LoadBlock(zx, &zx->auto_mount_tap, zx->cpu.a, zx->cpu.de, va);
			if(err != TAP_OK) 
				zx_CON_Printf("\tTAP ERROR: %d", err);

		}
		else {
			// data block
			zx_CON_Printf("\tloading DATA (len=%d) from automount file: %s... ", zx->cpu.de, zx->auto_mount_tap.tap_file_name);

			uint16_t va = zx->cpu.ix;
			err = zx_TAP_LoadBlock(zx, &zx->auto_mount_tap, zx->cpu.a, zx->cpu.de, va);
			if (err != TAP_OK)
				zx_CON_Printf("\tTAP ERROR: %d", err);
		}

		// adapt z80 registers
		//zx->cpu.hl = zx->cpu.ix;	// test test test
		zx->cpu.ix += zx->cpu.de;	// move pointer in ix as if the rom loader had actually run
		zx->cpu.de = 0;				// no remaining bytes to be loaded

		if (err != TAP_OK) {
			zx->cpu.f &= ~Z80_CF;			// clear carry (indicates an error)
			zx_CON_Printf("error: TAP error, code=%d\n", err);
		}
		else {
			zx->cpu.f |= Z80_CF;			// set carry (success)
			//printf("success.\n");
		}
		// 053F SA/LD-RET
		z80_prefetch(&zx->cpu, 0x053F);		// continue at ROM LD-BYTES routine exit point (original: 05e2)

	}
	else if (zx->auto_mount_tap.state == TAP_REQUEST_DIRECTORY) {
		// I believe this is unfinished business: was supposed to be able so issue something like
		// 'load "$"' in order to produce a listing of .tap files in the current directory
		// not clear as to how&when I'll follow this up
		if (zx->cpu.a == 0x00) {
			zx_CON_Printf("\tprocessing directory request (header)");
			// create a fake header block
			static char name[] = "DIRECTORY";
			zx_TAP_CreateHeaderBlock(zx, 0, zx->cpu.ix, name, 0, 0, 0);
			zx->cpu.ix += zx->cpu.de;		// move pointer in ix as if the rom loader had actually loaded a header
		}
		else {
			// not doing anything here
			zx_CON_Printf("\tprocessing directory request (data)");
		}
		zx->cpu.de = 0;					// no remaining bytes to be loaded
		zx->cpu.f |= Z80_CF;			// set carry flag (success)
		z80_prefetch(&zx->cpu, 0x053F);	// continue at ROM LD-BYTES routine exit point (053F SA/LD-RET)
	}
	else {
		zx_CON_Printf("\tno tap mount: just passing on to ROM...");
	}
#endif
}

// 48k spectrum ROM save bytes routine trap
// called on 48k and 128k
// This relies on _trap_SA_SPACE having detected our special filename prefixes
// and cleaned them up if required
void _trap_SA_BYTES(Z80 *z80) {

   	ltb_printf("CPU trap: entering 48k ROM routine \"SA-BYTES\"");

#if 0
	// each "Save" will cause two calls into this function:
	// one for the header and one for the actual data 
	// we thus keep the file open between the two calls and verify the correct order

	static uint8_t block_type;
	uint8_t check;


	// DE holds the size of the block to save, IX its address 
	// and A a flag value (either 0x00 (header) or 0xff (program/data))

	/* Format https://worldofspectrum.org/faq/reference/48kreference.htm */


	if (zx->cpu.a == 0x00) {
		block_type = 0x00;
		check = block_type;

		// header: DE points to a 17 bytes block of data containing the file type byte (0,1,2,3)
		// 10 bytes name (blanks padded)
		// 2 bytes length of the data block to follow (msb first)
		// 4 bytes "parameters" (which we don't need to concern ourselves with here)

		uint8_t header[17];
		uint16_t va = zx->cpu.ix;				// start address (virtual)
		uint16_t header_block_len = zx->cpu.de;	// should always be 17 for header blocks
		//uint16_t file_block_len = header_block_len + 2;
		for (int i = 0; i < header_block_len; i++) {
			pointer_t addr = zx_MMU_GetPhysicalAddress(&zx->mmu, va++);
			header[i] = *((uint8_t *)addr);
			check = check ^ header[i];
		}
	
		if (zx->auto_mount_tap.state == TAP_REQUEST_MOUNT) {
			// if we come here with state=TAP_REQUEST_MOUNT then the user has issued
			// a save "$name" request (the details regarding "name" are stored in the automount info in ZX)
			// we just pass on the filename to the WriteBlock() function which in turn will call 
			// P_FileWrite() which will create the file and cache name&filehandle 
			zx_CON_Printf("\t writing header block: requested automount filename=%s, length=%d", zx->auto_mount_tap.tap_file_name, zx->cpu.de);
			zx_TAP_WriteBlock(zx, zx->auto_mount_tap.tap_file_name, block_type, header_block_len, check, (pointer_t)header);
			zx->auto_mount_tap.state = TAP_MOUNTED;
		}
		else if (zx->auto_mount_tap.state == TAP_MOUNTED) {
			// the user has issued a SAVE "$" command (no name specified): we are supposed to save using the details
			// of a previous automount (this also works if that has been a LOAD command)
			// In this case though the header data as stored by the BASIC ROM will just contain the "$" character as
			// the name: we need to fix this by replacing it with the data we have stored inside the automount info
			// and re-doing the checksum before we can write the header to the file 
			check = block_type;
			for (int i = 0; i < header_block_len; i++) {
				// fixup the name inside the header
				if(i > 0 && i <= 10 && zx->auto_mount_tap.tap_base_name[i - 1] != 0)
					header[i] = zx->auto_mount_tap.tap_base_name[i - 1];
				// update new checksum
				check = check ^ header[i];
			}

			zx_CON_Printf("\twriting header block: existing automount filename=%s, length=%d", zx->auto_mount_tap.tap_file_name, zx->cpu.de);
			zx_TAP_WriteBlock(zx, zx->auto_mount_tap.tap_file_name, block_type, header_block_len, check, (pointer_t)header);

		}
		else {
			zx_CON_Printf("\tno tap mount: just passing save request on to the BASIC ROM...");
		}
	}
	else if (zx->cpu.a == 0xff) {
		assert(block_type == 0x00);		// previous block must have been a header

		// DATA BLOCK
		block_type = 0xff;
		check = block_type;			// init checksum with block type
		uint16_t va = zx->cpu.ix;	// start address
		uint16_t data_block_len = zx->cpu.de;
		
		if (zx->auto_mount_tap.state == TAP_MOUNTED) {
			zx_CON_Printf("\twriting data block to current automount: length=%d", data_block_len);
			zx_TAP_WriteBlock(zx, zx->auto_mount_tap.tap_file_name, block_type, data_block_len, check, (pointer_t)va);
		}
		else {
			zx_CON_Printf("\tno tap mount: just passing save request to ROM...");
		}
	}

	// short cut to the exit of the ROM routine setting the counter 
	// for the final delay wait in there (register b) to 0
	zx->cpu.b = 0;
	z80_prefetch(&zx->cpu, 0x53c);
#endif
}

// cputraps.c
