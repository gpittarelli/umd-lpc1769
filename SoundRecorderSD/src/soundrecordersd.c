/*
 ===============================================================================
 Name        : soundrecordersd.c
 Author      : George Pittarelli
 Description :

   Implements a basic sound recorder on the LPC1769, using an SD card
   to store audio samples, and as SSD1289-driven TFT display

   Two buttons and a mic are wired for input, and an LED and speaker
   are used for output. One button, when pressed, triggers a DMA
   transfer from the ADC to an audio buffer and the other button
   starts a DMA transfer from the buffer to the DAC.

 ===============================================================================
 */

#include "soundrecordersd.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information.
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

// Staging buffer, which we will read a full block from the SD card
// into and then de-compress/process into an audio buffer.
uint8_t sd_block[SD_BLOCK_LEN];

// Two audio buffers. These should each be the length of a
// decompressed sd card block. One buffer will be playing while the
// other is being loaded.
uint32_t audio_buffer1[AUDIO_BUFFER_LEN], audio_buffer2[AUDIO_BUFFER_LEN];

__attribute__ ((section(".ahb_ram")))
DMALinkedListNode playback_node1, playback_node2,
  record_node1, record_node2;

// Alias DMA Channels
static LPC_GPDMACH_TypeDef * const RECORD_CHANNEL = LPC_GPDMACH0,
                           * const PLAYBACK_CHANNEL = LPC_GPDMACH1;

typedef enum {
  WAITING = 0,
  RECORDING_BUFFER1 = 1,
  RECORDING_BUFFER2 = 2,
  PLAYING_BUFFER1 = 3,
  PLAYING_BUFFER2 = 4
} ProgramState;
ProgramState current_state = WAITING;

void EINT3_IRQHandler (void)
{
  LPC_SC->EXTINT = _BV(3);		/* clear interrupt */
  LPC_GPIOINT->IO2IntClr = _BV(13);		/* clear interrupt */

  RECORDING_LED_TOGGLE();
}

void DMA_IRQHandler(void) {
  const static ProgramState TRANSITIONS[] = {
    [WAITING] = 0,
    [RECORDING_BUFFER1] = RECORDING_BUFFER2,
    [RECORDING_BUFFER2] = RECORDING_BUFFER1,
    [PLAYING_BUFFER1] = PLAYING_BUFFER2,
    [PLAYING_BUFFER2] = PLAYING_BUFFER1
  };

  current_state = TRANSITIONS[current_state];

  PLAYING_LED_TOGGLE();

  LPC_GPDMA->DMACIntTCClear = 0xff;
}

void wait_for_state(ProgramState desired_state) {
  while (current_state != desired_state)
    ;
}

// memcpy, unpack 8 bit fields to 32 bit fields shifted for the DAC
void transfer_from_block(uint8_t *source, uint32_t *dest, uint32_t len) {
  while (len--) {
    *dest++ = ((uint32_t) (*source++)) << 8;
  }
}

// memcpy, pack 32 bit fields from the ADC in to 8 bit fields
void transfer_to_block(uint32_t *source, uint8_t *dest, uint32_t len) {
  while (len--) {
    *dest++ = ((*source++) >> 8) & 0xff;
  }
}

void load_dma_node(LPC_GPDMACH_TypeDef * const channel,
                   DMALinkedListNode * const node) {
  channel->DMACCSrcAddr  = node->sourceAddr;
  channel->DMACCDestAddr = node->destAddr;
  channel->DMACCControl  = node->dmaControl;
  channel->DMACCLLI      = node->nextNode;
}

void playback() {
  uint32_t cur_block = 0;

  sd_read_block(sd_block, cur_block++);
  transfer_from_block(sd_block, audio_buffer1, SD_BLOCK_LEN);

  current_state = PLAYING_BUFFER1;
  load_dma_node(PLAYBACK_CHANNEL, &playback_node1);
  PLAYBACK_CHANNEL->DMACCConfig |= _BV(0);

  while (PLAY_BUTTON_READ()) {
    sd_read_block(sd_block, cur_block++);
    transfer_from_block(sd_block, audio_buffer2, SD_BLOCK_LEN);

    wait_for_state(PLAYING_BUFFER2);

    sd_read_block(sd_block, cur_block++);
    if (current_state == PLAYING_BUFFER1) {
      RECORDING_LED_TOGGLE();
    }
    transfer_from_block(sd_block, audio_buffer1, SD_BLOCK_LEN);

    wait_for_state(PLAYING_BUFFER1);
  }

  current_state = WAITING;
  PLAYBACK_CHANNEL->DMACCConfig &= ~(_BV(0));
}

