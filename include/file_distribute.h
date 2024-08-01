#ifndef FILE_DISTRIBUTE_H
#define FILE_DISTRIBUTE_H

char** generate_directories(int num_days);
char** generate_file_names(char* path, int day, int num_hours_per_day);
char** distribute_file_names(char** file_names, int num_files, int rank, int size);
char* get_day_from_dir_name(const char *str);

#endif // FILE_DISTRIBUTE_H
