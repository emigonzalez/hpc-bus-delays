#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>

#include "ticket_map.hpp"

unsigned long TicketMap::create_ticket_hash(const std::string& str) {
    std::hash<std::string> hasher;
    return hasher(str) % size;
}

void TicketMap::ticket_map_insert(const std::string& key, size_t passenger_count) {
    size_t bucket_index = create_ticket_hash(key);
    TicketEntry* entry = buckets[bucket_index];

    TicketEntry* prev = nullptr;
    while (entry != nullptr && entry->key != key) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == nullptr) {
        entry = new TicketEntry(key);
        if (prev == nullptr) {
            buckets[bucket_index] = entry;
        } else {
            prev->next = entry;
        }
        count++;
    }

    entry->passenger_count = passenger_count;
}

TicketEntry* TicketMap::ticket_map_search(const std::string& key) {
    size_t bucket_index = create_ticket_hash(key);
    TicketEntry* entry = buckets[bucket_index];

    while (entry != nullptr && entry->key != key) {
        entry = entry->next;
    }

    return entry;
}

void TicketMap::print_ticket_map() {
    for (const auto& bucket : buckets) {
        TicketEntry* entry = bucket;
        while (entry != nullptr) {
            std::cout << "Key: " << entry->key << std::endl;
            std::cout << "CantPasajeros: " << entry->passenger_count << std::endl;
            entry = entry->next;
        }
    }
}
