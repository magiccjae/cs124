//******************************************************************************
//  Lab 9a - Etch-a-Sketch
//
//  Description:
//
//	"Write a C language program that uses the sampled values from two
//	potentiometers to draw (etch) lines on the LCD. Use a low-pass filter
//	to smooth out the sampled values. Program one push button to clear
//	the LCD and another to toggle the size of the drawing pen. Display
//	the pen coordinates in the lower right corner of the display."
//
//   Author:	Paul Roper, Brigham Young University
//				November 2012
//
//  Built with Code Composer Studio Version: 5.2.0.00069
//*******************************************************************************
//
#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "etch-a-sketch.h"
#include <math.h>

//--SYSTEM CONSTANTS/VARIABLES--------------------------------------------------
//
#define myCLOCK	8000000					// clock speed
#define WDT_CLK	32000					// 32 Khz WD clock (@1 Mhz)
#define	WDT_CTL	WDT_MDLY_32				// WDT SMCLK, ~32ms
#define	WDT_CPS	myCLOCK/WDT_CLK			// WD clocks / second count

#define scale(x) (long)160*x/1024	//scale the potentiometers

#define N_SAMPLES	8
#define N_SHIFT		3

#define LCDDELAY		1300
#define DEBOUNCE_CNT	20

int thickness = 1;
int THRESHOLD = 3;
int WDT_debounce_cnt = 0;
int switches = 0;
int LCDdelay = 32000;


volatile int WDT_cps_cnt;				// WD counts/second

extern const uint16 byu1_image[];				// BYU logo
extern const uint16 etch_a_sketch_image[];		// etch-a-sketch image
extern const uint16 etch_a_sketch1_image[];		// etch-a-sketch writing

void drawline (int x0, int y0, int x1, int y1)
{
	int dy = abs(y1 - y0);
	int dx = abs(x1 - x0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;

	int err = (dx > dy ? dx : -dy)/2, e2;

	for(;;)
	{
		lcd_point(x0, y0, thickness);

		if(x0 == x1 && y0 == y1)
		{
			break;
		}

		e2 = err;

		if(e2 > -dx)
		{
			err -= dy;
			x0 += sx;
		}

		if(e2 < dy)
		{
			err += dx;
			y0  += sy;
		}

	}
	lcd_backlight(ON);
	LCDdelay = LCDDELAY;
	return;

}


//--main------------------------------------------------------------------------
//

void main(void)
{
	P1IES |= 0x0f;
	P1IE |= 0x0f;
	ERROR2(RBX430_init(_8MHZ));					// init RBX430 board
	ERROR2(lcd_init());							// init lcd
	ERROR2(ADC_init());							// init a/d converter

	// configure Watchdog
	WDT_cps_cnt = WDT_CPS;						// set WD 1 second counter
	WDTCTL = WDT_CTL;							// set WC interval to ~32ms
	IE1 |= WDTIE;								// enable WDT interrupt

	__bis_SR_register(GIE);						// enable interrupts

	// update display (interrupts enabled)
	lcd_clear();								// clear LCD
	lcd_backlight(ON);							// turn on LCD backlight

/*	lcd_wordImage(etch_a_sketch_image, (160-111)/2, 45, 1);
	lcd_wordImage(byu1_image, (160-60)/2, 78, 1);
	lcd_wordImage(etch_a_sketch1_image, (160-141)/2, 12, 1);
*/

	int x0 = 0;
	int y0 = 0;

	while (1)
	{

		int i;
		int x = 0;
		for(i=0; i<N_SAMPLES; i++)
		{
			x += 1023 - ADC_read(LEFT_POT);
		}
		x += 1 << (N_SHIFT-1);
		x >>= N_SHIFT;
		int x1 = scale(x);

		int y = 0;
		for(i=0; i<N_SAMPLES; i++)
		{
			y += 1023 - ADC_read(RIGHT_POT);
		}
		y += 1 << (N_SHIFT-1);
		y >>= N_SHIFT;
		int y1 = scale(y);

		if(x0 == 0 || y0 == 0)
		{
			x0 = x1;
			y0 = y1;
		}

		lcd_cursor(110, 0);						// output coordinates
		lcd_printf("%d,%d   ", x1, y1);

		if(abs(x1-x0) > THRESHOLD || abs(y1-y0) > THRESHOLD)
		{
			drawline(x0, y0, x1, y1);
			x0 = x1;
			y0 = y1;
		}



	}
} // end main

// Port 1 ISR
#pragma vector = PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
	LCDdelay = LCDDELAY;
	P1IFG &= ~0x0f;						// P1.0-3 IFG cleared
	WDT_debounce_cnt = DEBOUNCE_CNT;	// enable debounce
}
// end Port_1_ISR

//--Watchdog Timer ISR----------------------------------------------------------
//
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
	LCDdelay--;
	if(LCDdelay <= 0)
	{
		lcd_backlight(OFF);
	}
	else
	{
		lcd_backlight(ON);
	}
// Check for switch debounce

	if(WDT_debounce_cnt && (--WDT_debounce_cnt == 0))
	{
		switches = (P1IN ^ 0x0f) & 0x0f;
		if(switches == 1)
		{
			lcd_clear();
		}
		if(switches == 2)
		{
			if(thickness == 1)
			{
				thickness = 3;
			}
			else
			{
				thickness = 1;
			}
		}
	}

	if (--WDT_cps_cnt == 0)						// 1 second?
	{
		WDT_cps_cnt = WDT_CPS;					// reset counter
		LED_GREEN_TOGGLE;						// toggle green LED
	}
	return;
} // end WDT_ISR
