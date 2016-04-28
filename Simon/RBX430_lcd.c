//	RBX430_lcd.c
//******************************************************************************
//******************************************************************************
//	LL        CCCCC     DDCDDD
//	LL       CC   CC    DD   DD
//	LL      CC          DD    DD
//	LL      CC          DD    DD
//	LL      CC          DD    DD
//	LL       CC   CC    DD   DD
//	LLLLLL    CCCCC     DDDDDD
//******************************************************************************
//******************************************************************************
//	Author:			Paul Roper
//	Revision:		1.0	03/05/2012	RBX430-1
//					1.1				divu8, image1, image2
//					1.2	09/17/2012	fill fixes
//					1.3	11/07/2012	lcd_bitImage, lcd_wordImage
//
//	Description:	Controller firmware for YM160160C/ST7529 LCD
//
//	Built with CCSv5.2 w/cgt 3.0.0
//******************************************************************************
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "msp430x22x4.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"

static uint16 lcd_dmode;			// lcd mode
static uint8 lcd_y;					// row (0-159)
static uint8 lcd_x;					// column (0-159)

extern uint16 i2c_fSCL;				// i2c timing constant

#define DELAY_1MS	1000

void WriteCmd(uint8 c);
int ReadData(void);
void WriteData(uint8 c);
void WriteData_word(uint16 data);
void DelayMs(uint16 time);

//******************************************************************************
//******************************************************************************
//	Function: void lcd_init()
//
//	PreCondition: none
//	Input: none
//	Output: none
//	Side Effects: none
//	Overview: resets LCD, initializes PMP
//
//	Note: Sitronix	ST7529 controller drive
//					1/160 Duty, 1/13 Bias
//
//	initialization Sitronix ST7529 constants
#define VOP_CODE	335				// vop = 14.0v
#define LCD_DELAY	50				// 50 ms

uint8 lcd_init(void)
{
	int i;
	LCD_RW_H;					// set RW high (read)
	LCD_A0_H;					// set A0 high (data)
	DelayMs(LCD_DELAY);

	// Hold in reset
	LCD_A0_L;					// set A0 low (command) RESET
	DelayMs(LCD_DELAY);

	// Release from reset
	LCD_A0_H;					// set A0 high (data)
	DelayMs(LCD_DELAY);

	WriteCmd(0x30);				// Ext = 0

	WriteCmd(0x94);				// Sleep Out

	WriteCmd(0xd1);				// OSC On
	WriteCmd(0x20);				// Power Control Set
		WriteData(0x08);		// Booster Must Be On First
	DelayMs(2);

	WriteCmd(0x20);				// Power Control Set
		WriteData(0x0b);		// Booster, Regulator, Follower ON

	WriteCmd(0x81);				// Electronic Control
		WriteData(VOP_CODE & 0x3f);
		WriteData(VOP_CODE >> 6);

	WriteCmd(0xca);				// Display Control
		WriteData(0x00);		// CLD=0
		WriteData(0x27);		// Duty=(160/4-1)=39
		WriteData(0x00);		// FR Inverse-Set Value ???

	WriteCmd(0xa6);				// Normal Display

	WriteCmd(0xbb);				// COM Scan Direction
		WriteData(0x01);		// 0->79 159->80

	WriteCmd(0xbc);				// Data Scan Direction
		WriteData(0x01);		// CI=0, LI=1
		WriteData(0x01);		// CLR=1 (P3/P2/P1)
		WriteData(0x01);		// 2B3P

	WriteCmd(0x31);				// Ext = 1

	WriteCmd(0x20);				// set gray level for odd frames
	for (i = 0; i < 32; i += 2) WriteData(i);
	WriteCmd(0x21);				// set gray level for even frames
	for (i = 1; i < 32; i += 2) WriteData(i);

	WriteCmd(0x32);				// Analog Circuit Set
		WriteData(0x00);		// OSC Frequency =000 (Default)
		WriteData(0x01);		// Booster Efficiency=01(Default)
		WriteData(0x01);		// Bias=1/13

	WriteCmd(0x34);				// Software init

	WriteCmd(0x30);				// Ext = 0
	WriteCmd(0xaf);				// Display On

	lcd_dmode = 0;
	lcd_y = HD_Y_MAX - 1;
	lcd_x = 0;					// column (0-159)
	return 0;
} // end  lcd_init


//******************************************************************************
//	Sitronix ST7529 controller functions
//
//	void WriteCmd(uint8 c)
//	int ReadData(void)
//	void WriteData(uint8 c)
//	void WriteData_word(uint16 w)
//
//		A0	RW	A0 + ~RW	Function
//		--	--	--------	-----------------
//		0	0	   1		Control Write
//		0	1	   0		Control Read (Reset)
//		1	0      1		Display Write
//		1	1	   1		Display Read
//
//
void WriteCmd(uint8 c)
{
	P2DIR = 0xff;		// output to P2
	P2OUT = c;			// set data on output lines
	LCD_RW_L;			// set RW low (write)
	LCD_A0_L;			// set A0 low (command)
//	LCD_CS_L;			// drop CS
	LCD_E_H;			// toggle E
	LCD_E_L;
//	LCD_CS_H;			// raise CS
	return;
} // end WriteCmd


int ReadData(void)
{
	int data;

	P2DIR = 0x00;		// input from P2
	LCD_A0_H;			// set A0 high (data)
	LCD_RW_H;			// set RW high (read)
//	LCD_CS_L;			// drop CS
	LCD_E_H;			// toggle E
	_no_operation();	// nop
	data = P2IN;		// read data
	LCD_E_L;
//	LCD_CS_H;			// raise CS
	return data;
} // end ReadData


void WriteData(uint8 c)
{
	P2DIR = 0xff;		// output to P2
	P2OUT = c;			// set data on output lines
	LCD_RW_L;			// set RW low (write)
	LCD_A0_H;			// set A0 high (data)
//	LCD_CS_L;			// drop CS
	LCD_E_H;			// toggle E
	LCD_E_L;
//	LCD_CS_H;			// raise CS
	return;
} // end WriteData


