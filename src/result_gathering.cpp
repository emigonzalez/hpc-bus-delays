#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include "result_gathering.hpp"

#define UNUSED(x) (void)(x) // to suppress warning about unused variable

void get_delay_fields(const std::string& row, std::string& variante, std::string& fecha_hora_paso, std::string& retraso, std::string& cod_parada, std::string& X, std::string& Y) {
    std::istringstream ss(row);
    std::string token;

    // VFD,variante,codigo_bus,linea,hora,ordinal,fecha_hora_paso,retraso,vecinos,cod_parada,X,Y
    std::getline(ss, token, ','); // VFD
    std::getline(ss, variante, ',');
    std::getline(ss, token, ','); // codigo_bus
    std::getline(ss, token, ','); // linea
    std::getline(ss, token, ','); // hora
    std::getline(ss, token, ','); // ordinal
    std::getline(ss, fecha_hora_paso, ',');
    std::getline(ss, retraso, ','); // retraso
    std::getline(ss, token, ','); // vecinos
    std::getline(ss, cod_parada, ',');
    std::getline(ss, X, ',');
    std::getline(ss, Y, ',');
}

void get_field_in_row_by_index(const std::string& row, int index, std::string& field) {
    std::istringstream ss(row);
    std::string token;
    int i = 0;

    while (std::getline(ss, token, ',')) {
        i++;
        if (i == index) {
            field = token;
            break;
        }
    }
}

std::string create_key(const std::string& row, size_t& bus_stop, size_t& passenger_count) {
    std::istringstream ss(row);
    std::string fecha, codigo_parada_origen, sevar_codigo, cantidad_pasajeros;

    std::getline(ss, fecha, ',');
    std::getline(ss, codigo_parada_origen, ',');
    std::getline(ss, sevar_codigo, ',');
    std::getline(ss, cantidad_pasajeros, ',');

    UNUSED(sevar_codigo);

    bus_stop = std::stoi(codigo_parada_origen);
    passenger_count = std::stoi(cantidad_pasajeros);

    std::string key = sevar_codigo + "_" + fecha + "_" + codigo_parada_origen;
    return key;
}

void get_sales_fields(const std::string& row, std::string& fecha, std::string& codigo_parada_origen, std::string& sevar_codigo, std::string& cantidad_pasajeros) {
    std::istringstream ss(row);

    std::getline(ss, fecha, ',');
    std::getline(ss, codigo_parada_origen, ',');
    std::getline(ss, sevar_codigo, ',');
    std::getline(ss, cantidad_pasajeros, ',');
}

TicketMap* group_tickets(const std::string& filename) {
    if (filename.empty()) {
        std::cerr << "Error: filename is empty\n";
        return nullptr;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << "\n";
        return nullptr;
    }

    std::string line;
    TicketMap* ticket_map = new TicketMap();

    // Read and skip the header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines

        std::string key;
        size_t bus_stop;
        size_t passenger_count;
        key = create_key(line, bus_stop, passenger_count);

        // Add the row to the corresponding group
        if (!key.empty()) {
            ticket_map_insert(ticket_map, key.c_str(), passenger_count);
        }
    }

    return ticket_map;
}

std::string copy_string(const std::string& s) {
    return s.substr(0, s.length() - 2);
}

std::unique_ptr<DelayMap> summarize_delays(DelayMap* delay_map) {
    auto new_map = std::unique_ptr<DelayMap>(create_delay_map());

    size_t key_count;
    auto entries = delay_map_get_all_keys(delay_map, &key_count);

    for (size_t i = 0; i < key_count; i++) {
        for (size_t j = 0; j < entries[i]->row_count; j++) {
            auto delay = entries[i]->rows[j];

            std::string variante, fecha_hora_paso, retraso, cod_parada, X, Y;
            get_delay_fields(delay->row, variante, fecha_hora_paso, retraso, cod_parada, X, Y);
            std::string fecha = fecha_hora_paso.substr(0, fecha_hora_paso.find(' '));

            if (std::stof(retraso) < 0 || std::stof(retraso) > 50) {
                continue;
            }

            std::string key = variante + "_" + fecha + "_" + cod_parada;

            auto exist = delay_map_search(new_map.get(), key.c_str());

            std::string r;
            if (exist != nullptr) {
                double retraso_d = exist->max_delay + std::stof(retraso);
                r = std::to_string(retraso_d);
            } else {
                r = retraso;
            }

            std::string new_Y = copy_string(Y); // Remove line break
            std::string new_row = fecha + "," + variante + "," + r + "," + cod_parada + "," + X + "," + new_Y;
            delay_map_insert_row(new_map.get(), key.c_str(), new_row.c_str());
        }
    }

    return new_map;
}

void generate_csv(DelayMap* delay_map, const std::string& sales_filename, const std::string& output_filename) {
    auto new_delay = summarize_delays(delay_map);

    auto ticket_map = group_tickets(sales_filename);

    std::ofstream file(output_filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create file\n";
        return;
    }

    // Write the CSV header
    file << "fecha,variante,retraso,cod_parada,X,Y,cantidad_pasajeros\n";

    size_t key_count;
    auto entries = delay_map_get_all_keys(new_delay.get(), &key_count);

    for (size_t i = 0; i < key_count; i++) {
        auto ticket_entry = ticket_map_search(ticket_map, entries[i]->key);
        if (ticket_entry != nullptr) {
            file << entries[i]->row << "," << ticket_entry->passenger_count << "\n";
        }
    }
}