void record() {
  uint32_t cur_block = 0;

  current_state = RECORDING_BUFFER1;
  load_dma_node(RECORD_CHANNEL, &record_node1);
  RECORD_CHANNEL->DMACCConfig |= _BV(0);

  while (RECORD_BUTTON_READ()) {
    wait_for_state(RECORDING_BUFFER2);

    transfer_to_block(audio_buffer1, sd_block, SD_BLOCK_LEN);
    sd_write_block(sd_block, cur_block++);

    wait_for_state(RECORDING_BUFFER1);

    transfer_to_block(audio_buffer2, sd_block, SD_BLOCK_LEN);
    sd_write_block(sd_block, cur_block++);

  }

  RECORD_CHANNEL->DMACCConfig &= ~(_BV(0));
}

typedef struct {
  uint32_t P2_0 : 2;
  uint32_t P2_1 : 2;
  uint32_t P2_2 : 2;
  uint32_t P2_3 : 2;
  uint32_t P2_4 : 2;
  uint32_t P2_5 : 2;
  uint32_t P2_6 : 2;
  uint32_t P2_7 : 2;
  uint32_t P2_8 : 2;
  uint32_t P2_9 : 2;
  uint32_t P2_10 : 2;
  uint32_t P2_11 : 2;
  uint32_t P2_12 : 2;
  uint32_t P2_13 : 2;
} ULPC_PINSEL4_t;

