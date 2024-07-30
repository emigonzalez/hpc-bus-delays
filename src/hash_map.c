#include "hash_map.h"
#include <string.h>
#include <stdio.h>

unsigned long hash(const char *key, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % size;
}

HashMap *create_hash_map() {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == NULL) {
        perror("Failed to create hash map");
        exit(EXIT_FAILURE);
    }
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (Entry **)calloc(map->size, sizeof(Entry *));
    if (map->buckets == NULL) {
        perror("Failed to create hash map buckets");
        free(map);
        exit(EXIT_FAILURE);
    }
    return map;
}

void free_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            for (size_t j = 0; j < temp->vfd_row_count; j++) {
                free(temp->vfd_rows[j]);
            }
            free(temp->vfd_rows);
            for (size_t j = 0; j < temp->vft_row_count; j++) {
                free(temp->vft_rows[j]);
            }
            free(temp->vft_rows);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

void resize_hash_map(HashMap *map) {
    size_t new_size = map->size * 2;
    Entry **new_buckets = calloc(new_size, sizeof(Entry *));
    if (!new_buckets) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry) {
            Entry *next = entry->next;
            unsigned long index = hash(entry->key, new_size);
            entry->next = new_buckets[index];
            new_buckets[index] = entry;
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = new_buckets;
    map->size = new_size;
}

Entry *insert_to_vfts(Entry *entry, const char *row) {
    if (entry->vft_row_count >= entry->row_capacity) {
        entry->row_capacity *= 2;
        entry->vft_rows = (char **)realloc(entry->vft_rows, entry->row_capacity * sizeof(char *));
        if (entry->vft_rows == NULL) {
            perror("Failed to reallocate memory for vft_rows");
            exit(EXIT_FAILURE);
        }
    }
    entry->vft_rows[entry->vft_row_count] = strdup(row);
    entry->vft_row_count++;
    return entry;
}

Entry *insert_to_vfds(Entry *entry, const char *row) {
    if (entry->vfd_row_count == entry->row_capacity) {
        entry->row_capacity *= 2;
        entry->vfd_rows = realloc(entry->vfd_rows, entry->row_capacity * sizeof(char *));
        if (!entry->vfd_rows) {
            fprintf(stderr, "Error: memory allocation failed\n");
            return NULL;
        }
    }
    entry->vfd_rows[entry->vfd_row_count] = strdup(row);
    entry->vfd_row_count++;
    return entry;
}

Entry *hash_map_insert_vfd(HashMap *map, const char *key, const char *row) {
    if ((float)map->count / map->size > LOAD_FACTOR) {
        resize_hash_map(map);
    }

    unsigned long index = hash(key, map->size);
    Entry *entry = map->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Key exists, add the row
            return insert_to_vfds(entry, row);
        }
        entry = entry->next;
    }

    // Key does not exist, create a new entry
    Entry *new_entry = malloc(sizeof(Entry));
    if (!new_entry) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return NULL;
    }
    new_entry->key = strdup(key);
    if (!new_entry->key) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(new_entry);
        return NULL;
    }
    new_entry->row_capacity = INITIAL_ROW_CAPACITY;
    new_entry->vfd_rows = malloc(new_entry->row_capacity * sizeof(char *));
    if (!new_entry->vfd_rows) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(new_entry->key);
        free(new_entry);
        return NULL;
    }
    new_entry->vfd_rows[0] = strdup(row);
    new_entry->vfd_row_count = 1;
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->count++;
    return new_entry;
}

Entry *hash_map_insert_vft(HashMap *map, const char *key, const char *row) {
    Entry *entry = hash_map_search(map, key);
    if (entry == NULL) {
        unsigned long index = hash(key, map->size);
        entry = (Entry *)malloc(sizeof(Entry));
        if (entry == NULL) {
            perror("Failed to allocate memory for Entry");
            exit(EXIT_FAILURE);
        }
        entry->key = strdup(key);
        entry->vfd_rows = NULL;
        entry->vfd_row_count = 0;
        entry->vft_rows = (char **)malloc(INITIAL_ROW_CAPACITY * sizeof(char *));
        entry->row_capacity = INITIAL_ROW_CAPACITY;
        entry->vft_row_count = 0;
        entry->next = map->buckets[index];
        map->buckets[index] = entry;
        map->count++;
    }
    if (entry->vft_row_count >= entry->row_capacity) {
        entry->row_capacity *= 2;
        entry->vft_rows = (char **)realloc(entry->vft_rows, entry->row_capacity * sizeof(char *));
        if (entry->vft_rows == NULL) {
            perror("Failed to reallocate memory for vft_rows");
            exit(EXIT_FAILURE);
        }
    }
    entry->vft_rows[entry->vft_row_count] = strdup(row);
    entry->vft_row_count++;
    return entry;
}

Entry *hash_map_search(HashMap *map, const char *key) {
    unsigned long index = hash(key, map->size);
    Entry *entry = map->buckets[index];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

void repoint_vfts_to_vfd_map(Entry* vfd_entry, Entry* vft_entry) {
    // Repoint the vfd entry's rows to the new entry's rows
    vfd_entry->vft_rows = vft_entry->vft_rows;
    vfd_entry->vft_row_count = vft_entry->vft_row_count;
    vfd_entry->row_capacity = (vfd_entry->row_capacity > vft_entry->row_capacity) ? vfd_entry->row_capacity : vft_entry->row_capacity;

    // Nullify the vft entry's vft_rows to avoid double free
    vft_entry->vft_rows = NULL;
    vft_entry->vft_row_count = 0;
    vft_entry->row_capacity = vft_entry->row_capacity;
}

void print_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            printf("Key: %s\n", entry->key);

            printf("VFTs:\n");
            printf("  Tipo Dia, Variante, Frecuencia, Cod Ubic Parada, Ordinal, Hora, Dia Anterior, Latitud, Longitud\n");
            for (size_t j = 0; j < entry->vft_row_count; j++) {
                printf("  %s", entry->vft_rows[j]);
            }

            printf("VFDs:\n");
            printf("  ID, Codigo Empresa, Frecuencia, Codigo Bus, Variante, Linea, Sublinea, Tipo Linea, Destino, Subsistema, Version, Velocidad, Latitud, Longitud, Fecha\n");
            for (size_t j = 0; j < entry->vfd_row_count; j++) {
                printf("  %s", entry->vfd_rows[j]);
            }

            entry = entry->next;
        }
    }
}
