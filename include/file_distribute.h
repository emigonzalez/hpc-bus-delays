#ifndef FILE_DISTRIBUTE_H
#define FILE_DISTRIBUTE_H

char** generate_file_names(int num_days, int num_hours_per_day);
char** distribute_file_names(char** file_names, int num_files, int rank, int size);

#endif // FILE_DISTRIBUTE_H
