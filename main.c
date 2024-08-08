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

#define DEFAULT_FROM_DAY 1
#define DEFAULT_NUM_DAYS 30
#define DEFAULT_NUM_HOURS_PER_DAY 24

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

    // Inicializar MPI
    MPI_Init(&argc, &argv);

    // Obtener el rango del proceso y el tamaño total de los procesos
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
        // Cada proceso recibe una porción del arreglo de nombres de archivo
        int count = DEFAULT_NUM_DAYS / size;
        char local_filenames[count][11];

        // Los nombres de archivo a distribuir
        char filenames[DEFAULT_NUM_DAYS][11] = {
            "2024-06-01","2024-06-02","2024-06-03","2024-06-04","2024-06-05",
            "2024-06-06","2024-06-07","2024-06-08","2024-06-09","2024-06-10",
            "2024-06-11","2024-06-12","2024-06-13","2024-06-14","2024-06-15",
            "2024-06-16","2024-06-17","2024-06-18","2024-06-19","2024-06-20",
            "2024-06-21","2024-06-22","2024-06-23","2024-06-24","2024-06-25",
            "2024-06-26","2024-06-27","2024-06-28","2024-06-29","2024-06-30"
        };

        MPI_Scatter(filenames, count * 11, MPI_CHAR, local_filenames, count * 11, MPI_CHAR, 0, MPI_COMM_WORLD);

        // Cada proceso imprime los archivos que le fueron asignados
        printf("\n Proceso %d procesa los siguientes dias: \n", rank);
        for (int i = 0; i < count; i++) {
            printf("%s\n", local_filenames[i]);
        }

        char** directories = (char**)malloc((count + 1) * sizeof(char*));
        for (int day = 0; day < count; day++) {
            directories[day] = (char*)malloc(25 * sizeof(char));
            sprintf(directories[day], "data/capturas/%s", local_filenames[day]);
        }
        directories[count] = NULL;

        if (rank == 0) {
            // Codigo del master
            master_code(size, num_hours_per_day, directories);
        } else {
            // Codigo de los esclavos
            worker_code(rank, num_hours_per_day, directories);
        }
        free_string_array(directories, count);
    } else {
        // Correr todo el codigo si es el unico proceso lanzado
        run_single_instance(from_day, num_days, num_hours_per_day);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
