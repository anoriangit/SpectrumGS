// tapfile.c
// .tap file support


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "spectrum.h"
#include "tapfile.h"
//#include <fatfs/ff.h>


// file functions: here for now
// will move to a dedicated source file at some point

#define P_ERR_FILE_OK	        0
#define P_ERR_FILE_OPEN_ERROR   -1
#define P_ERR_FILE_SEEK_ERROR   -2
#define P_ERR_FILE_READ_ERROR   -3
#define P_ERR_FILE_WRITE_ERROR  -4

#define P_FILE_WRITE_MODE_CREATE 0      // create if doesn't exist, overwrite otherwise
#define P_FILE_WRITE_MODE_APPEND 1

// Write data to file
//	- either creates a new file (overwrites existing) or appends to given file
//	- the file pointer is being cached inbetween calls so that several
//		write calls to the same file can re-use it without having to re-open the file
//	- file_name must be properly 0 terminated
// Return: number of bytes written or negative error code

int P_WriteFileData(const char *fname, int mode, void *data, size_t len) {

	return 0;
	
#if 0
	static FIL *cur_file = NULL;
    static FIL file;
    UINT n_written = 0;

    // note: only "create new" implemented right now
    if(!cur_file) {
    	FRESULT Result = f_open(&file, fname, FA_WRITE | FA_CREATE_ALWAYS);
	    if (Result != FR_OK) {
		    return P_ERR_FILE_OPEN_ERROR;
	    }
        cur_file = &file;
    }

	if (f_write(cur_file, data, (UINT)len, &n_written) != FR_OK || n_written != len) {
        return P_ERR_FILE_WRITE_ERROR;
    }
	return (int)n_written;
#endif

}

// maximum block size for tap IO is 64k
//static uint8_t IOBUF[0xffff];

// write a word to spectrum memory in the std. lo->hi order
static void _write_word(Z80 *z80, uint16_t va, uint16_t word) {

    z80->write(0, va,   (word & 0x00ff));
    z80->write(0, va+1,((word & 0xff00)>>8));

}

// Create a tap format data block in spectrum memory at adress va
// - block type will be either 0x00 for headers or 0xff for data blocks
// - header blocks always have an implicit len of 17:
//	[1 byte type] [10 bytes name] [2 bytes len of data block to follow][2 bytes param1][2 bytes param2]
//		- name is padded to full len with blanks if needed
//		- param1 is the autostart line for basic programs
//		- param2 is the offset of the variables area for var saves
// On tape, or in a .tap file, each data block is prefixed by: [2 bytes block len][1 bytes block type(header or data)]
// and has [1 byte XOR checksum] appended

// Create a raw header block in spectrum memory (no tape pre- and postfixes)
// Note that block_type encodes: Program (0), Number array (1), Character Array (2) or Code (3) here (not header/data)
void TAP_CreateHeaderBlock (
	zx_spectrum_t *zx, uint8_t block_type, uint16_t va, 
	char *name, uint16_t data_len, uint16_t p1, uint16_t p2) {
	
    Z80 *z80 = zx->cpu;
	z80->write(0, va++, block_type);
	int name_len = (int)strlen(name);
	if (name_len > 10) name_len = 10;
	int i = 0;
	
	for(; i < name_len; i++) 
		z80->write(0, va++, name[i]);
	
	if (i < 10) {
		// need to pad with spaces
		for (; i < 10; i++)
			z80->write(0, va++, ' ');
	}

	_write_word(z80, va, data_len); va += 2;
	_write_word(z80, va, p1); va += 2;
	_write_word(z80, va, p2); va += 2;
}

void *_get_physical_address(zx_spectrum_t *zx, uint16_t va) {
    //return &zx->spectrum_memory[va];
	return 0;
}

