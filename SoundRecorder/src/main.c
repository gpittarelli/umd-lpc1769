/*
 ===============================================================================
 Name        : main.c
 Author      :
 Version     :
 Description : main definition
 ===============================================================================
 */

#include "main.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

uint32_t audio_buffer[AUDIO_BUFFER_LEN];

__attribute__ ((section(".ahb_ram")))
DMALinkedListNode dma_ll_pool[DMA_LL_POOL_SIZE];

uint_fast8_t waiting_for_input = 1;

void DMA_IRQHandler(void) {

  waiting_for_input = 0;
}

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // We need to input and output samples at 44.1khz
  // And the ADC samples with
  // Run PLL 0 at 44.1MHz
  PLL_init(147, 8, 10);

  // Peripheral power (Note: DAC is always powered)
  LPC_SC->PCONP = PC_ADC | PC_GPDMA;

  // Configure pins
  //   P0.23 as AD0.0 (1 at bit 14)
  //   P0.26 as AOUT  (2 at bit 20)
  LPC_PINCON->PINSEL1 = (1 << 14) | (2 << 20);
  STATUS_LED_OUTPUT();
  RECORD_BUTTON_INPUT();
  PLAY_BUTTON_INPUT();

  // We want to sample at 44.1khz, and a full sample takes 65 cycles,
  // so we want an ADC clock of 44,100*65 = 2,866,500.
  //
  // A/D Control Register
  //  1 in bit 0 - Select AD0.0 to be sampled
  //       bits 15:8 - Set clock to 2,866,500MHz
  //  0 in bit 16 - Disable burst mode (enabled later)
  //  1 in bit 21 - Not in power-down mode
  //  0 in bits 26:24 - don't start a conversion yet
  LPC_ADC->ADCR = _BV(0) | (((CLOCK_SPEED/2866500) - 1) << 1) | _BV(21);

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

  // Build up linked lists, for recording and playback
  // store 'pointers' *_ll_node as indexes into the array
  uint_fast16_t ll_idx = 0;
  DMALinkedListNode *playback_node, *record_node;

  // TODO: Move these loops to a DMA library file

  // Setup DMA Linked List for recording audio
  uint32_t sound_buf_pos = (uint32_t) audio_buffer;
  uint_fast16_t transfers_this_node;
  uint_fast16_t sound_buf_remaining = AUDIO_BUFFER_LEN;
  playback_node = &(dma_ll_pool[ll_idx]);
  while ((sound_buf_remaining > 0) && (ll_idx < DMA_LL_POOL_SIZE)) {
    dma_ll_pool[ll_idx].sourceAddr = (uint32_t) sound_buf_pos;
    dma_ll_pool[ll_idx].destAddr = (uint32_t) &(LPC_DAC->DACR);
    dma_ll_pool[ll_idx].nextNode = (uint32_t) (&(dma_ll_pool[ll_idx + 1]));

    // Transfer a page if possible, or whatever remains of the buffer
    // if less than a page is available
    transfers_this_node = MIN(0xfff, sound_buf_remaining);

    // Linked List control information (Same as DMA channel control register)
    //  Transfer size: transfers_this_node (bits 11:0)
    //  Source burst size: 32 (4, bits 14:12)
    //  Destination burst size: 1 (0, bits 17:15)
    //  Source transfer width: word (2, bits 20:18)
    //  Destination transfer width: word (2, bits 23:21)
    //  Source increment: increment (1, bit 26)
    //  Destination increment: don't increment (0, bit 27)
    //  Terminal count interrupt: disabled (0, bit 31)
    dma_ll_pool[ll_idx].dmaControl = transfers_this_node
                                    | (4 << 12)
                                    | (2 << 18) | (2 << 21) | _BV(26);

    sound_buf_pos += transfers_this_node;
    sound_buf_remaining -= transfers_this_node;
    ll_idx++;
  }

  // Set last node to terminate the transfer
  dma_ll_pool[ll_idx - 1].nextNode = 0;

  // Setup DMA Linked List for playing audio
  sound_buf_pos = (uint32_t) audio_buffer;
  sound_buf_remaining = AUDIO_BUFFER_LEN;
  record_node = &(dma_ll_pool[ll_idx]);
  while ((sound_buf_remaining > 0) && (ll_idx < DMA_LL_POOL_SIZE)) {
    dma_ll_pool[ll_idx].sourceAddr = (uint32_t) &(LPC_ADC ->ADDR0);
    dma_ll_pool[ll_idx].destAddr = (uint32_t) sound_buf_pos;
    dma_ll_pool[ll_idx].nextNode = (uint32_t) &(dma_ll_pool[ll_idx+1]);

    // Transfer a page if possible, or whatever remains of the buffer
    // if less than a page is available
    transfers_this_node = MIN(0xfff, sound_buf_remaining);

    // Linked List control information (Same as DMA channel control register)
    //  Transfer size: transfers_this_node (bits 11:0)
    //  Source burst size: 1 (0, bits 14:12)
    //  Destination burst size: 32 (4, bits 17:15)
    //  Source transfer width: word (2, bits 20:18)
    //  Destination transfer width: word (2, bits 23:21)
    //  Source increment: don't increment (0, bit 26)
    //  Destination increment: increment (1, bit 27)
    //  Terminal count interrupt: disabled (0, bit 31)
    dma_ll_pool[ll_idx].dmaControl = transfers_this_node
                                    | (4 << 15)
                                    | (2 << 18) | (2 << 21) | _BV(27);

    sound_buf_pos += transfers_this_node;
    sound_buf_remaining -= transfers_this_node;
    ll_idx++;
  }

  // Set last node to terminate the transfer
  dma_ll_pool[ll_idx - 1].nextNode = 0;

  // DMA Channel 0 Config (used for playback)
  //  Leave channel disabled (0 at bit 0)
  //  Source peripheral: 0 (default, bits  5:1)
  //  Destination peripheral: DAC (7, bits  10:6)
  //  Transfer Type: memory-to-peripheral (1, bits 13:11)
  LPC_GPDMACH0->DMACCConfig = (7 << 6) | (1 << 11);

  // DMA Channel 1 Config (used for recording)
  //  Leave channel disabled (0 at bit 0)
  //  Source peripheral: ADC (4, bits  5:1)
  //  Destination peripheral: memory (default, bits  10:6)
  //  Transfer Type: peripheral-to-memory (2, bits 13:11)
  LPC_GPDMACH1->DMACCConfig = (4 << 1) | (2 << 11);

  while (1) {
    if (
  }
  return 0;
}
