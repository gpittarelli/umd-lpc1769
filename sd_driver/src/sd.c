#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "../inc/LPC17xx.h"
#include "../inc/core_cm3.h"
#include "sd.h"
#include "spi.h"

#define SD_MAX_RESP_TRIES 100
#define SD_MAX_RESET_TRIES 100
#define SD_BLOCK_LEN 512

static int sd_version;

void sd_command(uint8_t index, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t crc, uint8_t* response, uint16_t response_len) //{{{
{
	uint16_t tries;
	uint8_t command[6];
	uint8_t rx = 0;

	memset(response, 0, response_len);

	// fill command buffer
	command[0] = 0b01000000 | index; // command index
	command[1] = a1; // arg 0 
	command[2] = a2; // arg 1
	command[3] = a3; // arg 2
	command[4] = a4; // arg 3
	command[5] = crc; // CRC

	// transmit command
	spi_txrx(command, NULL, 6);

	// read until stop bit
	tries = 0;
	response[0] = 0xFF;
	while ((response[0] & 0x80) != 0 && tries < SD_MAX_RESP_TRIES)
	{
		spi_txrx(NULL, response, 1);
		tries++;
	}

	// special bit for the idle command, if no idle response give up now
	if ((response[0] & 0x80) != 0 || (index == 0 && !(response[0] & 0x01)))
		return;

	if (response_len > 1)
	{
		// get the rest of the response
		spi_txrx(NULL, (response+1), response_len);
	}

	/* read until the busy flag is cleared, 
	 * this also gives the SD card at least 8 clock pulses to give 
	 * it a chance to prepare for the next CMD */
	rx = 0;
	while (rx == 0)
		spi_txrx(NULL, &rx, 1);
} //}}}

int sd_init() //{{{
{
	/* Embed : SPI initialization code
	 *  Clock:
	 *    Start the SPI clock at < 200 KHz, after the card is
   	 *    initalized you can increase the clock rate to < 50 MHz
 	 *  Bits:
	 *    Use 8-bit mode
	 *  Polarity:
	 *    CPOL = 0 and CPHA = 0.
	 *    These tell the SPI peripheral the edges of clock to set up and lock in data
	 *  Mode:
	 *    Master mode
	 */

        spi_init()

	LPC_GPIO0->FIODIR |= GPIO_SD_CS_m; // enable chip select
	LPC_GPIO0->FIOSET = GPIO_SD_CS_m; // turn off the chip select (high)

	unsigned char resp[10];

	spi_txrx(NULL, NULL, 10); // send at least 74 clock pulses so the card enters native mode

	// keep trying to reset
	uint16_t tries = 0;
	resp[0] = 0;
	while(resp[0] != 0x01 && tries < SD_MAX_RESET_TRIES)
	{
		LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
		sd_command(0x00, 0x00, 0x00, 0x00, 0x00, 0x95, resp, 1); // CMD0, R1
		LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
		tries++;
	}
	if (tries >= SD_MAX_RESET_TRIES)
		return -1;

	// check voltage range and check for V2
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(0x08, 0x00, 0x00, 0x01, 0xAA, 0x87, resp, 5); // CMD8, R7
	LPC_GPIO0->FIOSET = GPIO_SD_CS_m;

	// V2 and voltage range is correct, have to do this for V2 cards
	if (resp[0] == 0x01)
	{
		if (!(resp[1] == 0 && resp[2] == 0 && resp[3] == 0x01 && resp[4] == 0xAA)) // voltage range is incorrect
			return -2;
	}

	// the initialization process
	while (resp[0] != 0x00) // 0 when the card is initialized
	{
		LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
		sd_command(55, 0x00, 0x00, 0x00, 0x00, 0x00, resp, 1); // CMD55
		LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
		if (resp[0] != 0x01)
			return -3;
		LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
		sd_command(41, 0x40, 0x00, 0x00, 0x00, 0x00, resp, 1); // ACMD41 with HCS (bit 30) HCS is ignored by V1 cards
		LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
	}

	// Embed : set the SPI clock to a much higher rate

	// check the OCR register to see if it's a high capacity card
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	`dsd_command(58, 0x00, 0x00, 0x00, 0x00, 0x00, resp, 5); // CMD58
	LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
	if ((resp[1] & 0x40) > 0)
		sd_version = 2; // V2 card
	else
		// set the block length CMD16 to 512
		sd_version = 1; // V1 card
	return 0;
} //}}}

char sd_read_block(uint8_t* block, uint32_t block_num) //{{{
{
	// TODO bounds checking
	uint8_t rx = 0xFF;

	// send the single block command
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(17, (0xFF000000 & block_num) >> 24, (0xFF0000 & block_num) >> 16, (0xFF00 & block_num) >> 8, 0xFF & block_num, 0, &rx, 1); // CMD17

	// Could be an issue here where the last 8 of SD command contains the token, but I doubt this happens

	if (rx != 0x00)
		return 0;

	// read until the data token is received
	rx = 0xFF;
	while (rx != 0b11111110)
		spi_txrx(NULL, &rx, 1);

	spi_txrx(NULL, block, SD_BLOCK_LEN); // read the block
	spi_txrx(NULL, NULL, 2); // throw away the CRC
	spi_txrx(NULL, NULL, 1); // 8 cycles to prepare the card for the next command

	LPC_GPIO0->FIOSET = GPIO_SD_CS_m;

	return 1;
} //}}}

char sd_write_block(uint8_t* block, uint32_t block_num) //{{{
{
	// TODO bounds checking
	uint8_t rx = 0xFF;
	uint8_t tx[2];

	// send the single block write 
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(24, (0xFF000000 & block_num) >> 24, (0xFF0000 & block_num) >> 16, (0xFF00 & block_num) >> 8, 0xFF & block_num, 0, &rx, 1); // CMD24

	// Could be an issue here where the last 8 of SD command contains the token, but I doubt this happens
	if (rx != 0x00)
		return 0;

	// tick clock 8 times to start write operation 
	spi_txrx(NULL, NULL, 1);

	// write data token
	tx[0] = 0xFE;
	spi_txrx(tx, NULL, 1);

	// write data
	memset(tx, 0, sizeof(tx));
	spi_txrx(block, NULL, SD_BLOCK_LEN); // write the block
	spi_txrx(tx, NULL, 2); // write a blank CRC
	spi_txrx(NULL, &rx, 1); // get the response

	// check if the data is accepted
	if (!((rx & 0xE) >> 1 == 0x2))
		return 0;

	// wait for the card to release the busy flag
	rx = 0;
	while (rx == 0)
		spi_txrx(NULL, &rx, 1);

	spi_txrx(NULL, NULL, 1); // 8 cycles to prepare the card for the next command

	LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
	return 1;
} //}}}
