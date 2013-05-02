/*
 * SSD1306demo.c
 *
 * Created: 4/29/2013 7:10:21 PM
 *  Author: kmm
 */ 

#define F_CPU 9000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "ss1306-config.h"
#include "ss1306.h"
#include "nyan.h"

const uint8_t sine_table[64] PROGMEM = {
	128, 140, 152, 165, 176, 188, 198, 208,
	218, 226, 234, 240, 245, 250, 253, 254,
	255, 254, 253, 250, 245, 240, 234, 226,
	218, 208, 198, 188, 176, 165, 152, 140,
	128, 115, 103,  90,  79,  67,  57,  47,
	 37,  29,  21,  15,  10,   5,   2,   1,
	  0,   1,   2,   5,  10,  15,  21,  29,
	 37,  47,  57,  67,  79,  90, 103, 115
};

struct vec2f {
	float x;
	float y;
} ball_pos, ball_vel;

float grav = 0.1;

int main(void)
{
	DDRB |= SS1306_OLED_PORTB_MASK;
	DDRC |= SS1306_OLED_PORTC_MASK;
	
	oled_init();
	oled_clear();
	uint8_t shift = 0;
	uint8_t ctr = 0;
	uint8_t px = 0, py = 0;
	uint8_t blank[] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t ball[]  = {0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00};
	ball_pos.x = (float)(rand() >> 26);
	ball_pos.y = 0;
	ball_vel.x = (5.0F / (rand() & 0xFF)) + 2.5;

    while(1)
    {
		ball_vel.y += grav;
		ball_pos.x += ball_vel.x;
		ball_pos.y += ball_vel.y;
		uint8_t x = (uint8_t)ball_pos.x;
		uint8_t y = (uint8_t)ball_pos.y;
		
		oled_move(7, 0);
		oled_puts("STRAIGHT BALLIN'");
		
		oled_putxy(px, py, &blank);
		oled_putxy(x, y, &ball);
		px = x;
		py = y;
		
		if(ball_pos.x > SS1306_OLED_GEOM_W - 8) {
			ball_pos.x = SS1306_OLED_GEOM_W - 8;
			ball_vel.x *= -1;
		}
		else if(ball_pos.x < 0) {
			ball_pos.x = 0;
			ball_vel.x *= -1;
		}
		
		if(ball_pos.y > SS1306_OLED_GEOM_H - 8) {
			ball_pos.y = SS1306_OLED_GEOM_H - 8;
			ball_vel.y *= -0.8;
		} 
		else if(ball_pos.y < 0) {
			ball_pos.y = 0;
			ball_vel.y *= -0.8;
		}
		
		if(ball_pos.y >=  SS1306_OLED_GEOM_H - 8 && ball_pos.x <= 0) {
			ball_vel.y = -3.0;
		}
		_delay_ms(10);			
    }
}