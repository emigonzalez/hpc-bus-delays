#define _XOPEN_SOURCE 700 // to suppress warning about strptime
#define MAX_LINE_LENGTH 1024
#define MAX_GROUPS 10000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_grouping.h"
#include "date_to_day_type.h"

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

// Function to convert frecuencia to minutes
int convertir_a_minutos(const char* frecuencia) {
    // Extract hours and minutes from the padded string
    int frecuencia_number = atoi(frecuencia) / 10;
    int horas = frecuencia_number / 100;
    int minutos = frecuencia_number % 100;

    // printf("Frecuencia: %d, hora: %d in minutes: %d\n", frecuencia_number, horas, minutos);

    // Convert hours to minutes and add minutes
    int total_minutos = (horas * 60) + minutos;

    return total_minutos;
}

void time_to_frecuencia(const char* time, char* resultado) {
    // Remove colon (:) from the time part
    char time_without_colon[5]; // HHMM + null terminator
    snprintf(time_without_colon, sizeof(time_without_colon), "%c%c%c%c",
             time[0], time[1], time[3], time[4]);

    // Add a 0 at the end of time string without colon
    strcat(time_without_colon, "0");
    strcpy(resultado, time_without_colon);
}

// Function to adjust the date based on time and frequency
void ajustar_fecha(const char* fecha, const char* frecuencia, char* resultado) {
    // Split fecha into date and time
    char date_part[11];
    char time_part[6]; // HH:MM without seconds
    sscanf(fecha, "%10s %5s", date_part, time_part);

    char time_frecuencia[6];
    time_to_frecuencia(time_part, time_frecuencia);

    int time_number = convertir_a_minutos(time_frecuencia);
    int frecuencia_number = convertir_a_minutos(frecuencia);

    // printf("FECHA: %s, FRECUENCIA: %s, TIMEN: %d, FN: %d \n", fecha, frecuencia, time_number, frecuencia_number);

    // If difference more than 21 hour difference, it means that bus departues the day before snapshot.
    // Subtract a day from the date
    if (frecuencia_number > time_number && frecuencia_number - time_number > (21 * 60)) {
        struct tm tm_date;
        memset(&tm_date, 0, sizeof(struct tm));
        strptime(date_part, "%Y-%m-%d", &tm_date);

        // Subtract one day
        time_t t = mktime(&tm_date);
        t -= 86400; // Subtract one day in seconds
        struct tm* new_tm_date = localtime(&t);

        // Format new date back to string
        strftime(resultado, 11, "%Y-%m-%d", new_tm_date);
    } else {
        // If no adjustment needed, return the original date
        strcpy(resultado, date_part);
    }
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

void create_vfd(char* date_str, const char* variante, const char* frecuencia, char* key) {
    // Extract the date (yyyy-mm-dd) from the date string
    char date[11]; // yyyy-mm-dd is 10 characters + null terminator    
    ajustar_fecha(date_str, frecuencia, date);

    // Create the key in the format variante_frecuencia_yyyy-mm-dd
    sprintf(key, "%s_%s_%s", variante, frecuencia, date);
}

void create_vft(char* fecha, const char* variante, const char* frecuencia, char* key) {
    // Split fecha into date and time
    char date_part[11];
    char time_part[6]; // HH:MM without seconds
    sscanf(fecha, "%10s %5s", date_part, time_part);
    char day[3];
    char month[3];
    char year[5];
    sscanf(date_part, "%4s-%2s-%2s", year, month, day);

    int day_type = 0;
    if (atoi(year) == 2024 && atoi(month) == 6 && atoi(day) == 19) {
        day_type = 3;
    } else {
        day_type = day_to_date_type(date_part);
    }

    // Create the key in the format variante_frecuencia_daytype
    sprintf(key, "%s_%s_%d", variante, frecuencia, day_type);
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
                // printf("VARIANTE: %s. frecuencia: %s LATITUD: %s. LONGITUD: %s. \n", variante, frecuencia, latitud, longitud);
                free(line_copy); // Free the copy if the row is skipped
                continue; // Skip this row
            }

            // Create the group key
            char key[50];
            create_vfd(fecha, variante, frecuencia, key);

            printf("%s.%s \n", key, fecha);

            // Add the row to the corresponding group
            add_to_group(key, line);
            free(line_copy); // Free the copy after processing
        }

        fclose(file);
    }

    // Example: Print grouped data
    // for (int i = 0; i < group_count; i++) {
    //     printf("VFD: %s\n", groups[i]->key);
    //     for (int j = 0; j < groups[i]->row_count; j++) {
    //         printf(" %d ", groups[i]->rows[j]); // Print the entire row as it was read
    //     }
    // }

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

int main() {
    char** assigned_files = (char**)malloc(sizeof(char*));
    assigned_files[0] = "stm-buses-2024-07-16_00.csv";

    group_data_by_bus_and_time(assigned_files);
    return 0;
}
