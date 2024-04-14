/*
 * usart.h
 *
 * Created: 26/02/2024 10:18:58
 *  Author: mateu
 */ 


#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>

extern unsigned long f_cpu;

void usart_init(void);
void usart_write(uint8_t data);
uint8_t usart_read(void);
void usart_flush(void);


#endif /* USART_H_ */