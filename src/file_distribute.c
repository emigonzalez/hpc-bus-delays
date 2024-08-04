#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_distribute.h"

char** generate_directories(int from_day, int num_days) {
    char** directories = (char**)malloc(num_days * sizeof(char*));
    for (int day = 1; day <= num_days; day++) {
        directories[(day-1)] = (char*)malloc(26 * sizeof(char));
        sprintf(directories[(day-1)], "data/capturas/2024-06-%02d", from_day + day-1);
    }
    return directories;
}

char** generate_location_file_names(char* path, int day, int num_hours_per_day) {
    char** file_names = (char**)malloc(num_hours_per_day * sizeof(char*));
    for (int hour = 0; hour < num_hours_per_day; hour++) {
        file_names[hour] = (char*)malloc(53 * sizeof(char));
        sprintf(file_names[hour], "%s/stm-buses-2024-06-%02d_%02d.csv", path, day, hour);
    }
    return file_names;
}

char* generate_schedule_file_name(char* path, int day) {
    char* file_name = (char*)malloc(50 * sizeof(char));
    sprintf(file_name, "%s/uptu_pasada_variante_2024-06-%02d.csv", path, day);

    return file_name;
}

char* generate_delay_file_name(char* path, int day) {
    char* file_name = (char*)malloc(38 * sizeof(char));
    sprintf(file_name, "%s/retrasos_2024-06-%02d.csv", path, day);

    return file_name;
}

int distribute(char** file_names, int num_files, int rank, int size, char*** assigned_files) {
    int files_per_process = num_files / size;
    int extra_files = num_files % size;
    int task = rank-1;

    int start_idx = task * files_per_process + (task < extra_files ? task : extra_files);
    int end_idx = start_idx + files_per_process + (task < extra_files);

    int assigned_count = end_idx - start_idx;
    *assigned_files = (char**)malloc((assigned_count + 1) * sizeof(char*));
    for (int i = start_idx; i < end_idx; i++) {
        *assigned_files[i - start_idx] = strdup(file_names[i]);
    }
    *assigned_files[assigned_count] = NULL; // Null-terminate the array

    return assigned_count;
}

char* get_day_from_dir_name(const char *str) {
    size_t len = strlen(str);
    if (len < 2) {
        return NULL; // or handle this case as needed
    }
    return (char*)str + len - 2; // cast to char* to avoid const warning
}
