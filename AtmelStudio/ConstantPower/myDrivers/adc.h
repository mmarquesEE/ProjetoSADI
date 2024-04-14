/*
 * adc.h
 *
 * Created: 26/02/2024 10:08:50
 *  Author: mateu
 */ 


#ifndef ADC_H_
#define ADC_H_


#include <avr/io.h>

void adc_init(void);
uint16_t adc_read(uint8_t channel);


#endif /* ADC_H_ */