#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "data_grouping.h"

#define MAX_LINE_LENGTH 1024
#define MAX_GROUPS 10000

/** TODO
 * Al agrupar por vfd, conseguir vft.
 * Almacenar los descartes. Descartar vfds sin vft.
 * retornar lista de vfds (solo nombres)
 */

typedef struct {
    char* key;
    char** rows;
    int row_count;
    int row_capacity;
} Group;

Group* groups[MAX_GROUPS];
int group_count = 0;

int is_valid_departure_time(const char* frecuencia) {
    int len = strlen(frecuencia);
    if (len < 4 || len > 6) return 0; // Must be 4 to 6 characters long to include the trailing zero

    int minutes = (atoi(frecuencia + len - 3) / 10) % 60; // Last three digits divided by 10 for minutes
    int hours = atoi(strndup(frecuencia, len - 3)); // Remaining digits for hours

    return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
}

void add_to_group(const char* key, const char* row) {
    // Search for the group
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i]->key, key) == 0) {
            // Add row to existing group
            if (groups[i]->row_count == groups[i]->row_capacity) {
                groups[i]->row_capacity *= 2;
                groups[i]->rows = realloc(groups[i]->rows, groups[i]->row_capacity * sizeof(char*));
            }
            groups[i]->rows[groups[i]->row_count++] = strdup(row);
            return;
        }
    }
    
    // Create new group
    Group* new_group = malloc(sizeof(Group));
    new_group->key = strdup(key);
    new_group->row_capacity = 10;
    new_group->rows = malloc(new_group->row_capacity * sizeof(char*));
    new_group->row_count = 0; // Initialize row count to 0
    new_group->rows[new_group->row_count++] = strdup(row);
    
    groups[group_count++] = new_group;
}

void parse_date_and_create_key(char* date_str, char* key, const char* variante, const char* frecuencia) {
    // Extract the date (yyyy-mm-dd) from the date string
    char date[11]; // yyyy-mm-dd is 10 characters + null terminator
    strncpy(date, date_str, 10);
    date[10] = '\0';

    // Create the key in the format variante_frecuencia_yyyy-mm-dd
    sprintf(key, "%s_%s_%s", variante, frecuencia, date);
}

void group_data_by_bus_and_time(char** assigned_files) {
    for (int i = 0; assigned_files[i] != NULL; i++) {
        FILE* file = fopen(assigned_files[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", assigned_files[i]);
            continue;
        }
        
        char line[MAX_LINE_LENGTH];
        // Skip header
        fgets(line, MAX_LINE_LENGTH, file);
        
        while (fgets(line, MAX_LINE_LENGTH, file)) {
            // Split the line into columns
            char* line_copy = strdup(line);
            char* id = strtok(line_copy, ",");
            char* codigoEmpresa = strtok(NULL, ",");
            char* frecuencia = strtok(NULL, ",");
            char* codigoBus = strtok(NULL, ",");
            char* variante = strtok(NULL, ",");
            char* linea = strtok(NULL, ",");
            char* sublinea = strtok(NULL, ",");
            char* tipoLinea = strtok(NULL, ",");
            char* tipoLineaDesc = strtok(NULL, ",");
            char* destino = strtok(NULL, ",");
            char* destinoDesc = strtok(NULL, ",");
            char* subsistema = strtok(NULL, ",");
            char* subsistemaDesc = strtok(NULL, ",");
            char* version = strtok(NULL, ",");
            char* velocidad = strtok(NULL, ",");
            char* latitud = strtok(NULL, ",");
            char* longitud = strtok(NULL, ",");
            char* fecha = strtok(NULL, ",");

            // Check if latitud or longitud is 0
            if (strcmp(latitud, "0") == 0 || strcmp(longitud, "0") == 0 || !is_valid_departure_time(frecuencia)) {
                printf("VARIANTE: %s. frecuencia: %s LATITUD: %s. LONGITUD: %s. \n", variante, frecuencia, latitud, longitud);
                free(line_copy); // Free the copy if the row is skipped
                continue; // Skip this row
            }

            // Create the group key
            char key[50];
            parse_date_and_create_key(fecha, key, variante, frecuencia);

            // Add the row to the corresponding group
            add_to_group(key, line);
            free(line_copy); // Free the copy after processing
        }

        fclose(file);
    }

    // Example: Print grouped data
    for (int i = 0; i < group_count; i++) {
        printf("Group: %s\n", groups[i]->key);
        for (int j = 0; j < groups[i]->row_count; j++) {
            printf("  %s", groups[i]->rows[j]); // Print the entire row as it was read
        }
    }

    // Free allocated memory for groups
    for (int i = 0; i < group_count; i++) {
        free(groups[i]->key);
        for (int j = 0; j < groups[i]->row_count; j++) {
            free(groups[i]->rows[j]);
        }
        free(groups[i]->rows);
        free(groups[i]);
    }
}

// int main() {
//     char** assigned_files = (char**)malloc(sizeof(char*));
//     assigned_files[0] = "stm-buses-2024-06-09_09.csv";

//     group_data_by_bus_and_time(assigned_files);
//     return 0;
// }