void WriteData_word(uint16 data)
{
	P2DIR = 0xff;		// output to P2
	P2OUT = data >> 8;	// set data on output lines
	LCD_RW_L;			// set RW low (write)
	LCD_A0_H;			// set A0 high (data)
	LCD_E_H;			// toggle E
	LCD_E_L;

	_no_operation();	// nop
	P2OUT = data;		// set data on output lines
	LCD_E_H;			// toggle E
	LCD_E_L;
	return;
} // end WriteData_word


//******************************************************************************
//	Function: void DelayMs(WORD time)
//
//	PreCondition: none
//	Input: time - delay in ms
//	Output: none
//	Side Effects: none
//	Overview: delays execution on time specified in ms
//
//	Note: none
//
//******************************************************************************

void DelayMs(uint16 time)
{
	uint16 delay;

	while (time--) for (delay = i2c_fSCL * DELAY_1MS; delay > 0; --delay);
	return; 
} // end DelayMs


//******************************************************************************
//	fast uint8 / 3
//
//	q = (n >> 2) + (n >> 4);		// q = n*0.0101 (approx).
//	q += (q >> 4);					// q = n*0.01010101.
//	q += (q >> 8);					// (not needed for uint8/3)
//	r = n - q * 3;					// 0 <= r <= 15.
//	return q + ((11 * r) >> 5);		// Returning q + r/3.
//
//	See Hacker's Delight, Henry S. Warren, Jr., 10-3
//
unsigned divu3(unsigned n)
{
	unsigned q, r, t;
	q = (n >> 2);					// q = n*0.0101 (approx).
	q += (q >> 2);
	t = (q >> 2);					// q = n*0.01010101.
	q += (t >> 2);
	r = n - ((q << 1) + q);			// 0 <= r <= 15.
	t = r << 3;						// Returning q + r/3.
	return q + ((r + r + r + t) >> 5);
} // end divu3


//******************************************************************************
//	set lcd x, y
//
void lcd_set_x_y(uint8 x, uint8 y)
{
	WriteCmd(0x75);					// set line address
		WriteData(y);				// from line 0 - 159
		WriteData(0x9f);

	WriteCmd(0x15);					// set column address
		WriteData(divu3(x));		// from col 0 - 160/3
		WriteData(0x35);
	return;
} // end lcd_set_x_y


//******************************************************************************
//	lcd read word
//
uint16 lcd_read_word(int16 column, int16 row)
{
	WriteCmd(0x75);					// set line address
		WriteData(row);				// from line 0 - 159
		WriteData(0x9f);

	WriteCmd(0x15);					// set column address
		WriteData(column);			// from col 0 - 160
		WriteData(0x35);
	WriteCmd(0x5d);					// RAMRD - read from memory
	ReadData();						// Dummy read
	return (ReadData() << 8) + ReadData();
} // end lcd_read_word


//******************************************************************************
//	lcd write word
//
void lcd_write_word(int16 column, int16 row, uint16 data)
{
	WriteCmd(0x75);					// set line address
		WriteData(row);				// from line 0 - 159
		WriteData(0x9f);

	WriteCmd(0x15);					// set column address
		WriteData(column);			// from col 0 - 160
		WriteData(0x35);
	WriteCmd(0x5c);					// RAMWR - write to memory
	WriteData(data >> 8);			// write high byte
	WriteData(data & 0x00ff);		// write low byte
	return;
} // end lcd_write_word


//******************************************************************************
//	clear lcd screen
//
void lcd_clear()
{
	lcd_set(0xffdf);				// clear lcd
} // end lcd_clear


//******************************************************************************
//	set lcd screen
//
void lcd_set(uint16 value)
{ 
	int i; 

	lcd_set_x_y(0, 0);			// upper right corner
	WriteCmd(0x5c);				// start write

	// whole screen - rows x columns
//	for (i = 0; i < 160 * (divu3((160*2))); i++)
	for (i = 160 * (divu3((160*2))); i > 0; --i)
	{
		WriteData_word(value);
	} 
	lcd_dmode = 0;				// reset mode
	lcd_y = HD_Y_MAX - 1;		// upper left hand corner
	lcd_x = 0;
	return;
} // end  lcd_set


//******************************************************************************
//	Display Image Functions:
//
//	uint8 lcd_image(const uint8* image, int16 x, int16 y)
//	uint8 lcd_bitImage(const uint8* image, int16 x, int16 y, uint8 flag)
//	uint8 lcd_wordImage(const uint16* image, int16 x, int16 y, uint8 flag)
//
//******************************************************************************
//	uint8 lcd_image(const uint8* image, int16 x, int16 y)
//
//	IN:		const char* image ->	uint8 width
//									uint8 height
//									(8-bit column value) x width
//									...
//									... height % 8 rows.
//
//	OUT:	Return 0
//
uint8 lcd_image(const uint8* image, int16 x, int16 y)
{
	int16 x1, y1, data, mask;
	int16 right = x + *image++;			// stop at right side of image
	int16 bottom = y;					// finish at bottom
	y += *image++;						// get top of image

	while (y > bottom)					// display from top down
	{
		for (x1 = x; x1 < right; ++x1)	// display from left to right
		{
			data = *image++;			// get image byte
			y1 = y;
			for (mask = 0x80; mask; mask >>= 1)
			{
				if (data & mask) lcd_point(x1, --y1, 1);
				else if (1) lcd_point(x1, --y1, 0);
			}
		}
		y -= 8;							// next row
	}	
	return 0;
} // end lcd_image


