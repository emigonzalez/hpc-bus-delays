#ifndef MASTER_H
#define MASTER_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void master_code(int size, int from_day, int num_days);
void run_single_instance(int from_day, int num_days, int num_hours_per_day);

#endif // MASTER_H
