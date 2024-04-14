/*
 * ConstantPower.c
 *
 * Created: 24/02/2024 17:23:24
 * Author : mateu
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#include <util/delay.h>
#include "myDrivers/adc.h"
#include "myDrivers/dac.h"
#include "myDrivers/usart.h"
#include "myDrivers/utils.h"
#include "myDrivers/lcd.h"
#include <string.h>

unsigned long f_cpu = F_CPU;
void update_display(void);
void timer0_init(void);
void extint_init(void);

// Asynchronously receive set point and config ----------------------
volatile uint8_t ch_mode[2] = {1,0};
volatile uint16_t ch_lvl[2] = {100,100};

volatile uint8_t crr_ch = 0;
volatile uint8_t cmd[14] = {0};
volatile uint8_t cmd_count = 0;

ISR(USART_RX_vect)
{
	cmd[cmd_count++] = usart_read();
	
	if ((cmd[cmd_count] == ';') | (cmd_count > 14)){
		cmd_count = 0;
		if(cmd[cmd_count] == ';'){
			uint8_t mode[2]; int8_t lvl[2];
			parse_cmd(cmd, mode, lvl);
			
			for(uint8_t i=0; i<2; i++){
				ch_mode[i] = mode[i] == 'X' ? ch_mode[i]:mode[i];
				ch_lvl[i] = lvl[i] == -1 ? ch_lvl[i]:lvl[i];
			}
			update_display();
		}	
	}
}
// ------------------------------------------------------------------

// Periodically control current -------------------------------------

#define IMAX 2.048
#define PMAX IMAX*3.0

#define SAMPLE_TIME 100
volatile uint16_t timer0_counter = SAMPLE_TIME;

ISR(TIMER0_COMPA_vect)
{
	if (timer0_counter == SAMPLE_TIME){
		uint16_t dacval[2] = {0};
			
		for(uint8_t i=0; i<2; i++){
			// Compute control signal:
			float I = ((float) ch_lvl[i])/100.0*IMAX;
			if(ch_mode[i] == 0){
				// Read ADC channel i:
				uint16_t adcval = adc_read(i);
				float V = ((float) adcval)/1023.0 * 5.0;
				I = ((float) ch_lvl[i])/100.0*PMAX/V;
			}
			dacval[i] = (uint16_t) (I/5.0 * 4095.0);
		}
		dac_write(dacval);
		timer0_counter = 0; return;
	}
	timer0_counter += 1;
}
// ------------------------------------------------------------------

// Navigate through display menu ------------------------------------
ISR(INT0_vect)
{
	if(crr_ch == 0) crr_ch = 1;
	else crr_ch = 0;
	update_display();
}

// ------------------------------------------------------------------

#define LED PORTB1
int main(void)
{
	sei();
	
	DDRB |= (1<<LED);
	
	usart_init();
	extint_init();
	adc_init();
	dac_init();
	lcd_init();
	timer0_init();
	
	update_display();
	while(1){
		PORTB |=  (1<<LED); _delay_ms(500);
		PORTB &= ~(1<<LED); _delay_ms(500);
	}
}

void update_display(void)
{
	char line1[16];
	char line2[16];
	char *line[2] = {line1,line2};
	for (uint8_t i=0; i < 2; i++){
		sprintf(line[i],"(%c)CH%d %c %.2f %c",
			crr_ch == i ? '*':' ',
			i+1,
			ch_mode[i] ? 'I':'P',
			((float) ch_lvl[i]/100.0)*(ch_mode[i] ? IMAX:PMAX),
			ch_mode[i] ? 'A':'W'
		);
		lcd_set_position(i,0);
		lcd_write_string(line[i]);
	}
}

void timer0_init(void)
{
	// Configure Timer 0 for 1ms sample interval
	TCCR0A |= 0x02;
	TCCR0B |= 0x03;
	OCR0A = 249;
	TIMSK0 |= 0x02;
}

void extint_init(void)
{
	EICRA = 10;
	EIMSK = 3;
}

