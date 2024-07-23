#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_distribute.h"

char** generate_file_names(int num_days, int num_hours_per_day) {
    int num_files = num_days * num_hours_per_day;
    char** file_names = (char**)malloc(num_files * sizeof(char*));
    for (int day = 1; day <= num_days; day++) {
        for (int hour = 0; hour < num_hours_per_day; hour++) {
            file_names[(day-1) * num_hours_per_day + hour] = (char*)malloc(30 * sizeof(char));
            sprintf(file_names[(day-1) * num_hours_per_day + hour], "bus_locations_%02d_%02d.txt", day, hour);
        }
    }
    return file_names;
}

char** distribute_file_names(char** file_names, int num_files, int rank, int size) {
    int files_per_process = num_files / size;
    int extra_files = num_files % size;

    int start_idx = rank * files_per_process + (rank < extra_files ? rank : extra_files);
    int end_idx = start_idx + files_per_process + (rank < extra_files);

    int assigned_count = end_idx - start_idx;
    char** assigned_files = (char**)malloc((assigned_count + 1) * sizeof(char*));
    for (int i = start_idx; i < end_idx; i++) {
        assigned_files[i - start_idx] = strdup(file_names[i]);
    }
    assigned_files[assigned_count] = NULL; // Null-terminate the array

    return assigned_files;
}
