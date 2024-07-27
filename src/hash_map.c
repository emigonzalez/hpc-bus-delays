#include "hash_map.h"
#include <string.h>
#include <stdio.h>

#define INITIAL_SIZE 16
#define LOAD_FACTOR 0.75
#define INITIAL_ROW_CAPACITY 4

static unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

HashMap *create_hash_map() {
    HashMap *map = malloc(sizeof(HashMap));
    if (!map) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = calloc(map->size, sizeof(Entry *));
    if (!map->buckets) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(map);
        exit(EXIT_FAILURE);
    }
    return map;
}

void free_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry) {
            Entry *tmp = entry;
            entry = entry->next;
            for (size_t j = 0; j < tmp->row_count; j++) {
                free(tmp->rows[j]);
            }
            free(tmp->rows);
            free(tmp->key);
            free(tmp);
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
            unsigned long index = hash(entry->key) % new_size;
            entry->next = new_buckets[index];
            new_buckets[index] = entry;
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = new_buckets;
    map->size = new_size;
}

int hash_map_insert(HashMap *map, const char *key, const char *row) {
    if ((float)map->count / map->size > LOAD_FACTOR) {
        resize_hash_map(map);
    }

    unsigned long index = hash(key) % map->size;
    Entry *entry = map->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Key exists, add the row
            if (entry->row_count == entry->row_capacity) {
                entry->row_capacity *= 2;
                entry->rows = realloc(entry->rows, entry->row_capacity * sizeof(char *));
                if (!entry->rows) {
                    fprintf(stderr, "Error: memory allocation failed\n");
                    return 0;
                }
            }
            entry->rows[entry->row_count] = strdup(row);
            entry->row_count++;
            return 1;
        }
        entry = entry->next;
    }

    // Key does not exist, create a new entry
    Entry *new_entry = malloc(sizeof(Entry));
    if (!new_entry) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return 0;
    }
    new_entry->key = strdup(key);
    if (!new_entry->key) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(new_entry);
        return 0;
    }
    new_entry->row_capacity = INITIAL_ROW_CAPACITY;
    new_entry->rows = malloc(new_entry->row_capacity * sizeof(char *));
    if (!new_entry->rows) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(new_entry->key);
        free(new_entry);
        return 0;
    }
    new_entry->rows[0] = strdup(row);
    new_entry->row_count = 1;
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->count++;
    return 1;
}

Entry *hash_map_search(HashMap *map, const char *key) {
    unsigned long index = hash(key) % map->size;
    Entry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}
