#include <mpi.h>
#include <stdio.h>
#include "file_distribute.h"

void distribute_file_names(char** file_names, int num_files, int rank, int size) {
    int files_per_process = num_files / size;
    int extra_files = num_files % size;

    int start_idx = rank * files_per_process + (rank < extra_files ? rank : extra_files);
    int end_idx = start_idx + files_per_process + (rank < extra_files);

    

    for (int i = start_idx; i < end_idx; i++) {
        // Read file and process data
        // read_file_and_load_data(file_names[i]);
        printf("pid: %d, start_idx: %d, end_idx: %d, file_name: %s \n", rank, start_idx, end_idx, file_names[i]);
    }
}
