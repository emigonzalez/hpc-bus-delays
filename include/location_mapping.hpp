#ifndef LOCATION_MAPPING_H
#define LOCATION_MAPPING_H

#include <string>
#include "hash_map.hpp"

HashMap* group_schedules(std::string horarios);
int map_locations_to_schedules(std::string fileName, std::string date, HashMap* vft_map);

#endif // LOCATION_MAPPING_H