//******************************************************************************
//	output LCD B/W bit image
//
//	flag = 0	blank image
//	       1	output LCD RAM image
//	       2	fill image area
//
//	IN:		const char* image ->	uint8 width
//									uint8 height
//									(8-bit row value) x (width % 8)
//									...
//									... height rows.
//
//			x position must be divisible by 3
//
//	OUT:	Return 0
//
uint8 lcd_bitImage(const uint8* image, int16 x, int16 y, uint8 flag)
{
	int16 i, data, index;
	uint8 bits, mask;
	int16 width = *image++;				// get width/height
	int16 height = *image++;
	int16 bottom = y;

	x += width - 1;						// move to top, left (make 0 based)
	y += height;

	while (y > bottom)					// display from top down
	{
		lcd_set_x_y(159 - x, y);		// upper right corner
		WriteCmd(0x5c);					// write to memory

		data = 0x0000;					// fill
		switch (flag)
		{
			case 0:
				data = 0xffdf;			// erase

			case 2:
			{
				for (i = width; i > 0; i -= 3)	// display from right to left
				{
					WriteData_word(data);
				}
			}
			break;

			default:
			case 1:
			{
				image += (width >> 3);		// point to end of image line
				mask = 0x80;
				data = 0xffdf;				// assume all off
				index = 0;

				for (i = width; i > 0; --i)		// display from right to left
				{
					mask <<= 1;					// adjust mask
					if (mask == 0)
					{
						mask = 0x01;			// reset mask
						bits = *--image;		// get next data byte
					}
					if (bits & mask)
					{
						if (index == 0) data &= 0xffe0;
						else if (index == 1) data &= 0xf83f;
						else data &= 0x07df;
					}
					if (++index == 3)
					{
						WriteData_word(data);
						data = 0xffdf;			// assume all off
						index = 0;
					}
				}
				if (index) WriteData_word(data);	// flush data
				image += (width >> 3);			// point to end of image line
			}
			break;
		}
		y--;									// next row
	}
	return 0;
} // end lcd_bitImage


//******************************************************************************
//	output 4-bit gray-scale LCD ram image (3 4-bit pixels / word)
//
//	flag = 0	blank image
//	       1	output LCD RAM image
//	       2	fill image area
//
//	IN:		const uint16* image ->	uint16 width
//									uint16 height
//									(16-bit LCD RAM value) x (width / 3) R to L order
//									...
//									... height rows.
//
//	OUT:	Return 0;
//
//	Note:	1. x position must be divisible by 3
//			2. 0x--FF used to compress 0 values
//			3. RAM value = #define M2B3P(P0,P1,P2) ((0xc0*P1|0x1f*P2)^0xff),((0Xf8*P0|0x07*P1)^0xff)
//			4. ccf- = special run of 3 pixels
//				ccff = run of 3 pixels off
//				ccfe = run of 3 pixels on
//				ccf0,pppp = run of pppp pixels
//
uint8 lcd_wordImage(const uint16* image, int16 x, int16 y, uint8 flag)
{
	int16 x1;
	int16 runCnt = 0;
	uint16 runPixels;

	int16 width = *image++;				// get width/height
	int16 height = *image++;
	int16 bottom = y;

	x += width - 1;						// move to top, left (make 0 based)
	y += height;
//	width = divu3((width + 2));			// 3 pixels per 2 bytes (round up)

	while (y > bottom)					// display from top down
	{
		lcd_set_x_y(159 - x, y);		// upper right corner
		WriteCmd(0x5c);					// write to memory

//		for (x1 = width; x1 > 0; --x1)			// display from right to left
		for (x1 = width; x1 > 0; x1 -= 3)			// display from right to left
		{
			switch (flag)						// switch on mode
			{
				case 0:
					WriteData_word(0xffdf);		// write 3 pixels off
					break;

				case 1:
					// check for run of 3 pixels
					if (runCnt)
					{
						--runCnt;
						WriteData_word(runPixels);		// output 3 run pixels
						break;
					}

					// check for special code (ccfx)
					if (*image & 0x0020)
					{
						runCnt = *image++;				// get special code
						switch (runCnt & 0x00ff)		// switch to special case
						{
							case 0x00ff:
								runPixels = 0xffdf;		// 3 pixels off
								break;

							case 0x00fe:
								runPixels = 0x0000;		// 3 pixels on
								break;

							case 0x00f0:
							default:
								runPixels = ~*image++;	// run of 3 pixels
								break;
						}
						runCnt >>= 8;					// get run count
//						++x1;						// setup 1st output
						x1 += 3;						// setup 1st output
						break;
					}

					WriteData_word(~*image++);
					break;

				default:
				case 2:
					WriteData_word(0x0000);		// write 3 pixels on (fill)
					break;
			}
		}
		y--;									// next row
	}
	return 0;
} // end lcd_wordImage


//******************************************************************************
//	Fill Image
//
//	IN:		x, y			lower right coordinates
//			width,height	area to blank
//			flag = 0		blank image
//	    		   1		(unused)
//	    		   2		fill image area
//
//	OUT:	return 0;
//
uint8 lcd_fill(int16 x, int16 y, uint16 width, uint16 height, uint8 flag)
{
	uint16 area[2];
	area[0] = width;
	area[1] = height;
	return lcd_wordImage(area, x, y, flag);
}


//******************************************************************************
//	Blank Image
//
//	IN:		x, y			lower right coordinates
//			width,height	area to blank
//
//	OUT:	return 0;
//
uint8 lcd_blank(int16 x, int16 y, uint16 width, uint16 height)
{
	int16 x0, y0;

	for (x0 = x + width - 1; x0 >= x; --x0)
	{
		for (y0 = y + height - 1; y0 >= y; --y0)
		{
			lcd_point(x0, y0, 0);
		}
	}
	return 0;
} // end lcd_blank


//******************************************************************************
//	change lcd volume (brightness)
//
void lcd_volume(uint16 volume)
{
	WriteCmd(0x81);						// Electronic Control
		WriteData(volume & 0x3f);
		WriteData(volume >> 6);
	return;
} // end lcd_volume


//******************************************************************************
//	Turn ON/OFF LCD backlight
//
void lcd_backlight(uint8 backlight)
{
	if (backlight)
	{
		BACKLIGHT_ON;					// turn on backlight
	}
	else
	{
		BACKLIGHT_OFF;					// turn off backlight
	}
	return;
} // end lcd_backlight


//******************************************************************************
//	Display Mode
//
//	IN:		mode
//	OUT:	old mode
//
//	xxxx xxxx xxxx xxxx
//	           \\\\ \\\\___ LCD_PROPORTIONAL		proportional font
//	            \\\\ \\\___ LCD_REVERSE_FONT		reverse font
//	             \\\\ \\___ LCD_2X_FONT				2x font
//	              \\\\ \___ LCD_FRAM_CHARACTER		write to FRAM
//	               \\\\____ LCD_REVERSE_DISPLAY		reverse display
//	                \\\____
//		             \\____
//	                  \____
//
//	~mode = Turn OFF mode bit(s)
//
uint16 lcd_mode(int16 mode)
{
	if (mode)
	{
		// set/reset mode bits
		if (mode > 0) lcd_dmode |= mode;	// set mode bits
		else lcd_dmode &= mode;				// reset mode bits
	}
	else
	{
		lcd_dmode = 0;
	}
	return lcd_dmode;
} // end lcd_mode


