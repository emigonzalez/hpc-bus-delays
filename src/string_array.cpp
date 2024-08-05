#include <mpi.h>
#include <cstring>
#include <iostream>

void free_string_array(char** array) {
    if (array != nullptr) {
        for (int i = 0; array[i] != nullptr; i++) {
            delete[] array[i];
        }
        delete[] array;
    }
}

void send_string_array(char **strings, int num_strings, int dest, int tag, MPI_Comm comm) {
    // Send the number of strings first
    MPI_Send(&num_strings, 1, MPI_INT, dest, tag, comm);

    // Send each string
    for (int i = 0; i < num_strings; i++) {
        int length = std::strlen(strings[i]) + 1; // Include the null terminator
        MPI_Send(&length, 1, MPI_INT, dest, tag, comm); // Send the length of the string
        MPI_Send(strings[i], length, MPI_CHAR, dest, tag, comm); // Send the actual string
    }
}

void recv_string_array(char ***strings, int *num_strings, int source, int tag, MPI_Comm comm) {
    // Receive the number of strings first
    MPI_Recv(num_strings, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);

    // Allocate memory for the array of strings
    *strings = new char*[*num_strings];

    // Receive each string
    for (int i = 0; i < *num_strings; i++) {
        int length;
        MPI_Recv(&length, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE); // Receive the length of the string
        (*strings)[i] = new char[length]; // Allocate memory for the string
        MPI_Recv((*strings)[i], length, MPI_CHAR, source, tag, comm, MPI_STATUS_IGNORE); // Receive the actual string
    }
}

void print_string_array(char** array) {
    std::cout << "\nPRINTING ARRAY\n";

    for (int i = 0; array[i] != nullptr; i++) {
        std::cout << array[i];
    }
}
