#ifndef LOCATION_MAPPING_H
#define LOCATION_MAPPING_H

#include "data_grouping.h"

const char *vfd_filename = "data/vfd.csv";
const char *capturas_filename = "data/capturas.csv";
const char *horarios_filename = "data/horarios.csv";

HashMap* group_schedules(char* horarios);
void map_locations_to_schedules();

#endif // LOCATION_MAPPING_H
