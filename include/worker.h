#ifndef WORKER_H
#define WORKER_H

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

void worker_code(int rank, int num_hours_per_day);

#endif // WORKER_H
