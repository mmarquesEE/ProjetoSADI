/*
 * dac.h
 *
 * Created: 26/02/2024 10:12:34
 *  Author: mateu
 */ 


#ifndef DAC_H_
#define DAC_H_


#include <avr/io.h>

#define SDI		PORTB3
#define SCK		PORTB5
#define LDAC	PORTB0

void dac_init(void);
void dac_write(uint16_t * data);


#endif /* DAC_H_ */