ULPC_PINSEL4_t *ULPC_PINSEL4 = (ULPC_PINSEL4_t *)0x4002C010;

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC->CLKSRCSEL = 1;

  // We need to input and output samples at 44.1khz
  // And the ADC samples with
  // Run PLL 0 at 44.1MHz
  PLL_init(147, 8, 10);
  SystemCoreClockUpdate();

  TFT_init();

  TFT_fill(0xffff);
  TFT_box_outline(1, 1, 237, 317, 2, 0xFF00);

  TFT_string(FONT_16x16, "SD Card Music", 5, 5, 0, 0xFFFF, 16, 16);

  touch_init();

  // Peripheral power (Note: DAC is always powered)
  LPC_SC->PCONP |= PC_ADC | PC_GPDMA;

  // Undivided peripheral clock for DAC (bits 23:22)
  LPC_SC->PCLKSEL0 |= (1 << 22);

  // Configure pins
  //   P0.23 as AD0.0 (1 at bit 14)
  //   P0.26 as AOUT  (2 at bit 20)
  LPC_PINCON->PINSEL1 = (1 << 14) | (2 << 20);
  //   P2.13 as EINT3 (1 at bit 26)
  LPC_PINCON->PINSEL4 = (1 << 26);
  RECORDING_LED_OUTPUT();
  PLAYING_LED_OUTPUT();
  RECORD_BUTTON_INPUT();
  PLAY_BUTTON_INPUT();

  sd_init();

  NVIC_DisableIRQ(EINT3_IRQn);
  LPC_GPIOINT->IO2IntEnF = _BV(13);	// EXTINT on P2.13 triggers on falling edge
  LPC_SC->EXTMODE = _BV(3);	  // EINT3 is edge senstive
  LPC_SC->EXTPOLAR = 0;				// EINT3 is falling edge sensitive
  LPC_SC->EXTINT = _BV(3);		// clear interrupt
  NVIC_EnableIRQ(EINT3_IRQn);

  // We want to sample at 44.1khz, and a full sample takes 65 cycles,
  // so we want an ADC clock of 44,100*65 = 2,866,500.
  //
  // A/D Control Register
  //  1 in bit 0 - Select AD0.0 to be sampled
  //       bits 15:8 - Set clock to 2,866,500MHz
  //  1 in bit 16 - Enable burst mode
  //  1 in bit 21 - Not in power-down mode
  //  0 in bits 26:24 - don't start a conversion yet
  LPC_ADC->ADCR = _BV(0) | ((((SystemCoreClock/2866500)/4)) << 8)
                 | _BV(16) | _BV(21);

  // A/D Interrupt Enable Register
  //  1 in bit 0 - Interrupt when conversion on ADC channel 0 completes
  LPC_ADC->ADINTEN = _BV(0);

  // DAC Control Register
  //  Enable counter (1 at bit 2)
  //  Enable DMA (1 at bit 3)
  LPC_DAC->DACCTRL = _BV(2) | _BV(3);

  // DAC Counter Value
  //  44.1MHz / 1000 = 44.1kHz
  LPC_DAC->DACCNTVAL = (1000 - 1);

  LPC_GPDMA->DMACConfig |= 1;

  // Setup two nodes to play audio and loop around each other
  playback_node1.sourceAddr = (uint32_t) audio_buffer1;
  playback_node1.destAddr = (uint32_t) &(LPC_DAC->DACR);
  playback_node1.nextNode = (uint32_t) &playback_node2;

  playback_node2.sourceAddr = (uint32_t) audio_buffer2;
  playback_node2.destAddr = (uint32_t) &(LPC_DAC->DACR);
  playback_node2.nextNode = (uint32_t) &playback_node1;

  // Linked List control information (Same as DMA channel control register)
  //  Transfer size: AUDIO_BUFFER_LEN (bits 11:0)
  //  Source burst size: 32 (4, bits 14:12)
  //  Destination burst size: 1 (0, bits 17:15)
  //  Source transfer width: word (2, bits 20:18)
  //  Destination transfer width: word (2, bits 23:21)
  //  Source increment: increment (1, bit 26)
  //  Destination increment: don't increment (0, bit 27)
  //  Terminal count interrupt: enabled (bit 31)
  playback_node1.dmaControl = playback_node2.dmaControl =
      AUDIO_BUFFER_LEN
    | (4 << 12)
    | (2 << 18) | (2 << 21) | _BV(26)
    | _BV(31);

  // Setup two nodes to record audio and loop around each other
  record_node1.sourceAddr = (uint32_t) &(LPC_ADC->ADDR0);
  record_node1.destAddr = (uint32_t) audio_buffer1;
  record_node1.nextNode = (uint32_t) &record_node2;

  record_node2.sourceAddr = (uint32_t) &(LPC_ADC->ADDR0);
  record_node2.destAddr = (uint32_t) audio_buffer2;
  record_node2.nextNode = (uint32_t) &record_node1;

  // Linked List control information (Same as DMA channel control register)
  //  Transfer size: AUDIO_BUFFER_LEN (bits 11:0)
  //  Source burst size: 1 (0, bits 14:12)
  //  Destination burst size: 32 (4, bits 17:15)
  //  Source transfer width: word (2, bits 20:18)
  //  Destination transfer width: word (2, bits 23:21)
  //  Source increment: don't increment (0, bit 26)
  //  Destination increment: increment (1, bit 27)
  //  Terminal count interrupt: enabled (bit 31)
  record_node1.dmaControl = record_node2.dmaControl =
    AUDIO_BUFFER_LEN
    | (4 << 15)
    | (2 << 18) | (2 << 21) | _BV(27)
    | _BV(31);

  //  Leave channel disabled (0 at bit 0)
  //  Source peripheral: ADC (4, bits  5:1)
  //  Destination peripheral: memory (default, bits  10:6)
  //  Transfer Type: peripheral-to-memory (2, bits 13:11)
  //  Enable terminal count interrupt (bit 15)
  RECORD_CHANNEL->DMACCConfig = (4 << 1) | (2 << 11) | _BV(15);

  //  Leave channel disabled (0 at bit 0)
  //  Source peripheral: memory (default, bits  5:1)
  //  Destination peripheral: DAC (7, bits  10:6)
  //  Transfer Type: memory-to-peripheral (1, bits 13:11)
  //  Enable terminal count interrupt (bit 15)
  PLAYBACK_CHANNEL->DMACCConfig = (7 << 6) | (1 << 11) | _BV(15);

  NVIC_EnableIRQ(DMA_IRQn);

  while (1) {
    if (PLAY_BUTTON_READ()) {
      TFT_string(FONT_16x16, "PLAYING", 10, 40, 0, 0xFFFF, 16, 16);
      playback();
      TFT_string(FONT_16x16, "IDLE   ", 10, 40, 0, 0xFFFF, 16, 16);
    } else if (RECORD_BUTTON_READ()) {
      TFT_string(FONT_16x16, "RECORDING", 10, 40, 0, 0xFFFF, 16, 16);
      record();
      TFT_string(FONT_16x16, "IDLE     ", 10, 40, 0, 0xFFFF, 16, 16);
    }
  }
  return 0;
}
