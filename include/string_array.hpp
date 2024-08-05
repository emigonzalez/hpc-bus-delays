#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <vector>
#include <string>
#include <mpi.h>
#include <iostream>

void recv_string_array(std::vector<std::string> &strings, int *num_strings, int source, int tag, MPI_Comm comm);
void send_string_array(const std::vector<std::string> &strings, int num_strings, int dest, int tag, MPI_Comm comm);
void free_string_array(std::vector<std::string> &array);
void print_string_array(const std::vector<std::string> &array);

#endif // STRING_ARRAY_H
