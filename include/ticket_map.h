#ifndef TICKET_MAP_H
#define TICKET_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_SIZE 1000
#define LOAD_FACTOR 0.75

// typedef struct Ticket {
//     size_t bus_stop;
//     size_t passenger_count;
//     char* row;
// } Ticket;

typedef struct TicketEntry {
    char *key;
    size_t passenger_count;
    // size_t row_count;
    struct TicketEntry *next;
} TicketEntry;

typedef struct {
    TicketEntry **buckets;
    size_t size;    // Number of buckets
    size_t count;   // Number of key-value pairs
} TicketMap;

TicketMap *create_ticket_map();
TicketEntry *ticket_map_insert(TicketMap *map, const char *key, size_t passenger_count);
TicketEntry *ticket_map_search(TicketMap *map, const char *key);
void free_ticket_map(TicketMap *map);
void print_ticket_map(TicketMap *map);

#endif // TICKET_MAP_H