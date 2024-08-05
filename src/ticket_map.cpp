#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>

class TicketEntry {
public:
    std::string key;
    size_t passenger_count;
    TicketEntry* next;

    TicketEntry(const std::string& key) : key(key), passenger_count(0), next(nullptr) {}
};

class TicketMap {
public:
    TicketMap(size_t initial_size = 16) : size(initial_size), count(0), buckets(initial_size) {}

    ~TicketMap() = default; // Smart pointers manage the memory

    unsigned long create_ticket_hash(const std::string& str) const {
        std::hash<std::string> hasher;
        return hasher(str) % size;
    }

    void ticket_map_insert(const std::string& key, size_t passenger_count) {
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
                prev->next = (entry);
            }
            count++;
        }

        entry->passenger_count = passenger_count;
    }

    TicketEntry* ticket_map_search(const std::string& key) const {
        size_t bucket_index = create_ticket_hash(key);
        TicketEntry* entry = buckets[bucket_index];

        while (entry != nullptr && entry->key != key) {
            entry = entry->next;
        }

        return entry;
    }

    void print_ticket_map() const {
        for (const auto& bucket : buckets) {
            TicketEntry* entry = bucket;
            while (entry != nullptr) {
                std::cout << "Key: " << entry->key << std::endl;
                std::cout << "CantPasajeros: " << entry->passenger_count << std::endl;
                entry = entry->next;
            }
        }
    }

private:
    size_t size;
    size_t count;
    std::vector<TicketEntry*> buckets;
};
