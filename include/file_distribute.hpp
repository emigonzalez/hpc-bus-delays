#ifndef FILE_DISTRIBUTE_H
#define FILE_DISTRIBUTE_H

#include <vector>
#include <string>

std::vector<std::string> generate_directories(int from_day, int num_days);
std::vector<std::string> generate_location_file_names(std::string path, int day, int num_hours_per_day);
std::string generate_schedule_file_name(std::string path, int day);
std::string generate_delay_file_name(std::string path, int day);
int distribute(std::vector<std::string> file_names, int num_files, int rank, int size, std::vector<std::string> assigned_files);
std::string get_day_from_dir_name(std::string str);

#endif // FILE_DISTRIBUTE_H
