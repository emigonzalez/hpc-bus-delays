#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_distribute.h"
#include "schedule_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // File names and schedule file
    char* schedule_file = "data/bus_schedules.csv";

    // Generate the list of file names (example for June, 24 files per day)
    int num_days = 2;
    int num_hours_per_day = 5;
    char** file_names = generate_file_names(num_days, num_hours_per_day);

    // Calculate total number of files
    int num_files = num_days * num_hours_per_day;

    // Distribute file names among processes
    char** assigned_files = distribute_file_names(file_names, num_files, rank, size);

    printf("BUS LOCATIONS \n");
    // Each process reads its assigned location files
    for (int i = 0; assigned_files[i] != NULL; i++) {
        printf("Process %d reading file %s\n", rank, assigned_files[i]);
        // read_file_and_process_data(assigned_files[i]);
    }

    printf("BUS SCHEDULES \n");
    // Each process reads the entire schedule file
    printf("Process %d reading schedule file %s\n", rank, schedule_file);
    read_schedule_data(schedule_file, rank);

    // Group data by bus number, time, and day of month/type of day
    group_data_by_bus_and_time();

    // Map grouped locations to grouped schedules
    map_locations_to_schedules();

    // Calculate delays using the Python black box
    calculate_delays();

    // Gather results from all processes
    gather_results();

    // Free allocated memory
    for (int i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);

    MPI_Finalize();
    return 0;
}
