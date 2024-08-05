#ifndef MASTER_H
#define MASTER_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "file_distribute.hpp"
#include "data_grouping.hpp"
#include "location_mapping.hpp"
#include "delay_calculation.hpp"
#include "result_gathering.hpp"
#include "string_array.hpp"
#include "worker.hpp"

void master_code(int size, int from_day, int num_days);
void run_single_instance(int from_day, int num_days, int num_hours_per_day);

#endif // MASTER_H
