#define UNUSED(x) (void)(x) // to suppress warning about unused variable

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "delay_calculation.h"

// Path to the Python script
const char *script_name = "calcular-retrasos.py";

void run_python_script(const char *script_name, int day) {
    // Call the Python script
    char command[256];
    snprintf(command, sizeof(command), "python3.10 %s n 2024-06-%02d", script_name, day);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return;
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

void python_calculate_delays(int day) {
    // Call the function to run the Python script with the hash map pointer
    run_python_script(script_name, day);

    return;
}

int process_row(DelayMap* delay_map, char* line) {
    char* line_copy = strdup(line);
    if (line_copy == NULL) {
        perror("Failed to duplicate line");
        return -1;
    }

    char* vfd_key = strtok(line_copy, ",");
    if (vfd_key == NULL || strcmp(vfd_key, "VFD") == 0) {
        free(line_copy);
        return -1;
    }


    char* variante = strtok(NULL, ",");
    char* codigo_bus = strtok(NULL, ",");
    char* linea = strtok(NULL, ",");
    char* hora = strtok(NULL, ",");
    char* ordinal = strtok(NULL, ",");
    char* fecha_hora_paso = strtok(NULL, ",");
    char* retraso = strtok(NULL, ",");

    UNUSED(variante);
    UNUSED(codigo_bus);
    UNUSED(linea);
    UNUSED(hora);
    UNUSED(fecha_hora_paso);

    delay_map_insert(delay_map, vfd_key, atoi(ordinal), atof(retraso), line);

    free(line_copy);
    return 1;
}

int map_delays(DelayMap* delay_map, char* filename) {
    fprintf(stderr, "FILENAME: %s \n", filename);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    size_t read;

    // Read and skip the header line
    if ((read = getline(&line, &len, file)) != -1);

    // Ensure line is freed before the next read
    free(line);
    line = NULL;    

    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1) continue; // Skip empty lines
        process_row(delay_map, line);
        free(line);
        line = NULL;
    }

    fclose(file);
    return 1;
}