//******************************************************************************
//	access lcd point at x,y
//
//	IN:		x = column coordinate
//			y = row coordinate
//			flag	0 = turn single point off
//					1 = turn single point on
//					2 = turn double point off
//					3 = turn double point on
//					4 =
//					5 =
//					6 =
//					7 =
//					8 =
//					9 =
//					10 =
//				   -1 = read point (0 or 1)
//
//	flag =	0000 0000
//	         \\\\ \\\\
//	          \\\\ \\\\_ 0=erase, 1=draw
//	           \\\\ \\\_ 1=double
//	            \\\\ \\_ 1=triple
//	             \\\\ \_ 1=
//	              \\\\
//	               \\\\_ 0=no fill, 1=fill
//	                \\\_
//	                 \\_
//	                  \_ 1 = read
//
//										-ooo-	ooooo
//										ooooo	ooooo
//						-o-		ooo		ooXoo	ooXoo
//				oo		oXo		oXo		ooooo	ooooo
//		o		Xo		-o-		ooo		-ooo-	ooooo
//
//		0/1		2/3		4/5		6/7		8/9		10/11	12/13	14/15
//
//	return results
//
uint8 lcd_point(int16 x, int16 y, int16 flag)
{
	uint8 pixel1, pixel2;

	// return 1 if out of range
	if ((x < 0) || (x >= HD_X_MAX)) return 1;
	if ((y < 0) || (y >= HD_Y_MAX)) return 1;

	if (flag < 0)
	{
		flag = 128;						// set flag = 0x80 to read
	}
	else
	{
		uint16 on_off = flag & 0x01;
		flag &= 0x000f;
		switch (flag)
		{
			case 10:
			case 11:
				lcd_point(x-2, y+2, on_off);	//	ooooo
				lcd_point(x+2, y+2, on_off);	//	ooooo
				lcd_point(x-2, y-2, on_off);	//	ooXoo
				lcd_point(x+2, y-2, on_off);	//	ooooo
												//	ooooo
			case 8:
			case 9:
				lcd_point(x-1, y+2, on_off);	//	-ooo-
				lcd_point(x, y+2, on_off);		//	ooooo
				lcd_point(x+1, y+2, on_off);	//	ooXoo
												//	ooooo
				lcd_point(x-2, y+1, on_off);	//	-ooo-
				lcd_point(x+2, y+1, on_off);
				lcd_point(x-2, y, on_off);
				lcd_point(x+2, y, on_off);
				lcd_point(x-2, y-1, on_off);
				lcd_point(x+2, y-1, on_off);

				lcd_point(x-1, y-2, on_off);
				lcd_point(x, y-2, on_off);
				lcd_point(x+1, y-2, on_off);


			case 6:								// double point on/off
			case 7:
				lcd_point(x-1, y+1, on_off);	//	ooo
				lcd_point(x+1, y+1, on_off);	//	oXo
				lcd_point(x-1, y-1, on_off);	//	ooo
				lcd_point(x+1, y-1, on_off);

			case 4:								// double point on/off
			case 5:
				lcd_point(x, y+1, on_off);		//	-o-
				lcd_point(x-1, y, on_off);		//	oXo
				lcd_point(x, y, on_off);		//	-o-
				lcd_point(x+1, y, on_off);
				lcd_point(x, y-1, on_off);
				return 0;

			case 2:								// double point on/off
			case 3:
				lcd_point(x, y+1, on_off);		//	oo
				lcd_point(x, y, on_off);		//	Xo
				lcd_point(x+1, y+1, on_off);
				lcd_point(x+1, y, on_off);
				return 0;

			default:							// mask flag to ON or OFF
				flag &= 0x0001;
				break;
		}
	}

	// translate point
	x = 159 - x;
//	y = 159 - y;

	lcd_set_x_y(x, y);					// upper right corner

	// read point
	WriteCmd(0xe0);						// RMWIN - read and modify write
	ReadData();							// Dummy read
	pixel1 = ReadData();				// Start read cycle for pixel 2/1
	pixel2 = ReadData();				// Start read cycle for pixel 1/0

	{
		// process point
		uint16 xd3 = divu3(x);
		xd3 = x - xd3 - xd3 - xd3;		// x - divu3(x) * 3
		switch (flag)
		{
			case 0:						// turn point off
				switch (xd3)
				{
					case 0:
						pixel2 |= 0x1f;
						break;

					case 1:
						pixel1 |= 0x07;
						pixel2 |= 0xc0;
						break;

					case 2:
					default:
						pixel1 |= 0xf8;
				}
				break;

			case 1:					// turn point on
				switch (xd3)
				{
					case 0:
						pixel2 &= 0xc0;
						break;

					case 1:
						pixel1 &= 0xf8;
						pixel2 &= 0x1f;
						break;

					case 2:
					default:
						pixel1 &= 0x07;
				}
				break;

			default:
			case 128:					// read point
			switch (xd3)
			{
				case 0:
					return (pixel2 & 0x1f) ? 1 : 0;

				case 1:
					return ((pixel1 & 0x07) && (pixel2 & 0xc0)) ? 1 : 0;

				case 2:
					default:
					return (pixel1 & 0xf8) ? 1 : 0;
			}
		}
	}
	WriteData(pixel1);					// Write pixels back
	WriteData(pixel2);

	WriteCmd(0xee);						// RMWOUT - cancel read modify write mode
	return 0;							// return success
} // end lcd_point


