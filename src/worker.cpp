#include "delay_calculation.hpp"
#include "file_distribute.hpp"
#include "location_mapping.hpp"
#include "string_array.hpp"
#include "worker.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <mpi.h>

// Perform the task of processing files and delays
void perform_task(int rank, std::vector<std::string> assigned_days, int num_hours_per_day, DelayMap* delay_map) {
    // Each process reads its assigned directories
    for (const auto& day : assigned_days) {
        std::string day_str = get_day_from_dir_name(day);
        std::cerr << "\n Process " << rank << " reading file " << day << " from day " << day_str << "\n";

        std::vector<std::string> capturas = generate_location_file_names(day, std::stoi(day_str), num_hours_per_day);

        if (capturas.empty()) {
            continue;
        }

        std::string horarios = generate_schedule_file_name("data/horarios", std::stoi(day_str));

        if (horarios.empty()) {
            continue;
        }

        std::cout << "HORARIOS: " << horarios << std::endl;
        HashMap* vft_map = group_schedules(horarios);

        if (vft_map == nullptr) {
            continue;
        }

        // Iterate over each location file
        for (int j = 0; j < num_hours_per_day; ++j) {
            // Generate VFD map and all fields to be picked by Python script
            int ok = map_locations_to_schedules(capturas[j], day, vft_map);

            if (!ok) continue;

            int len = capturas[j].length();
            std::cout << " " << capturas[j].substr(len - 28) << " \n";

            // Run Python script
            python_calculate_delays(std::stoi(day_str));
        }

        std::string delay_file = generate_delay_file_name("data/retrasos", std::stoi(day_str));
        map_delays(delay_map, delay_file);
        std::cerr << "####### FINISHED DELAY " << rank << " FOR FILE: " << delay_file << " ########\n";
    }
}

// Function for workers to receive tasks
void receive_tasks(int rank, int num_hours_per_day, DelayMap* delay_map) {
    // Worker processes
    std::vector<std::string> received_strings;
    int num_strings;

    // Receive the array of strings from the master process
    recv_string_array(received_strings, &num_strings, 0, 0, MPI_COMM_WORLD);

    std::cerr << "\n ******* Process " << rank << " received array: " << num_strings << " ***********\n";

    // Perform task (e.g., process delays)
    perform_task(rank, received_strings, num_hours_per_day, delay_map);

    std::cerr << "\n ******* THE END Process " << rank << " received array: " << num_strings << " ***********\n";
}

// Worker code for MPI processing
void worker_code(int rank, int num_hours_per_day) {
    // Create and populate the local delay map
    DelayMap* worker_map = new DelayMap();

    // Receive tasks from the master
    receive_tasks(rank, num_hours_per_day, worker_map);

    size_t key_count = worker_map->get_key_count();
    MPI_Ssend(&key_count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

    std::vector<DelayEntry*> entries = worker_map->delay_map_get_all_keys(key_count);
    // Iterate over the map and send each key and its rows to the master
    for (DelayEntry* entry: entries) {
        // Send the key length
        int key_length = static_cast<int>(entry->key.size() + 1);
        MPI_Ssend(&key_length, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

        // Send the key
        MPI_Ssend(&entry->key, key_length, MPI_CHAR, 0, 3, MPI_COMM_WORLD);

        size_t row_count = entry->rows.size();
        // Send the number of rows
        MPI_Ssend(&row_count, 1, MPI_UINT64_T, 0, 4, MPI_COMM_WORLD);

        // Send each row
        for (auto row : entry->rows) {
            int row_length = static_cast<int>(row->row.size() + 4);
            MPI_Ssend(&row_length, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);
            MPI_Ssend(&row->row, row_length, MPI_CHAR, 0, 6, MPI_COMM_WORLD);
        }
    }

    // Notify the master that all data has been sent
    MPI_Ssend(nullptr, 0, MPI_CHAR, 0, 7, MPI_COMM_WORLD);

    // Clean up worker map
    delete worker_map;
}
