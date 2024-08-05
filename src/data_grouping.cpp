#define _XOPEN_SOURCE 700 // to suppress warning about strptime

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <unordered_map>

// Include the necessary headers for your custom functions
#include "data_grouping.hpp"
#include "date_to_day_type.hpp"
#include "hash_map.hpp"

using namespace std;

// Function to check if departure time is valid
int is_valid_departure_time(const string& frecuencia) {
    int frecuencia_int = stoi(frecuencia) / 10;
    if (frecuencia_int == 0 && frecuencia != "0")
        return -1;

    int minutes = frecuencia_int % 100; // Last three digits divided by 10 for minutes
    int hours = frecuencia_int / 100;   // Remaining digits for hours

    return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59) ? 1 : -1;
}

// Function to convert frecuencia to minutes
int convertir_a_minutos(const string& frecuencia) {
    int frecuencia_number = stoi(frecuencia) / 10;
    int horas = frecuencia_number / 100;
    int minutos = frecuencia_number % 100;

    return (horas * 60) + minutos;
}

// Function to convert time to frecuencia format
void time_to_frecuencia(const string& time, string& resultado) {
    resultado = time.substr(0, 2) + time.substr(3, 2) + "0";
}

// Function to compute extreme delay
int extreme_delay(const string& fecha, const string& frecuencia) {
    string date_part, time_part;
    istringstream(fecha) >> date_part >> time_part;

    string time_frecuencia;
    time_to_frecuencia(time_part, time_frecuencia);

    int time_number = convertir_a_minutos(time_frecuencia);
    int frecuencia_number = convertir_a_minutos(frecuencia);
    int time_difference = abs(frecuencia_number - time_number);

    return (time_difference > (3 * 60) && time_difference < (21 * 60)) ? 1 : 0;
}

// Function to add seconds to date
void add_seconds_to_date(const string& date, int seconds, string& resultado) {
    struct tm tm_date = {};
    strptime(date.c_str(), "%Y-%m-%d", &tm_date);

    time_t t = mktime(&tm_date) + seconds;
    struct tm* new_tm_date = localtime(&t);

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", new_tm_date);
    resultado = buffer;
}

// Function to adjust the date based on time and frequency
void ajustar_fecha(const string& fecha, const string& frecuencia, string& resultado) {
    string date_part, time_part;
    istringstream(fecha) >> date_part >> time_part;

    string time_frecuencia;
    time_to_frecuencia(time_part, time_frecuencia);

    int time_number = convertir_a_minutos(time_frecuencia);
    int frecuencia_number = convertir_a_minutos(frecuencia);

    if (frecuencia_number > time_number && frecuencia_number - time_number > (21 * 60)) {
        add_seconds_to_date(date_part, -86400, resultado);
    } else if (frecuencia_number < 60 && abs(frecuencia_number - time_number) > (21 * 60)) {
        add_seconds_to_date(date_part, 86400, resultado);
    } else {
        resultado = date_part;
    }
}

// Function to create a VFD key
std::string create_vfd_key(const string& line) {
    vector<string> fields(17); // Adjust based on the number of fields
    istringstream iss(line);
    string token;
    int i = 0;
    while (getline(iss, token, ',')) {
        if (i < 17) fields[i++] = token;
    }

    if (
        fields[15].empty() ||
        fields[16].empty() ||
        is_valid_departure_time(fields[2]) < 0 ||
        extreme_delay(fields[17], fields[2]) > 0
    ) {
        return nullptr; // Skip this row
    }

    string date;
    ajustar_fecha(fields[17], fields[2], date);

    string key = fields[4] + "_" + fields[2] + "_" + date;
    return (key);
}

// Function to create a VFT key
std::string create_vft_key(const string& line) {
    vector<string> fields(9);
    istringstream iss(line);
    string token;
    int i = 0;
    while (getline(iss, token, ';')) {
        if (i < 9) fields[i++] = token;
    }

    if (fields[0].empty() || fields[1].empty() || fields[2].empty() || fields[6].empty()) {
        cerr << "Error: Missing data from row to VFT" << endl;
        return nullptr;
    }

    int frecuencia_int = stoi(fields[2]) / 10;
    int hora_int = stoi(fields[5]);
    if (fields[6] == "N" && (hora_int - frecuencia_int) < 0) {
        return nullptr;
    }

    int tipo_dia_int = stoi(fields[0]);
    if (fields[6] == "S") {
        tipo_dia_int = (tipo_dia_int == 1) ? 3 : tipo_dia_int - 1;
    }

    string key = fields[1] + "_" + fields[2] + "_" + to_string(tipo_dia_int);
    return (key);
}

// Function to group data by VFT
HashMap* group_data_by_vft(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return nullptr;
    }

    HashMap* map = new HashMap();
    if (!map) {
        cerr << "Error creating hash map" << endl;
        return nullptr;
    }

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines

        auto line_copy = line;
        auto key = create_vft_key(line_copy);
        if (!key.empty()) {
            // map->add_vft_to_map(map.get(), key->c_str(), line.c_str());
        }
    }

    return map;
}

// Function to create VFT from VFD
std::string create_vft_from_vfd(const string& vfd) {
    auto vfd_copy = vfd;
    istringstream iss(vfd_copy);
    string variante, frecuencia, date;

    getline(iss, variante, '_');
    getline(iss, frecuencia, '_');
    getline(iss, date);

    if (variante.empty() || frecuencia.empty() || date.empty()) {
        return nullptr;
    }

    int tipo_dia_int = date_to_date_type(date);

    string key = variante + "_" + frecuencia + "_" + to_string(tipo_dia_int);
    return (key);
}

// Function to group data by VFD
HashMap* group_data_by_vfd(const string& filename, HashMap* vft_map) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return nullptr;
    }

    if (!vft_map) {
        cerr << "NO VFT MAP PROVIDED" << endl;
        return nullptr;
    }

    HashMap* vfd_map = new HashMap();
    HashMap* discarded_vfds = new HashMap();
    if (!vfd_map || !discarded_vfds) {
        cerr << "Error creating hash maps" << endl;
        return nullptr;
    }

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines

        auto line_copy = line;
        auto vfd_key = create_vfd_key(line_copy);
        if (!vfd_key.empty()) {
            auto vfd_entry = vft_map->hash_map_search(vfd_key);

            if (vfd_entry) {
                // insert_to_vfds(vfd_entry, line.c_str());
            } else if (!discarded_vfds->hash_map_search(vfd_key)) {
                auto vft_key = create_vft_from_vfd(vfd_key);
                if (vft_key.empty()) {
                    auto vft_entry = vft_map->hash_map_search(vft_key);
                    if (vft_entry) {
                        auto vfd_entry = vfd_map->hash_map_insert_vfd(vfd_key, line.c_str());
                        repoint_vfts_to_vfd_map(vfd_entry, vft_entry);
                    } else {
                        discarded_vfds->hash_map_insert_vfd(vfd_key, nullptr);
                    }
                }
            }
        }
    }

    return vfd_map;
}
