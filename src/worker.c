#include "worker.h"

void perform_task(int rank, char* assigned_days, int num_hours_per_day, DelayMap *delay_map) {
    // Each process reads its assigned directories
    char * day_str = get_day_from_dir_name(assigned_days);
    fprintf(stderr,"\n   Proceso %d leyendo archivos '%s' del dia  %s   \n", rank, assigned_days, day_str);

    char** capturas = generate_location_file_names(assigned_days, atoi(day_str), num_hours_per_day);

    if (capturas == NULL) {
        return;
    }
    
    char* horarios = generate_schedule_file_name("data/horarios", atoi(day_str));

    if (horarios == NULL) {
            free_string_array(capturas, num_hours_per_day);
        return;
    }

    HashMap* vft_map = group_schedules(horarios, rank);

    if (vft_map == NULL) {
            free_string_array(capturas, num_hours_per_day);
        free(horarios);
        return;
    }

    HashMap* vfd_map = create_hash_map();

    // Iterate over each location file
    for (int j = 0; j < num_hours_per_day; j++) {
        // Generate VFD map and all fields to be picked by Python script
        int ok = map_locations_to_schedules(capturas[j], assigned_days, vft_map, vfd_map);
    

        if (!ok) continue;

        int len = strlen(capturas[j]);
        printf(" %s \n", capturas[j] + len - 28);

    }
    // printf("VFD HashMap (Buckets: %zu, Keys: %zu)       ", vfd_map->size, vfd_map->count);

    // Run Python script
    python_calculate_delays(atoi(day_str));

    char* delay_file = generate_delay_file_name("data/retrasos", atoi(day_str));
    map_delays(delay_map, delay_file);

        fprintf(stderr,"####### FINALIZADO ATRASOS PROCESO %d PARA LA CARPETA: %s ########\n", rank, delay_file);
        // Free the allocated memory

        free(delay_file); delay_file = NULL;
        free_vfd_hash_map(vfd_map); vfd_map = NULL;
        free_hash_map(vft_map); vft_map = NULL;
        free(horarios); horarios = NULL;
        free_string_array(capturas, num_hours_per_day);

    // Synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);
}

void worker_code(int rank, int num_hours_per_day, char** strings) {
    // Iterate over each day
    for (int i = 0; strings[i] != NULL; i++) {
        DelayMap *worker_map = create_delay_map();

        // Perform task (e.g., process delays)
        perform_task(rank, strings[i], num_hours_per_day, worker_map);

        int key_count = worker_map->count;
        // printf("Before MPI_Send %d, %d \n", 0, key_count);
        MPI_Ssend(&key_count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        // printf("After MPI_Send %d \n", 0);

        // Iterate over the map and send each key and its rows to the master
        for (size_t i = 0; i < worker_map->size; i++) {
            DelayEntry *entry = worker_map->buckets[i];
            while (entry) {
                // Send the key length
                int key_length = strlen(entry->key) + 1;
                // printf("Before MPI_Send %d %d, %d \n", rank, 2, key_length);
                MPI_Ssend(&key_length, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
                // printf("After MPI_Send %d %d \n", rank, 2);
                // printf("Before MPI_Send %d %d, %s \n", rank, 3, entry->key);
                MPI_Ssend(entry->key, key_length, MPI_CHAR, 0, 3, MPI_COMM_WORLD);
                // printf("After MPI_Send %d %d \n", rank, 3);
                // Send the number of rows
                // printf("Before MPI_Send %d %d, %ld \n", rank, 4, entry->row_count);
                MPI_Ssend(&entry->row_count, 1, MPI_UINT64_T, 0, 4, MPI_COMM_WORLD);
                // printf("After MPI_Send %d %d \n", rank, 4);

                // Send each row
                for (size_t j = 0; j < entry->row_count; j++) {
                    int row_length = strlen(entry->rows[j]->row) + 4;
                    // printf("Before MPI_Send %d %d, %d \n", rank, 5, row_length);
                    MPI_Ssend(&row_length, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);
                    // printf("After MPI_Send %d %d \n", rank, 5);
                    // printf("Before MPI_Send %d %d, %s \n", rank, 6, entry->rows[j]->row);
                    MPI_Ssend(entry->rows[j]->row, row_length, MPI_CHAR, 0, 6, MPI_COMM_WORLD);
                    // printf("After MPI_Send %d %d \n", rank, 6);
                }

                entry = entry->next;
            }
        }
        // Notify the master that all data has been sent
        MPI_Ssend(NULL, 0, MPI_CHAR, 0, 7, MPI_COMM_WORLD);

        // Clean up worker map
        free_delay_map(worker_map);
        worker_map = NULL;
    }

    printf("\n PROCESO %d FINALIZO! \n", rank);
}
