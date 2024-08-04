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
#include "master.h"

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

    if (rank == 0) {
        // Master code
        master_code(size, FROM_DAY, NUM_DAYS);
    } else {
        // Worker code
        worker_code(rank, NUM_HOURS_PER_DAY);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
