#ifndef DATA_GROUPING_H
#define DATA_GROUPING_H

#include "hash_map.h"

HashMap* group_data_by_vft(char* assigned_files);
void group_data_by_vfd(char* filename, HashMap* vft_map, HashMap* vfd_map);

#endif // DATA_GROUPING_H
