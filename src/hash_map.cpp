#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

struct Entry {
    std::string key;
    std::vector<std::string> vfd_rows;
    std::vector<std::string> vft_rows;
    int bus_stop = 0;
    double delay = 0.0;
};

class HashMap {
public:
    HashMap(size_t initial_size = INITIAL_SIZE) : size(initial_size) {}

    Entry* hash_map_insert_vft(const std::string& key, const std::string& row);
    Entry* hash_map_insert_vfd(const std::string& key, const std::string& row);
    Entry* hash_map_insert_vfd_delays(const std::string& key, const std::string& row, int bus_stop, double delay);
    Entry* hash_map_search(const std::string& key) const;
    void print_hash_map() const;
    std::vector<Entry*> get_all_keys(size_t& key_count) const;

private:
    size_t size;
    std::unordered_map<std::string, Entry*> map;

    static constexpr size_t INITIAL_SIZE = 16;
    static constexpr float LOAD_FACTOR = 0.75;

    void resize_hash_map();
};

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
