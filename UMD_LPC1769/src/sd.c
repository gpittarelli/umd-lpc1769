#include "UMDLPC/system/sd.h"

static int sd_version;

void sd_command(uint8_t index, uint8_t a1, uint8_t a2,
                uint8_t a3, uint8_t a4, uint8_t crc,
                uint8_t* response, uint16_t response_len) //{{{
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
	 *    These tell the SPI peripheral the edges of clock to set up
   *    and lock in data
	 *  Mode:
	 *    Master mode
	 */

  spi_init();

	LPC_GPIO0->FIODIR |= GPIO_SD_CS_m; // enable chip select
	LPC_GPIO0->FIOSET = GPIO_SD_CS_m; // turn off the chip select (high)

	unsigned char resp[10];

  // send at least 74 clock pulses so the card enters native mode
	spi_txrx(NULL, NULL, 10);

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
		if (!(resp[1] == 0 && resp[2] == 0 && resp[3] == 0x01 && resp[4] == 0xAA))
      // voltage range is incorrect
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

    // ACMD41 with HCS (bit 30) HCS is ignored by V1 cards
		sd_command(41, 0x40, 0x00, 0x00, 0x00, 0x00, resp, 1);

		LPC_GPIO0->FIOSET = GPIO_SD_CS_m;
	}

	// Set clock speed to 10MHz
  LPC_SSP0->CPSR = SystemCoreClock / 10000000;

	// check the OCR register to see if it's a high capacity card
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(58, 0x00, 0x00, 0x00, 0x00, 0x00, resp, 5); // CMD58
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
	sd_command(17, (0xFF000000 & block_num) >> 24,
                 (0xFF0000 & block_num) >> 16,
                 (0xFF00 & block_num) >> 8,
                 0xFF & block_num, 0, &rx, 1); // CMD17

	// Could be an issue here where the last 8 of SD command contains
	// the token, but I doubt this happens

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
	sd_command(24, (0xFF000000 & block_num) >> 24,
                 (0xFF0000 & block_num) >> 16,
                 (0xFF00 & block_num) >> 8,
                 0xFF & block_num, 0, &rx, 1); // CMD24

	// Could be an issue here where the last 8 of SD command contains
	// the token, but I doubt this happens
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


/***************************************************************
 * WARNING: This DMA transfer is a work in progress, it does NOT
 *          work yet.
 ***************************************************************/
char sd_read_block_dma(uint8_t* block, uint32_t block_num,
                       LPC_GPDMACH_TypeDef * const dma_channel) {
	// TODO bounds checking
	uint8_t rx = 0xFF;

	// send the single block command
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(17, (0xFF000000 & block_num) >> 24,
                 (0xFF0000 & block_num) >> 16,
                 (0xFF00 & block_num) >> 8,
                 0xFF & block_num, 0, &rx, 1); // CMD17

	// Could be an issue here where the last 8 of SD command contains
	// the token, but I doubt this happens

	if (rx != 0x00)
		return 0;

	// read until the data token is received
	rx = 0xFF;
	while (rx != 0b11111110)
		spi_txrx(NULL, &rx, 1);

  dma_channel->DMACCSrcAddr  = (uint32_t) LPC_SSP0->DR;
  dma_channel->DMACCDestAddr = (uint32_t) block;
  dma_channel->DMACCLLI      = 0;

  // Linked List control information (Same as DMA channel control register)
  //  Transfer size: SD_BLOCK_LEN () (bits 11:0)
  //  Source burst size: 32 (4, bits 14:12)
  //  Destination burst size: 1 (0, bits 17:15)
  //  Source transfer width: word (2, bits 20:18)
  //  Destination transfer width: word (2, bits 23:21)
  //  Source increment: don't increment (0, bit 26)
  //  Destination increment: increment (1, bit 27)
  dma_channel->DMACCControl  = (SD_BLOCK_LEN)
                               | (4 << 12)
                               | (2 << 18) | (2 << 21) | (1 << 27);

  // Enable read dma
  LPC_SSP0->DMACR = 1;

  //  Enable channel (1 at bit 0)
  //  Source peripheral: memory (default, bits  5:1)
  //  Destination peripheral: SSP0 RX (1, bits  10:6)
  //  Transfer Type: peripheral-to-memory (2, bits 13:11)
  dma_channel->DMACCConfig = (1 << 0) | (2 << 11);

  while (!(dma_channel->DMACCConfig & 1));

  // Clean up SSP0 DMA settings
  LPC_SSP0->DMACR = 0;

	spi_txrx(NULL, NULL, 2); // throw away the CRC
	spi_txrx(NULL, NULL, 1); // 8 cycles to prepare the card for the next command

	LPC_GPIO0->FIOSET = GPIO_SD_CS_m;

	return 1;
}

/***************************************************************
 * WARNING: This DMA transfer is a work in progress, it does NOT
 *          work yet.
 ***************************************************************/
char sd_write_block_dma(uint8_t* block, uint32_t block_num,
                        LPC_GPDMACH_TypeDef * const dma_channel) {
  // TODO bounds checking
	uint8_t rx = 0xFF;
	uint8_t tx[2];

	// send the single block write
	LPC_GPIO0->FIOCLR = GPIO_SD_CS_m;
	sd_command(24, (0xFF000000 & block_num) >> 24,
                 (0xFF0000 & block_num) >> 16,
                 (0xFF00 & block_num) >> 8,
                 0xFF & block_num, 0, &rx, 1); // CMD24

	// Could be an issue here where the last 8 of SD command contains
	// the token, but I doubt this happens
	if (rx != 0x00)
		return 0;

	// tick clock 8 times to start write operation
	spi_txrx(NULL, NULL, 1);

	// write data token
	tx[0] = 0xFE;
	spi_txrx(tx, NULL, 1);

	// write data
	memset(tx, 0, sizeof(tx));

  dma_channel->DMACCSrcAddr  = (uint32_t) block;
  dma_channel->DMACCDestAddr = (uint32_t) &(LPC_SSP0->DR);
  dma_channel->DMACCLLI      = 0;

  // Linked List control information (Same as DMA channel control register)
  //  Transfer size: AUDIO_BUFFER_LEN (bits 11:0)
  //  Source burst size: 32 (4, bits 14:12)
  //  Destination burst size: 1 (0, bits 17:15)
  //  Source transfer width: word (2, bits 20:18)
  //  Destination transfer width: word (2, bits 23:21)
  //  Source increment: increment (1, bit 26)
  //  Destination increment: don't increment (0, bit 27)
  dma_channel->DMACCControl  = (SD_BLOCK_LEN / 4)
                               | (4 << 12)
                               | (2 << 18) | (2 << 21) | (1 << 26);

  // Enable transmit DMA
  LPC_SSP0->DMACR = 2;

  //  Enable channel (1 at bit 0)
  //  Source peripheral: memory (default, bits  5:1)
  //  Destination peripheral: SSP0 TX (0, bits  10:6)
  //  Transfer Type: memory-to-peripheral (1, bits 13:11)
  dma_channel->DMACCConfig = (1 << 0) | (1 << 11);

  while (!(dma_channel->DMACCConfig & 1));

  // Clean up SSP0 DMA settings
  LPC_SSP0->DMACR = 0;

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
}
