#ifndef DATA_GROUPING_H
#define DATA_GROUPING_H

#include "hash_map.h"

HashMap* group_data_by_vft(char* assigned_files);
HashMap* group_data_by_vfd(char* assigned_files, HashMap* vft_map);

#endif // DATA_GROUPING_H
