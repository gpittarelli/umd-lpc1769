#ifndef _UMDLPC_SD_H
#define _UMDLPC_SD_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "UMDLPC/system/dma.h"
#include "UMDLPC/system/spi.h"
#include "../inc/LPC17xx.h"
#include "../inc/core_cm3.h"

#define SD_BLOCK_LEN 512
#define SD_MAX_RESP_TRIES 100
#define SD_MAX_RESET_TRIES 100
#define SD_INVALID_SECTOR 0xFFFFFFFF

#define GPIO_SD_CS_m (1<<6) // 498A: Defined as P0

int sd_init();
void sd_command(uint8_t index, uint8_t a1, uint8_t a2,
                uint8_t a3, uint8_t a4, uint8_t crc,
                uint8_t* response, uint16_t response_len);
char sd_read_block(uint8_t* block, uint32_t block_num);
char sd_write_block(uint8_t* block, uint32_t block_num);

/***************************************************************
 * WARNING: These DMA transfer functions are a work in progress,
 *          they do NOT work yet.
 ***************************************************************/
char sd_read_block_dma(uint8_t* block, uint32_t block_num,
                       LPC_GPDMACH_TypeDef * const channel);
char sd_write_block_dma(uint8_t* block, uint32_t block_num,
                        LPC_GPDMACH_TypeDef * const channel);

#endif
