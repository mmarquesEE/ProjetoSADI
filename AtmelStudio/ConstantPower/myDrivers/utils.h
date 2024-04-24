/*
 * utils.h
 *
 * Created: 03/04/2024 14:47:53
 *  Author: mateu
 */ 


#ifndef UTILS_H_
#define UTILS_H_


int parse_commands(
	char *input,
	volatile char * ch_mode,
	volatile uint16_t * ch_lvl,
	volatile uint16_t ch_list[][2][2],
	volatile uint8_t * start_list
);


#endif /* UTILS_H_ */