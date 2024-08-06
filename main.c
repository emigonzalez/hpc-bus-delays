/*
 * main.c
 *
 * Descripcion: 
 *   Punto de entrada donde se inicializa MPI y cada proceso ejecuta sus tareas.
 * 
 * Autores:
 *   Emiliano Gonzalez
 *   Gabriel Acosta
 *
 * Fecha: 4 de Agosto de 2024
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "file_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"
#include "string_array.h"
#include "worker.h"
#include "master.h"

#define DEFAULT_FROM_DAY 10
#define DEFAULT_NUM_DAYS 1
#define DEFAULT_NUM_HOURS_PER_DAY 2

/**
 * @brief Maneja las señales de interrupciones para finalizar el codigo de forma segura.
 * 
 * @param signal Tipo de interrupcion e.g. SIGINT y SIGTERM
 * 
 * @return void
 */
void handle_signal(int signal) {
    printf("Received signal %d, exiting...\n", signal);

    // If some condition is met, forcefully exit
    if (signal > 0) {
        MPI_Abort(MPI_COMM_WORLD, signal);
    } else {
        MPI_Finalize();
    }

    exit(signal);
}

/**
 * @brief Funcion principal de la aplicacion.
 *
 * and performs the main operations. It also handles any necessary cleanup
 * before exiting.
 *
 * @param argc El numero de argumentos recibidos.
 * @param argv Un arreglo de strings con los argumentos recibidos.
 * 
 * @return Retorna cero si ejecuto correctamente. De lo contrario retorna otro entero distinto de cero.
 */
int main(int argc, char** argv) {
    int from_day = DEFAULT_FROM_DAY;
    int num_days = DEFAULT_NUM_DAYS;
    int num_hours_per_day = DEFAULT_NUM_HOURS_PER_DAY;

    // Parse command-line arguments
    if (argc > 1) {
        from_day = atoi(argv[1]);
    }
    if (argc > 2) {
        num_days = atoi(argv[2]);
    }
    if (argc > 3) {
        num_hours_per_day = atoi(argv[3]);
    }

    if (
        from_day < 0 || from_day > 30 ||
        num_days < 0 || num_days > 30 ||
        num_hours_per_day < 0 || num_hours_per_day > 24 ||
        from_day + num_days > 31
    ) {
        printf("INVALID VALUES. FROM_DAY = %d, NUM_DAYS = %d NUM_HOURS_PER_DAY = %d\n", from_day, num_days, num_hours_per_day);
        exit(1);
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal
    signal(SIGSEGV, handle_signal);  // Handle segmentation faults
    signal(SIGABRT, handle_signal);  // Handle abort signals

    if (size > 1) {
        if (rank == 0) {
            // Master code
            master_code(size, from_day, num_days, num_hours_per_day);
        } else {
            // Worker code
            worker_code(rank, num_hours_per_day);
        }
    } else {
        // Run all the code if there's a single process running
        run_single_instance(from_day, num_days, num_hours_per_day);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
