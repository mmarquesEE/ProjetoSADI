/*
 * usart.c
 *
 * Created: 26/02/2024 10:19:11
 *  Author: mateu
 */ 

#include "usart.h"

void usart_init(void)
{
	unsigned int ubrr = F_CPU/16/9600 - 1;
	
	// Set baud rate
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) (ubrr);
	
	// Enable receiver and transmitter
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	
	// Enable RX, TX and Data Register empty interrupts
	UCSR0B |= (1<<RXCIE0);// | (1<<TXCIE0) | (1<<UDRIE0);
	
	// Set frame format: 8data, 2stop bit
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void usart_write(char data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)));
	
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

uint8_t usart_read(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	
	/* Get and return received data from buffer */
	return UDR0;
}

void usart_writeline(char *data)
{
	uint8_t data_count = 0;
	while(data[data_count] != '\0'){
		usart_write(data[data_count++]);
	}
}

void usart_flush(void)
{
	unsigned char dummy;
	while (UCSR0A & (1<<RXC0)) dummy = UDR0;
}

