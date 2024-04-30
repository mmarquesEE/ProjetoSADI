/*
 * adc.c
 *
 * Created: 26/02/2024 10:09:15
 *  Author: mateu
 */ 

#include "adc.h"

void adc_init(void)
{
	ADMUX |= (1<<REFS0); // Set AVcc as voltage reference
	
	/* ADC Frequency:
	F = 16MHz / 128 = 125 kHz
	*/
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1);
	
	// Enable ADC and start conversion
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
}

uint16_t adc_read(uint8_t channel)
{
	ADMUX &= 0xF0; // Clear ADC selection
	ADMUX |= (0x07 & channel); // Set new channel
	
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	
	return ADCL | (ADCH<<8);
}
