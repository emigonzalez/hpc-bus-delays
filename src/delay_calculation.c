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


void calculate_delays() {
    printf("\nCALLING PYTHON SCRIPT %s \n", script_name);
    // Call the function to run the Python script with the hash map pointer
    run_python_script(script_name);

    printf("END PYTHON :) \n");

    return;
}
