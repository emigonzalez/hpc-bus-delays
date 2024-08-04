#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void recv_string_array(char ***strings, int *num_strings, int source, int tag, MPI_Comm comm);
void send_string_array(char **strings, int num_strings, int dest, int tag, MPI_Comm comm);
void free_string_array(char** array);
void print_string_array(char** array);

#endif // STRING_ARRAY_H