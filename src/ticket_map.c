#include "ticket_map.h"

// Helper function to calculate hash value
unsigned long create_ticket_hash(const char *str, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % size;
}

// Create a new hash map
TicketMap *create_ticket_map() {
    TicketMap *map = (TicketMap *)malloc(sizeof(TicketMap));
    if (!map) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (TicketEntry **)calloc(map->size, sizeof(TicketEntry *));
    if (!map->buckets) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free(map);
        exit(EXIT_FAILURE);
    }
    return map;
}

// Create a new entry
TicketEntry *create_ticket_entry(const char *key) {
    TicketEntry *entry = (TicketEntry *)malloc(sizeof(TicketEntry));
    if (!entry) {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    entry->key = strdup(key);
    entry->passenger_count = 0;
    entry->next = NULL;
    return entry;
}

// Function to insert a ticket into the TicketMap
TicketEntry *ticket_map_insert(TicketMap *map, const char *key, size_t passenger_count) {
    // Compute the hash to find the bucket
    size_t bucket_index = create_ticket_hash(key, map->size);
    TicketEntry *entry = map->buckets[bucket_index];

    // Traverse the bucket to find an existing entry with the same key
    TicketEntry *prev = NULL;
    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    // If entry doesn't exist, create a new one
    if (entry == NULL) {
        entry = create_ticket_entry(key);
        if (prev == NULL) {
            // Insert at the head of the bucket
            map->buckets[bucket_index] = entry;
        } else {
            // Insert after the previous entry
            prev->next = entry;
        }
        map->count++;
    }

    // Create a new ticket and add it to the entry's rows
    entry->passenger_count = passenger_count;

    return entry;
}

// Search for an entry in the hash map
TicketEntry *ticket_map_search(TicketMap *map, const char *key) {
    unsigned long index = create_ticket_hash(key, map->size);
    TicketEntry *entry = map->buckets[index];

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    return entry;
}

// Free the memory allocated for the hash map
void free_ticket_map(TicketMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        TicketEntry *entry = map->buckets[i];
        while (entry != NULL) {
            TicketEntry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

// Print the hash map
void print_ticket_map(TicketMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        TicketEntry *entry = map->buckets[i];
        while (entry != NULL) {
            printf("Key: %s\n", entry->key);
            printf("CantPasajeros: %ld", entry->passenger_count);
            entry = entry->next;
        }
    }
}
