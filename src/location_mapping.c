#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "location_mapping.h"

const char* temp = "data/temp";

HashMap* group_schedules(char* horarios) {
    HashMap* vft_map = group_data_by_vft(horarios);

    if (vft_map == NULL) {
        fprintf(stderr, "COULD NOT GENERATE VFT.\n");
        return NULL;
    } else {
        fprintf(stderr,"VFT GENERATED FOR %s     \n", horarios);
        return vft_map;
    }
}

char* generate_file_name(const char* path, const char* file_name_prefix, char* fileName) {
    size_t len = strlen(fileName);
    char* date_from_filename = (char*)fileName + len - 10;

    size_t result_len = strlen(path) + strlen(file_name_prefix) + 17;
    char* result = (char*)malloc(result_len * sizeof(char));
    sprintf(result, "%s/%s_%s.csv", path, file_name_prefix, date_from_filename);

    return result;
}

int generate_vfd_file(char* date, HashMap* map) {
    const char *vfd_filename = generate_file_name(temp, "vfd", date);
    const char *capturas_filename = generate_file_name(temp, "capturas", date);
    const char *horarios_filename = generate_file_name(temp, "horarios", date);

    FILE *vfd_file = fopen(vfd_filename, "w");
    if (!vfd_file) {
        perror("Failed to create vfd_file");
        return -1;
    }

    FILE *capturas_file = fopen(capturas_filename, "w");
    if (!capturas_file) {
        perror("Failed create capturas file");
        fclose(vfd_file);
        return -1;
    }

    FILE *horarios_file = fopen(horarios_filename, "w");
    if (!horarios_file) {
        perror("Failed to create horarios file");
        fclose(vfd_file);
        fclose(capturas_file);
        return -1;
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
    return 1;
}

int map_locations_to_schedules(char* fileName, char* date, HashMap* vft_map, HashMap* vfd_map) {
    group_data_by_vfd(fileName, vft_map, vfd_map);

    if (vfd_map == NULL) {
        fprintf(stderr, "MAPA VFD INVALIDO PARA %s.\n", fileName);
        return -1;
    }

    fprintf(stderr,"       VFD GENERADO.    ");

    printf("VFD HashMap (Buckets: %zu, Keys: %zu)       ", vfd_map->size, vfd_map->count);

    int ok = generate_vfd_file(date, vfd_map);

    return ok || -1;
}
