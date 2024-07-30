#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>

#define INITIAL_SIZE 1000
#define LOAD_FACTOR 0.75
#define INITIAL_ROW_CAPACITY 1000 // TODO CHECK THIS

typedef struct Entry {
    char *key;
    char **vfd_rows;
    size_t vfd_row_count;
    size_t vfd_row_capacity;
    char **vft_rows;
    size_t vft_row_count;
    size_t vft_row_capacity;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    size_t size;    // Number of buckets
    size_t count;   // Number of key-value pairs
} HashMap;

HashMap *create_hash_map();
Entry *hash_map_insert_vft(HashMap *map, const char *key, const char *row);
Entry *hash_map_insert_vfd(HashMap *map, const char *key, const char *row);
Entry *insert_to_vfds(Entry *entry, const char *row);
Entry *hash_map_search(HashMap *map, const char *key);
void free_hash_map(HashMap *map);
void print_hash_map(HashMap *map);
void repoint_vfts_to_vfd_map(Entry* vfd_entry, Entry* vft_entry);
Entry** get_all_keys(HashMap *map, size_t *key_count);

#endif // HASH_MAP_H
