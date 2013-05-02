/*
 * ssd1306_bitbang.h
 *
 * Created: 4/21/2013 2:16:29 PM
 *  Author: kmm
 */ 

#define H(port, pin) port |= _BV((pin))
#define L(port, pin) port &= ~(_BV((pin)))

#define BOX_TL 0x10
#define BOX_TR 0x0E
#define BOX_HL 0x40
#define BOX_VL 0x5D
#define BOX_BL 0x0D
#define BOX_BR 0x1D

#ifndef SS1306_H_
#define SS1306_H_

void oled_write(uint8_t dc, uint8_t data);
void oled_init();
void oled_display_en(uint8_t state);
void oled_set_clock(uint8_t val);
void oled_set_addr_mode(uint8_t mode);
void oled_put_tile(uint8_t *tile, uint8_t limit);
void oled_putc_raw(char c);
void oled_putc(char c);
void oled_home();
void oled_move_raw(uint8_t row, uint8_t col);
void oled_move(uint8_t row, uint8_t col);
void oled_clear();
void oled_fill(uint8_t row, uint8_t col, uint8_t count, uint8_t max, uint32_t pattern, int8_t pshift);
void oled_putxy(uint8_t left_pxl, uint8_t top_pxl, uint8_t *tile);
void oled_puts(char *str);
void oled_bigdigit(uint8_t top, uint8_t left, uint8_t num);
void oled_box(uint8_t top, uint8_t left, uint8_t width, uint8_t height);

#endif