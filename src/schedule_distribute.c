#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schedule_distribute.h"

void read_schedule_data_and_distribute(char* schedule_filename, int rank, int size) {
    if (rank == 0) {
        FILE* file = fopen(schedule_filename, "r");
        if (file == NULL) {
            perror("Error opening file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char** schedule_data = NULL;
        size_t num_rows = 0;
        size_t allocated_rows = 100000;
        schedule_data = (char**)malloc(allocated_rows * sizeof(char*));

        char line[1024];
        while (fgets(line, sizeof(line), file)) {
            if (num_rows == allocated_rows) {
                allocated_rows *= 2;
                schedule_data = (char**)realloc(schedule_data, allocated_rows * sizeof(char*));
            }
            schedule_data[num_rows] = strdup(line);
            num_rows++;
        }
        fclose(file);

        int rows_per_process = num_rows / size;
        int extra_rows = num_rows % size;

        for (int i = 1; i < size; i++) {
            int start_idx = i * rows_per_process + (i < extra_rows ? i : extra_rows);
            int end_idx = start_idx + rows_per_process + (i < extra_rows);

            int num_rows_to_send = end_idx - start_idx;
            MPI_Send(&num_rows_to_send, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            for (int j = start_idx; j < end_idx; j++) {
                int len = strlen(schedule_data[j]) + 1;
                MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(schedule_data[j], len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            }
        }

        for (size_t i = 0; i < num_rows; i++) {
            free(schedule_data[i]);
        }
        free(schedule_data);
    } else {
        int num_rows_to_receive;
        MPI_Recv(&num_rows_to_receive, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char** schedule_data = (char**)malloc(num_rows_to_receive * sizeof(char*));
        for (int i = 0; i < num_rows_to_receive; i++) {
            int len;
            MPI_Recv(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            schedule_data[i] = (char*)malloc(len * sizeof(char));
            MPI_Recv(schedule_data[i], len, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < num_rows_to_receive; i++) {
            free(schedule_data[i]);
        }
        free(schedule_data);
    }

    return;
}
