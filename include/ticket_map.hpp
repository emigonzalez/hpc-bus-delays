#ifndef TICKET_MAP_H
#define TICKET_MAP_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define INITIAL_SIZE 1000
#define LOAD_FACTOR 0.75

struct TicketEntry {
    std::string key;
    size_t passenger_count;
    std::unique_ptr<TicketEntry> next; // Use unique_ptr for automatic memory management
};

class TicketMap {
public:
    TicketMap(size_t initial_size = INITIAL_SIZE);
    TicketEntry* insert(const std::string &key, size_t passenger_count);
    TicketEntry* search(const std::string &key) const;
    void print() const;
    
private:
    std::vector<std::unique_ptr<TicketEntry>> buckets;
    size_t size;    // Number of buckets
    size_t count;   // Number of key-value pairs

    size_t hash(const std::string &key) const;
    void rehash();
};

#endif // TICKET_MAP_H
