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

;	Cycles = 33003800
;	 MCLK = 33003800 cycles / 10 seconds = 3300380 Mhz
;	  CPI = 33003800 / 22002571 = 1.500
;	   MIPS = 3300380 / 1.5 / 1000000 = 2.200
;
;*******************************************************************************
           .cdecls C,LIST, "msp430.h"       ; MSP430

counter1 .equ	100
counter2 .equ	37500
counter3 .equ   10000


;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
RESET:      mov.w   #0x0280,SP              ; init stack pointer        (2 cycles, 2 instructions)
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT					(5 cycles, 3 instructions)
            bis.b   #0x01,&P1DIR            ; set P1.0 as output		(5 cycles, 3 instructions)

mainloop:   bis.b   #0x01,&P1OUT            ; turn on LED				(5 cycles, 3 instructions)
            mov.w   #counter1,r15           ; use R15 as delay counter	(2 cycles, 2 instructions)
            mov.w	#counter2,r14			; use R14 as delay counter	(2 cycles, 2 instructions)
			mov.w   #counter3,r13			; use R13 as delay counter	(2 cycles, 2 instructions)

turnoff: 	dec.w	r13				     	; delay counter				(1 cycle, 1 instruction)
			  jnz	turnoff					; n							(2 cycles, 1 instruction)
			bic.b	#0x01,&P1OUT			; turn off P1.0				(5 cycles, 3 instructions)

keepoff:	call	#delayloop
			dec.w	r15
			  jnz	keepoff
			  jz	mainloop


delayloop:  push 	r13
			mov.w	#counter2,r13

delayloop2:	dec.w   r13                     ; subroutine of 1/10 second
              jnz   delayloop2              ; n
			pop		r13
            ret				                ; end of subroutine



;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   RESET                   ; start address
            .end
