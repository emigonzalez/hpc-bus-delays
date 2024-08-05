#ifndef DELAY_CALCULATION_H
#define DELAY_CALCULATION_H

#include "delay_map.h"

void python_calculate_delays(int day);
int map_delays(DelayMap* delay_map, char* filename);

#endif // DELAY_CALCULATION_H