//******************************************************************************
//	draw circle of radius r0 and center x0,y0
//
void lcd_circle(int16 x0, int16 y0, uint16 r0, uint8 pen)
{
	int16 x, y, d;
	int16 i, j;

	x = x0;
	y = y0 + r0;
	d =  3 - r0 * 2;

	do
	{
		if (pen & 0x04)
		{
			for (i = x0 - (x - x0); i <= x; ++i)
    		{
        		lcd_point(i, y, pen);
    			lcd_point(i,  y0 - (y - y0), pen);
    		}
    		for (j = y0 - (x - x0); j <= y0 + (x - x0); ++j)
    		{
    			for (i = x0 - (y - y0); i <= x0 + (y - y0); ++i)
    			{
    				lcd_point(i, j, pen);
    			}
    		}
		}
		else
		{
			lcd_point(x, y, pen);
    		lcd_point(x, y0 - (y - y0), pen);
    		lcd_point(x0 - (x - x0), y, pen);
    		lcd_point(x0 - (x - x0), y0 - (y - y0), pen);

    		lcd_point(x0 + (y - y0), y0 + (x - x0), pen);
    		lcd_point(x0 + (y - y0), y0 - (x - x0), pen);
    		lcd_point(x0 - (y - y0), y0 + (x - x0), pen);
    		lcd_point(x0 - (y - y0), y0 - (x - x0), pen);
		}
		if (d < 0)
		{
			d = d +  ((x - x0) << 2) + 6;
		}
		else
		{
			d = d + (((x - x0) - (y - y0)) << 2) + 10;
			--y;
		}
		++x;
	} while ((x - x0) <= (y - y0));
	return;
} // end lcd_circle


//******************************************************************************
//	draw square of radius r0 and center x0,y0
//
//	pen =	0000 0000
//	         \\\\ \\\\
//	          \\\\ \\\\_ 0=erase, 1=draw
//	           \\\\ \\\_ 0=single, 1=double
//	            \\\\ \\_ 0=no fill, 1=fill
//
void lcd_square(int16 x0, int16 y0, uint16 r0, uint8 pen)
{
	lcd_rectangle(x0 - r0, y0 - r0, r0 + r0, r0 + r0, pen);
	return;
} // end lcd_square


//******************************************************************************
//	draw star of radius r0 and center x0,y0
//
//	pen =	0000 0000
//	         \\\\ \\\\
//	          \\\\ \\\\_ 0=erase, 1=draw
//	           \\\\ \\\_ 0=single, 1=double
//	            \\\\ \\_ 0=no fill, 1=fill
//
void lcd_star(int16 x0, int16 y0, uint16 r0, uint8 pen)
{
	int16  x, y, r;

//	lcd_triangle(x0, y0 - (r0 * 5 / 6), r0 * 5 / 6, pen);
//	lcd_triangle(x0, y0 + (r0 * 5 / 6), r0 * 5 / 6, pen | 0x08);


	y = y0 + r0;					// start at top
//	r = r0 << 2;					// radius * 4
	r = 0;
	do
	{
		for (x = (x0 - r/4); x <= (x0 + r/4); ++x)
		{
			lcd_point(x, y, pen);
		}
		--y;
	} while ((r += 6) <= r0 << 2);

	do
	{
		for (x = (x0 - r/4); x <= (x0 + r/4); ++x)
		{
			lcd_point(x, y, pen);
		}
		--y;
		--r;
	} while (y >= (y0 - r0));

	return;
} // end lcd_star


//******************************************************************************
//	draw triangle of radius r0 and center x0,y0
//
//	pen =	0000 0000
//	         \\\\ \\\\
//	          \\\\ \\\\_ 0=erase, 1=draw
//	           \\\\ \\\_ 0=single, 1=double
//	            \\\\ \\_ 0=no fill, 1=fill
//			     \\\\ \_ 0=
//
void lcd_triangle(int16 x0, int16 y0, uint16 r0, uint8 pen)
{
	int16  x, y;

	if ((pen & 0x08) == 0)
	{
		y = y0 - r0;					// start at bottom
		r0 <<= 1;						// radius * 2
		do
		{
			for (x = (x0 - r0/2); x <= (x0 + r0/2); ++x)
			{
				lcd_point(x, y, pen);
			}
			++y;
		} while (r0--);
	}
	else
	{
		y = y0 + r0;					// start at top
		r0 <<= 1;						// radius * 2
		do
		{
			for (x = (x0 - r0/2); x <= (x0 + r0/2); ++x)
			{
				lcd_point(x, y, pen);
			}
			--y;
		} while (r0--);
	}
	return;
} // end lcd_triangle


//******************************************************************************
//	draw rectangle at lower left (x0,y0) of width w, height h
//
//	pen =	0000 0000
//	         \\\\ \\\\
//	          \\\\ \\\\_ 0=erase, 1=draw
//	           \\\\ \\\_ 0=single, 1=double
//	            \\\\ \\_ 0=no fill, 1=fill
//
void lcd_rectangle(int16 x, int16 y, uint16 w, uint16 h, uint8 pen)
{
	int16  x0, y0;
	int8 fill_flag = (pen & 0x04) ? 1 : 0;
	pen &= 0x03;

	if (w-- == 0) return;
	for (y0 = y; y0 <= y + h; ++y0)
	{
		lcd_point(x, y0, pen);
		if ((y0 == y) || (y0 == y + h) || fill_flag)
		{
			for (x0 = x + 1; x0 < x + w; ++x0)
			{
				lcd_point(x0, y0, pen);
			}
		}
		lcd_point(x + w, y0, pen);
	}
	return;
} // end lcd_rectangle


