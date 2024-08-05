#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include "location_mapping.hpp"

const std::string temp = "data/temp";

HashMap* group_schedules(const std::string& horarios) {
    // std::cout << "\nGENERATING VFT...\n";
    HashMap* vft_map = group_data_by_vft(horarios);

    if (vft_map == nullptr) {
        std::cerr << "COULD NOT GENERATE VFT.\n";
        return nullptr;
    } else {
        std::cerr << "VFT GENERATED\n";
        return vft_map;
    }
}

std::string generate_file_name(const std::string& path, const std::string& file_name_prefix, const std::string& fileName) {
    std::string date_from_filename = fileName.substr(fileName.length() - 10);
    std::ostringstream result;
    result << path << "/" << file_name_prefix << "_" << date_from_filename << ".csv";
    return result.str();
}

int generate_vfd_file(const std::string& date, HashMap* map) {
    std::string vfd_filename = generate_file_name(temp, "vfd", date);
    std::string capturas_filename = generate_file_name(temp, "capturas", date);
    std::string horarios_filename = generate_file_name(temp, "horarios", date);

    std::ofstream vfd_file(vfd_filename);
    if (!vfd_file) {
        std::perror("Failed to create vfd_file");
        return -1;
    }

    std::ofstream capturas_file(capturas_filename);
    if (!capturas_file) {
        std::perror("Failed to create capturas file");
        return -1;
    }

    std::ofstream horarios_file(horarios_filename);
    if (!horarios_file) {
        std::perror("Failed to create horarios file");
        return -1;
    }

    // Write the CSV header
    vfd_file << "vfd,cant_capturas,cant_horarios\n";
    capturas_file << "id,codigoEmpresa,frecuencia,codigoBus,variante,linea,sublinea,tipoLinea,tipoLineaDesc,destino,destinoDesc,subsistema,subsistemaDesc,version,velocidad,latitud,longitud,fecha\n";
    horarios_file << "tipo_dia;cod_variante;frecuencia;cod_ubic_parada;ordinal;hora;dia_anterior;X;Y\n";

    // Write the data
    for (const auto& pair : map->get_map()) {
        Entry* entry = pair.second;
        // Write to vfd_file
        vfd_file << entry->key << "," << entry->vfd_rows.size() << "," << entry->vft_rows.size() << "\n";

        // Write to capturas_file
        for (size_t j = 0; j < entry->vfd_rows.size(); ++j) {
            capturas_file << entry->vfd_rows[j];
        }

        // Write to horarios_file
        for (size_t j = 0; j < entry->vft_rows.size(); ++j) {
            horarios_file << entry->vft_rows[j];
        }
    }

    // Close the files
    vfd_file.close();
    capturas_file.close();
    horarios_file.close();
    return 1;
}

int map_locations_to_schedules(const std::string& fileName, const std::string& date, HashMap* vft_map) {
    // std::cout << "GENERATING VFD...\n";
    HashMap* vfd_map = group_data_by_vfd(fileName, vft_map);

    if (vfd_map == nullptr) {
        std::cerr << "INVALID VFD MAPS FOR " << fileName << ".\n";
        return -1;
    }

    std::cerr << "VFD GENERATED.\n";
    // Example: Print grouped data
    // std::cout << "\nPRINTING MAP...\n";
    // print_hash_map(vfd_map);

    std::cout << "HashMap (" << vfd_map->get_size() << ", " << vfd_map->get_key_count() << ")\n";

    int ok = generate_vfd_file(date, vfd_map);

    // Free the hash map
    delete vfd_map;
    return ok || -1;
}
