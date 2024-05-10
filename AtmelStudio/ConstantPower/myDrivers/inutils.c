/*
 * utils.c
 *
 * Created: 03/04/2024 14:50:10
 *  Author: mateu
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"


static int parse_set_mode(
	char *params,
	volatile char ch_mode[],
	volatile uint16_t ch_lvl[2]
) {
	int channel1 = -1, channel2 = -1;
	char mode1, mode2;
	
	int count = sscanf(params, " (%d,%d) [%c,%c]", &channel1, &channel2, &mode1,&mode2);
	
	if (count != 4) {
		count = sscanf(params, " (%d,%d) [%c]", &channel1, &channel2, &mode1);
		if (count != 3){
			count = sscanf(params, " (%d) [%c]", &channel1, &mode1);
		} else {
			mode2 = mode1;
		}
	}

	if (count == 2 || count == 3 || count == 4) {
		if ((channel1 == 1 || channel1 == 2) && (mode1 == 'I' || mode1 == 'P')) {
			ch_mode[channel1 - 1] = mode1;
			ch_lvl[channel1 - 1] = 0;
			
			if ((channel2 == 1 || channel2 == 2) && (mode2 == 'I' || mode2 == 'P')){
				ch_mode[channel2 - 1] = mode2;
				ch_lvl[channel2 - 1] = 0;
			}
				
			return 0;
		}
	}
	return -1;
}

static int parse_set_level(
	char *params,
	volatile uint16_t ch_lvl[]
) {
	int channel1 = -1, channel2 = -1;
	int value1, value2;
	
	int count = sscanf(params, " (%d,%d) [%d,%d]", &channel1, &channel2, &value1,&value2);
	
	if (count != 4) {
		count = sscanf(params, " (%d,%d) [%d]", &channel1, &channel2, &value1);
		if (count != 3){
			count = sscanf(params, " (%d) [%d]", &channel1, &value1);
		} else {
			value2 = value1;
		}
	}

	if ((count == 2 || count == 3 || count == 4) && (value1 >= 0 && value1 <= 1023) 
		&& (value2 >= 0 && value2 <= 1023))
	{
		if (channel1 == 1 || channel1 == 2) {
			ch_lvl[channel1 - 1] = value1;
			if (channel2 == 1 || channel2 == 2) {
				ch_lvl[channel2 - 1] = value2;
			}
			return 0;
		}
	}
	return -1;
}

static void clear_list_values(volatile uint32_t ch_list[MAX_LIST_LEN][2][2])
{
	for(uint8_t i=0; i<MAX_LIST_LEN; i++){
		for(uint8_t j=0; j<2; j++){
			for (uint8_t k=0; k<2; k++)
				ch_list[i][j][k] = 0;
		}
	}
}

static int parse_list_values(
	char *start,
	char *end,
	int channel,
	volatile uint32_t ch_list[MAX_LIST_LEN][2][2],
	int listIndex
) {
	char numBuffer[6];
	int ncount = 0;
	while (start < end && ncount < MAX_LIST_LEN) {
		char *comma = strchr(start, ',');
		if (!comma || comma > end) comma = end;
		int len = comma - start;
		strncpy(numBuffer, start, len);
		numBuffer[len] = '\0';
		int num;
		if (sscanf(numBuffer, "%d", &num) == 1) {
			if(listIndex == 0 && (num < 0 || num > 1023))
				return -1;
			else if(listIndex == 1 && (num <= 0))
				return -1;
			
			ch_list[ncount][listIndex][channel] = num;
			ncount++;
		} else
			return -1;

		start = comma + 1;
	}
	return 0;
}

static int parse_set_list(
	char *params,
	volatile uint32_t ch_list[MAX_LIST_LEN][2][2]
) {
	int channel1, channel2 = -1;
	char *firstBracket = strchr(params, '[');
	if (!firstBracket)
		return -1;
	
	char *middleBracket = strchr(firstBracket + 1, '[');
	char *lastBracket = strrchr(params, ']');
	if (!middleBracket || !lastBracket || middleBracket >= lastBracket)
		return -1;

	clear_list_values(ch_list);
	int numParams = sscanf(params, " (%d,%d)", &channel1, &channel2);

	if ((numParams == 1 || numParams == 2) && (channel1 == 1 || channel1 == 2)) {
		char *list1Start = firstBracket + 1;
		char *list1End = middleBracket - 1;
		int8_t err = parse_list_values(list1Start, list1End, channel1 - 1, ch_list, 0);
		
		if ((channel2 == 1 || channel2 == 2) && (channel1 != channel2))
			err += parse_list_values(list1Start, list1End, channel2 - 1, ch_list, 0);

		char *list2Start = middleBracket + 1;
		char *list2End = lastBracket;
		err += parse_list_values(list2Start, list2End, channel1 - 1, ch_list, 1);

		if ((channel2 == 1 || channel2 == 2) && (channel1 != channel2))
			err += parse_list_values(list2Start, list2End, channel2 - 1, ch_list, 1);
		
		if(err != 0){
			clear_list_values(ch_list);
			return -1;
		}
		return 0;
	}
	return -1;
}

static int parse_start_stop(
	char *command,
	volatile uint8_t start_list[2],
	volatile uint16_t ch_lvl[2]
) {
	char *params = strchr(command, '(');
	if (!params)
		return -1;

	char action[6];
	if (sscanf(command, "%5s", action) != 1)
		return -1;

	if (strncmp(action, "START", 5) != 0 && strncmp(action, "STOP", 4) != 0)
		return -1;

	int number1 = 0, number2 = 0;
	int numParams = sscanf(params, " (%d,%d)", &number1, &number2);
	int isStart = strncmp(action, "START", 5) == 0;

	if ((numParams == 1 || numParams == 2) && (number1 == 1 || number1 == 2)) {
		start_list[number1 - 1] = isStart ? 1 : 0;
		ch_lvl[number1 - 1] = 0;
		
		if (numParams == 2 && (number2 == 1 || number2 == 2)){
			start_list[number2 - 1] = isStart ? 1 : 0;
			ch_lvl[number2 - 1] = 0;
		}
		return 0;
	}
	return -1;
}

int parse_commands(
	char *input,
	volatile char ch_mode[2],
	volatile uint16_t ch_lvl[2],
	volatile uint32_t ch_list[MAX_LIST_LEN][2][2],
	volatile uint8_t start_list[2]
)
{
	if (input[strlen(input) - 1] != ';')
		return -1;

	input[strlen(input) - 1] = '\0';

	char *start = input;
	char *end = input + strlen(input) - 1;
	while (isspace((unsigned char)*start)) start++;
	while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

	if (strncmp(start, "SET MODE", 8) == 0) {
		return parse_set_mode(start + 8, ch_mode, ch_lvl);
	}
	else if (strncmp(start, "SET LEVEL", 9) == 0) {
		return parse_set_level(start + 9, ch_lvl);
	}
	else if (strncmp(start, "SET LIST", 8) == 0) {
		return parse_set_list(start + 8, ch_list);
	}
	else {
		return parse_start_stop(start, start_list, ch_lvl);
	}
	return -1;
}