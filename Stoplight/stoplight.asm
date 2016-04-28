;*******************************************************************************
;   CS/ECEn 124 Lab 1 - blinky.asm: Software Toggle P1.0
;
;   Description: Toggle P1.0 by xor'ing P1.0 inside of a software loop.
;
;             MSP430G5223
;             -----------
;            |       P1.0|-->LED1-RED LED
;            |       P1.3|<--S2
;            |       P1.6|-->LED2-GREEN LED
;
;*******************************************************************************
           .cdecls C,LIST, "msp430.h"       ; MSP430

counter1 .equ	50
counter2 .equ	10
counter3 .equ	2
counter4 .equ	100
onetenth .equ	37500
six		 .equ	6
twenty	 .equ	20


;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
RESET:      mov.w   #0x0280,SP              ; init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x41,&P1DIR            ; set P1.0&P1.6 as output

mainloop:   bis.b	#0x40,&P1OUT			; turn on green LED
			bic.b	#0x01,&P1OUT			; turn off red LED
			mov.w	#counter1,r15

greenon:	call	#delayloop				; call subroutine
			dec.w	r15
			  jnz	greenon

			mov.w	#six,r14				; 6 times

greenlong:	xor.w	#0x40,&P1OUT
			mov.w	#counter2,r15

greenlong1:	call	#delayloop				; 1 second
			dec.w	r15
			  jnz	greenlong1

greenlong2:
			dec.w	r14
			  jnz	greenlong

			mov.w	#twenty,r14				; 20 times

greenshort:	xor.w	#0x40,&P1OUT
			mov.w	#counter3,r15

greenshort1:call	#delayloop
			dec.w	r15
			  jnz	greenshort1

greenshort2:
			dec.w	r14
			  jnz	greenshort

			mov.w	#counter4,r15

redon:
			bic.b	#0x40,&P1OUT
			bis.b	#0x01,&P1OUT
			call	#delayloop
			dec.w	r15
			  jnz	redon
			  jz	mainloop

delayloop:  push 	#onetenth

delayloop2:	dec.w   0(SP)                     ; subroutine of 1/10 second
              jnz   delayloop2              ; n
			pop		r13
            ret				                ; end of subroutine

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   RESET                   ; start address
            .end
