/*
 * lcd.h
 *
 * Created: 08/04/2024 09:15:44
 *  Author: mateu
 */ 


#ifndef LCD_H_
#define LCD_H_


void i2c_init(unsigned long fscl);
int i2c_start(uint8_t address);
void i2c_stop(void);
int i2c_send(uint8_t data);
void lcd_toggle(void);
void lcd_half_cmd(uint8_t upper_half);
void lcd_cmd(uint8_t cmd);
void lcd_init(void);
void lcd_set_position(uint8_t row, uint8_t column);
void lcd_write_char(char data);
void lcd_write_string(char * data);


#endif /* LCD_H_ */