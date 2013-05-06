/*
 ===============================================================================
 Name        : sdaudio.c
 Author      : George Pittarelli
 Description :

   A simple example of reading audio samples from an SD card and then
   playing them over the DAC.

   Two buttons and a mic are wired for input, and an LED and speaker
   are used for output. One button, when pressed, triggers a DMA
   transfer from the ADC to an audio buffer and the other button
   starts a DMA transfer from the buffer to the DAC.

 ===============================================================================
 */

#include "sdaudio.h"

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

  LPC_GPDMA->DMACIntTCClear = 0xf;
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

  while (1) {
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

//  current_state = WAITING;
//  PLAYBACK_CHANNEL->DMACCConfig &= ~(_BV(0));
}

void record() {
  uint32_t cur_block = 0;

  current_state = RECORDING_BUFFER1;
  load_dma_node(PLAYBACK_CHANNEL, &record_node1);
  PLAYBACK_CHANNEL->DMACCConfig |= _BV(0);

  while (RECORD_BUTTON_READ()) {
    wait_for_state(RECORDING_BUFFER2);

    transfer_to_block(audio_buffer1, sd_block, SD_BLOCK_LEN);
    sd_write_block(sd_block, cur_block++);

    wait_for_state(PLAYING_BUFFER1);

    transfer_to_block(audio_buffer2, sd_block, SD_BLOCK_LEN);
    sd_write_block(sd_block, cur_block++);

  }

  PLAYBACK_CHANNEL->DMACCConfig &= ~(_BV(0));
}

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC->CLKSRCSEL = 1;

  // We need to input and output samples at 44.1khz
  // And the ADC samples with
  // Run PLL 0 at 44.1MHz
  PLL_init(147, 8, 10);

  // Peripheral power (Note: DAC is always powered)
  LPC_SC->PCONP |= PC_ADC | PC_GPDMA;

  // Undivided peripheral clock for DAC (bits 23:22)
  LPC_SC->PCLKSEL0 |= (1 << 22);

  // Configure pins
  //   P0.23 as AD0.0 (1 at bit 14)
  //   P0.26 as AOUT  (2 at bit 20)
  LPC_PINCON->PINSEL1 = (1 << 14) | (2 << 20);

  RECORDING_LED_OUTPUT();
  PLAYING_LED_OUTPUT();
  RECORD_BUTTON_INPUT();
  PLAY_BUTTON_INPUT();

  sd_init();

  // We want to sample at 44.1khz, and a full sample takes 65 cycles,
  // so we want an ADC clock of 44,100*65 = 2,866,500.
  //
  // A/D Control Register
  //  1 in bit 0 - Select AD0.0 to be sampled
  //       bits 15:8 - Set clock to 2,866,500MHz
  //  0 in bit 16 - Disable burst mode (enabled later)
  //  1 in bit 21 - Not in power-down mode
  //  0 in bits 26:24 - don't start a conversion yet
  LPC_ADC->ADCR = _BV(0) | (((CLOCK_SPEED/2866500) - 1) << 1)
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

  playback();

  return 0;
}
