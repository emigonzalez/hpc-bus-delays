#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>

#include "delay_map.hpp"
#include "delay_calculation.hpp"

// Path to the Python script
const std::string script_name = "calcular-retrasos.py";

void run_python_script(const std::string& script_name, int day) {
    // Call the Python script
    std::string command = "python3.10 " + script_name + " n 2024-06-" + (day < 10 ? "0" : "") + std::to_string(day);
    FILE* fp = popen(command.c_str(), "r");
    if (fp == nullptr) {
        perror("popen");
        exit(1);
    }

    // Read the Python script output (if any)
    char output[256];
    while (fgets(output, sizeof(output), fp) != nullptr) {
        std::cout << output;
    }

    // Wait for the Python script to finish
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
    }
}

void python_calculate_delays(int day) {
    // Call the function to run the Python script with the hash map pointer
    run_python_script(script_name, day);
}

int process_row(DelayMap* delay_map, const std::string& line) {
    std::istringstream line_stream(line);
    std::string vfd_key, variante, codigo_bus, linea, hora, ordinal, fecha_hora_paso, retraso;

    std::getline(line_stream, vfd_key, ',');
    if (vfd_key.empty() || vfd_key == "VFD") {
        return -1;
    }

    std::getline(line_stream, variante, ',');
    std::getline(line_stream, codigo_bus, ',');
    std::getline(line_stream, linea, ',');
    std::getline(line_stream, hora, ',');
    std::getline(line_stream, ordinal, ',');
    std::getline(line_stream, fecha_hora_paso, ',');
    std::getline(line_stream, retraso, ',');

    delay_map->delay_map_insert(vfd_key, std::stoi(ordinal), std::stof(retraso), line);

    return 1;
}

int map_delays(DelayMap* delay_map, const std::string& filename) {
    std::cerr << "FILENAME: " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return -1;
    }

    std::string line;
    // Read and skip the header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines
        process_row(delay_map, line);
    }

    return 1;
}
