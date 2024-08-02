#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "location_mapping.h"

HashMap* group_schedules(char* horarios) {
    printf("\nGENERATING VFT...\n");
    HashMap* vft_map = group_data_by_vft(horarios);

    if (vft_map == NULL) {
        fprintf(stderr, "COULD NOT GENERATE VFT.\n");
        return NULL;
    } else {
        printf("VFT GENERATED.\n\n");
        return vft_map;
    }
}

void generate_vfd_file(HashMap* map, const char *vfd_filename, const char *capturas_filename, const char *horarios_filename) {
    FILE *vfd_file = fopen(vfd_filename, "w");
    if (!vfd_file) {
        perror("Failed to create vfd_file");
        return;
    }

    FILE *capturas_file = fopen(capturas_filename, "w");
    if (!capturas_file) {
        perror("Failed create capturas file");
        fclose(vfd_file);
        return;
    }

    FILE *horarios_file = fopen(horarios_filename, "w");
    if (!horarios_file) {
        perror("Failed to create horarios file");
        fclose(vfd_file);
        fclose(capturas_file);
        return;
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
}

void map_locations_to_schedules(char* fileName, HashMap* vft_map) {
    printf("GENERATING VFD...\n");
    HashMap* vfd_map = group_data_by_vfd(fileName, vft_map);

    if (vfd_map == NULL) {
        fprintf(stderr, "INVALID VFD MAPS FOR %s.\n", fileName);
        return;
    }

    printf("VFD GENERATED.\n");
    // Example: Print grouped data
    // printf("\nPRINTING MAP...\n");
    // print_hash_map(vfd_map);

    printf("HashMap size: %zu\n", vfd_map->size);
    printf("HashMap count: %zu\n", vfd_map->count);

    generate_vfd_file(vfd_map, vfd_filename, capturas_filename, horarios_filename);

    // Free the hash map
    free_vfd_hash_map(vfd_map);
    return;
}
