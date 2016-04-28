			.title	"morse_codes.asm"
;*******************************************************************************
;     Project:  L06 Morse Code
;      Author:  Student Code
;
; Description:  International Morse Code
;
;*******************************************************************************

		.def	numbers
		.def	letters
		.def	MPYU
		.def	DIVU

;  numbers--->N0$--->DASH,DASH,DASH,DASH,DASH,END      ; 0
;             N1$--->DOT,DASH,DASH,DASH,DASH,END       ; 1
;             ...
;             N9$--->DASH,DASH,DASH,DASH,DOT,END       ; 9

;  letters--->A$---->DOT,DASH,END                      ; A
;             B$---->DASH,DOT,DOT,DOT,END              ; B
;             ...
;             Z$---->DASH,DASH,DOT,DOT,END             ; Z



;  numbers--->[.word N0$]--->[N0$: .byte DASH,DASH,DASH,DASH,DASH,END]     ; 0
;             [.word N1$]--->[N1$: .byte DOT,DASH,DASH,DASH,DASH,END]      ; 1
;                ...
;             [.word N9$]--->[N9$: .byte DASH,DASH,DASH,DASH,DOT,END]      ; 9

;  letters--->[.word A$]---->[A$: .byte DOT,DASH,END]                      ; A
;             [.word B$]---->[B$: .byte DASH,DOT,DOT,DOT,END]              ; B
;                ...
;             [.word Z$]---->[Z$: .byte DASH,DASH,DOT,DOT,END]             ; Z


;------------------------------------------------------------------------------
;	Morse Code equates
END         .equ    0
DOT         .equ    1
DASH        .equ    2

numbers:
		.word	N0$
		.word	N1$
		.word	N2$
		.word	N3$
		.word	N4$
		.word	N5$
		.word	N6$
		.word	N7$
		.word	N8$
		.word	N9$

letters:
		.word	A$
		.word	B$
		.word	C$
		.word	D$
		.word	E$
		.word	F$
		.word	G$
		.word	H$
		.word	I$
		.word	J$
		.word	K$
		.word	L$
		.word	M$
		.word	N$
		.word	O$
		.word	P$
		.word	Q$
		.word	R$
		.word	S$
		.word	T$
		.word	U$
		.word	V$
		.word	W$
		.word	X$
		.word	Y$
		.word	Z$

N0$:	.byte	DASH,DASH,DASH,DASH,DASH,END	; 0
N1$:	.byte	DOT,DASH,DASH,DASH,DASH,END		; 1
N2$:	.byte	DOT,DOT,DASH,DASH,DASH,END		; 2
N3$:	.byte	DOT,DOT,DOT,DASH,DASH,END		; 3
N4$:	.byte	DOT,DOT,DOT,DOT,DASH,END		; 4
N5$:	.byte	DOT,DOT,DOT,DOT,DOT,END			; 5
N6$:	.byte	DASH,DOT,DOT,DOT,DOT,END		; 6
N7$:	.byte	DASH,DASH,DOT,DOT,DOT,END		; 7
N8$:	.byte	DASH,DASH,DASH,DOT,DOT,END		; 8
N9$:	.byte	DASH,DASH,DASH,DASH,DOT,END		; 9

A$:		.byte	DOT,DASH,END					; A
B$:		.byte	DASH,DOT,DOT,DOT,END			; B
C$:		.byte	DASH,DOT,DASH,DOT,END			; C
D$:		.byte	DASH,DOT,DOT,END				; D
E$:		.byte	DOT,END							; E
F$:		.byte	DOT,DOT,DASH,DOT,END			; F
G$:		.byte	DASH,DASH,DOT,END				; G
H$:		.byte	DOT,DOT,DOT,DOT,END				; H
I$:		.byte	DOT,DOT,END						; I
J$:		.byte	DOT,DASH,DASH,DASH,END			; J
K$:		.byte	DASH,DOT,DASH,END				; K
L$:		.byte	DOT,DASH,DOT,DOT,END			; L
M$:		.byte	DASH,DASH,END					; M
N$:		.byte	DASH,DOT,END					; N
O$		.byte	DASH,DASH,DASH,END				; O
P$:		.byte	DOT,DASH,DASH,DOT,END			; P
Q$:		.byte	DASH,DASH,DOT,DASH,END			; Q
R$:		.byte	DOT,DASH,DOT,END				; R
S$:		.byte	DOT,DOT,DOT,END					; S
T$:		.byte	DASH,END						; T
U$:		.byte	DOT,DOT,DASH,END				; U
V$:		.byte	DOT,DOT,DOT,DASH,END			; V
W$:		.byte	DOT,DASH,DASH,END				; W
X$:		.byte	DASH,DOT,DOT,DASH,END			; X
Y$:		.byte	DASH,DOT,DASH,DASH,END			; Y
Z$:		.byte	DASH,DASH,DOT,DOT,END			; Z
		.align	2

;------------------------------------------------------------------------------
; Integer Subroutines Definitions: Software Multiply
; See SLAA024 - MSP430 Family Mixed-Signal Microcontroller Application Reports
;
; UNSIGNED MULTIPLY: r6|r7 = r4 x r5
;
MPYU:	clr.w	r7				; 0 -> LSBs RESULT
		clr.w	r6				; 0 -> MSBs RESULT

; UNSIGNED MULTIPLY AND ACCUMULATE: r6|r7 = (r4 x r5) + r6|r7
;
MACU:	push	r4
		push	r5
		push	r8
		clr.w	r8				; MSBs MULTIPLIER

L$01:	bit.w	#1,r4			; TEST ACTUAL BIT 5-4
		  jz	L$02			; IF 0: DO NOTHING
		add.w	r5,r7			; IF 1: ADD MULTIPLIER TO RESULT
		addc.w	r8,r6

L$02:	rla.w	r5				; MULTIPLIER x 2
		rlc.w	r8				;
		rrc.w	r4				; NEXT BIT TO TEST
		  jnz	L$01			; IF BIT IN CARRY: FINISHED

L$03:	pop		r8
		pop		r5
		pop		r4
		ret


;------------------------------------------------------------------------------
; Integer Subroutines Definitions: Software Divide
; See SLAA024 - MSP430 Family Mixed-Signal Microcontroller Application Reports
;
; UNSIGNED DIVISION SUBROUTINE 32–BIT BY 16–BIT
; UNSIGNED DIVIDE: r5 R r4 = r4|r5 / r6
; RETURN: CARRY = 0: OK CARRY = 1: QUOTIENT > 16 BITS
;
DIVU:	push	r7
		push	r8
		clr.w	r7				; clear result
		mov.w	#17,r8			; initialize loop counter

D$01:	cmp.w	r6,r4			; subtrahend < minuhend?
		  jlo	D$02			; n
		sub.w	r6,r4			; y, subtract out

D$02:	rlc.w	r7				; next bit, overflow?
		  jc	D$04			; y, result > 16 bits
		dec.w	r8				; n, decrement loop counter, done?
		  jz	D$03			; y, terminate w/o error
		rla.w	r5				; n,
		rlc.w	r4				; adjust result, enter bit in result?
		  jnc	D$01			; n
		sub.w	r6,r4			; y
		setc
		jmp		D$02

D$03:	clrc					; no error, c = 0

D$04:	mov.w	r7,r5			; return r5 = r4|r5 / r6
		pop		r8
		pop		r7
		ret						; error indication in c

		.end
