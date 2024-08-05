#ifndef DELAY_CALCULATION_H
#define DELAY_CALCULATION_H

#include "delay_map.hpp"

void python_calculate_delays(int day);
int map_delays(DelayMap* delay_map, std::string filename);

#endif // DELAY_CALCULATION_H
