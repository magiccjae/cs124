/**************************************************************************
; Project: simon.c
; Author: Student, Fall2012
;
; Description: A MSP430 assembly language program that plays the game of Simon.
;
; 1. Each round of the game starts by the LEDs flashing several times.
; 2. The colored LEDs (along with the associated tones) then flash one at
; a time in a random sequence.
; 3. The push button switches are sampled and compared with the original
; sequence of colors/tones.
; 4. The sampling the switches continues until the end of the sequence is
; successfully reached or a mistake is made.
; 5. Some congratulatory tune is played if the sequence is correct or some
; raspberry sound is output if a mistake is made.
; 6. If the complete sequence is successfully reproduced, the game is
; repeated with one additional color/tone added to the sequence.
; Otherwise, the game starts over with the default number of colors/tones.
;
; Requirements:
; Timer_B output (TB2) is used for hardware PWM of the transducer (buzzer).
; Subroutines in at least one assembly and one C file are used by your program.
; ALL subroutines must be correctly implemented using Callee-Save protocol.
;
; Bonus:
;
; -Port 1 interrupts are used to detect a depressed switch.
; -Use LCD to display round, score, highest score, and lowest score.
; -Turn on LCD backlight with any activity.
; -Turn off LCD backlight after 5 seconds of inactivity.
;
;*******************************************************************************/
//
// MSP430F2274
// .-----------------------------.
// SW1-->|P1.0^ P2.0|<->LCD_DB0
// SW2-->|P1.1^ P2.1|<->LCD_DB1
// SW3-->|P1.2^ P2.2|<->LCD_DB2
// SW4-->|P1.3^ P2.3|<->LCD_DB3
// ADXL_INT-->|P1.4 P2.4|<->LCD_DB4
// AUX INT-->|P1.5 P2.5|<->LCD_DB5
// SERVO_1<--|P1.6 (TA1) P2.6|<->LCD_DB6
// SERVO_2<--|P1.7 (TA2) P2.7|<->LCD_DB7
// | |
// LCD_A0<--|P3.0 P4.0|-->LED_1 (Green)
// i2c_SDA<->|P3.1 (UCB0SDA) (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
// i2c_SCL<--|P3.2 (UCB0SCL) (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
// LCD_RW<--|P3.3 P4.3|-->LED_4 (Red)
// TX/LED_5 (G)<--|P3.4 (UCA0TXD) (TB1) P4.4|-->LCD_BL
// RX-->|P3.5 (UCA0RXD) (TB2) P4.5|-->SPEAKER
// RPOT-->|P3.6 (A6) (A15) P4.6|-->LED 6 (R)
// LPOT-->|P3.7 (A7) P4.7|-->LCD_E
// '-----------------------------'
//
//******************************************************************************
// includes
#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include <time.h>
#include <stdio.h>
//------------------------------------------------------------------------------
// NOTE: LOOK in RBX430.h for some macros to use
//------------------------------------------------------------------------------
// defined constants
#define myCLOCK 8000000 // 8 Mhz clock
#define WDT_CTL WDT_MDLY_32 // WD configuration (Timer, SMCLK, ~32 ms)
#define WDT_CPI 32000 // WDT Clocks Per Interrupt (@1 Mhz)
#define WDT_IPS myCLOCK/WDT_CPI // WDT counts/second (32 ms)

#define TONE 2000 // beep frequency
#define DELAY 100 // beep duration--------------------changed to 50

//-----------------------------------------------------------
// external/internal prototypes
extern int rand16(void); // get random #
int getSwitch(void); // get switch pressed
void toneON(uint16 tone); // Turn on Tone
void toneOFF(void); // Turn off Tone
void delay (void); // Start delay of program
void LEDs (int number); // Set the leds based on the number passed in
void lcd_backlight(uint8 backlight);
void victory_tone(void);
//void lcd_printf(char* fmt, ...);
//-----------------------------------------------------------
// global variables
volatile int WDTSecCnt; // WDT second counter
volatile int WDT_Delay; // WDT delay counter
volatile int fivesec = 6;

//------------------------------------------------------------------------------
//gameplay delay
void small_delay()
{
    int counter = 32000;
    int i = 32000;
    for (i; i >= 0; i--)
    {
        counter--;
    }
    return;
}

