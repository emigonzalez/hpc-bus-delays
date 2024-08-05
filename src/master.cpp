#include "master.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <mpi.h>

const std::string sales_filename = "data/viajes/viajes_por_Variante_dia_parada.csv";
const std::string output_filename = "data/retrasos/resumen.csv";

// Function for master to distribute tasks
void distribute_tasks(int size, int from_day, int num_days) {
    // Generate the list of dir names
    std::vector<std::string> directorios = generate_directories(from_day, num_days);

    for (int i = 1; i < size; i++) {
        // Distribute dirs among processes
        std::vector<std::string> assigned_days;
        int len = distribute(directorios, num_days, i, size, assigned_days);

        // Send the dirs to worker
        send_string_array(assigned_days, len, i, 0, MPI_COMM_WORLD);
    }
}

void get_bus_stop_delay_from_row(const std::string& row, size_t* bus_stop, double* delay) {
    std::string buffer = row;
    std::string token;
    int index = 0;

    // Tokenize the row based on commas
    size_t pos = 0;
    while ((pos = buffer.find(',')) != std::string::npos) {
        token = buffer.substr(0, pos);
        index++;
        if (index == 6) {
            *bus_stop = std::stoi(token);
        } else if (index == 8) {
            *delay = std::stod(token);
        }
        buffer.erase(0, pos + 1);
    }
}

void master_code(int size, int from_day, int num_days) {
    // Master process
    distribute_tasks(size, from_day, num_days);

    // Create the master delay map
    DelayMap* master_map = new DelayMap();

    for (int i = 1; i < size; i++) {
        // Receive the number of key-value pairs 
        int key_count;
        MPI_Recv(&key_count, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int j = 0; j < key_count; j++) {
            // Receive the length of the key
            int key_length;
            MPI_Recv(&key_length, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the key
            std::string key(key_length, '\0');
            MPI_Recv(&key[0], key_length, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the number of rows
            size_t row_count;
            MPI_Recv(&row_count, 1, MPI_UINT64_T, i, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive the rows
            std::vector<std::string> rows(row_count);
            for (size_t j = 0; j < row_count; j++) {
                int row_length;
                MPI_Recv(&row_length, 1, MPI_INT, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                rows[j].resize(row_length);
                MPI_Recv(&rows[j][0], row_length, MPI_CHAR, i, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            for (const auto& row : rows) {
                // Insert the key and rows into the master map
                size_t bus_stop;
                double delay;
                get_bus_stop_delay_from_row(row, &bus_stop, &delay);
                master_map->delay_map_insert(key.c_str(), bus_stop, delay, row.c_str());
            }
        }
    }

    // Wait for all worker processes to complete
    for (int i = 1; i < size; i++) {
        MPI_Recv(NULL, 0, MPI_BYTE, i, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    generate_csv(master_map, sales_filename.c_str(), output_filename.c_str());

    // Clean up master map
    delete master_map;

    // Collect results and finalize the delay map
    fprintf(stderr, "Master: All tasks completed.\n");
}

void run_single_instance(int from_day, int num_days, int num_hours_per_day) {
    // Generate the list of dir names
    std::vector<std::string> directorios = generate_directories(from_day, num_days);

    if (directorios.empty()) {
        return;
    }

    std::vector<std::string> assigned_days;
    distribute(directorios, num_days, 0, 1, assigned_days);

    if (assigned_days.empty()) {
        return;
    }

    DelayMap* delay_map = new DelayMap();

    perform_task(0, assigned_days, num_hours_per_day, delay_map);

    if (delay_map == NULL) return;

    generate_csv(delay_map, sales_filename.c_str(), output_filename.c_str());

    delete delay_map;
}
