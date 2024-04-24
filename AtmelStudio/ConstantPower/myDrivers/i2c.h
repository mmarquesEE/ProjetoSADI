/*
 * i2c.h
 *
 * Created: 20/04/2024 19:17:29
 *  Author: mmarq
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <util/twi.h>

void i2c_init(unsigned long fscl);
int i2c_start(uint8_t address);
void i2c_stop(void);
int i2c_send(uint8_t data);

#endif /* I2C_H_ */