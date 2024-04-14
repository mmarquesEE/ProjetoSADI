/*
 * dac.c
 *
 * Created: 26/02/2024 10:12:47
 *  Author: mateu
 */ 

#include "dac.h"
#define F_CPU 16000000UL
#include <util/delay.h>


void dac_init(void)
{
	/* Set SDI, SCK and LDAC output */
	DDRB |= (1<<SDI)|(1<<SCK)|(1<<LDAC);
	PORTB |= (1<<LDAC);
	
	/* Enable SPI, Master, set clock rate F_CPU/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void dac_write(uint16_t * data)
{	
	for(uint8_t i=0; i<2; i++){
		uint8_t msb = (i<<7)|(0x30)|((data[i] & 0x0300)>>8);
		uint8_t lsb = (data[i] & 0x00FF);
		
		SPDR = msb;
		while(!(SPSR & (1<<SPIF)));
		
		SPDR = lsb;
		while(!(SPSR & (1<<SPIF)));
	}
	_delay_ms(1);
	PORTB &= ~(1<<LDAC);
	_delay_ms(1);
	PORTB |= (1<<LDAC);
}