#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "delay_map.hpp"

// Helper function to calculate hash value
size_t DelayMap::create_hash(const std::string& str) const {
    unsigned long hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % size;
}

// Insert a delay into the entry, maintaining sorted order and removing duplicates
void DelayMap::insert_sorted(DelayEntry* entry, Delay* new_delay) {
    auto it = std::find_if(entry->rows.begin(), entry->rows.end(),
                           [&](const Delay* delay) {
                               return delay->bus_stop == new_delay->bus_stop;
                           });

    if (it != entry->rows.end()) {
        double delay_n = std::fabs((*it)->delay);
        double new_delay_n = std::fabs(new_delay->delay);
        double prev_delay_n = (it != entry->rows.begin()) ? std::fabs((*(it - 1))->delay) : 0;

        if (std::fabs(prev_delay_n - delay_n) > std::fabs(prev_delay_n - new_delay_n)) {
            (*it)->delay = new_delay->delay;
            (*it)->row = new_delay->row;
        }
        return;
    }

    // Insert in sorted order
    auto insert_pos = std::lower_bound(entry->rows.begin(), entry->rows.end(), new_delay->bus_stop,
                                       [](const Delay* d, size_t bus_stop) {
                                           return d->bus_stop < bus_stop;
                                       });
    entry->rows.insert(insert_pos, std::move(new_delay));
    entry->max_delay = std::max(entry->max_delay, entry->rows.back()->delay);
}

// Insert a row into the hash map
void DelayMap::delay_map_insert(const std::string& key, size_t bus_stop, double delay, const std::string& row) {
    size_t index = create_hash(key);
    DelayEntry* entry = buckets[index];
    DelayEntry* prev = nullptr;

    while (entry != nullptr && entry->key != key) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == nullptr) {
        entry = new DelayEntry(key);
        if (prev == nullptr) {
            buckets[index] = (entry);
        } else {
            prev->next = (entry);
        }
        count++;
    }

    auto delay_entry = new Delay(bus_stop, delay, row);
    insert_sorted(entry, delay_entry);
}

// Search for an entry in the hash map
DelayEntry* DelayMap::delay_map_search(const std::string& key) const {
    size_t index = create_hash(key);
    DelayEntry* entry = buckets[index];

    while (entry != nullptr && entry->key != key) {
        entry = entry->next;
    }

    return entry;
}

// Print the hash map
void DelayMap::print_delay_map() const {
    for (const auto& bucket : buckets) {
        DelayEntry* entry = bucket;
        while (entry != nullptr) {
            std::cout << "Key: " << entry->key << std::endl;
            for (const auto& delay : entry->rows) {
                std::cout << "\tBus Stop: " << delay->bus_stop << ", Delay: " << delay->delay << ", Row: " << delay->row << std::endl;
            }
            entry = entry->next;
        }
    }
}

// Get all keys from the hash map
std::vector<DelayEntry*> DelayMap::delay_map_get_all_keys(size_t& key_count) const {
    key_count = 0;
    std::vector<DelayEntry*> keys;

    for (const auto& bucket : buckets) {
        DelayEntry* entry = bucket;
        while (entry != nullptr) {
            keys.push_back(entry);
            ++key_count;
            entry = entry->next;
        }
    }

    return keys;
}