//******************************************************************************
//******************************************************************************
//	ASCII character set for LCD
//
const unsigned char cs[][5] = {
  { 0x00,0x00,0x00,0x00,0x00 },  // SP ----- -OO-- OO-OO ----- -O--- OO--O -O--- -OO--
  { 0xfa,0xfa,0x00,0x00,0x00 },  // !  ----- -OO-- OO-OO -O-O- -OOO- OO--O O-O-- -OO--
  { 0xe0,0xc0,0x00,0xe0,0xc0 },  // "  ----- -OO-- O--O- OOOOO O---- ---O- O-O-- -----
  { 0x24,0x7e,0x24,0x7e,0x24 },  // #  ----- -OO-- ----- -O-O- -OO-- --O-- -O--- -----
  { 0x24,0xd4,0x56,0x48,0x00 },  // $  ----- -OO-- ----- -O-O- ---O- -O--- O-O-O -----
  { 0xc6,0xc8,0x10,0x26,0xc6 },  // %  ----- ----- ----- OOOOO OOO-- O--OO O--O- -----
  { 0x6c,0x92,0x6a,0x04,0x0a },  // &  ----- -OO-- ----- -O-O- --O-- O--OO -OO-O -----
  { 0xc0,0xc0,0x00,0x00,0x00 },  // '  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0x7c,0x82,0x00,0x00,0x00 },  // (  ---O- -O--- ----- ----- ----- ----- ----- -----
  { 0x82,0x7c,0x00,0x00,0x00 },  // )  --O-- --O-- -O-O- --O-- ----- ----- ----- ----O
  { 0x10,0x7c,0x38,0x7c,0x10 },  // #  --O-- --O-- -OOO- --O-- ----- ----- ----- ---O-
  { 0x10,0x10,0x7c,0x10,0x10 },  // +  --O-- --O-- OOOOO OOOOO ----- OOOOO ----- --O--
  { 0x07,0x06,0x00,0x00,0x00 },  // ,  --O-- --O-- -OOO- --O-- ----- ----- ----- -O---
  { 0x10,0x10,0x10,0x10,0x10 },  // -  --O-- --O-- -O-O- --O-- -OO-- ----- -OO-- O----
  { 0x06,0x06,0x00,0x00,0x00 },  // .  ---O- -O--- ----- ----- -OO-- ----- -OO-- -----
  { 0x04,0x08,0x10,0x20,0x40 },  // /  ----- ----- ----- ----- -O--- ----- ----- -----
//
//{ 0x42,0xfe,0x02,0x00,0x00 },  // 1
  { 0x7c,0x8a,0x92,0xa2,0x7c },  // 0  -OOO- --O-- -OOO- -OOO- ---O- OOOOO --OOO OOOOO
  { 0x00,0x42,0xfe,0x02,0x00 },  // 1  O---O -OO-- O---O O---O --OO- O---- -O--- ----O
  { 0x46,0x8a,0x92,0x92,0x62 },  // 2  O--OO --O-- ----O ----O -O-O- O---- O---- ---O-
  { 0x44,0x92,0x92,0x92,0x6c },  // 3  O-O-O --O-- --OO- -OOO- O--O- OOOO- OOOO- --O--
  { 0x18,0x28,0x48,0xfe,0x08 },  // 4  OO--O --O-- -O--- ----O OOOOO ----O O---O -O---
  { 0xf4,0x92,0x92,0x92,0x8c },  // 5  O---O --O-- O---- O---O ---O- O---O O---O -O---
  { 0x3c,0x52,0x92,0x92,0x8c },  // 6  -OOO- -OOO- OOOOO -OOO- ---O- -OOO- -OOO- -O---
  { 0x80,0x8e,0x90,0xa0,0xc0 },  // 7  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0x6c,0x92,0x92,0x92,0x6c },  // 8  -OOO- -OOO- ----- ----- ---O- ----- -O--- -OOO-
  { 0x60,0x92,0x92,0x94,0x78 },  // 9  O---O O---O ----- ----- --O-- ----- --O-- O---O
  { 0x36,0x36,0x00,0x00,0x00 },  // :  O---O O---O -OO-- -OO-- -O--- OOOOO ---O- O---O
  { 0x37,0x36,0x00,0x00,0x00 },  // ;  -OOO- -OOOO -OO-- -OO-- O---- ----- ----O --OO-
  { 0x10,0x28,0x44,0x82,0x00 },  // <  O---O ----O ----- ----- -O--- ----- ---O- --O--
  { 0x24,0x24,0x24,0x24,0x24 },  // =  O---O ---O- -OO-- -OO-- --O-- OOOOO --O-- -----
  { 0x82,0x44,0x28,0x10,0x00 },  // >  -OOO- -OO-- -OO-- -OO-- ---O- ----- -O--- --O--
  { 0x60,0x80,0x9a,0x90,0x60 },  // ?  ----- ----- ----- -O--- ----- ----- ----- -----
//
  { 0x7c,0x82,0xba,0xaa,0x78 },  // @  -OOO- -OOO- OOOO- -OOO- OOOO- OOOOO OOOOO -OOO-
  { 0x7e,0x90,0x90,0x90,0x7e },  // A  O---O O---O O---O O---O O---O O---- O---- O---O
  { 0xfe,0x92,0x92,0x92,0x6c },  // B  O-OOO O---O O---O O---- O---O O---- O---- O----
  { 0x7c,0x82,0x82,0x82,0x44 },  // C  O-O-O OOOOO OOOO- O---- O---O OOOO- OOOO- O-OOO
  { 0xfe,0x82,0x82,0x82,0x7c },  // D  O-OOO O---O O---O O---- O---O O---- O---- O---O
  { 0xfe,0x92,0x92,0x92,0x82 },  // E  O---- O---O O---O O---O O---O O---- O---- O---O
  { 0xfe,0x90,0x90,0x90,0x80 },  // F  -OOO- O---O OOOO- -OOO- OOOO  OOOOO O---- -OOO-
  { 0x7c,0x82,0x92,0x92,0x5c },  // G  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0xfe,0x10,0x10,0x10,0xfe },  // H  O---O -OOO- ----O O---O O---- O---O O---O -OOO-
  { 0x82,0xfe,0x82,0x00,0x00 },  // I  O---O --O-- ----O O--O- O---- OO-OO OO--O O---O
  { 0x0c,0x02,0x02,0x02,0xfc },  // J  O---O --O-- ----O O-O-- O---- O-O-O O-O-O O---O
  { 0xfe,0x10,0x28,0x44,0x82 },  // K  OOOOO --O-- ----O OO--- O---- O---O O--OO O---O
  { 0xfe,0x02,0x02,0x02,0x02 },  // L  O---O --O-- O---O O-O-- O---- O---O O---O O---O
  { 0xfe,0x40,0x20,0x40,0xfe },  // M  O---O --O-- O---O O--O- O---- O---O O---O O---O
  { 0xfe,0x40,0x20,0x10,0xfe },  // N  O---O -OOO- -OOO- O---O OOOOO O---O O---O -OOO-
  { 0x7c,0x82,0x82,0x82,0x7c },  // O  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0xfe,0x90,0x90,0x90,0x60 },  // P  OOOO- -OOO- OOOO- -OOO- OOOOO O---O O---O O---O
  { 0x7c,0x82,0x92,0x8c,0x7a },  // Q  O---O O---O O---O O---O --O-- O---O O---O O---O
  { 0xfe,0x90,0x90,0x98,0x66 },  // R  O---O O---O O---O O---- --O-- O---O O---O O-O-O
  { 0x64,0x92,0x92,0x92,0x4c },  // S  OOOO- O-O-O OOOO- -OOO- --O-- O---O O---O O-O-O
  { 0x80,0x80,0xfe,0x80,0x80 },  // T  O---- O--OO O--O- ----O --O-- O---O O---O O-O-O
  { 0xfc,0x02,0x02,0x02,0xfc },  // U  O---- O--O- O---O O---O --O-- O---O -O-O- O-O-O
  { 0xf8,0x04,0x02,0x04,0xf8 },  // V  O---- -OO-O O---O -OOO- --O-- -OOO- --O-- -O-O-
  { 0xfc,0x02,0x3c,0x02,0xfc },  // W  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0xc6,0x28,0x10,0x28,0xc6 },  // O  O---O O---O OOOOO -OOO- ----- -OOO- --O-- -----
  { 0xe0,0x10,0x0e,0x10,0xe0 },  // Y  O---O O---O ----O -O--- O---- ---O- -O-O- -----
  { 0x86,0x8a,0x92,0xa2,0xc2 },  // Z  -O-O- O---O ---O- -O--- -O--- ---O- O---O -----
  { 0xfe,0x82,0x82,0x00,0x00 },  // [  --O-- -O-O- --O-- -O--- --O-- ---O- ----- -----
  { 0x40,0x20,0x10,0x08,0x04 },  // \  -O-O- --O-- -O--- -O--- ---O- ---O- ----- -----
  { 0x82,0x82,0xfe,0x00,0x00 },  // ]  O---O --O-- O---- -O--- ----O ---O- ----- -----
  { 0x20,0x40,0x80,0x40,0x20 },  // ^  O---O --O-- OOOOO -OOO- ----- -OOO- ----- OOOOO
  { 0x02,0x02,0x02,0x02,0x02 },  // _  ----- ----- ----- ----- ----- ----- ----- -----
//
  { 0xc0,0xe0,0x00,0x00,0x00 },  // `  -OO-- ----- O---- ----- ----O ----- --OOO -----
  { 0x04,0x2a,0x2a,0x2a,0x1e },  // a  -OO-- ----- O---- ----- ----O ----- -O--- -----
  { 0xfe,0x22,0x22,0x22,0x1c },  // b  --O-- -OOO- OOOO- -OOO- -OOOO -OOO- -O--- -OOOO
  { 0x1c,0x22,0x22,0x22,0x14 },  // c  ----- ----O O---O O---O O---O O---O OOOO- O---O
  { 0x1c,0x22,0x22,0x22,0xfc },  // d  ----- -OOOO O---O O---- O---O OOOO- -O--- O---O
  { 0x1c,0x2a,0x2a,0x2a,0x10 },  // e  ----- O---O O---O O---O O---O O---- -O--- -OOOO
  { 0x10,0x7e,0x90,0x90,0x80 },  // f  ----- -OOOO OOOO- -OOO- -OOOO -OOO- -O--- ----O
  { 0x18,0x25,0x25,0x25,0x3e },  // g  ----- ----- ----- ----- ----- ----- ----- -OOO-
//
  { 0xfe,0x10,0x10,0x10,0x0e },  // h  O---- -O--- ----O O---- O---- ----- ----- -----
  { 0xbe,0x02,0x00,0x00,0x00 },  // i  O---- ----- ----- O---- O---- ----- ----- -----
  { 0x02,0x01,0x01,0x21,0xbe },  // j  O---- -O--- ---OO O--O- O---- OO-O- OOOO- -OOO-
  { 0xfe,0x08,0x14,0x22,0x00 },  // k  OOOO- -O--- ----O O-O-- O---- O-O-O O---O O---O
  { 0xfe,0x02,0x00,0x00,0x00 },  // l  O---O -O--- ----O OO--- O---- O-O-O O---O O---O
  { 0x3e,0x20,0x18,0x20,0x1e },  // m  O---O -O--- ----O O-O-- O---- O---O O---O O---O
  { 0x3e,0x20,0x20,0x20,0x1e },  // n  O---O -OO-- O---O O--O- OO--- O---O O---O -OOO-
  { 0x1c,0x22,0x22,0x22,0x1c },  // o  ----- ----- -OOO- ----- ----- ----- ----- -----
//
  { 0x3f,0x22,0x22,0x22,0x1c },  // p  ----- ----- ----- ----- ----- ----- ----- -----
  { 0x1c,0x22,0x22,0x22,0x3f },  // q  ----- ----- ----- ----- -O--- ----- ----- -----
  { 0x22,0x1e,0x22,0x20,0x10 },  // r  OOOO- -OOOO O-OO- -OOO- OOOO- O--O- O---O O---O
  { 0x12,0x2a,0x2a,0x2a,0x04 },  // s  O---O O---O -O--O O---- -O--- O--O- O---O O---O
  { 0x20,0x7c,0x22,0x22,0x04 },  // t  O---O O---O -O--- -OOO- -O--- O--O- O---O O-O-O
  { 0x3c,0x02,0x04,0x3e,0x00 },  // u  O---O O---O -O--- ----O -O--O O-OO- -O-O- OOOOO
  { 0x38,0x04,0x02,0x04,0x38 },  // v  OOOO- -OOOO OOO-- OOOO- --OO- -O-O- --O-- -O-O-
  { 0x3c,0x06,0x0c,0x06,0x3c },  // w  O---- ----O ----- ----- ----- ----- ----- -----
//
  { 0x22,0x14,0x08,0x14,0x22 },  // x  ----- ----- ----- ---OO --O-- OO--- -O-O- -OO--
  { 0x39,0x05,0x06,0x3c,0x00 },  // y  ----- ----- ----- --O-- --O-- --O-- O-O-- O--O-
  { 0x26,0x2a,0x2a,0x32,0x00 },  // z  O---O O--O- OOOO- --O-- --O-- --O-- ----- O--O-
  { 0x10,0x7c,0x82,0x82,0x00 },  // {  -O-O- O--O- ---O- -OO-- ----- --OO- ----- -OO--
  { 0xee,0x00,0x00,0x00,0x00 },  // |  --O-- O--O- -OO-- --O-- --O-- --O-- ----- -----
  { 0x82,0x82,0x7c,0x10,0x00 },  // }  -O-O- -OOO- O---- --O-- --O-- --O-- ----- -----
  { 0x40,0x80,0x40,0x80,0x00 },  // ~  O---O --O-- OOOO- ---OO --O-- OO--- ----- -----
  { 0x60,0x90,0x90,0x60,0x00 }   // _  ----- OO--- ----- ----- ----- ----- ----- -----
//{ 0x02,0x06,0x0a,0x06,0x02 }   // _
};


