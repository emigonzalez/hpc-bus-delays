#ifndef LOCATION_MAPPING_H
#define LOCATION_MAPPING_H

#include "data_grouping.h"

HashMap* group_schedules(char* horarios);
void map_locations_to_schedules(char* fileName, char* date, HashMap* vft_map);

#endif // LOCATION_MAPPING_H
