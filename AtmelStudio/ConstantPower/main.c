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
#include "myDrivers/i2c.h"
#include "myDrivers/adc.h"
#include "myDrivers/dac.h"
#include "myDrivers/usart.h"
#include "myDrivers/utils.h"
#include "myDrivers/lcd.h"
#include <string.h>
#include <stdio.h>

unsigned long f_cpu = F_CPU;
void update_display(void);
void timer0_init(void);
void extint_init(void);

// Menu related variables
volatile uint8_t channel_sel = 0;
volatile char mode_sel = 0;
volatile uint16_t level_sel = 0;
volatile uint8_t current_page = 0;
volatile uint8_t confirm = 0;

// Asynchronously receive set point and config ----------------------
volatile char ch_mode[2] = {'I','I'};
volatile uint16_t ch_lvl[2] = {0,0};
volatile uint16_t ch_list[30][2][2] = {0};
volatile uint8_t start_list[2] = {0,0};

#define CMD_MAX_SIZE 311

ISR(USART_RX_vect)
{
	static char cmd[CMD_MAX_SIZE] = {0};
	static uint16_t cmd_count = 0;
	
	cmd[cmd_count++] = usart_read();
	
	if (((cmd[cmd_count - 1] == ';') && cmd_count > 1) || (cmd_count >= CMD_MAX_SIZE)) {
		cmd[cmd_count] = '\0';

		if (cmd[0] == ';' && parse_commands(cmd, ch_mode, ch_lvl, ch_list, start_list)) {
			current_page = 0;
			confirm = 0;
			update_display();
		}
		cmd_count = 0;
	}
}
// ------------------------------------------------------------------

// Periodically control current -------------------------------------

#define IMAX 1.68
#define PMAX IMAX*8
#define SAMPLE_TIME 10

volatile float I[2] = {0,0};
volatile float V[2] = {0,0};

ISR(TIMER0_COMPA_vect)
{
	static uint16_t timer0_counter = SAMPLE_TIME;
	
	if (timer0_counter == SAMPLE_TIME){
		for(uint8_t i=0; i<2; i++){
			// Read ADC channel i:
			uint16_t adcval = adc_read(i);
			V[i] = (((float) adcval)/1023.0) * (3.0 * 5.0);
			
			// Compute control signal:
			if(ch_mode[i] == 'P')
				I[i] = (((float) ch_lvl[i])/1023.0)*(PMAX/V[i]);
			else
				I[i] = (((float) ch_lvl[i])/1023.0)*IMAX;
				
			uint32_t dacval = (uint32_t) (3.0*I[i]/5.0 * 4095.0);
			dac_write(i,(uint16_t)(dacval > 4095.0 ? 4095.0 : dacval));
		}
		level_sel = adc_read(2);
		update_display();
		timer0_counter = 0; return;
	}
	timer0_counter += 1;
}
// ------------------------------------------------------------------

// Navigate through display menu ------------------------------------

ISR(INT0_vect)
{	
	if(start_list[0] || start_list[1]){
		parse_commands("STOP LIST;",ch_mode,ch_lvl,ch_list,start_list);
	} else {
		switch(current_page){
			case 0:
				channel_sel = channel_sel ? 0:1;
				break;
			case 1:
				mode_sel = mode_sel == 'I'? 'P':'I';
				break;
			case 2:
				confirm = confirm ? 0:1;
				break;
		}
	}
	update_display();
}

ISR(INT1_vect)
{
	if(start_list[0] || start_list[1]){
		parse_commands("STOP LIST;",ch_mode,ch_lvl,ch_list,start_list);
	} else {
		current_page++;
		if(current_page > 2){
			if(confirm){
				ch_mode[channel_sel] = mode_sel;
				ch_lvl[channel_sel] = level_sel;
			}
			current_page = 0;
			confirm = 0;
		}
	}
	update_display();
}
// ------------------------------------------------------------------

#define LED PORTC3
int main(void)
{
	sei();
	
	DDRC |= (1<<LED);
	
	usart_init();
	extint_init();
	adc_init();
	i2c_init(100000);
	lcd_init();
	timer0_init();
	
	mode_sel = ch_mode[channel_sel];
	level_sel = ch_lvl[channel_sel];
		
	while(1){
		PORTC |=  (1<<LED); _delay_ms(500);
		PORTC &= ~(1<<LED); _delay_ms(500);
	}
}

void update_display(void)
{
	char line1[33] = {' '};
	char line2[33] = {' '};
	char *line[2] = {line1,line2};
	switch(current_page){
		case 0:
			for (uint8_t i=0; i < 2; i++){
				sprintf(line[i],"%c%c%cC%d %c %5.2f %c (%5.2fV  %5.2fA)",
					start_list[i] ? ' ':'(',
					start_list[i] ? ' ':(channel_sel == i ? '*':' '),
					start_list[i] ? ' ':')',
					i+1,
					ch_mode[i],
					((float) ch_lvl[i]/1023.0)*(ch_mode[i] == 'I'? IMAX:PMAX),
					ch_mode[i] == 'I' ? 'A':'W',
					V[i],I[i]
				);
				lcd_set_position(i,0);
				lcd_write_string(line[i]);
			}
			break;
		case 1:
			sprintf(line[0],"CHANNEL %d| MODE: %c  LVL: %5.2f %c",
				channel_sel+1,
				ch_mode[channel_sel],
				((float) ch_lvl[channel_sel]/1023.0)*(ch_mode[channel_sel] == 'I' ? IMAX:PMAX),
				ch_mode[channel_sel] == 'I' ? 'A':'W'
			);
			sprintf(line[1],"NEW: MODE %c | LEVEL %5.2f %c",
				mode_sel,
				((float) level_sel/1023.0)*(mode_sel == 'I' ? IMAX:PMAX),
				mode_sel == 'I' ? 'A':'W'
			);
			lcd_set_position(0,0);lcd_write_string(line[0]);
			lcd_set_position(1,0);lcd_write_string(line[1]);
			break;
		case 2:
			sprintf(line[0],"ARE YOU SURE?");
			sprintf(line[1],"(%c)YES            (%c)NO",
				confirm ? '*':' ',
				confirm ? ' ':'*'
			);
			lcd_set_position(0,0);lcd_write_string(line[0]);
			lcd_set_position(1,0);lcd_write_string(line[1]);
			break;
		default:
			break;
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

