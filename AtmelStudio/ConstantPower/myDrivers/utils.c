/*
 * utils.c
 *
 * Created: 03/04/2024 14:50:10
 *  Author: mateu
 */ 

#include "utils.h"

void parse_cmd(volatile uint8_t *cmd, uint8_t *mode_out, int8_t *level_out)
{
	uint8_t mode[2] = {'X','X'};
	int16_t level[2] = {-1,-1};
		
	uint8_t i = 0, nidx = 0, cidx = 0;
	
	while (cmd[i] != ';') {
		if ((cmd[i] == 'I' || cmd[i] == 'P' || cmd[i] == 'X') & (cidx < 2))
				mode[cidx++] = cmd[i];
		else if (cmd[i] >= '0' && cmd[i] <= '9' && nidx < 2) {
			int16_t num = 0;
			while (cmd[i] != ',' && cmd[i] != ';' && cmd[i] >= '0' && cmd[i] <= '9')
			{
				num = num * 10 + (cmd[i] - '0');
				i++;
			}
			i--;
			level[nidx++] = num > 100 ? -1 : num;
		}
		i++;
	}
	for(uint8_t k=0; k<2; k++){
		mode_out[k] = mode[k];
		level_out[k] = level[k];
	}
}