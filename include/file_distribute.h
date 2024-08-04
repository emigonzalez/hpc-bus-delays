#ifndef FILE_DISTRIBUTE_H
#define FILE_DISTRIBUTE_H

char** generate_directories(int from_day, int num_days);
char** generate_location_file_names(char* path, int day, int num_hours_per_day);
char* generate_schedule_file_name(char* path, int day);
char* generate_delay_file_name(char* path, int day);
int distribute(char** file_names, int num_files, int rank, int size, char*** assigned_files);
char* get_day_from_dir_name(const char *str);

#endif // FILE_DISTRIBUTE_H
