/*
 ____    ____    ____       _     __       __     ____
/\  _`\ /\  _`\ /\  _`\   /' \  /'__`\   /'__`\  /'___\
\ \,\L\_\ \,\L\_\ \ \/\ \/\_, \/\_\L\ \ /\ \/\ \/\ \__/
 \/_\__ \\/_\__ \\ \ \Small Scale_/_\_<_\ \ \ \ \ \  _``\
   /\ \L\ \/\ \L\ \ \ \_\ \ \ \ \/\ \L\ \\ \ \_\ \ \ \L\ \
   \ `\____\ `\____\ \____/  \ \_\ \____/ \ \____/\ \____/
    \/_____/\/_____/\/___/    \/_/\/___/   \/___/  \/___/

I'll bet some SSD1306 OLED DRIVER ROUTINES would only ENHANCE my EMBEDDED DISPLAY TECHNOLOGY

[!] This is my module; there are many others like it, but this one is mine.
The module I have is a Chinese import stenciled "Long Qiu" (and not much else).
Everything happens over a 7-pin SIP pin header labled IF2.
Your mileage may vary with other modules. I don't have a parallel one, but rewriting oled_write to use
a parallel bus is a trivial exercise left to the reader.

[!] SSD1306 Display Memory 101
Buffer is 8kbit (1kB) addressed with two auto-incrementable pointers as 8 pages * (128 columns * 8 bits)

         Display Memory        Column
     Cols 0    ->   127       
		+---------------+       +---+
  PAGE0 | XCCCC...CCCCC |       | 0 |
R PAGE1 | CCCCC...CCCCC |   C =>|...|  * 127 => PAGE
o  ...  | ............. |       | 7 |
w PAGE6 | CCCCC...CCCCC |       +---+
s PAGE7 | CCCCC...CCCCC |         ^--- 8 1-bit pixels per column byte, arranged vertically regardless of addressing mode
        +---------------+ 
		 X => Pointer at PAGE 0, COL 0
		 
[!] Fuck yeah tile graphics	 
For the purposes of oled_move(), oled_home(), oled_puts(), oled_putc() and most everything else,
the display is a 16x8 array of 8x8 character cells. Functions expect horizontal addressing mode, other modes
will make them act wanky. Pixels aren't really addressable; the tiny1634 doesn't have enough RAM for a local framebuffer,
and the SPI link to the display is one way so there's no read-modify-write using the display frame buffer.
Given the memory layout I'm reasonably certain the SSD1306 was intended to be driven, perhaps primarily, as a tile graphic display.

[!] Blast from the past
Character generator table is stored in progmem as "font", and contains 128 cells in
PETSCII layout (i.e. you can generate them from C64 font files). Included is the canonical 8x8 C64 font,
with a couple minor changes to make box-drawn digits look better.
Characters are rotated 90 degrees clockwise (so we don't have to waste AVR cycles flipping tiles).
The pointy part of an 'A' should point that way -> if you're doing it right.
It's irritating to go alone, take this: http://www.min.at/prinz/o/software/pixelfont/

[!] Fast as a (LOGO) turtle
Despite software bit-bang and zero optimization it's more than usably fast on a 9MHz Tiny1634; I'm guessing a full repaint at over 15Hz.
Much much faster if moving the pointers and updating incrementally (like a terminal!). Add graphic tiles and make the next-gen POPStation.

[!] Cute lil feller
Basic character display functions are less than 3k compiled, 1k of which is the 128-cell chargen data. 
Chargen can be trimmed to 288ish bytes if only the bare minimum alphanumerics are required.

[!] Zero to Pixels
> Set pin/port defs in ssd1306-config.h
> oled_init()
> oled_clear() // buffer is full of entropy on boot
> oled_home()  // pointers should end up here, but let's make sure
> oled_puts("POOP") // POOP

[!] We built this city in AVR studio 6 using avr-gcc
Very little AVR-specific operations outside of the headers and PROGMEM macros, porting to PIC/ARM/HP9000/M88k/etc should be trivial.

[!] See Also
Ladyada's framebuffer-based arduinolib (c++) SSD1306 implementation: https://github.com/adafruit/Adafruit_SSD1306
RTFM: https://www.adafruit.com/datasheets/SSD1306.pdf 

[!] Legal Fictions
Original work released under terms of the BSD license.
Included chargen/font data generated from c64_lower.64c font and used without permission. 
(Chargen data is assumed to be of negligible economic value and public domain/abandoned. Lawyer at me if you got beef.)

-------------------------------------------------------------------------------
Inspired by tile-based arcade gfx, hacked together by kmm/smallscaleresearch
April 2013
Contact: kmm CHR(0x40) rmlabs.net
-------------------------------------------------------------------------------
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "ss1306-config.h"
#include "ss1306.h"
#include "c64_lower.h"

// Low-level bit-bang SPI write routine
// Port this bit to hardware SPI/I2C/Paralell/LVDS/PCI-E/Infiniband if you need faster transport.
// dc arg specifies data or control buffer, 0 -> control, 1 -> data
void oled_write(uint8_t dc, uint8_t data) {
	// add delays to stretch bit times if needed
	// SSD1306 needs something like 50ns hold...probably not needed unless you port this to an ARM or AVR32.
if(dc > 0) { H(PORTC, SS1306_OLED_DC); }
	else { L(PORTC, SS1306_OLED_DC); }
	
	L(PORTC, SS1306_OLED_CS);
	for(int8_t bit = 7; bit >= 0; bit--)
	{
		L(PORTC, SS1306_OLED_CLK);
		if((1 << bit) & data) { H(PORTC, SS1306_OLED_DAT); } 
		else { L(PORTC, SS1306_OLED_DAT); }
		H(PORTC, SS1306_OLED_CLK);
	}
	L(PORTC, SS1306_OLED_CLK);
	H(PORTC, SS1306_OLED_CS);
}

// These defaults are for a 128x64 configuration, adapt as needed
// seg remap, com scan, mux ratio, addr mode are suggested things to poke at
void oled_init() {
	L(PORTB, SS1306_OLED_RST);
	_delay_ms(10);
	H(PORTB, SS1306_OLED_RST);
	_delay_ms(10);
	oled_write(0, 0xAE); // display off
	
	oled_write(0, 0xD5); // clock
	oled_write(0, 0x81); // upper nibble is rate, lower nibble is divisor
	
	oled_write(0, 0xA8); // mux ratio
	oled_write(0, 0x3F); // rtfm
	
	oled_write(0, 0xD3); // display offset
	oled_write(0, 0x00); // rtfm
	oled_write(0, 0x00);
	
	oled_write(0, 0x8D); // charge pump
	oled_write(0, 0x14); // enable
	
	oled_write(0, 0x20); // memory addr mode
	oled_write(0, 0x00); // horizontal
	
	oled_write(0, 0xA1); // segment remap
	
	oled_write(0, 0xA5); // display on
	
	oled_write(0, 0xC8); // com scan direction
	oled_write(0, 0xDA); // com hardware cfg
	oled_write(0, 0x12); // alt com cfg
	
	oled_write(0, 0x81); // contrast aka current
	oled_write(0, 0x7F); // 128 is midpoint
	
	oled_write(0, 0xD9); // precharge
	oled_write(0, 0x11); // rtfm
	
	oled_write(0, 0xDB); // vcomh deselect level
	oled_write(0, 0x20); // rtfm
	
	oled_write(0, 0xA6); // non-inverted
	
	oled_write(0, 0xA4); // display scan on
	oled_write(0, 0xAF); // drivers on
}

void oled_display_en(uint8_t state) {
}

void oled_set_clock(uint8_t val) {
	oled_write(0, 0xD5);
	oled_write(0, val);
}

void oled_set_addr_mode(uint8_t mode) {
	// 0 -> horizontal (write column, increment column pointer, at column overrun reset column pointer and increment page pointer)
	// 1 -> vertical (write column, increment page pointer, at page overrun reset page pointer and increment column pointer)
	// 2 -> page (write column, increment column pointer, reset column pointer at overrun)
	oled_write(0, 0x20);
	oled_write(0, (mode < 2) ? mode : 0);
}

void oled_put_tile(uint8_t *tile, uint8_t limit) {
	for(uint16_t i = 0; i < limit; i++) {
		oled_write(1, *tile++);
	}
}

void oled_putc_raw(char c) {
	for(uint16_t i = c << 3; i < (c << 3) + 8; i++) {
		oled_write(1, pgm_read_byte(font + i));
	}
}

void oled_putc(char c) {
	// remap from petscii to ascii, shifts drawing characters into the lower 32 ascii cells
	if(c > 'A' && c < 'Z') { }               // upper-case ascii range
	else if(c > 'a' && c < 'z') { c -= 96; } // lower-case ascii range
	else if(c > 31 && c < 64) { }            // numbers and symbols
	else if(c < 32) { c += 96; }             // low ascii
	oled_putc_raw(c);
}

void oled_home() {
	oled_write(0, 0x21); // column range
	oled_write(0, 0x00); //  set start to 0
	oled_write(0, 0x7F); //  set end to 0x7F
	oled_write(0, 0x22); // row range
	oled_write(0, 0x00); //  set start to 0
	oled_write(0, 0x07); //  set end to 0x07
}

void oled_move_raw(uint8_t row, uint8_t col) {
	if(col > 127) { col = 127; }
	if(row > 7) { row = 7; }
	
	oled_write(0, 0x21);     // set column
	oled_write(0, col);      // start = col
	oled_write(0, 0x7F);     // end = col max
	oled_write(0, 0x22);     // set row
	oled_write(0, row);      // start = row
	oled_write(0, 0x07);     // end = row max
}

void oled_move(uint8_t row, uint8_t col) {
	if(col > 15) { col = 15; }
	if(row > 7) { row = 7; }
	
	oled_move_raw(row, col << 3);
}

void oled_clear() {
	oled_home();
	for(uint16_t i = 1024; i > 0; i--) {
		oled_write(1, 0x00);
	}	
}

void oled_fill(uint8_t row, uint8_t col, uint8_t count, uint8_t max, uint32_t pattern, int8_t pshift) {
	oled_move(row, col);
	uint8_t pstate = 0;
	for(max; max > 0; max--) {
		if(count > max) {
			if(pshift < 0) {
				oled_write(1, (pattern >> (pstate++ << 3)) & 0xFF);
			} 
			else 
			{
				oled_write(1, (pattern >> (pstate++ + pshift)) & 0xFF);
			}			
			pstate = (pstate > 3) ? 0 : pstate;
		}
		else {
			oled_write(1, 0x00);
		}		
	}
	
}

// Draw a tile at an arbitrary pixel location (top, left) using an 8 byte tile buffer referenced by *tile.
// Slower than oled_putc(), potentially substantially so; only use for things that need
// finer grained positioning than is possible with tile cells, like sprites.
// Clips right and bottom edges properly; untested and not expected to work with negative positions.
void oled_putxy(uint8_t left_pxl, uint8_t top_pxl, uint8_t *tile) {
	uint8_t tbuf[8], obuf[8];
	uint8_t top_cell = top_pxl >> 3;
	uint8_t left_cell = left_pxl >> 3;
	int8_t voff = top_pxl - ((top_cell << 3) - 1);
	int8_t hoff = left_pxl - ((left_cell << 3) - 1);
	
	if(voff == 0 && hoff == 0) {
		oled_move(top_cell, left_pxl >> 3);
		oled_put_tile(tile, 8);
		return;
	}
	else {
		for(uint8_t tcol = 0; tcol < 8; tcol++) { // tile column
			tbuf[tcol] = (tile[tcol]) << ((uint8_t)voff); // shift left (down) by voff
			obuf[tcol] = (tile[tcol]) >> (8 - (uint8_t)voff); //shift right (up) by voff
		}
		
		oled_move_raw(top_cell, left_pxl); // move_raw(row[0:7], column[0:127]) rows and pixels for extra confusion
		oled_put_tile(&tbuf, (left_pxl > (SS1306_OLED_GEOM_W - 8)) ? 8 - hoff : 8);
		if(top_pxl < (SS1306_OLED_GEOM_H - 8)) {
			oled_move_raw((top_cell + 1), left_pxl);
			oled_put_tile(&obuf, (left_pxl > (SS1306_OLED_GEOM_W - 8)) ? 8 - hoff : 8);
		}		
	}	
}

void oled_puts(char *str) {
	while(*str != 0) {
		oled_putc(*str++);
	}
}

// box graphics digit (a single digit, not byte or word; use this to render output of an int->bcd conversion etc)
void oled_bigdigit(uint8_t top, uint8_t left, uint8_t num) {
	const uint8_t chartable[] PROGMEM = { 0x10, 0x0E, 0x5D, 0x5D, 0x0D, 0x1D, // zero
							              0x20, 0x0E, 0x20, 0x5D, 0x20, 0x11, // one
		                                  0x10, 0x0E, 0x10, 0x1D, 0x0D, 0x1D, // two
							              0x10, 0x0E, 0x20, 0x13, 0x0D, 0x1D, // three
							              0x5F, 0x5F, 0x0D, 0x13, 0x20, 0x5E, // four
							              0x10, 0x0E, 0x0D, 0x0E, 0x0D, 0x1D, // five
							              0x10, 0x0E, 0x0B, 0x0E, 0x0D, 0x1D, // six
							              0x10, 0x0E, 0x20, 0x5B, 0x20, 0x5E, // seven
							              0x10, 0x0E, 0x0B, 0x13, 0x0D, 0x1D, // eight
							              0x10, 0x0E, 0x0d, 0x13, 0x0D, 0x1D  // nine
						                };
	if(num > 9) { return; }
	oled_move(top, left);
	for(uint8_t i = 0; i < 6; i++) {
		if(i == 2 || i == 4) { oled_move(++top, left); }
		oled_putc(chartable[(num * 6) + i]);
	}		
		
}

void oled_box(uint8_t top, uint8_t left, uint8_t width, uint8_t height) {
	oled_move(top, left);
	oled_putc(BOX_TL); for(uint8_t i = 0; i < width - 2; i++) { oled_putc(BOX_HL);} oled_putc(BOX_TR);
	for(uint8_t i = top+1; i < top+height-1; i++) {
		oled_move(i, left); 
		oled_putc(BOX_VL); 
		oled_move(i, left + width - 1);
		oled_putc(BOX_VL);
	}
	oled_move(top + height - 1, left);
	oled_putc(BOX_BL); for(uint8_t i = 0; i < width - 2; i++) { oled_putc(BOX_HL);} oled_putc(BOX_BR);		
}

/* 

"I tell you, we are here on Earth to fart around, and don't let anybody tell you different." - Kurt Vonnegut

*/