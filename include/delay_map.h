#ifndef DELAY_MAP_H
#define DELAY_MAP_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_SIZE 1000
#define LOAD_FACTOR 0.75

typedef struct Delay {
    size_t bus_stop;
    double delay; // delay in minutes
    char* row;
} Delay;

typedef struct DelayEntry {
    char *key;
    Delay **rows;
    char* row;
    size_t row_count;
    double max_delay;
    struct DelayEntry *next;
} DelayEntry;

typedef struct {
    DelayEntry **buckets;
    size_t size;    // Number of buckets
    size_t count;   // Number of key-value pairs
} DelayMap;

DelayMap *create_delay_map();
DelayEntry *delay_map_insert(DelayMap *map, const char *key, size_t bus_stop, double delay, const char *row); // insertion sort by Delay->bus_stop
DelayEntry *delay_map_search(DelayMap *map, const char *key);
void free_delay_map(DelayMap *map);
void print_delay_map(DelayMap *map);
DelayEntry** delay_map_get_all_keys(DelayMap *map, size_t *key_count);
DelayEntry *delay_map_insert_row(DelayMap *map, const char *key, const char *row);

#endif // DELAY_MAP_H