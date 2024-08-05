#define _XOPEN_SOURCE 700 // to suppress warning about strptime
#define UNUSED(x) (void)(x) // to suppress warning about unused variable

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_grouping.h"
#include "date_to_day_type.h"

int is_valid_departure_time(const char* frecuencia) {
    int frecuencia_int = atoi(frecuencia) / 10;

    if (frecuencia_int == 0 && strcmp(frecuencia, "0") != 0)
        return -1;

    int minutes = frecuencia_int % 100; // Last three digits divided by 10 for minutes
    int hours = frecuencia_int / 100; // Remaining digits for hours

    return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59) ? 1 : -1;
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

int extreme_delay(const char* fecha, const char* frecuencia) {
    // Split fecha into date and time
    char date_part[11];
    char time_part[6]; // HH:MM without seconds
    sscanf(fecha, "%10s %5s", date_part, time_part);

    char time_frecuencia[6];
    time_to_frecuencia(time_part, time_frecuencia);

    int time_number = convertir_a_minutos(time_frecuencia);
    int frecuencia_number = convertir_a_minutos(frecuencia);
    int time_difference = abs(frecuencia_number - time_number);

    if (time_difference > (3 * 60) && time_difference < (21 * 60)) {
        return 1;
    } else {
        return 0;
    }
}

void add_seconds_to_date(const char* date, int seconds, char* resultado) {
    struct tm tm_date;
    memset(&tm_date, 0, sizeof(struct tm));
    strptime(date, "%Y-%m-%d", &tm_date);

    // Add or substract n seconds
    time_t t = mktime(&tm_date);
    t += seconds; //
    struct tm* new_tm_date = localtime(&t);

    // Format new date back to string
    strftime(resultado, 11, "%Y-%m-%d", new_tm_date);
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
    if (frecuencia_number > time_number && frecuencia_number - time_number > (21 * 60)) {
        // Subtract a day from the date
        add_seconds_to_date(date_part, -86400, resultado);
    } else if (frecuencia_number < 60 && abs(frecuencia_number - time_number) > (21 * 60)) {
        // Add a day from the date
        add_seconds_to_date(date_part, 86400, resultado);
    } else {
        // If no adjustment needed, return the original date
        strcpy(resultado, date_part);
    }
}

Entry* add_vft_to_map(HashMap *map, const char *key, const char *row) {
    Entry* entry = hash_map_insert_vft(map, key, row);
    if (!entry) {
        fprintf(stderr, "Error: failed to add VFT to map\n");
        exit(EXIT_FAILURE);
    }
    return entry;
}

Entry* add_vfd_to_map(HashMap *map, const char *key, const char *row) {
    if (key == NULL || row == NULL) return NULL;

    Entry* vfd_entry = hash_map_insert_vfd(map, key, row);
    if (!vfd_entry) {
        fprintf(stderr, "Error: failed to add VFD to map\n");
    }
    return vfd_entry;
}

char* create_vfd_key(char* line) {
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
    if (
        strcmp(latitud, "0") == 0 ||
        strcmp(longitud, "0") == 0 ||
        strcmp(latitud, "-0.0") == 0 ||
        strcmp(longitud, "-0.0") == 0 ||
        is_valid_departure_time(frecuencia) < 0 ||
        extreme_delay(fecha, frecuencia) > 0
    ) {
        return NULL; // Skip this row
    }

    // Extract the date (yyyy-mm-dd) from the date string
    char date[11]; // yyyy-mm-dd is 10 characters + null terminator    
    ajustar_fecha(fecha, frecuencia, date);

    // Create the group key "<variante>_<frecuencia>_<dia>\0"
    size_t key_length = strlen(variante) + strlen(frecuencia) + strlen(date) + 2 + 1; 
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the key in the format variante_frecuencia_yyyy-mm-dd
    snprintf(key, key_length, "%s_%s_%s", variante, frecuencia, date);

    return key;
}

