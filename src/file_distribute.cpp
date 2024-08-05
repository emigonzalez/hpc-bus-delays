#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

#include "file_distribute.hpp"

// Generate directory names
std::vector<std::string> generate_directories(int from_day, int num_days) {
    std::vector<std::string> directories;
    for (int day = 1; day <= num_days; ++day) {
        std::ostringstream oss;
        oss << "data/capturas/2024-06-" << std::setw(2) << std::setfill('0') << from_day + day - 1;
        directories.push_back(oss.str());
    }
    return directories;
}

// Generate location file names
std::vector<std::string> generate_location_file_names(const std::string& path, int day, int num_hours_per_day) {
    std::vector<std::string> file_names;
    for (int hour = 0; hour < num_hours_per_day; ++hour) {
        std::ostringstream oss;
        oss << path << "/stm-buses-2024-06-" << std::setw(2) << std::setfill('0') << day << "_" 
            << std::setw(2) << std::setfill('0') << hour << ".csv";
        file_names.push_back(oss.str());
    }
    return file_names;
}

// Generate schedule file name
std::string generate_schedule_file_name(const std::string& path, int day) {
    std::ostringstream oss;
    oss << path << "/uptu_pasada_variante_2024-06-" << std::setw(2) << std::setfill('0') << day << ".csv";
    return oss.str();
}

// Generate delay file name
std::string generate_delay_file_name(const std::string& path, int day) {
    std::ostringstream oss;
    oss << path << "/retrasos_2024-06-" << std::setw(2) << std::setfill('0') << day << ".csv";
    return oss.str();
}

// Distribute files among MPI processes
std::vector<std::string> distribute(std::vector<std::string> file_names, int rank, int size) {
    std::vector<std::string> assigned_files;
    int num_files = file_names.size();
    int files_per_process = num_files / size;
    int extra_files = num_files % size;
    int task = rank;

    int start_idx = task * files_per_process + (task < extra_files ? task : extra_files);
    int end_idx = start_idx + files_per_process + (task < extra_files ? 1 : 0);

    assigned_files.clear();
    for (int i = start_idx; i < end_idx; ++i) {
        assigned_files.push_back(file_names[i]);
    }

    return assigned_files;
}

// Get day from directory name
std::string get_day_from_dir_name(const std::string& str) {
    if (str.size() < 2) {
        return ""; // or handle this case as needed
    }
    return str.substr(str.size() - 2);
}
