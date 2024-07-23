#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "schedule_distribute.h"

void read_schedule_data(const char* schedule_file, int rank) {
    FILE* file = fopen(schedule_file, "r");
    if (file == NULL) {
        perror("Error opening schedule file");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Read and process the schedule data as needed
    // Example: print the first few lines to verify reading
    char line[256];
    int line_count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line_count < 5) {
            printf("PID: %d. Schedule: %s", rank, line);
        }
        line_count++;
    }

    fclose(file);
}
