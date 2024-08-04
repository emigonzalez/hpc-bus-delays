#define UNUSED(x) (void)(x) // to suppress warning about unused variable

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "delay_calculation.h"

// Path to the Python script
const char *script_name = "calcular-retrasos.py";

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

void python_calculate_delays() {
    printf("\nCALLING PYTHON SCRIPT %s \n", script_name);
    // Call the function to run the Python script with the hash map pointer
    run_python_script(script_name);

    printf("END PYTHON :) \n");

    return;
}

void process_row(DelayMap* delay_map, char* line) {
    char* line_copy = strdup(line);
    if (line_copy == NULL) {
        perror("Failed to duplicate line");
        free(line);
        return;
    }

    char* vfd_key = strtok(line_copy, ",");
    if (vfd_key == NULL || strcmp(vfd_key, "VFD") == 0) {
        free(line_copy);
        return;
    }

    // printf("\n#################################\n");
    // printf("VFD KEY: %s\n", vfd_key);

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
    // printf("#################################\n");
}

void map_delays(DelayMap* delay_map, char* filename) {
    fprintf(stderr, "FILENAME: %s \n", filename);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    size_t read;

    // Read and skip the header line
    if ((read = getline(&line, &len, file)) != -1) {
        // Optionally, print or process the header line
        printf("Header: %s \n", line);
        // VFD,variante,codigo_bus,linea,hora,ordinal,fecha_hora_paso,retraso
    }

    // Ensure line is freed before the next read
    free(line);
    line = NULL;
    int i = 1;
    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1) continue; // Skip empty lines
        // printf("Processing VFD: %d \n", i);
        process_row(delay_map, line);
        i++;
        free(line);
        line = NULL;
    }

    fclose(file);
}
