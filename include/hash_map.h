#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>

typedef struct Entry {
    char *key;
    char **rows;
    size_t row_count;
    size_t row_capacity;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    size_t size;
    size_t count;
} HashMap;

HashMap *create_hash_map();
void free_hash_map(HashMap *map);
int hash_map_insert(HashMap *map, const char *key, const char *row);
Entry *hash_map_search(HashMap *map, const char *key);

#endif // HASH_MAP_H
