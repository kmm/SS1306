SS1306
======
(a compact plain-C tile graphics library for the SSD1306 OLED display controller)

<pre> ____    ____    ____       _     __       __     ____
/\  _`\ /\  _`\ /\  _`\   /' \  /'__`\   /'__`\  /'___\
\ \,\L\_\ \,\L\_\ \ \/\ \/\_, \/\_\L\ \ /\ \/\ \/\ \__/
 \/_\__ \\/_\__ \\ \ \Small Scale_/_\_&lt;_\ \ \ \ \ \  _``\
   /\ \L\ \/\ \L\ \ \ \_\ \ \ \ \/\ \L\ \\ \ \_\ \ \ \L\ \
   \ `\____\ `\____\ \____/  \ \_\ \____/ \ \____/\ \____/
    \/_____/\/_____/\/___/    \/_/\/___/   \/___/  \/___/
</pre>

I'll bet some SSD1306 OLED DRIVER ROUTINES would only ENHANCE my EMBEDDED DISPLAY TECHNOLOGY

[!] This is my module; there are many others like it, but this one is mine.
The module I have is a Chinese import stenciled "Long Qiu" (and not much else).
Everything happens over a 7-pin SIP pin header labled IF2.
Your mileage may vary with other modules. I don't have a parallel one, but rewriting oled_write to use
a parallel bus is a trivial exercise left to the reader.

[!] SSD1306 Display Memory 101
Buffer is 8kbit (1kB) addressed with two auto-incrementable pointers as 8 pages * (128 columns * 8 bits)

<pre>
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
</pre>	 

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
