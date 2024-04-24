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


static int parse_set_mode(
	char *params,
	volatile char *ch_mode,
	volatile uint16_t *ch_lvl
) {
	int channel1 = -1, channel2 = -1;
	char mode;
	
	int count = sscanf(params, " (%d,%d) %c", &channel1, &channel2, &mode);
	
	if (count != 3) {
		count = sscanf(params, " (%d) %c", &channel1, &mode);
	}

	if (count == 2 || count == 3) {
		if ((channel1 == 1 || channel1 == 2) && (mode == 'I' || mode == 'P')) {
			ch_mode[channel1 - 1] = mode;
			ch_lvl[channel1 - 1] = 0;
			if (channel2 == 1 || channel2 == 2) {
				ch_mode[channel2 - 1] = mode;
				ch_lvl[channel2 - 1] = 0;
			}
			return 0;
		}
	}
	return -1;
}

static int parse_set_level(
	char *params,
	volatile uint16_t *ch_lvl
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

static int parse_list_values(
	char *start,
	char *end,
	int channel,
	volatile uint16_t ch_list[][2][2],
	int listIndex
) {
	char numBuffer[6];
	int ncount = 0;
	while (start < end && ncount < 30) {
		char *comma = strchr(start, ',');
		if (!comma || comma > end) comma = end;
		int len = comma - start;
		strncpy(numBuffer, start, len);
		numBuffer[len] = '\0';
		int num;
		if (sscanf(numBuffer, "%d", &num) == 1 && num >= 0 && num <= 1023) {
			ch_list[ncount][listIndex][channel] = num;
			ncount++;
			} else {
			return -1;
		}
		start = comma + 1;
	}
	return 0;
}

static int parse_set_list(
	char *params,
	volatile uint16_t ch_list[][2][2]
) {
	int channel1, channel2 = -1;
	char *firstBracket = strchr(params, '[');
	if (!firstBracket)
	return -1;
	
	char *middleBracket = strchr(firstBracket + 1, '[');
	char *lastBracket = strrchr(params, ']');
	if (!middleBracket || !lastBracket || middleBracket >= lastBracket)
	return -1;


	int numParams = sscanf(params, " (%d,%d)", &channel1, &channel2);

	if ((numParams == 1 || numParams == 2) && (channel1 == 1 || channel1 == 2)) {
		char *list1Start = firstBracket + 1;
		char *list1End = middleBracket - 1;
		parse_list_values(list1Start, list1End, channel1 - 1, ch_list, 0);
		
		if (channel2 == 1 || channel2 == 2) {
			parse_list_values(list1Start, list1End, channel2 - 1, ch_list, 0);
		}

		char *list2Start = middleBracket + 1;
		char *list2End = lastBracket;
		parse_list_values(list2Start, list2End, channel1 - 1, ch_list, 1);

		if (channel2 == 1 || channel2 == 2) {
			parse_list_values(list2Start, list2End, channel2 - 1, ch_list, 1);
		}

		return 0;
	}
	return -1;
}

static int parse_start_list(
	char *command,
	volatile uint8_t *start_list
) {
	char *params = strchr(command, '(');
	if (!params)
	return -1;

	char action[6];
	if (sscanf(command, "%5s", action) != 1)
	return -1;

	if (strncmp(action, "START", 5) != 0) {
		return -1;
	}

	int number1 = 0, number2 = 0;
	int numParams = sscanf(params, " (%d,%d)", &number1, &number2);

	if (numParams == 1 && (number1 == 1 || number1 == 2)) {
		start_list[number1 - 1] = 1;
		return 0;
		} else if (numParams == 2 && (number1 == 1 || number1 == 2) && (number2 == 1 || number2 == 2)) {
		start_list[number1 - 1] = 1;
		start_list[number2 - 1] = 1;
		return 0;
	}
	return -1;
}

void stop_list(uint8_t channels_off, volatile uint16_t * ch_lvl, volatile uint8_t * start_list)
{
	start_list[0] = 0;
	start_list[1] = 0;
	if(channels_off){
		ch_lvl[0] = 0;
		ch_lvl[1] = 0;
	}
}

int parse_commands(
	char *input,
	volatile char * ch_mode,
	volatile uint16_t * ch_lvl,
	volatile uint16_t ch_list[][2][2],
	volatile uint8_t * start_list
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
		stop_list(1,ch_lvl,start_list);
		return parse_set_mode(start + 8, ch_mode, ch_lvl);
	}
	else if (strncmp(start, "SET LEVEL", 9) == 0) {
		stop_list(0,ch_lvl,start_list);
		return parse_set_level(start + 9, ch_lvl);
	}
	else if (strncmp(start, "SET LIST", 8) == 0) {
		stop_list(0,ch_lvl,start_list);
		return parse_set_list(start + 8, ch_list);
	}
	else if (strncmp(start, "START LIST", 10) == 0) {
		stop_list(1,ch_lvl,start_list);
		return parse_start_list(start, start_list);
	}
	else if (strncmp(start, "STOP LIST", 9) == 0) {
		stop_list(1,ch_lvl,start_list);
		return 0;
	}
	else return -1;
}