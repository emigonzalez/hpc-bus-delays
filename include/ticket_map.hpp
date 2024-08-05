#ifndef TICKET_MAP_H
#define TICKET_MAP_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define INITIAL_SIZE 1000
#define LOAD_FACTOR 0.75

class TicketEntry {
public:
    std::string key;
    size_t passenger_count;
    TicketEntry* next;

    TicketEntry(const std::string& key) : key(key), passenger_count(0), next(nullptr) {}
};

class TicketMap {
public:
    TicketMap(size_t initial_size = INITIAL_SIZE);
    TicketEntry* insert(const std::string &key, size_t passenger_count);
    TicketEntry* search(const std::string &key) const;
    void print() const;
    unsigned long create_ticket_hash(const std::string &str);
    void ticket_map_insert(const std::string &key, size_t passenger_count);
    TicketEntry *ticket_map_search(const std::string &key);

    void print_ticket_map();

private:
    std::vector<TicketEntry*> buckets;
    size_t size;    // Number of buckets
    size_t count;   // Number of key-value pairs

    size_t hash(const std::string &key) const;
    void rehash();
};

#endif // TICKET_MAP_H
