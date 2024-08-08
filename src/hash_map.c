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
        return NULL;
    }
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (Entry **)calloc(map->size, sizeof(Entry *));
    if (map->buckets == NULL) {
        perror("Failed to create hash map buckets");
        free(map);
        return NULL;
    }
    return map;
}

void free_entry_vfts(Entry *entry) {
    for (size_t j = 0; j < entry->vft_row_count; j++) {
        free(entry->vft_rows[j]);
    }
    free(entry->vft_rows);
    entry->vft_row_count = 0;
    entry->vft_rows = NULL;
};

void free_entry_vfds(Entry* entry) {
    for (size_t j = 0; j < entry->vfd_row_count; j++) {
        free(entry->vfd_rows[j]);
    }
    free(entry->vfd_rows);
    entry->vfd_row_count = 0;
    entry->vfd_rows = NULL;
}

void free_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free_entry_vfts(temp);
            free_entry_vfds(temp);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

void free_vfd_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free_entry_vfds(temp);
            temp->vft_row_count = 0;
            temp->vft_rows = NULL;
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
        return;
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

Entry* create_entry(const char *key) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    if (!entry) {
        fprintf(stderr, "Error creating entry for key %s: memory allocation failed\n", key);
        return NULL;
    }

    entry->key = strdup(key);
    if (!entry->key) {
        fprintf(stderr, "Error adding key %s to entry: memory allocation failed for key\n", key);
        free(entry);
        return NULL;
    }

    entry->vfd_rows = NULL;
    entry->vfd_row_count = 0;
    entry->vft_rows = NULL;
    entry->vft_row_count = 0;
    entry->bus_stop = 0;
    entry->delay = 0.0;
    entry->next = NULL;
    return entry;
}

Entry *find_or_create_entry(HashMap *map, const char *key) {
    if ((float)map->count / map->size > LOAD_FACTOR) {
        resize_hash_map(map);
    }

    unsigned long index = hash(key, map->size);
    Entry *entry = map->buckets[index];
    Entry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        entry = create_entry(key);
        if (prev == NULL) {
            map->buckets[index] = entry;
        } else {
            prev->next = entry;
        }
        map->count++;
    }
    return entry;
}

Entry *insert_to_vfts(Entry *entry, const char *row) {
    // Allocate memory for a new row
    char **temp = (char **)realloc(entry->vft_rows, (entry->vft_row_count + 1) * sizeof(char *));
    if (!temp) {
        fprintf(stderr, "Error: insert_to_vfts memory reallocation failed for vft_rows \n");
        return NULL;
    }
    entry->vft_rows = temp;
    
    // Add the new row
    entry->vft_rows[entry->vft_row_count] = strdup(row);
    if (!entry->vft_rows[entry->vft_row_count]) {
        fprintf(stderr, "Error: insert_to_vfts when adding row to entry\n");
        return NULL;
    }
    entry->vft_row_count++;
    entry->vft_rows[entry->vft_row_count] = NULL;
    return entry;
}

Entry *insert_to_vfds(Entry *entry, const char *row) {
    // Allocate memory for a new row
    char **temp = (char **)realloc(entry->vfd_rows, (entry->vfd_row_count + 1) * sizeof(char *));
    if (!temp) {
        fprintf(stderr, "Error: insert_to_vfds memory reallocation failed for vfd_rows\n");
        return NULL;
    }
    entry->vfd_rows = temp;

    // Add the new row
    entry->vfd_rows[entry->vfd_row_count] = strdup(row);
    if (!entry->vfd_rows[entry->vfd_row_count]) {
        fprintf(stderr, "Error: insert_to_vfds when adding row to entry\n");
        return NULL;
    }
    entry->vfd_row_count++;
    entry->vfd_rows[entry->vfd_row_count] = NULL;
    return entry;
}

Entry *hash_map_insert_vfd(HashMap *map, const char *key, const char *row) {
    Entry *entry = find_or_create_entry(map, key);
    return insert_to_vfds(entry, row);
}

Entry *hash_map_insert_vfd_delays(HashMap *map, const char *key, const char *row, int bus_stop, double delay) {
    Entry *entry = find_or_create_entry(map, key);
    if (entry->bus_stop == bus_stop) {

    } else {
        entry->bus_stop = bus_stop;
        entry->delay = delay;
    }
    return insert_to_vfds(entry, row);
}

Entry *hash_map_insert_vft(HashMap *map, const char *key, const char *row) {
    Entry *entry = find_or_create_entry(map, key);
    return insert_to_vfts(entry, row);
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
            if (entry->bus_stop > 0) {
                printf("  VFD, Variante, Codigo_bus, Linea, Hora, Ordinal, Fecha_hora_paso, Retraso\n");
            } else {
                printf("  ID, Codigo Empresa, Frecuencia, Codigo Bus, Variante, Linea, Sublinea, Tipo Linea, Destino, Subsistema, Version, Velocidad, Latitud, Longitud, Fecha\n");
            }

            for (size_t j = 0; j < entry->vfd_row_count; j++) {
                printf("  %s", entry->vfd_rows[j]);
            }

            entry = entry->next;
        }
    }
}

int compare_keys(const void *a, const void *b) {
    Entry *entry_a = *(Entry **)a;
    Entry *entry_b = *(Entry **)b;
    return strcmp(entry_a->key, entry_b->key);
}

Entry** get_all_keys(HashMap *map, size_t *key_count) {
    *key_count = 0;
    
    // First, count the number of entries
    for (size_t i = 0; i < map->size; ++i) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            (*key_count)++;
            entry = entry->next;
        }
    }
    
    if (*key_count == 0) {
        return NULL;
    }

    // Allocate memory for the array of entries
    Entry **keys = (Entry **)malloc(*key_count * sizeof(Entry *));
    if (!keys) {
        fprintf(stderr, "Error: memory allocation for keys array failed\n");
        return NULL;
    }

    // Populate the array with entries
    size_t index = 0;
    for (size_t i = 0; i < map->size; ++i) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            keys[index++] = entry;
            entry = entry->next;
        }
    }

    // Sort the keys alphabetically by the key field
    qsort(keys, *key_count, sizeof(Entry *), compare_keys);

    return keys;
}