//-----------------------------------------------------------
	void main(void) {
		ERROR2(lcd_init());

		RBX430_init(_8MHZ); // init board---------------------------------changed this from 1 to 8
		// Configure the Watchdog
		WDTCTL = WDT_CTL; // Set Watchdog interval
		WDTSecCnt = WDT_IPS; // set WD 1 second counter
		WDT_Delay = 0; // reset delay counter
		IE1 |= WDTIE; // enable WDT interrupt

		TBR = 0; //Timer B SMCLK, /1, up mode
		TBCTL= TBSSEL_2|ID_0|MC_1; //
		TBCCTL2 = OUTMOD_3; // output mode = set/reset
		P4DIR |= 0x2f; // P4.5 output (buzzer)
		P4SEL |= 0x20; // select alternate output (TB2) for P4.5

		__bis_SR_register(GIE); // enable interrupts


		while(1){ //Run infinitely
			int i = 0;
			int r12=0; // value used for random numbers
			for(i ; i < 4; i++) { // loop through 4 times
				int temp; // temporary value set to r12 and modified to save r12
				r12 += 21845; // if we use rand16() then we get a random sequence every time
				temp = r12; // Generate "random" number
				temp &= 0x0fff; //

				temp += TONE;
				toneON(temp); // Turn on tones
				LEDs(temp); // turn on leds
				delay(); // Delay
			}
			toneOFF(); // Turn off tone

			delay(); // get switch

			int LED_array[10];
			int sound_array[10];
			int LED_pin[4] = {1,2,4,8};
			int tone_freq[4] = {3000, 8000, 13000, 18000};
			int raspberry = 32000;
			int user_input[10];
			int round_num = 0;
			int go = 1;
			srand(time(NULL));
			do{
				int new = rand() % 4; // simon creates a new random number between 0 and 3
				int new_LED = LED_pin[new];  // that number then corresponds with a tone and LED from the above arrays
				int new_tone = tone_freq[new];

				LED_array[round_num] = new_LED;
				sound_array[round_num] = new_tone;


				for (i = 0; i <= round_num; i++) //playing the new sequence
				{
					LEDs(LED_array[i]);
					toneON(sound_array[i]);
					delay();
					LED_4_OFF;
					LED_3_OFF;
					LED_2_OFF;
					LED_1_OFF;
					toneOFF();
					delay();
				}
				toneOFF();
				//delay();

				//int input;

				for (i = 0; i <= round_num; i++)  //user input and cross check against stored combo
				{
					int input = getSwitch();
					LEDs(input);
					if (input==1)
						toneON(tone_freq[0]);
					else if (input==2)
						toneON(tone_freq[1]);
					else if (input==4)
						toneON(tone_freq[2]);
					else if (input==8){
						toneON(tone_freq[3]);
					}
					delay();
					LED_4_OFF;
					LED_3_OFF;
					LED_2_OFF;
					LED_1_OFF;
					toneOFF();
					if (input != LED_array[i])
					{
						toneON(raspberry);
						toneON(raspberry);
						delay();
						toneOFF();
						go = 0;
						break;
					}
					delay();
				}
				if (!go) break;

				victory_tone();
				round_num++;
			}while(go);
		}
	}


//------------------------------------------------------------------------------
// LEDs
void LEDs (int number){
P4OUT &= 0xf0; // bic.w #0x0f,&P4OUT Turn on led's
number &= 0x0f;
P4OUT |= number; // bis.w r12, &P4OUT Turn on led's based on value of number
return;
}
/* Note: The AND with a 0xf0 and the OR with number is because a bis and bic only
* modify the values that are changed and leave the others alone. So the And only clears
* the last 4 bits and leaves the other bits alone, and the or will only set the bits that
* are set in number, thereby creating a bis and bic in C.
*/

//------------------------------------------------------------------------------
// output tone subroutine
void toneON(uint16 tone)
{
TBCCR0 = tone; // set beep frequency/duty cycle
TBCCR2 = tone >> 1; // 50% duty cycle
return;
} // end toneON

void toneOFF (){
TBCCR0 = 0; // clear beep frequency/duty cycle
return;
} //end ToneOFF

//------------------------------------------------------------------------------
// Get Switch pressed using polling method
int getSwitch (){
int button;

do { // do-while loop used to wait until user presses switch
button = P1IN;
button &= 0x0f;
button ^=0x0f;
} while (button == 0); // button pressed? N-Loop

fivesec = 6;

return button; // return which button was pressed
}

void delay (){
WDT_Delay = DELAY;
__bis_SR_register(LPM0_bits + GIE); // enable interrupts and goto sleep (LPM0)
}

//------------------------------------------------------------------------------
// Watchdog Timer ISR
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
// decrement delay (if non-zero)
if (WDT_Delay && (--WDT_Delay == 0)) // Delay over?
__bic_SR_register_on_exit(LPM0_bits); // Y-Wake UP Processor

if (--WDTSecCnt == 0) // WDTSecCnt 0?
{ // Y-
	fivesec --;
	if(fivesec <= 0)
	{
		lcd_backlight(OFF);
	}
	else
	{
		lcd_backlight(ON);
	}
WDTSecCnt = WDT_IPS; // reset counter
LED_GREEN_TOGGLE; // toggle green LED
}
} // end WDT_ISR(void)

void victory_tone() {
int victory[10] = {6000, 7000, 6000}; // value used for random numbers
int i = 0;
for(i ; i <= 4; i++) { // loop through 4 times
toneON(victory[i]); // Turn on tones
delay(); // Delay
}
toneOFF(); // Turn off tone
return;
}
