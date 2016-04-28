			.title	"morse.asm"
;*******************************************************************************
;     Project:  morse.asm
;      Author:  Jaehun Lee
;				I wrote this code
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;   Revisions:
;
;              RBX430-1                                    eZ430 Rev C
;              OPTION A                                     OPTION B
;           .------------.                               .------------.
;     SW1-->|P1.0^   P3.0|-->LCD_A0         (RED) LED<--|P1.0     P3.0|-->LCD_RST
;     SW2-->|P1.1^   P3.1|<->SDA          (GREEN) LED<--|P1.1     P3.1|<->SDA
;     SW3-->|P1.2^   P3.2|-->SCL                   NC-->|P1.2     P3.2|-->SCL
;     SW4-->|P1.3^   P3.3|-->LCD_RW                NC-->|P1.3     P3.3|-->LED_3 (Y)
;    INT1-->|P1.4    P3.4|-->LED_5 (GREEN)         NC-->|P1.4     P3.4|<--NC
;    INTA-->|P1.5    P3.5|<--RX                    NC-->|P1.5     P3.5|<--NC
;    SVO1<--|P1.6    P3.6|<--RPOT                  NC-->|P1.6     P3.6|<--NC
;    SVO2<--|P1.7    P3.7|<--LPOT                  NC-->|P1.7     P3.7|<--NC
;           |            |                              |             |
;  LCD_D0<->|P2.0    P4.0|-->LED_1 (G)           SW_1-->|P2.0^    P4.0|<--NC
;  LCD_D1<->|P2.1    P4.1|-->LED_2 (O)           SW_2-->|P2.1^    P4.1|<--NC
;  LCD_D2<->|P2.2    P4.2|-->LED_3 (Y)           SW_3-->|P2.2^    P4.2|<--NC
;  LCD_D3<->|P2.3    P4.3|-->LED_4 (R)           SW_4-->|P2.3^    P4.3|<--RPOT
;  LCD_D4<->|P2.4    P4.4|-->LCD_BL            LCD_BL<--|P2.4     P4.4|<--LPOT
;  LCD_D5<->|P2.5    P4.5|-->SPEAKER               NC-->|P2.5     P4.5|-->SPEAKER
;  LCD_D6<->|P2.6    P4.6|-->LED_6 (RED)    (G) LED_1<--|P2.6     P4.6|-->LED_4 (R)
;  LCD_D7<->|P2.7    P4.7|-->LCD_E          (O) LED_2<--|P2.7     P4.7|<--NC
;           .------------.                               .------------.
;
;*******************************************************************************
            .cdecls C,LIST,"msp430.h"       ; include c header

;------------------------------------------------------------------------------
;   System equates
myCLOCK     .equ    1200000                 ; 1.2 Mhz clock
WDT_CTL     .equ    WDT_MDLY_0_5            ; WD configuration (Timer, SMCLK, 0.5 ms)
WDT_CPI     .equ    500                     ; WDT Clocks Per Interrupt (@1 Mhz)
WDT_IPS     .equ    myCLOCK/WDT_CPI         ; WDT Interrupts Per Second
STACK       .equ    0x0600                  ; top of stack
END         .equ    0
DOT         .equ    1
DASH        .equ    2

;------------------------------------------------------------------------------
;   External references
            .ref    numbers                 ; codes for 0-9
            .ref    letters                 ; codes for A-Z
letterTb:	.set	letters-'A'*2
numberTb:	.set	numbers-'0'*2

;  numbers--->N0$--->DASH,DASH,DASH,DASH,DASH,END      ; 0
;             N1$--->DOT,DASH,DASH,DASH,DASH,END       ; 1
;             ...
;             N9$--->DASH,DASH,DASH,DASH,DOT,END       ; 9
;
;  letters--->A$---->DOT,DASH,END                      ; A
;             B$---->DASH,DOT,DOT,DOT,END              ; B
;             ...
;             Z$---->DASH,DASH,DOT,DOT,END             ; Z

;	Morse code is composed of dashes and dots, or phonetically, "dits" and "dahs".
;	There is no symbol for a space in Morse, though there are rules when writing them.

;	1. One dash is equal to three dots
;	2. The space between parts of the letter is equal to one dot
;	3. The space between two letters is equal to three dots
;	4. The space between two words is equal to seven dots.

;	5 WPM = 60 sec / (5 * 50) elements = 240 milliseconds per element.
;	element = (WDT_IPS * 6 / WPM) / 5

;	Morse Code equates
ELEMENT     .equ    WDT_IPS*240/1000
DEBOUNCE	.equ	10
                            
