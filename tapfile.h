// ----------------------------------------------------------------------------
// tap files support

#pragma once

#include <stdint.h>
#include "z80cpu.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_TAP_NAME_SIZE      9      // does not include trailing NULL
#define MAX_TAP_FILE_NAME_SIZE 256

#define TAP_OK                0
#define TAP_ERR_BLOCK_TYPE    1
#define TAP_ERR_BLOCK_LEN     2
#define TAP_ERR_CHECKSUM      3
#define TAP_ERR_NOT_MOUNTED   4
#define TAP_ERR_FILE_ERROR    5       // some error occured in the underlying platform file io code

enum tapstate { TAP_UN_MOUNTED, TAP_REQUEST_MOUNT, TAP_MOUNTED, TAP_REQUEST_DIRECTORY };

typedef struct {
    char tap_base_name[MAX_TAP_NAME_SIZE + 1];      // this is the filename "inside" the tap as used by the ROM
    char tap_file_name[MAX_TAP_FILE_NAME_SIZE + 1]; // tap filename on disk
    uint32_t read_index, write_index;
    enum tapstate state;
} tap_t;


void TAP_CreateHeaderBlock (zx_spectrum_t *zx, uint8_t block_type, uint16_t va, char *name, uint16_t data_len, uint16_t p1, uint16_t p2);
int TAP_LoadBlock(zx_spectrum_t *zx, tap_t *tap, uint8_t block_type, uint16_t block_len, uint16_t va);
int TAP_WriteBlock(zx_spectrum_t *zx, char *filename, uint8_t block_type, uint16_t block_len, uint8_t check, uint16_t va);

#ifdef __cplusplus
} // extern "C"
#endif


// tapfile.h
