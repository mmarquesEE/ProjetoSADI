/*
 * i2c.c
 *
 * Created: 20/04/2024 19:18:03
 *  Author: mmarq
 */ 

#include <avr/io.h>
#include <util/twi.h>

#define F_CPU 16000000UL

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