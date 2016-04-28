//*******************************************************************************
//   CS/ECEn 124 Lab 1 - blinky.asm: Software Toggle P1.0
//
//   Description: Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//
//             MSP430G5223
//             -----------
//            |       P1.0|-->LED1-RED LED
//            |       P1.3|<--S2
//            |       P1.6|-->LED2-GREEN LED
//
//*******************************************************************************

#include <msp430.h>

volatile unsigned int i;                    // volatile to prevent optimization

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P1DIR |= 0x01;                            // Set P1.0 to output direction

  for (;;)
  {
    P1OUT ^= 0x01;                          // Toggle P1.0 using exclusive-OR
    i = 32768;                                  // Delay
    while (--i);
  }
}
