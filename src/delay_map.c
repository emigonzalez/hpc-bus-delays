#include "delay_map.h"
#include <stdio.h>
#include <string.h>

// Helper function to calculate hash value
unsigned long create_hash(const char *str, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % size;
}

// Create a new hash map
DelayMap *create_delay_map() {
    DelayMap *map = (DelayMap *)malloc(sizeof(DelayMap));
    if (!map) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (DelayEntry **)calloc(map->size, sizeof(DelayEntry *));
    if (!map->buckets) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(map);
        exit(EXIT_FAILURE);
    }
    return map;
}

// Create a new entry
DelayEntry *create_delay_entry(const char *key) {
    DelayEntry *entry = (DelayEntry *)malloc(sizeof(DelayEntry));
    if (!entry) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    entry->key = strdup(key);
    entry->rows = NULL;
    entry->row = NULL;
    entry->row_count = 0;
    entry->max_delay = 0.0;
    entry->next = NULL;
    return entry;
}

// Insert a delay into the entry, maintaining sorted order and removing duplicates
void insert_sorted(DelayEntry *entry, Delay *new_delay) {
    // Check for duplicates
    for (size_t i = 0; i < entry->row_count; i++) {
        if (entry->rows[i]->bus_stop == new_delay->bus_stop) {
            // Update delay and row if a duplicate is found
            // printf("REPLACE ROW! i = %ld. ROW: %s", i, entry->rows[i]->row);
            double delay_n = fabs(entry->rows[i]->delay);
            double new_delay_n = fabs(new_delay->delay);
            double prev_delay_n = i > 0 ? fabs(entry->rows[i-1]->delay) : 0;

            // printf("DELAYS! prev: %f, actual: %f, new: %f, prev-actual: %f, prev-new: %f", prev_delay_n, delay_n, new_delay_n, fabs(prev_delay_n - delay_n), fabs(prev_delay_n - new_delay_n));

            if (fabs(prev_delay_n - delay_n) > fabs(prev_delay_n - new_delay_n)) {
                entry->rows[i]->delay = new_delay->delay;
                free(entry->rows[i]->row);
                entry->rows[i]->row = strdup(new_delay->row);
            }

            free(new_delay->row);
            free(new_delay);
            return;
        }
    }

    // Allocate new memory for rows array
    Delay **new_rows = (Delay **)malloc((entry->row_count + 1) * sizeof(Delay *));
    if (!new_rows) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Insert in sorted order
    size_t i;
    for (i = 0; i < entry->row_count && entry->rows[i]->bus_stop < new_delay->bus_stop; i++) {
        new_rows[i] = entry->rows[i];
    }

    new_rows[i] = new_delay;
    for (; i < entry->row_count; i++) {
        new_rows[i + 1] = entry->rows[i];
    }

    free(entry->rows);
    entry->rows = new_rows;
    entry->row_count++;
}

DelayEntry *delay_map_insert_row(DelayMap *map, const char *key, const char *row) {
    unsigned long index = create_hash(key, map->size);
    DelayEntry *entry = map->buckets[index];
    DelayEntry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        entry = create_delay_entry(key);
        if (prev == NULL) {
            map->buckets[index] = entry;
        } else {
            prev->next = entry;
        }
        map->count++;
    }

    entry->row = strdup(row);
    return entry;
}

// Insert a row into the hash map
DelayEntry *delay_map_insert(DelayMap *map, const char *key, size_t bus_stop, double delay, const char *row) {
    unsigned long index = create_hash(key, map->size);
    DelayEntry *entry = map->buckets[index];
    DelayEntry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        entry = create_delay_entry(key);
        if (prev == NULL) {
            map->buckets[index] = entry;
        } else {
            prev->next = entry;
        }
        map->count++;
    }

    Delay *delay_entry = (Delay *)malloc(sizeof(Delay));
    if (!delay_entry) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Parse the row to fill delay_entry data (assuming CSV format)
    delay_entry->bus_stop = bus_stop;
    delay_entry->delay = delay;
    delay_entry->row = strdup(row);
    entry->max_delay += delay;
    insert_sorted(entry, delay_entry);

    return entry;
}

// Search for an entry in the hash map
DelayEntry *delay_map_search(DelayMap *map, const char *key) {
    unsigned long index = create_hash(key, map->size);
    DelayEntry *entry = map->buckets[index];

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    return entry;
}

// Free the memory allocated for the hash map
void free_delay_map(DelayMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        DelayEntry *entry = map->buckets[i];
        while (entry != NULL) {
            DelayEntry *temp = entry;
            entry = entry->next;
            free(temp->key);
            for (size_t j = 0; j < temp->row_count; j++) {
                free(temp->rows[j]->row);
                free(temp->rows[j]);
            }
            free(temp->rows);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

// Print the hash map
void print_delay_map(DelayMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        DelayEntry *entry = map->buckets[i];
        while (entry != NULL) {
            printf("Key: %s\n", entry->key);
            for (size_t j = 0; j < entry->row_count; j++) {
                printf("\tBus Stop: %zu, Delay: %.2f, Row: %s\n", entry->rows[j]->bus_stop, entry->rows[j]->delay, entry->rows[j]->row);
            }
            entry = entry->next;
        }
    }
}

DelayEntry** delay_map_get_all_keys(DelayMap *map, size_t *key_count) {
    *key_count = 0;
    
    // First, count the number of entries
    for (size_t i = 0; i < map->size; ++i) {
        DelayEntry *entry = map->buckets[i];
        while (entry != NULL) {
            (*key_count)++;
            entry = entry->next;
        }
    }
    
    if (*key_count == 0) {
        return NULL;
    }

    // Allocate memory for the array of entries
    DelayEntry **keys = (DelayEntry **)malloc(*key_count * sizeof(DelayEntry *));
    if (!keys) {
        fprintf(stderr, "Error: memory allocation for keys array failed\n");
        exit(EXIT_FAILURE);
    }

    // Populate the array with entries
    size_t index = 0;
    for (size_t i = 0; i < map->size; ++i) {
        DelayEntry *entry = map->buckets[i];
        while (entry != NULL) {
            keys[index++] = entry;
            entry = entry->next;
        }
    }

    return keys;
}
