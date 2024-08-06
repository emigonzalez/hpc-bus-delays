#define UNUSED(x) (void)(x) // to suppress warning about unused variable

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "result_gathering.h"

void gather_results() {
    return;
}

void get_delay_fields(const char* row, char** variante, char** fecha_hora_paso, char** retraso, char** cod_parada, char** X, char** Y) {
    char* buffer = strdup(row);

    // VFD,variante,codigo_bus,linea,hora,ordinal,fecha_hora_paso,retraso,vecinos,cod_parada,X,Y
    strtok(buffer, ","); // VFD
    *variante = strtok(NULL, ",");
    strtok(NULL, ","); // codigo_bus
    strtok(NULL, ","); // linea
    strtok(NULL, ","); // hora
    strtok(NULL, ","); // ordinal
    *fecha_hora_paso = strtok(NULL, ",");
    *retraso = strtok(NULL, ","); // retraso
    strtok(NULL, ","); // vecinos
    *cod_parada = strtok(NULL, ",");
    *X = strtok(NULL, ",");
    *Y = strtok(NULL, ",");
}

void get_field_in_row_by_index(const char* row, int index, char** field) {
    char* buffer = strdup(row);

    char *token;
    int i = 0;

    // Tokenize the row based on commas
    token = strtok(buffer, ",");
    while (token != NULL) {
        i++;
        if (i == index) {
            *field = token;
        }
        token = strtok(NULL, ",");
    }
}

char* create_key(const char* row, size_t* bus_stop, size_t* passenger_count) {
    char buffer[256];
    strncpy(buffer, row, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Tokenize the row based on commas
    char* fecha = strtok(buffer, ",");
    char* codigo_parada_origen = strtok(NULL, ",");
    char* sevar_codigo = strtok(NULL, ",");
    char* cantidad_pasajeros = strtok(NULL, ",");

    UNUSED(sevar_codigo);

    *bus_stop = atoi(codigo_parada_origen);
    *passenger_count = atoi(cantidad_pasajeros);

    size_t key_length = strlen(sevar_codigo) + strlen(fecha) + strlen(codigo_parada_origen) + 3; // "<variante>_<fecha>_<parada>\0"
    char* key = (char*)malloc(key_length * sizeof(char));

    // Create the vft keydate
    snprintf(key, key_length, "%s_%s_%s", sevar_codigo, fecha, codigo_parada_origen);

    return key;
}

void get_sales_fields(const char* row, char** fecha, char** codigo_parada_origen, char** sevar_codigo, char** cantidad_pasajeros) {
    char* buffer = strdup(row);

    // Tokenize the row based on commas
    *fecha = strtok(buffer, ",");
    *codigo_parada_origen = strtok(NULL, ",");
    *sevar_codigo = strtok(NULL, ",");
    *cantidad_pasajeros = strtok(NULL, ",");
}

TicketMap* group_tickets(const char* filename) {
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
    size_t read;
    
    TicketMap* ticket_map = create_ticket_map();

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
        size_t bus_stop;
        size_t passenger_count;
        key = create_key(line_copy, &bus_stop, &passenger_count);

        // Add the row to the corresponding group
        if (key != NULL) {
            ticket_map_insert(ticket_map, key, passenger_count);
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

    if (line != NULL) free(line);
    fclose(file);

    return ticket_map;
}

char* copy_string(char* s) {
    int len = (strlen(s) - 2);
    char* n = (char*)malloc(len * sizeof(char));
    snprintf(n, len, "%s", s);
    return n;
}

DelayMap* summarize_delays(DelayMap* delay_map) {
    DelayMap* new_map = create_delay_map();

    size_t key_count;
    DelayEntry** entries = delay_map_get_all_keys(delay_map, &key_count);

    for (size_t i = 0; i < key_count; i++) {
        for (size_t j = 0; j < entries[i]->row_count; j++) {
            Delay* delay = entries[i]->rows[j];

            char* variante; char* fecha_hora_paso; char* retraso; char* cod_parada; char* X; char* Y;
            get_delay_fields(delay->row, &variante, &fecha_hora_paso, &retraso, &cod_parada, &X, &Y);
            char* fecha = strtok(fecha_hora_paso, " ");

            if (atof(retraso) < 0 || atof(retraso) > 50) {
                continue;
            }

            size_t key_length = strlen(fecha) + strlen(variante) + strlen(cod_parada) + 3; // "<v>_<f>_<p>\0"
            char* key = (char*)malloc(key_length * sizeof(char));

            // Create the vft keydate
            snprintf(key, key_length, "%s_%s_%s", variante, fecha, cod_parada);
            // variante, retraso, parada, X, Y

            DelayEntry* exist = delay_map_search(new_map, key);

            char r[100]; 
        
            if (exist != NULL) {
                double retraso_d = exist->max_delay + atof(retraso);
                sprintf(r, "%f", retraso_d); 
            } else {
                sprintf(r, "%s", retraso); 
            }

            size_t row_len = strlen(fecha) + strlen(variante) + strlen(r) + strlen(X) + strlen(Y) + strlen(cod_parada) + 5;
            char* new_row = (char*)malloc(row_len * sizeof(char));
            char* new_Y = copy_string(Y); // Remove line break
            sprintf(new_row, "%s,%s,%s,%s,%s,%s", fecha, variante, r, cod_parada, X, new_Y); 
            delay_map_insert_row(new_map, key, new_row);
        }
    }

    return new_map;
}


void generate_csv(DelayMap* delay_map, const char* sales_filename, const char* output_filename) {
    fprintf(stderr,"  DELAY SUMMARIZATION IN PROGRESS...     \n");
    DelayMap* new_delay = summarize_delays(delay_map);

    fprintf(stderr,"  PROCESSING SALES DATA...     \n");
    TicketMap* ticket_map = group_tickets(sales_filename);

    FILE *file = fopen(output_filename, "w");
    if (!file) {
        perror("Failed to create file");
        return;
    }

    fprintf(stderr,"  GENERATING CSV...     \n");

    // Write the CSV header
    fprintf(file, "fecha,variante,retraso,cod_parada,X,Y,cantidad_pasajeros\n");

    size_t key_count;
    DelayEntry** entries = delay_map_get_all_keys(new_delay, &key_count);

    for (size_t i = 0; i < key_count; i++) {
        TicketEntry* ticket_entry = ticket_map_search(ticket_map, entries[i]->key);
        if (ticket_entry != NULL) {
            // int len_row = strlen(entries[i]->row);
            fprintf(file, "%s,%ld\n", entries[i]->row, ticket_entry->passenger_count);
        } else {
            continue;
        }
    }

    fprintf(stderr,"  CSV GENERATED     \n");
}