// write a tap block from spectrum memory to a file 
// note that "va" is the virtual address of the tap data block inside spectrum memory space
int TAP_WriteBlock(zx_spectrum_t *zx, char *filename, uint8_t block_type, uint16_t block_len, uint8_t check, uint16_t va) {

	char fname[SPECTRUM_MAX_FILE_DIR_LEN];

	// complete the filename
	strcpy(fname, "./tap");     // add some setting for a "tap file dir" or something
	strcat(fname, "/");
	strcat(fname, filename);

    uint8_t *data = (uint8_t*)_get_physical_address(zx, va);

	uint16_t file_block_len = block_len + 2;
	if (block_type == 0x00) {
		P_WriteFileData(fname, P_FILE_WRITE_MODE_CREATE, &file_block_len, 2);	// write total block length (check byte order?)
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, &block_type, 1);
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, data, block_len);
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, &check, 1);
	}
	else if (block_type == 0xff) {
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, &file_block_len, 2);	// write total block length (check byte order?)
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, &block_type, 1);
		for (int i = 0; i < block_len; i++) {
			check = check ^ *((uint8_t *)data);
			P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, data++, 1);
		}
		P_WriteFileData(fname, P_FILE_WRITE_MODE_APPEND, &check, 1);

		// P_WriteFileData() caches it's filehandle between calls so that, as long
		// as filename remains the same, it can re-use it for appending more data 
		// and does not need to reopen the file on each call
		// Here we want the file forcibly closed though so that another .tap write with
		// the same name causes an overwrite and not an append
		//P_CloseFile();
	}

    return TAP_OK;
}


// Load a block of data from a mounted tap file
// 
// tap data block format on tape/file: {[blocklen:2][blocktype:1] |[datatype:1][data:blocklen-1]| [xor byte]}
//                                      ------- file header ----- ---- actual data block -------- -checksum-
//
// https://worldofspectrum.org/faq/reference/48kreference.htm

// block_type is 0x00 for header blocks or 0xff for data blocks
// block_len is the length of the block expected: this is actual length, not including the file-block-len prefix 
// va is the virtual address to which the block shall be loaded
// We check whether the tap is in mounted state, the block type and its len as well as the XOR code
// Returns TAP_OK (0) or an error code (see header)

int TAP_LoadBlock(zx_spectrum_t *zx, tap_t *tap, uint8_t block_type, uint16_t block_len, uint16_t va) {
#if 0
	int err;
	uint16_t file_block_len;
	uint8_t file_block_type, xor_check;
	char fname[SPECTRUM_MAX_FILE_DIR_LEN];


	// check mount state
	if (tap->state != TAP_MOUNTED)
		return TAP_ERR_NOT_MOUNTED;

	// prepare filename
	strcpy(fname, zx->settings.tap_file_dir);
	strcat(fname, "/");
	strcat(fname, tap->tap_file_name);

	// check file block len
	if ((err = P_ReadFileData(fname, tap->read_index, 2, (pointer_t)&file_block_len)) != P_ERR_OK) {
		//printf("ERROR: platform file error, code=%d", err);
		return TAP_ERR_FILE_ERROR;
	}
	tap->read_index += 2;

	if (file_block_len != block_len + 2)
		return TAP_ERR_BLOCK_LEN;

	// check block type
	if ((err = P_ReadFileData(fname, tap->read_index, 1, (pointer_t)&file_block_type)) != P_ERR_OK) {
		//printf("ERROR: platform file error, code=%d", err);
		return TAP_ERR_FILE_ERROR;
	}
	tap->read_index += 1;

	if (file_block_type != block_type)
		return TAP_ERR_BLOCK_TYPE;

	// read block data + trailing XOR byte
	if ((err = P_ReadFileData(fname, tap->read_index, block_len+1, (pointer_t)IOBUF)) != P_ERR_OK) {
		//printf("ERROR: platform file error, code=%d", err);
		return TAP_ERR_FILE_ERROR;
	}
	tap->read_index += block_len + 1;

	// copy the data to the destination and do the XOR test at the same time
	xor_check = block_type;
	for (int i = 0; i < block_len; i++) {
		xor_check = xor_check ^ IOBUF[i];
		zx_MMU_PutByte(&zx->mmu, IOBUF[i], va++);
	}

	// XOR check
	if (xor_check != IOBUF[block_len])
		return TAP_ERR_CHECKSUM;
#endif
	return TAP_OK;
}


// tapfile.c