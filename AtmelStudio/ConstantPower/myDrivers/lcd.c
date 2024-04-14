/*
 * lcd.c
 *
 * Created: 08/04/2024 09:16:00
 *  Author: mateu
 */ 

#include <avr/io.h>
#include <util/twi.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void i2c_init(unsigned long fscl)
{
	TWSR = 0;
	TWBR = (F_CPU/fscl - 16)/2;
}

int i2c_start(uint8_t address)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xf8)!=TW_START) return 0;
	TWDR = address;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xf8)!=TW_MT_SLA_ACK) return 0;
	return 1;
}

void i2c_stop(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

int i2c_send(uint8_t data)
{
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xf8)!=TW_MT_DATA_ACK) return 0;
	return 1;
}

void lcd_toggle(void)
{
	i2c_send(TWDR | 0x02);
	_delay_ms(1);
	i2c_send(TWDR & ~(0x02));
}

void lcd_half_cmd(uint8_t upper_half)
{
	i2c_send(TWDR & ~(0x01));
	i2c_send(TWDR & (0x0f));
	i2c_send(TWDR | (upper_half & 0xf0));
	lcd_toggle();
}

void lcd_cmd(uint8_t cmd)
{
	lcd_half_cmd(cmd & 0xf0);
	lcd_half_cmd((cmd & 0x0f)<<4);
}

void lcd_init(void)
{
	i2c_init(100000);
	i2c_start(0x70);
	_delay_ms(20);
	
	lcd_half_cmd(0x30);
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
	i2c_send(TWDR | 0x01);
	i2c_send(TWDR & (0x0f));
	i2c_send(TWDR | (data & 0xf0));
	lcd_toggle();
	i2c_send(TWDR & (0x0f));
	i2c_send(TWDR | ((data & 0x0f)<<4));
	lcd_toggle();
}

void lcd_write_string(char * data)
{
	uint8_t i = 0;
	while(data[i] != 0) lcd_write_char(data[i++]);
}
