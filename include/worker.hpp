#ifndef WORKER_H
#define WORKER_H

#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <vector>

#include "file_distribute.hpp"
#include "data_grouping.hpp"
#include "location_mapping.hpp"
#include "delay_calculation.hpp"
#include "result_gathering.hpp"
#include "string_array.hpp"

void worker_code(int rank, int num_hours_per_day);
void perform_task(int rank, std::vector<std::string> assigned_days, int num_hours_per_day, DelayMap *delay_map);

#endif // WORKER_H

