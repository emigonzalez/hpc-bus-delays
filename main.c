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
void receive_tasks(int rank, DelayMap *delay_map) {
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

void get_bus_stop_delay_from_row(const char* row, size_t* bus_stop, double* delay) {
    char buffer[256];
    strncpy(buffer, row, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    char *token;
    int index = 0;

    // Tokenize the row based on commas
    token = strtok(buffer, ",");
    while (token != NULL) {
        index++;
        if (index == 6) {
            *bus_stop = atoi(token);
        } else if (index == 8) {
            *delay = atof(token);
        }
        token = strtok(NULL, ",");
    }
}

void master_code(int size) {
    // Master process
    distribute_tasks(size - 1, size);

    // Create the master delay map
    DelayMap *master_map = create_delay_map();

    for (int i = 1; i < size; i++) {
        // Receive the number of key-value pairs 
        int key_count;
        MPI_Recv(&key_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int j = 0; j < key_count; j++) {
            // Receive the length of the key
            int key_length;
            MPI_Recv(&key_length, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the key
            char *key = (char *)malloc(key_length);
            MPI_Recv(key, key_length, MPI_CHAR, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the number of rows
            size_t row_count;
            MPI_Recv(&row_count, 1, MPI_UINT64_T, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the rows
            char **rows = (char **)malloc(row_count * sizeof(char *));
            for (size_t j = 0; j < row_count; j++) {
                int row_length;
                MPI_Recv(&row_length, 1, MPI_INT, i, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                rows[j] = (char *)malloc(row_length);
                MPI_Recv(rows[j], row_length, MPI_CHAR, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            for (size_t j = 0; j < row_count; j++) {
                // Insert the key and rows into the master map
                size_t bus_stop;
                double delay;
                get_bus_stop_delay_from_row(rows[j], &bus_stop, &delay);
                delay_map_insert(master_map, key, bus_stop, delay, rows[j]);
            }

            free(rows);
            free(key);
        }
        printf("\n FIN PID: %d \n", i);
    }

    printf("\n FIN FIN FIN!!!! \n");

    // Wait for all worker processes to complete
    for (int i = 1; i < size; i++) {
        MPI_Recv(NULL, 0, MPI_BYTE, i, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Now master_map contains all delays from all workers
    // printf("\n MASTER PRINTING MAP \n");
    // print_delay_map(master_map);
    // printf("\n FINISHED PRINTING MAP \n");

    // Clean up master map
    free_delay_map(master_map);

    // Collect results and finalize the delay map
    fprintf(stderr,"Master: All tasks completed.\n");
}

void worker_code(int rank) {
    // Create and populate the local delay map
    DelayMap *worker_map = create_delay_map();

    // Receive tasks from the master
    receive_tasks(rank, worker_map);

    int key_count = worker_map->count;
    // printf("Before MPI_Send %d, %d \n", 0, key_count);
    MPI_Send(&key_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    // printf("After MPI_Send %d \n", 0);

    // Iterate over the map and send each key and its rows to the master
    for (size_t i = 0; i < worker_map->size; i++) {
        DelayEntry *entry = worker_map->buckets[i];
        while (entry) {
            // Send the key length
            int key_length = strlen(entry->key) + 1;
            // printf("Before MPI_Send %d, %d \n", 1, key_length);
            MPI_Send(&key_length, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            // printf("After MPI_Send %d \n", 1);
            // printf("Before MPI_Send %d, %s \n", 2, entry->key);
            MPI_Send(entry->key, key_length, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
            // printf("After MPI_Send %d \n", 2);
            // Send the number of rows
            // printf("Before MPI_Send %d, %ld \n", 3, entry->row_count);
            MPI_Send(&entry->row_count, 1, MPI_UINT64_T, 0, 3, MPI_COMM_WORLD);
            // printf("After MPI_Send %d \n", 3);

            // Send each row
            for (size_t j = 0; j < entry->row_count; j++) {
                int row_length = strlen(entry->rows[j]->row) + 4;
                // printf("Before MPI_Send %d, %d \n", 4, row_length);
                MPI_Send(&row_length, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
                // printf("After MPI_Send %d \n", 4);
                // printf("Before MPI_Send %d, %s \n", 5, entry->rows[j]->row);
                MPI_Send(entry->rows[j]->row, row_length, MPI_CHAR, 0, 5, MPI_COMM_WORLD);
                // printf("After MPI_Send %d \n", 5);
            }

            entry = entry->next;
        }
    }

    // Notify the master that all data has been sent
    // printf("Before MPI_Send %d \n", 6);
    MPI_Send(NULL, 0, MPI_CHAR, 0, 6, MPI_COMM_WORLD);
    // printf("After MPI_Send %d \n", 6);

    // Clean up worker map
    free_delay_map(worker_map);
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

    if (rank == 0) {
        // Master code
        master_code(size);
    } else {
        // Worker code
        worker_code(rank);
    }

    // Free allocated memory
    // free_memory();

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
