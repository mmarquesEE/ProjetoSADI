/*
 * lcd.c
 *
 * Created: 08/04/2024 09:16:00
 *  Author: mateu
 */ 

#include <avr/io.h>
#include "lcd.h"

#define F_CPU 16000000UL
#include <util/delay.h>

void lcd_toggle(void)
{
	PORTB |= 0x02;
	_delay_ms(1);
	PORTB &= ~(0x02);
}

void lcd_half_cmd(uint8_t upper_half)
{
	PORTB &= ~(0x01);
	PORTB &= (0x03);
	PORTB |= ((upper_half & 0xf0)>>2);
	lcd_toggle();
}

void lcd_cmd(uint8_t cmd)
{
	lcd_half_cmd(cmd & 0xf0);
	lcd_half_cmd((cmd & 0x0f)<<4);
}

void lcd_init(void)
{
	DDRB |= 0x3f;
	_delay_ms(20);
	
	lcd_half_cmd(0x30);_delay_us(4100);
	lcd_half_cmd(0x30);_delay_us(100);
	lcd_half_cmd(0x30);
	lcd_half_cmd(0x20);
	lcd_cmd(0x28);
	lcd_cmd(0x0c);
	lcd_cmd(0x01);
	lcd_cmd(0x06);
	lcd_cmd(0x80);
}

void lcd_set_position(uint8_t row, uint8_t column)
{
	lcd_cmd(0x80 | (row*0x40) | column);
}

void lcd_write_char(char data)
{
	PORTB |= 0x01;
	PORTB &= 0x03;
	PORTB |= (data & 0xf0)>>2;
	lcd_toggle();
	PORTB &= 0x03;
	PORTB |= ((data & 0x0f)<<2);
	lcd_toggle();
}

void lcd_write_string(char * data)
{
	uint8_t i = 0;
	while(data[i] != 0) lcd_write_char(data[i++]);
	for(uint8_t j=i; j<33; j++)
		lcd_write_char(' ');
}
