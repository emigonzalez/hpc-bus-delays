#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "data_grouping.hpp"
#include "date_to_day_type.hpp"
#include "delay_calculation.hpp"
#include "delay_map.hpp"
#include "file_distribute.hpp"
#include "hash_map.hpp"
#include "location_mapping.hpp"
#include "master.hpp"
#include "result_gathering.hpp"
#include "string_array.hpp"
#include "ticket_map.hpp"
#include "worker.hpp"

Entry* HashMap::hash_map_insert_vft(const std::string& key, const std::string& row) {
    auto& entry = map[key];
    if (!entry) {
        entry = new Entry();
        entry->key = key;
    }
    entry->vft_rows.push_back(row);
    return entry;
}

Entry* HashMap::hash_map_insert_vfd(const std::string& key, const std::string& row) {
    auto& entry = map[key];
    if (!entry) {
        entry = new Entry();
        entry->key = key;
    }
    entry->vfd_rows.push_back(row);
    return entry;
}

Entry* HashMap::hash_map_insert_vfd_delays(const std::string& key, const std::string& row, int bus_stop, double delay) {
    auto& entry = map[key];
    if (!entry) {
        entry = new Entry();
        entry->key = key;
    }
    if (entry->bus_stop != bus_stop) {
        entry->bus_stop = bus_stop;
        entry->delay = delay;
    }
    entry->vfd_rows.push_back(row);
    return entry;
}

Entry* HashMap::hash_map_search(const std::string& key) const {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    return nullptr;
}

void HashMap::print_hash_map() const {
    for (const auto& pair : map) {
        const Entry* entry = pair.second;
        std::cout << "Key: " << entry->key << "\n";

        std::cout << "VFTs:\n";
        for (const auto& row : entry->vft_rows) {
            std::cout << "  " << row << "\n";
        }

        std::cout << "VFDs:\n";
        if (entry->bus_stop > 0) {
            for (const auto& row : entry->vfd_rows) {
                std::cout << "  " << row << "\n";
            }
        } else {
            for (const auto& row : entry->vfd_rows) {
                std::cout << "  " << row << "\n";
            }
        }
    }
}

std::vector<Entry*> HashMap::get_all_keys(size_t& key_count) const {
    key_count = map.size();
    std::vector<Entry*> keys;
    keys.reserve(key_count);
    for (const auto& pair : map) {
        keys.push_back(pair.second);
    }
    return keys;
}
