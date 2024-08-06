
#include "string_array.h"

void free_string_array(char** array) {
    if (array != NULL) {
         for (int i = 0; array[i] != NULL; i++) {
            free(array[i]);
        }
        free(array);
    }
}

void send_string_array(char **strings, int num_strings, int dest, int tag, MPI_Comm comm) {
    // Send the number of strings first
    MPI_Send(&num_strings, 1, MPI_INT, dest, tag, comm);

    // Send each string
    for (int i = 0; i < num_strings; i++) {
        int length = strlen(strings[i]) + 1; // Include the null terminator
        MPI_Send(&length, 1, MPI_INT, dest, tag, comm); // Send the length of the string
        MPI_Send(strings[i], length, MPI_CHAR, dest, tag, comm); // Send the actual string
    }
}

void recv_string_array(char ***strings, int *num_strings, int source, int tag, MPI_Comm comm) {
    // Receive the number of strings first
    MPI_Recv(num_strings, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);

    // Allocate memory for the array of strings
    *strings = (char **)malloc(*num_strings * sizeof(char *));

    // Receive each string
    for (int i = 0; i < *num_strings; i++) {
        int length;
        MPI_Recv(&length, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE); // Receive the length of the string
        (*strings)[i] = (char *)malloc(length * sizeof(char)); // Allocate memory for the string
        MPI_Recv((*strings)[i], length, MPI_CHAR, source, tag, comm, MPI_STATUS_IGNORE); // Receive the actual string
    }
}

void print_string_array(char** array) {
    printf("\nPRINTING ARRAY\n");

    for (int i = 0; array[i] != NULL; i++) {
        printf("%s", array[i]);
    }
}

void copy_string_array(char** src, char*** dest, int num_strings) {
    // Allocate memory for the array of string pointers
    *dest = (char**)malloc(num_strings * sizeof(char*));
    if (*dest == NULL) {
        fprintf(stderr, "Memory allocation failed for string array\n");
        return;
    }

    // Allocate memory and copy each string
    for (int i = 0; i < num_strings; ++i) {
        (*dest)[i] = (char*)malloc(strlen(src[i]) + 1); // +1 for the null terminator
        if ((*dest)[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for string %d\n", i);
            // Free already allocated memory in case of failure
            for (int j = 0; j < i; ++j) {
                free((*dest)[j]);
            }
            free(*dest);
            *dest = NULL;
            return;
        }
        strcpy((*dest)[i], src[i]);
    }
}