;------------------------------------------------------------------------------
;	Global variables						; RAM section
            .bss    beep_cnt,2              ; beeper flag
            .bss    delay_cnt,2             ; delay flag
            .bss	WDT_dcnt,2				; WDT second counter
            .bss	switches,2				; switches

;------------------------------------------------------------------------------
;   Program section
            .text                           ; program section
message:    .string "HELLO CS 124 WORLD "    ; message
            .byte   0
            .align  2						; align on word boundary

RESET:      mov.w   #STACK,SP               ; initialize stack pointer
            mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
            mov.b   #WDTIE,&IE1             ; enable WDT interrupt
            bis.b   #0x20,&P4DIR            ; set P4.5 as output (speaker)
            clr.w   beep_cnt                ; clear counters
            clr.w   delay_cnt
            bis.w   #GIE,SR                 ; enable interrupts

            bis.b	#0x10,&P3DIR			; green LED reset
            bis.b	#0x40,&P4DIR			; red LED reset
			bic.b	#0x40,&P4OUT			; red LED turn off
			bic.b	#0x10,&P3OUT			; green LED turn off
			mov.w	#WDT_IPS,r13

			bic.b	#0x0f,&P1DIR
			bis.b	#0x0f,&P1OUT
			bis.b	#0x0f,&P1REN
			bis.b	#0x0f,&P1IES
			bis.b	#0x0f,&P1IE



;   output 'A' in morse code
loop: 		mov.w	#message,r4				; point to message

loop02:		mov.b	@r4+,r5					; get character
			cmp		#0x00,r5				; check NULL
			 jeq	loop
			cmp		#0x41,r5				; letter or number?
			 jge	letter
			add.w	r5,r5
			mov.w	numberTb(r5),r5			; get pointer to number codes
			 jmp	loop10

letter:		add.w	r5,r5					; make word index
			mov.w	letterTb(r5),r5			; get pointer to letter codes

loop10:		mov.b	@r5+,r6					; get DOT, DASH, or END
			cmp.b	#DOT,r6					; dot?
			 jne	loop11
			call	#dotloop
			jmp 	loop10

loop11:		cmp.b	#DASH,r6
			 jne	loop12
			call	#dashloop
			jmp		loop10

loop12:		call	#spaceloop
			jmp		loop02

dotloop:    mov.w   #ELEMENT,r15            ; output DOT
            call    #beep
            mov.w   #ELEMENT,r15            ; delay 1 element
            call    #delay
            ret

dashloop:   mov.w   #ELEMENT*3,r15          ; output DASH
            call    #beep
            mov.w   #ELEMENT,r15            ; delay 1 element
            call    #delay
			ret

spaceloop:  mov.w   #ELEMENT*7,r15          ; output space
            call    #delay                  ; delay
            ret


;   beep r15 ticks of the watchdog timer
beep:       mov.w   r15,beep_cnt            ; start beep

beep02:     tst.w   beep_cnt                ; beep finished?
              jne   beep02                  ; n
            ret                             ; y

;   delay r15 ticks of the watchdog timer
delay:      mov.w   r15,delay_cnt           ; start delay

delay02:    tst.w   delay_cnt               ; delay done?
              jne   delay02                 ; n
            ret                             ; y

P1_ISR:		bic.b	#0x0f,&P1IFG
			mov.w	#DEBOUNCE,WDT_dcnt
			reti

;------------------------------------------------------------------------------
;   Watchdog Timer interrupt service routine
;
WDT_ISR:	dec.w	r13
			  jz	greenblink
		    tst.w   beep_cnt                ; beep on?
              jeq   WDT_02                  ; n
            dec.w   beep_cnt                ; y, decrement count
            xor.b   #0x20,&P4OUT            ; beep using 50% PWM
            xor.b	#0x40,&P4OUT			; turn on red LED

            tst.w	WDT_dcnt
             jeq	WDT_10
            dec.w	WDT_dcnt
             jne	WDT_10
            mov.b	&P1IN,switches
            xor.b	#0x0f,switches
            and.b	#0x0f,switches
			 jz		WDT_02
			xor.b	#0x20,&P4DIR

WDT_02:	    tst.w   delay_cnt               ; delay?
              jeq   WDT_10                  ; n
            dec.w   delay_cnt               ; y, decrement count

WDT_10:     reti                            ; return from interrupt

greenblink:	xor.b	#0x10,&P3OUT			; toggle green LED
			mov.w	#WDT_IPS,r13			; reset r13
			 jmp	WDT_ISR

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
			.sect	".int02"
			.word	P1_ISR

            .sect   ".int10"                ; Watchdog Vector
            .word   WDT_ISR                 ; Watchdog ISR

            .sect   ".reset"                ; PUC Vector
            .word   RESET                   ; RESET ISR
            .end
