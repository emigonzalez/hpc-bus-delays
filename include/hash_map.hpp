#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// class Entry {
// public:
//     std::string key;
//     std::vector<std::string> vfd_rows;
//     std::vector<std::string> vft_rows;
//     int bus_stop;
//     double delay;

//     Entry(const std::string& key, int bus_stop = 0, double delay = 0.0)
//         : key(key), bus_stop(bus_stop), delay(delay) {}
// };

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
    void free_hash_map();

    // New member functions to get the size and key count
    size_t get_size() const { return size; }
    size_t get_key_count() const { return map.size(); }
    const std::unordered_map<std::string, Entry*>& get_map() const { return map; }
private:
    size_t size;
    std::unordered_map<std::string, Entry*> map;

    static constexpr size_t INITIAL_SIZE = 1000;
    static constexpr double LOAD_FACTOR = 0.75;
};

// Helper functions
void repoint_vfts_to_vfd_map(Entry* vfd_entry, Entry* vft_entry);

#endif // HASH_MAP_H
