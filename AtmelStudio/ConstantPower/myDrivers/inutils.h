/*
 * utils.h
 *
 * Created: 03/04/2024 14:47:53
 *  Author: mateu
 */ 


#ifndef UTILS_H_
#define UTILS_H_

#ifndef MAX_LIST_LEN
#define MAX_LIST_LEN 30
#endif

int parse_commands(
	char *input,
	volatile char ch_mode[2],
	volatile uint16_t ch_lvl[2],
	volatile uint32_t ch_list[MAX_LIST_LEN][2][2],
	volatile uint8_t start_list[2]
);


#endif /* UTILS_H_ */