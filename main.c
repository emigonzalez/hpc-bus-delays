// #include <mpi.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <signal.h>

#include "file_distribute.h"
#include "schedule_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"

/*
#define NUM_DAYS 30
#define NUM_HOURS_PER_DAY 24

char** file_names = NULL;
char** assigned_files = NULL;

void handle_signal(int signal);
void free_memory();

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal

    // File names and schedule file
    char* schedule_file = "data/bus_schedules.csv";

    // Generate the list of file names (example for June, 24 files per day)
    file_names = generate_file_names(NUM_DAYS, NUM_HOURS_PER_DAY);

    // Calculate total number of files
    int num_files = NUM_DAYS * NUM_HOURS_PER_DAY;

    // Distribute file names among processes
    assigned_files = distribute_file_names(file_names, num_files, rank, size);

    printf("BUS LOCATIONS \n");
    // Each process reads its assigned location files
    for (int i = 0; assigned_files[i] != NULL; i++) {
        printf("Process %d reading file %s\n", rank, assigned_files[i]);
        // read_file_and_process_data(assigned_files[i]);
    }

    printf("BUS SCHEDULES \n");
    // Each process reads the entire schedule file
    printf("Process %d reading schedule file %s\n", rank, schedule_file);
    read_schedule_data(schedule_file, rank);

    // Group data by bus variant, time, and day of month/type of day
    group_data_by_bus_and_time(assigned_files);

    // Map grouped locations to grouped schedules
    map_locations_to_schedules();

    // Calculate delays using the Python black box
    calculate_delays();

    // Gather results from all processes
    gather_results();

    // Free allocated memory
    free_memory();

    MPI_Finalize();
    return 0;
}

void free_memory() {
    // Perform any necessary cleanup here
    if (file_names != NULL) {
        for (int i = 0; i < NUM_DAYS * NUM_HOURS_PER_DAY; i++) {
            free(file_names[i]);
        }
        free(file_names);
    }

    if (assigned_files != NULL) {
        for (int i = 0; assigned_files[i] != NULL; i++) {
            free(assigned_files[i]);
        }
        free(assigned_files);
    }
}

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

HashMap *vfd_map = NULL;
HashMap* vft_map = NULL;

void run_python_script(const char *script_name) {
    // Call the Python script
    char command[256];
    snprintf(command, sizeof(command), "python3.10 %s", script_name);
    // snprintf(command, sizeof(command), "python3.10 calcular-retrasos_conC.py");
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        exit(1);
    }

    // Read the Python script output (if any)
    char output[256];
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("%s", output);
    }

    // Wait for the Python script to finish
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
    }
}

void generate_vfd_file(HashMap* map, const char *vfd_filename, const char *capturas_filename, const char *horarios_filename) {
    FILE *vfd_file = fopen(vfd_filename, "w");
    if (!vfd_file) {
        perror("Failed to open vfd_file");
        return;
    }

    FILE *capturas_file = fopen(capturas_filename, "w");
    if (!capturas_file) {
        perror("Failed to open VFD rows file");
        fclose(vfd_file);
        return;
    }

    FILE *horarios_file = fopen(horarios_filename, "w");
    if (!horarios_file) {
        perror("Failed to open VFD rows file");
        fclose(vfd_file);
        fclose(capturas_file);
        return;
    }

    // Write the CSV header
    fprintf(vfd_file, "vfd,cant_capturas,cant_horarios\n");
    fprintf(capturas_file, "id,codigoEmpresa,frecuencia,codigoBus,variante,linea,sublinea,tipoLinea,tipoLineaDesc,destino,destinoDesc,subsistema,subsistemaDesc,version,velocidad,latitud,longitud,fecha\n");
    fprintf(horarios_file, "tipo_dia;cod_variante;frecuencia;cod_ubic_parada;ordinal;hora;dia_anterior;X;Y\n");

    size_t key_count;
    Entry **entries = get_all_keys(map, &key_count);

    // Write the data
    for (size_t i = 0; i < key_count; ++i) {
        Entry *entry = entries[i];
        // Write to vfd_file
        fprintf(vfd_file, "%s,%zu,%zu\n", entry->key, entry->vfd_row_count, entry->vft_row_count);

        // Write to capturas_file
        for (size_t j = 0; j < entry->vfd_row_count; ++j) {
            fprintf(capturas_file, "%s", entry->vfd_rows[j]);
        }

        // Write to horarios_file
        for (size_t j = 0; j < entry->vft_row_count; ++j) {
            fprintf(horarios_file, "%s", entry->vft_rows[j]);
        }
    }

    // Free allocated memory for keys array
    free(entries);

    // Close the file
    fclose(vfd_file);
    fclose(capturas_file);
    fclose(horarios_file);
}

int main() {
    fprintf(stderr, "BEGIN in C \n");

    char** capturas = (char**)malloc(24 * sizeof(char*));
    for (size_t i = 0; i < 24; i++) {
        capturas[i] = (char*)malloc(34 * sizeof(char));
        sprintf(capturas[i], "data/stm-buses-2024-06-10_%02ld.csv", i);
        printf("FILE %ld : %s\n", i, capturas[i]);
    }
    capturas[24] = NULL; // Terminate the list

    char* horarios = "data/horarios_paradas_vft.csv";
    fprintf(stderr, "\nGENERATING VFT...\n");
    vft_map = group_data_by_vft(horarios);
    fprintf(stderr, "VFT GENERATED.\n");

    for (int i = 0; capturas[i] != NULL; i++) {
        fprintf(stderr, "\n ####### RUNNING WITH FILE: %s ########\n", capturas[i]);

        fprintf(stderr, "\nGENERATING VFD...\n");
        vfd_map = group_data_by_vfd(capturas[i], vft_map);
        fprintf(stderr, "VFD GENERATED.\n");
    
        // Example: Print grouped data
        // printf("\nPRINTING MAP...\n");
        // print_hash_map(vfd_map);

        printf("HashMap size: %zu\n", vfd_map->size);
        printf("HashMap count: %zu\n", vfd_map->count);

        const char *vfd_filename = "data/vfd.csv";
        const char *capturas_filename = "data/capturas.csv";
        const char *horarios_filename = "data/horarios.csv";
        generate_vfd_file(vfd_map, vfd_filename, capturas_filename, horarios_filename);

        // Path to the Python script
        const char *script_name = "calcular-retrasos.py";

        fprintf(stderr, "\nCALLING PYTHON SCRIPT %s \n\n", script_name);
        printf("\nFILE: %s \n", capturas[i]);
        // Call the function to run the Python script with the hash map pointer
        run_python_script(script_name);

        fprintf(stderr, "\nEND PYTHON :) \n");    

        // Free the hash map
        free_vfd_hash_map(vfd_map);
        vfd_map = NULL;
    }

    free(capturas);
    free_hash_map(vft_map);
    fprintf(stderr, "END in C\n");
    return EXIT_SUCCESS;
}
