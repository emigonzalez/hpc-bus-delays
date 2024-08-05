#ifndef DELAY_MAP_H
#define DELAY_MAP_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cmath>

class Delay {
public:
    size_t bus_stop;
    double delay;
    std::string row;

    Delay(size_t bus_stop, double delay, const std::string& row)
        : bus_stop(bus_stop), delay(delay), row(row) {}
};

class DelayEntry {
public:
    std::string key;
    std::vector<Delay*> rows;
    std::string row;
    double max_delay;
    DelayEntry* next;

    DelayEntry(const std::string& key) : key(key), max_delay(0.0), next(nullptr) {}
};

class DelayMap {
public:
    DelayMap(size_t initial_size = INITIAL_SIZE)
        : size(initial_size), count(0), buckets(initial_size) {}

    void delay_map_insert(const std::string& key, size_t bus_stop, double delay, const std::string& row);
    DelayEntry* delay_map_search(const std::string& key) const;
    void print_delay_map() const;
    std::vector<DelayEntry*> delay_map_get_all_keys(size_t& key_count) const;
    size_t get_size() const { return size; }
    size_t get_key_count() const { return count; }
private:
    size_t size;
    size_t count;
    std::vector<DelayEntry*> buckets;

    static constexpr size_t INITIAL_SIZE = 1000;
    static constexpr double LOAD_FACTOR = 0.75;

    size_t create_hash(const std::string& str) const;
    void insert_sorted(DelayEntry* entry, Delay* new_delay);
};

#endif // DELAY_MAP_H
