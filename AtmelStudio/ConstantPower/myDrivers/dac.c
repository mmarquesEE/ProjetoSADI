/*
 * dac.c
 *
 * Created: 26/02/2024 10:12:47
 *  Author: mateu
 */ 

#include "i2c.h"
#include "dac.h"


void dac_write(uint8_t channel, uint16_t data)
{	
	i2c_start(0xc0 | ((channel & 0x01)<<1));
	i2c_send(0x0f & (data>>8));
	i2c_send(0x0ff & data);
	i2c_stop();
}