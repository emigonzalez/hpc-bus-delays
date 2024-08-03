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

#define FROM_DAY 10
#define NUM_DAYS 1
#define NUM_HOURS_PER_DAY 24

char* horarios = NULL;
char** capturas = NULL;
char** directorios = NULL;
char** assigned_days = NULL;
HashMap *vfd_map = NULL;
HashMap* vft_map = NULL;

void free_memory() {
    // Perform any necessary cleanup here
    if (capturas != NULL) {
        for (int i = 0; i < NUM_DAYS * NUM_HOURS_PER_DAY; i++) {
            free(capturas[i]);
        }
        free(capturas);
    }

    if (assigned_days != NULL) {
        for (int i = 0; assigned_days[i] != NULL; i++) {
            free(assigned_days[i]);
        }
        free(assigned_days);
    }

    if (directorios != NULL) {
         for (int i = 0; i < NUM_DAYS; i++) {
            free(directorios[i]);
        }
        free(directorios);
    }

    if (vft_map != NULL) free_hash_map(vft_map);
    if (vfd_map != NULL) free_hash_map(vfd_map);
}

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    if (FROM_DAY + NUM_DAYS > 31) {
        printf("INVALID VALUES. FROM_DAY = %d, NUM_DAYS = %d\n", FROM_DAY, NUM_DAYS);
        exit(1);
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal

    // Generate the list of file names (example for June, 24 files per day)
    directorios = generate_directories(FROM_DAY, NUM_DAYS);

    // Distribute dirs among processes
    assigned_days = distribute(directorios, NUM_DAYS, rank, size);

    // vft_map = group_schedules(horarios);

    // Each process reads its assigned directories
    for (int i = 0; assigned_days[i] != NULL; i++) {
        char * day_str = get_day_from_dir_name(assigned_days[i]);
        printf("Process %d reading file %s from day %s\n", rank, assigned_days[i], day_str);

        capturas = generate_location_file_names(assigned_days[i], atoi(day_str), NUM_HOURS_PER_DAY);
        horarios = generate_schedule_file_name("data/horarios", atoi(day_str));

        printf("HORARIOS: %s\n", horarios);
        vft_map = group_schedules(horarios);

        // Iterate over each location file
        for (int j = 0; j < NUM_HOURS_PER_DAY; j++) {
            // Generate VFD map and all fields to be picked by Python script
            map_locations_to_schedules(capturas[j], vft_map);

            printf("####### RUNNING WITH FILE: %s ########\n", capturas[j]);

            // Run Python script
            calculate_delays();
        }

        free(horarios);
        free_hash_map(vft_map);
    }

    // Free allocated memory
    free_memory();

    MPI_Finalize();
    return 0;
}
