#include "master.h"

const char* sales_filename = "data/viajes/viajes_por_Variante_dia_parada.csv";
const char* output_filename = "data/retrasos/resumen.csv";

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

void master_code(int size, int num_hours_per_day, char** strings) {
    DelayMap *master_map = create_delay_map();

    for (int k = 0; strings[k] != NULL; k++) {
        // Perform task (e.g., process delays)
        perform_task(0, strings[k], num_hours_per_day, master_map);

        for (int i = 1; i < size; i++) {
            // Receive the number of key-value pairs 
            int key_count;
            MPI_Recv(&key_count, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < key_count; j++) {
                // Receive the length of the key
                int key_length;
                MPI_Recv(&key_length, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Receive the key
                char *key = (char *)malloc(key_length);
                MPI_Recv(key, key_length, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Receive the number of rows
                size_t row_count;
                MPI_Recv(&row_count, 1, MPI_UINT64_T, i, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Receive the rows
                char **rows = (char **)malloc(row_count * sizeof(char *));
                for (size_t j = 0; j < row_count; j++) {
                    int row_length;
                    MPI_Recv(&row_length, 1, MPI_INT, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    rows[j] = (char *)malloc(row_length);
                    MPI_Recv(rows[j], row_length, MPI_CHAR, i, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                for (size_t j = 0; j < row_count; j++) {
                    // Insert the key and rows into the master map
                    size_t bus_stop;
                    double delay;
                    get_bus_stop_delay_from_row(rows[j], &bus_stop, &delay);
                    delay_map_insert(master_map, key, bus_stop, delay, rows[j]);
                    // free(rows[j]);
                }

                free(rows);
                free(key);
            }
        }

        // Wait for all worker processes to complete
        for (int i = 1; i < size; i++) {
            MPI_Recv(NULL, 0, MPI_BYTE, i, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    // Generate CSV with delays and people affected by it
    generate_csv(master_map, sales_filename, output_filename);

    // Clean up master map
    free_delay_map(master_map);

    // Collect results and finalize the delay map
    fprintf(stderr,"Master: All tasks completed.\n");
}

void run_single_instance(int from_day, int num_days, int num_hours_per_day) {
    // Generate the list of dir names
    char** directorios = generate_directories(from_day, num_days);

    if (directorios == NULL) {
        return;
    }

    DelayMap *delay_map = create_delay_map();

    for (int i = 0; directorios[i] != NULL; i++) {
        perform_task(0, directorios[i], num_hours_per_day, delay_map);
    }

    if (delay_map == NULL) return;

    generate_csv(delay_map, sales_filename, output_filename);

    free_delay_map(delay_map);

    fprintf(stderr,"All tasks completed.\n");
}
