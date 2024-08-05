#include <mpi.h>
#include <iostream>
#include <csignal>
#include <cstdlib>

#include "data_grouping.hpp"
#include "date_to_day_type.hpp"
#include "delay_calculation.hpp"
#include "delay_map.hpp"
#include "file_distribute.hpp"
#include "hash_map.hpp"
#include "location_mapping.hpp"
#include "master.hpp"
#include "result_gathering.hpp"
#include "string_array.hpp"
#include "ticket_map.hpp"
#include "worker.hpp"


constexpr int FROM_DAY = 10;
constexpr int NUM_DAYS = 3;
constexpr int NUM_HOURS_PER_DAY = 1;

void handle_signal(int signal) {
    std::cout << "Received signal " << signal << ", performing cleanup..." << std::endl;

    MPI_Finalize();

    std::cout << "Cleanup done, exiting..." << std::endl;
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    if (FROM_DAY + NUM_DAYS > 31) {
        std::cerr << "INVALID VALUES. FROM_DAY = " << FROM_DAY << ", NUM_DAYS = " << NUM_DAYS << std::endl;
        exit(1);
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    std::signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    std::signal(SIGTERM, handle_signal); // Handle termination signal

    if (size > 1) {
        if (rank == 0) {
            // Master code
            master_code(size, FROM_DAY, NUM_DAYS);
        } else {
            // Worker code
            worker_code(rank, NUM_HOURS_PER_DAY);
        }
    } else {
        run_single_instance(FROM_DAY, NUM_DAYS, NUM_HOURS_PER_DAY);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
