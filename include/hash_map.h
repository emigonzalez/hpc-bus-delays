#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Define the struct for hash map entries
typedef struct Entry {
    char *key;
    struct Entry *next;
} Entry;

// Define the struct for the hash map
typedef struct {
    Entry **buckets;
    size_t size;
    size_t count;
} HashMap;

// Function declarations
HashMap *create_hash_map();
void free_hash_map(HashMap *map);
int hash_map_insert(HashMap *map, const char *key);
Entry *hash_map_search(HashMap *map, const char *key);

#endif // HASH_MAP_H
