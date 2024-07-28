// #include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "file_distribute.h"
#include "schedule_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"

/*
#define NUM_DAYS 30
#define NUM_HOURS_PER_DAY 24

char** file_names = NULL;
char** assigned_files = NULL;

void handle_signal(int signal);
void free_memory();

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal

    // File names and schedule file
    char* schedule_file = "data/bus_schedules.csv";

    // Generate the list of file names (example for June, 24 files per day)
    file_names = generate_file_names(NUM_DAYS, NUM_HOURS_PER_DAY);

    // Calculate total number of files
    int num_files = NUM_DAYS * NUM_HOURS_PER_DAY;

    // Distribute file names among processes
    assigned_files = distribute_file_names(file_names, num_files, rank, size);

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

    // Group data by bus variant, time, and day of month/type of day
    group_data_by_bus_and_time(assigned_files);

    // Map grouped locations to grouped schedules
    map_locations_to_schedules();

    // Calculate delays using the Python black box
    calculate_delays();

    // Gather results from all processes
    gather_results();

    // Free allocated memory
    free_memory();

    MPI_Finalize();
    return 0;
}

void free_memory() {
    // Perform any necessary cleanup here
    if (file_names != NULL) {
        for (int i = 0; i < NUM_DAYS * NUM_HOURS_PER_DAY; i++) {
            free(file_names[i]);
        }
        free(file_names);
    }

    if (assigned_files != NULL) {
        for (int i = 0; assigned_files[i] != NULL; i++) {
            free(assigned_files[i]);
        }
        free(assigned_files);
    }
}

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}

*/

int main() {
    char** assigned_files = (char**)malloc(2 * sizeof(char*));
    assigned_files[0] = "data/uptu_pasada_variante.csv";
    assigned_files[1] = NULL; // Terminate the list

    group_data_by_vft(assigned_files);

    free(assigned_files);
    return EXIT_SUCCESS;
}
