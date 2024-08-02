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

#define FROM_DAY 1
#define NUM_DAYS 10
#define NUM_HOURS_PER_DAY 24

// Path to the Python script
const char *script_name = "calcular-retrasos.py";
char* horarios = "data/horarios_paradas_vft.csv";
char** capturas = NULL;
char** directorios = NULL;
char** assigned_days = NULL;
HashMap *vfd_map = NULL;
HashMap* vft_map = NULL;

void handle_signal(int signal);
void free_memory();
void run_python_script(const char *script_name);
void generate_vfd_file(HashMap* map, const char *vfd_filename, const char *capturas_filename, const char *horarios_filename);

int main(int argc, char** argv) {
    if (FROM_DAY + NUM_DAYS > 31) {
        printf("INVALID VALUES. FROM_DAY = %d, NUM_DAYS = %d\n", FROM_DAY, NUM_DAYS);
        exit(1);
    }

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal

    // Generate the list of file names (example for June, 24 files per day)
    directorios = generate_directories(FROM_DAY, NUM_DAYS);

    // Distribute dirs among processes
    assigned_days = distribute(directorios, NUM_DAYS, rank, size);

    printf("\nGENERATING VFT...\n");
    vft_map = group_data_by_vft(horarios);

    if (vft_map == NULL) {
        fprintf(stderr, "COULD NOT GENERATE VFT.\n");
        exit(1);
    } else
        printf("VFT GENERATED.\n\n");

    // Each process reads its assigned directories
    for (int i = 0; assigned_days[i] != NULL; i++) {
        char * day_str = get_day_from_dir_name(assigned_days[i]);
        printf("Process %d reading file %s from day %s\n", rank, assigned_days[i], day_str);

        capturas = generate_file_names(assigned_days[i], atoi(day_str), NUM_HOURS_PER_DAY);
        for (int j = 0; j < NUM_HOURS_PER_DAY; j++) {
            printf("####### RUNNING WITH FILE: %s ########\n", capturas[j]);

            printf("GENERATING VFD...\n");
            vfd_map = group_data_by_vfd(capturas[j], vft_map);

            if (vfd_map == NULL) {
                fprintf(stderr, "INVALID VFD MAPS FOR %s.\n", capturas[j]);
                continue;
            }

            printf("VFD GENERATED.\n");
            // Example: Print grouped data
            // printf("\nPRINTING MAP...\n");
            // print_hash_map(vfd_map);

            printf("HashMap size: %zu\n", vfd_map->size);
            printf("HashMap count: %zu\n", vfd_map->count);

            const char *vfd_filename = "data/vfd.csv";
            const char *capturas_filename = "data/capturas.csv";
            const char *horarios_filename = "data/horarios.csv";
            generate_vfd_file(vfd_map, vfd_filename, capturas_filename, horarios_filename);

            printf("\nCALLING PYTHON SCRIPT %s \n", script_name);
            printf("FILE: %s \n", capturas[j]);
            // Call the function to run the Python script with the hash map pointer
            run_python_script(script_name);

            printf("END PYTHON :) \n");    

            // Free the hash map
            free_vfd_hash_map(vfd_map);
            vfd_map = NULL;
        }
    }

    // Free allocated memory
    free_memory();

    MPI_Finalize();
    return 0;
}

void free_memory() {
    // Perform any necessary cleanup here
    if (capturas != NULL) {
        for (int i = 0; i < NUM_DAYS * NUM_HOURS_PER_DAY; i++) {
            free(capturas[i]);
        }
        free(capturas);
    }

    if (assigned_days != NULL) {
        for (int i = 0; assigned_days[i] != NULL; i++) {
            free(assigned_days[i]);
        }
        free(assigned_days);
    }

    if (vft_map != NULL) free_hash_map(vft_map);
    if (vfd_map != NULL) free_hash_map(vfd_map);
}

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}

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
        perror("Failed to create vfd_file");
        return;
    }

    FILE *capturas_file = fopen(capturas_filename, "w");
    if (!capturas_file) {
        perror("Failed create capturas file");
        fclose(vfd_file);
        return;
    }

    FILE *horarios_file = fopen(horarios_filename, "w");
    if (!horarios_file) {
        perror("Failed to create horarios file");
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
