#ifndef MASTER_H
#define MASTER_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "file_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"
#include "string_array.h"
#include "worker.h"

void master_code(int size, int num_hours_per_day, char** strings, DelayMap *master_map);
void run_single_instance(int from_day, int num_days, int num_hours_per_day);

#endif // MASTER_H