char* create_vft_key(char* line) {
    char* tipo_dia = strtok(line, ";");
    char* variante = strtok(NULL, ";");
    char* frecuencia = strtok(NULL, ";");
    char* cod_ubic_parada = strtok(NULL, ";");
    char* ordinal = strtok(NULL, ";");
    char* hora = strtok(NULL, ";");
    char* dia_anterior = strtok(NULL, ";");
    char* latitud = strtok(NULL, ";");
    char* longitud = strtok(NULL, ";");

    UNUSED(cod_ubic_parada);
    UNUSED(ordinal);
    UNUSED(hora);
    UNUSED(latitud);
    UNUSED(longitud);

    if (tipo_dia == NULL || variante == NULL || frecuencia == NULL || dia_anterior == NULL) {
        fprintf(stderr, "Error: Missing data from row to VFT");
        return NULL;
    }

    // Check if bus is from day before
    int frecuencia_int = atoi(frecuencia) / 10;
    int hora_int = atoi(hora);
    if (strcmp(dia_anterior, "N") == 0 && (hora_int - frecuencia_int) < 0) {
        // fprintf(stderr, "CAMPO DIA_ANTERIOR INVALIDO EN ROW: %s_%s_%d_%d_%s\n", tipo_dia, variante, frecuencia_int, hora_int, dia_anterior);
        return NULL;
    }

    int tipo_dia_int = atoi(tipo_dia);
    if (strcmp(dia_anterior, "S") == 0) {
        tipo_dia_int = (tipo_dia_int == 1) ? 3 : tipo_dia_int - 1;
    }

    size_t key_length = strlen(variante) + strlen(frecuencia) + 3 + 1; // "<cod_variante>_<frecuencia>_<tipo_dia>\0"
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the vft keydate
    snprintf(key, key_length, "%s_%s_%d", variante, frecuencia, tipo_dia_int);

    return key;
}

HashMap* group_data_by_vft(char* filename) {
    if (filename == NULL) {
        fprintf(stderr, "Error: filename pointer is NULL\n");
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Create a hash map for map
    HashMap *map = create_hash_map();

    // Read and skip the header line
    if ((read = getline(&line, &len, file)) != -1);

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
            add_vft_to_map(map, key, line);
            free(key);
        } else {
            free(line_copy);
            free(line);
            line = NULL;
            continue;
        }

        free(line_copy); // Free the copy after processing
        free(line);
        line = NULL;
    }

    free(line);
    fclose(file);

    return map;
}

char* create_vft_from_vfd(char* vfd) {
    if (vfd == NULL) return NULL;
    
    char* vfd_copy = strdup(vfd);
    char* variante = strtok(vfd_copy, "_");
    char* frecuencia = strtok(NULL, "_");
    char* date = strtok(NULL, "_");

    if (variante == NULL || frecuencia == NULL || date == NULL) return NULL;

    int tipo_dia_int = date_to_date_type(date);

    size_t key_length = strlen(variante) + strlen(frecuencia) + 3 + 1; // "<variante>_<frecuencia>_<tipo_dia>\0"
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the vft key
    snprintf(key, key_length, "%s_%s_%d", variante, frecuencia, tipo_dia_int);

    free(vfd_copy);
    return key;
}

HashMap* group_data_by_vfd(char* filename, HashMap* vft_map) {
    if (filename == NULL) {
        fprintf(stderr, "Error: filename pointer is NULL\n");
        return NULL;
    }

    if (vft_map == NULL) {
        fprintf(stderr, "NO VFT MAP PROVIDED\n");
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Create a hash map for vfd_map
    HashMap *vfd_map = create_hash_map();

    HashMap *discarded_vfds = create_hash_map();

    // Read and skip the header line
    if ((read = getline(&line, &len, file)) != -1);

    // Ensure line is freed before the next read
    free(line);
    line = NULL;

    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1) continue; // Skip empty lines

        // Split the line into columns
        char* line_copy = strdup(line);
        if (line_copy == NULL) {
            perror("Failed to duplicate line");
            free(line);
            continue;
        }

        char* vfd_key = NULL;
        // printf("\n#################################\n");
        vfd_key = create_vfd_key(line_copy);
        // printf("VFD KEY: %s\n", vfd_key);
        // printf("ROW: %s", line);

        if (vfd_key != NULL) {
            Entry* vfd_entry = hash_map_search(vfd_map, vfd_key);

            if (vfd_entry != NULL) {
                // Add the row to the vfd_map
                insert_to_vfds(vfd_entry, line);
            } else if (!hash_map_search(discarded_vfds, vfd_key)) {
                // Generate VFT and look for data in the vft_map
                char* vft_key = create_vft_from_vfd(vfd_key);
                // printf("VFT KEY: %s", vft_key);
                Entry* vft_entry = hash_map_search(vft_map, vft_key);

                if (vft_entry != NULL) {
                    Entry* vfd_entry = add_vfd_to_map(vfd_map, vfd_key, line);
                    repoint_vfts_to_vfd_map(vfd_entry, vft_entry);
                } else {
                    // printf(" does not exist.\n");
                    add_vfd_to_map(discarded_vfds, vfd_key, NULL);
                }

                free(vft_key);
            }

            free(vfd_key);
        }

        free(line_copy); // Free the copy after processing
        free(line);
        line = NULL;
        // printf("#################################\n");
    }

    fclose(file);

    free_hash_map(discarded_vfds);
    return vfd_map;
}
