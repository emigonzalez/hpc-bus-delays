#define _XOPEN_SOURCE 700 // to suppress warning about strptime
#define UNUSED(x) (void)(x) // to suppress warning about unused variable

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_grouping.h"
#include "date_to_day_type.h"
#include "hash_map.h"

/** TODO
 * Al agrupar por vfd, conseguir vft.
 * Almacenar los descartes. Descartar vfds sin vft.
 * retornar lista de vfds (solo nombres)
 */

void print_map(HashMap *groups) {
    // Display the keys and rows in the hash map
    printf("Keys and rows in hash map:\n");
    for (size_t i = 0; i < groups->size; i++) {
        Entry *entry = groups->buckets[i];
        while (entry) {
            printf("Key: %s\n", entry->key);
            for (size_t j = 0; j < entry->row_count; j++) {
                printf("  Row: %s", entry->rows[j]);
            }
            entry = entry->next;
        }
    }
}

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

void add_to_group(HashMap *groups, const char *key, const char *row) {
    if (!hash_map_insert(groups, key, row)) {
        fprintf(stderr, "Error: failed to add row to group\n");
        exit(EXIT_FAILURE);
    }
}

void create_vfd(char* date_str, const char* variante, const char* frecuencia, char* key) {
    // Extract the date (yyyy-mm-dd) from the date string
    char date[11]; // yyyy-mm-dd is 10 characters + null terminator    
    ajustar_fecha(date_str, frecuencia, date);

    // Create the key in the format variante_frecuencia_yyyy-mm-dd
    sprintf(key, "%s_%s_%s", variante, frecuencia, date);
}


void group_data_by_vfd(char** assigned_files) {
    if (assigned_files == NULL) {
        fprintf(stderr, "Error: assigned_files pointer is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Create a hash map for groups
    HashMap *groups = create_hash_map();

    for (int i = 0; assigned_files[i] != NULL; i++) {
        FILE* file = fopen(assigned_files[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", assigned_files[i]);
            continue;
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        // Read and skip the header line
        if ((read = getline(&line, &len, file)) != -1) {
            // Optionally, print or process the header line
            printf("Header: %s", line);
        }

        while ((read = getline(&line, &len, file)) != -1) {
            if (read <= 1) continue; // Skip empty lines

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

            UNUSED(id);
            UNUSED(codigoEmpresa);
            UNUSED(codigoBus);
            UNUSED(linea);
            UNUSED(sublinea);
            UNUSED(tipoLinea);
            UNUSED(tipoLineaDesc);
            UNUSED(destino);
            UNUSED(destinoDesc);
            UNUSED(subsistema);
            UNUSED(subsistemaDesc);
            UNUSED(version);
            UNUSED(velocidad);

            // Check if latitud or longitud is 0
            if (strcmp(latitud, "0") == 0 || strcmp(longitud, "0") == 0 || !is_valid_departure_time(frecuencia)) {
                // printf("VARIANTE: %s. frecuencia: %s LATITUD: %s. LONGITUD: %s. \n", variante, frecuencia, latitud, longitud);
                free(line_copy); // Free the copy if the row is skipped
                continue; // Skip this row
            }

            // Create the group key
            char key[50];
            create_vfd(fecha, variante, frecuencia, key);

            // printf("%s.%s \n", key, fecha);

            // Add the row to the corresponding group
            add_to_group(groups, key, line);
            free(line_copy); // Free the copy after processing
        }

        fclose(file);
        free(line);
    }

    // Example: Print grouped data
    print_map(groups);

    // Free the hash map
    free_hash_map(groups);
}

void group_data_by_vft(char** assigned_files) {
    if (assigned_files == NULL) {
        fprintf(stderr, "Error: assigned_files pointer is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Create a hash map for groups
    HashMap *groups = create_hash_map();

    for (int i = 0; assigned_files[i] != NULL; i++) {
        FILE* file = fopen(assigned_files[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", assigned_files[i]);
            continue;
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        // Read and skip the header line
        if ((read = getline(&line, &len, file)) != -1) {
            // Optionally, print or process the header line
            printf("Header: %s", line);
        }

        while ((read = getline(&line, &len, file)) != -1) {
            if (read <= 1) continue; // Skip empty lines

            // Split the line into columns
            char* line_copy = strdup(line);
            char* tipo_dia = strtok(line_copy, ";");
            char* variante = strtok(NULL, ";");
            char* frecuencia = strtok(NULL, ";");
            char* cod_ubic_parada = strtok(NULL, ";");
            char* ordinal = strtok(NULL, ";");
            char* hora = strtok(NULL, ";");
            char* dia_anterior = strtok(NULL, ";");
            // char* latitud = strtok(NULL, ";");
            // char* longitud = strtok(NULL, ";");

            UNUSED(cod_ubic_parada);
            UNUSED(ordinal);
            UNUSED(hora);
            // UNUSED(latitud);
            // UNUSED(longitud);

            // Check if bus is from day before
            int tipo_dia_int = atoi(tipo_dia);
            if (strcmp(dia_anterior, "S") == 0) {
                tipo_dia_int = (tipo_dia_int == 1) ? 3 : tipo_dia_int - 1;
            }

            // Create the group key
            char key[20];
            sprintf(key, "%s_%s_%s", variante, frecuencia, tipo_dia);

            // Add the row to the corresponding group
            add_to_group(groups, key, line);
            free(line_copy); // Free the copy after processing
        }

        fclose(file);
        free(line);
    }

    // Example: Print grouped data
    print_map(groups);

    // Free the hash map
    free_hash_map(groups);
}
