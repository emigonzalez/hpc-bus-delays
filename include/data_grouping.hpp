#ifndef DATA_GROUPING_H
#define DATA_GROUPING_H

#include "hash_map.hpp"

HashMap* group_data_by_vft(std::string assigned_files);
HashMap* group_data_by_vfd(std::string assigned_files, HashMap* vft_map);

#endif // DATA_GROUPING_H