//******************************************************************************
//	set lcd cursor position
//
//	Description: set the position at which the next character will be printed.
//
uint8 lcd_cursor(int16 x, int16 y)
{
	lcd_x = ((x >= 0) && (x < HD_X_MAX)) ? x : HD_X_MAX-1;
	lcd_y = ((y >= 0) && (y < HD_Y_MAX)) ? y : HD_Y_MAX-1;
	return 0;
} // end lcd_cursor


//******************************************************************************
//	write data to LCD
//
//	lcd_y = lower left-hand corner
//	lcd_x = column
//
#if 0
void lcd_WD(uint8 datum)
{
	int i;
	int y = lcd_y + CHAR_SIZE - 1;

	if (lcd_dmode & LCD_REVERSE_FONT) datum = ~datum;

	for (i=0x80; i; i >>= 1, --y)
	{
		if (i & datum)
		{
			lcd_point(lcd_x, y, 1);
		}
		else
		{
			if (!(lcd_dmode & LCD_OR_CHAR))
				lcd_point(lcd_x, y, 0);
		}
//		--y;
	}
	return;
} // end lcd_WD
#else
void lcd_WD(uint8 datum, uint16 y)
{
	int i;
	y += CHAR_SIZE - 1;

	if (lcd_dmode & LCD_REVERSE_FONT) datum = ~datum;

	for (i=0x80; i; i >>= 1, --y)
	{
		if (i & datum)
		{
			lcd_point(lcd_x, y, 1);
		}
		else
		{
			if (!(lcd_dmode & LCD_OR_CHAR))
				lcd_point(lcd_x, y, 0);
		}
//		--y;
	}
	return;
} // end lcd_WD
#endif


