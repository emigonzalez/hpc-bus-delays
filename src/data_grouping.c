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

void add_vft_to_map(HashMap *map, const char *key, VFT *vft) {
    if (!hash_map_insert_vft(map, key, vft)) {
        fprintf(stderr, "Error: failed to add VFT to map\n");
        exit(EXIT_FAILURE);
    }
}

void add_vfd_to_map(HashMap *map, const char *key, VFD *vfd) {
    if (!hash_map_insert_vfd(map, key, vfd)) {
        fprintf(stderr, "Error: failed to add VFD to map\n");
        exit(EXIT_FAILURE);
    }
}

VFT* row_to_vft(char* line) {
    char* tipo_dia = strtok(line, ";");
    char* variante = strtok(NULL, ";");
    char* frecuencia = strtok(NULL, ";");
    char* cod_ubic_parada = strtok(NULL, ";");
    char* ordinal = strtok(NULL, ";");
    char* hora = strtok(NULL, ";");
    char* dia_anterior = strtok(NULL, ";");
    // char* latitud = strtok(NULL, ";");
    // char* longitud = strtok(NULL, ";");

    if (tipo_dia == NULL || variante == NULL || frecuencia == NULL || dia_anterior == NULL) {
        fprintf(stderr, "Error: Missing data from row to VFT in line: %s", line);
        return NULL;
    }

    // Check if bus is from day before
    int tipo_dia_int = atoi(tipo_dia);
    if (strcmp(dia_anterior, "S") == 0) {
        tipo_dia_int = (tipo_dia_int == 1) ? 3 : tipo_dia_int - 1;
    }

    VFT* vft = create_vft();
    vft->tipo_dia = tipo_dia_int;
    vft->variante = strdup(variante);
    vft->frecuencia = strdup(frecuencia);
    vft->cod_ubic_parada = strdup(cod_ubic_parada);
    vft->ordinal = strdup(ordinal);
    vft->hora = strdup(hora);
    vft->dia_anterior = strdup(dia_anterior);
    vft->latitud = NULL;
    vft->longitud = NULL;

    return vft;
}

VFD* row_to_vfd(char* line) {
    char* id = strtok(line, ",");
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

    UNUSED(tipoLineaDesc);
    UNUSED(destinoDesc);
    UNUSED(subsistemaDesc);

    // Check if latitud or longitud is 0
    if (
        strcmp(latitud, "0") == 0 ||
        strcmp(longitud, "0") == 0 ||
        !is_valid_departure_time(frecuencia)
    ) {
        return NULL; // Skip this row
    }

    VFD* vfd = create_vfd();
    vfd->id = strdup(id);
    vfd->codigoEmpresa = strdup(codigoEmpresa);
    vfd->frecuencia = strdup(frecuencia);
    vfd->codigoBus = strdup(codigoBus);
    vfd->variante = strdup(variante);
    vfd->linea = strdup(linea);
    vfd->sublinea = strdup(sublinea);
    vfd->tipoLinea = strdup(tipoLinea);
    vfd->destino = strdup(destino);
    vfd->subsistema = strdup(subsistema);
    vfd->version = strdup(version);
    vfd->velocidad = strdup(velocidad);
    vfd->latitud = strdup(latitud);
    vfd->longitud = strdup(longitud);
    vfd->fecha = strdup(fecha);

    return vfd;
}

char* create_vfd_key(char* line) {
    VFD* vfd = row_to_vfd(line);

    // Extract the date (yyyy-mm-dd) from the date string
    char date[11]; // yyyy-mm-dd is 10 characters + null terminator    
    ajustar_fecha(vfd->fecha, vfd->frecuencia, date);

    // Create the group key "<variante>_<frecuencia>_<dia>\0"
    size_t key_length = strlen(vfd->variante) + strlen(vfd->frecuencia) + strlen(date) + 2 + 1; 
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the key in the format variante_frecuencia_yyyy-mm-dd
    snprintf(key, key_length, "%s_%s_%s", vfd->variante, vfd->frecuencia, date);

    free_vfd(vfd);
    return key;
}

char* create_vft_key(char* line) {
    VFT* vft = row_to_vft(line);
    if (vft == NULL) return NULL;

    size_t key_length = strlen(vft->variante) + strlen(vft->frecuencia) + 3 + 1; // "<variante>_<frecuencia>_<tipo_dia>\0"
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the vft key
    snprintf(key, key_length, "%s_%s_%d", vft->variante, vft->frecuencia, vft->tipo_dia);

    free_vft(vft);
    return key;
}

void group_data_by_vft(char** assigned_files) {
    if (assigned_files == NULL) {
        fprintf(stderr, "Error: assigned_files pointer is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Create a hash map for map
    HashMap *map = create_hash_map();

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

        // Ensure line is freed before the next read
        free(line);
        line = NULL;

        while ((read = getline(&line, &len, file)) != -1) {
            if (read <= 1) continue; // Skip empty lines

            // Split the line into columns
            char* line_copy = strdup(line);
            if (line_copy == NULL) {
                perror("Failed to duplicate line");
                continue;
            }

            char* key = NULL;
            key = create_vft_key(line_copy);

            // Add the row to the corresponding group
            if (key != NULL) {
                VFT* vft = row_to_vft(line);
                add_vft_to_map(map, key, vft);
                free(key);
            } else {
                perror("Invalid Data");
                continue;
            }

            free(line_copy); // Free the copy after processing
        }

        free(line);
        fclose(file);
    }

    // Example: Print grouped data
    print_hash_map(map);

    // Free the hash map
    free_hash_map(map);
}
