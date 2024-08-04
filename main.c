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

#define FROM_DAY 10
#define NUM_DAYS 1
#define NUM_HOURS_PER_DAY 24

char* horarios = NULL;
char** capturas = NULL;
char** directorios = NULL;
char** assigned_days = NULL;
HashMap* vft_map = NULL;

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    // free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}

void perform_task(int rank, char** assigned_days, DelayMap *delay_map) {
    // Each process reads its assigned directories
    for (int i = 0; assigned_days[i] != NULL; i++) {
        char * day_str = get_day_from_dir_name(assigned_days[i]);
        printf("Process %d reading file %s from day %s\n", rank, assigned_days[i], day_str);

        char** capturas = generate_location_file_names(assigned_days[i], atoi(day_str), NUM_HOURS_PER_DAY);
        char* horarios = generate_schedule_file_name("data/horarios", atoi(day_str));

        printf("HORARIOS: %s\n", horarios);
        HashMap* vft_map = group_schedules(horarios);

        // Iterate over each location file
        for (int j = 0; j < NUM_HOURS_PER_DAY; j++) {
            // Generate VFD map and all fields to be picked by Python script
            map_locations_to_schedules(capturas[j], assigned_days[i], vft_map);

            printf("####### RUNNING WITH FILE: %s ########\n", capturas[j]);

            // Run Python script
            // python_calculate_delays();
        }

        char* delay_file = generate_delay_file_name("data/retrasos", atoi(day_str));
        map_delays(delay_map, delay_file);
        free(delay_file);
        free(horarios); horarios = NULL;
        free_string_array(capturas); capturas = NULL;
        free_hash_map(vft_map); vft_map = NULL;
    }
}

// Function for workers to receive tasks
void receive_tasks(int rank, int size, DelayMap *delay_map) {
    // Worker processes
    char **received_strings;
    int num_strings;

    // Receive the array of strings from the master process
    recv_string_array(&received_strings, &num_strings, 0, 0, MPI_COMM_WORLD);

    printf("Process %d received array: %d\n", rank, num_strings);

    // Perform task (e.g., process delays)
    perform_task(rank, received_strings, delay_map);

    // Free the allocated memory
    free_string_array(received_strings);
}

// Function for master to distribute tasks
void distribute_tasks(int num_tasks, int size) {
    // Generate the list of dir names
    char** directorios = generate_directories(FROM_DAY, NUM_DAYS);

    for (int i = 1; i < size; i++) {
        // Distribute dirs among processes
        char** assigned_days = NULL;
        int len = distribute(directorios, NUM_DAYS, i, size, &assigned_days);
        
        // Send the dirs to worker
        send_string_array(assigned_days, len, i, 0, MPI_COMM_WORLD);

        // Free the array
        free_string_array(assigned_days);
    }

    free_string_array(directorios);
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

    DelayMap *delay_map = create_delay_map();
    // MPI_Win win;

    // Create or access the memory window for the delay map
    // create_delay_map_window(&delay_map, &win, rank, size);

    // Synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        // Master process
        distribute_tasks(size - 1, size);

        // Wait for all worker processes to complete
        for (int i = 1; i < size; i++) {
            MPI_Recv(NULL, 0, MPI_BYTE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Collect results and finalize the delay map
        fprintf(stderr,"Master: All tasks completed.\n");
    } else {
        // Worker processes access the delay map through the memory window
        // access_delay_map_window(&delay_map, win, rank);

        // Receive tasks from the master
        receive_tasks(rank, size, delay_map);

        // Notify the master that the task is complete
        MPI_Send(NULL, 0, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }

    // Free allocated memory
    // free_memory();
    free_delay_map(delay_map);

    // Finalize the MPI environment
    // MPI_Win_free(&win);
    MPI_Finalize();
    return 0;
}