//******************************************************************************
//	write character to LCD
//
unsigned char lcd_putchar(unsigned char c)
{
//	int i;
	uint16 i;

	switch (c)
	{
		case '\a':
		{
			lcd_dmode |= LCD_REVERSE_FONT;
			break;
		}

		case '\n':
		{
			lcd_y = (lcd_y - CHAR_SIZE * (lcd_dmode & ~LCD_2X_FONT ? 2 : 1)) % HD_Y_MAX;
		}

		case '\r':
		{
		 	lcd_x = 0;
		 	break;
		}

		default:
		{
			if ((c >= ' ') && (c <= '~'))
			{
				if (lcd_dmode & LCD_2X_FONT)
				{
					uint16 lcd_y2 = (lcd_y + CHAR_SIZE) % HD_Y_MAX;

					// leading space
 					lcd_WD(0x00, lcd_y2);
 					lcd_WD(0x00, lcd_y);
					if (++lcd_x >= HD_X_MAX) lcd_x = 0;

					for (i = 0; i < 5; ++i)
					{
						unsigned char mask1 = 0x01;
						unsigned int mask2 = 0x0001;
						unsigned int data = 0;

						while (mask1)
						{
							// double bits into data
							if (cs[c - ' '][i] & mask1) data |= mask2 | (mask2 << 1);
							mask1 <<= 1;
							mask2 <<= 2;
						}
	 					lcd_WD(data >> 8, lcd_y2);
			 			lcd_WD(data & 0x00ff, lcd_y);
						if (++lcd_x >= HD_X_MAX) lcd_x = 0;

	 					lcd_WD(data >> 8, lcd_y2);
			 			lcd_WD(data & 0x00ff, lcd_y);
						if (++lcd_x >= HD_X_MAX) lcd_x = 0;

						if (i && (lcd_dmode & LCD_PROPORTIONAL) && !cs[c - ' '][i]) break;
					}
					// trailing space
 					lcd_WD(0x00, lcd_y2);
 					lcd_WD(0x00, lcd_y);
					if (++lcd_x >= HD_X_MAX) lcd_x = 0;
				}
				else
				{
					lcd_WD(0x00, lcd_y);					// leading space
					for (i = 0; i < 5; )
					{
						lcd_WD(cs[c - ' '][i++], lcd_y);	// output character
						if (++lcd_x >= HD_X_MAX) lcd_x = 0;

						// check proportional flag
						if (i && (lcd_dmode & LCD_PROPORTIONAL) && !cs[c - ' '][i]) break;
					}
					lcd_WD(0x00, lcd_y);					// trailing space
					if (++lcd_x >= HD_X_MAX) lcd_x = 0;
				}
			}
		}
	}
	return c;
} // end my_putchar


//******************************************************************************
//	formatted print to lcd
//
uint16 lcd_printf(const char* fmt, ...)
{
	char printBuffer[PRINT_BUFFER_SIZE+1];
	char* s_ptr = printBuffer;
	uint16 s_length = 0;
	va_list arg_ptr;

	if (strlen(fmt) > PRINT_BUFFER_SIZE) ERROR2(SYS_ERR_PRINT);

	va_start(arg_ptr, fmt);					// create pointer to args
	vsprintf(s_ptr, fmt, arg_ptr);			// generate print string
	while (*s_ptr)
	{
		lcd_putchar(*s_ptr++);				// output string
		s_length++;
	}
	va_end(arg_ptr);						// destroy arg pointer
	return s_length;
} // end lcd_printf
