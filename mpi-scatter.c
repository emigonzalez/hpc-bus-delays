#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SIZE 4 // Número total de archivos

int main(int argc, char *argv[]) {
    int rank, numtasks, sendcount, recvcount, source;
    float sendbuf [SIZE][SIZE]  = {{1.0,2.0,3.0,4.0},{5.0,6.0,7.0,8.0},
    {9.0,10.0,11.0,12.0},{13.0,14.0,15.0,16.0}};
    
    float recvbuf[SIZE];
   
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // // Lista de nombres de archivo
    // char *files[NUM_FILES] = {
    //     "2024-06-01", "2024-06-02", "2024-06-03", "2024-06-04", "2024-06-05",
    //     "2024-06-06", "2024-06-07", "2024-06-08", "2024-06-09", "2024-06-10",
    //     "2024-06-11", "2024-06-12", "2024-06-13", "2024-06-14", "2024-06-15",
    //     "2024-06-16", "2024-06-17", "2024-06-18", "2024-06-19", "2024-06-20",
    //     "2024-06-21", "2024-06-22", "2024-06-23", "2024-06-24", "2024-06-25",
    //     "2024-06-26", "2024-06-27", "2024-06-28", "2024-06-29", "2024-06-30"
    // };

    // // Calcular el número de archivos por proceso
    // int files_per_process = NUM_FILES / size;
    // int remaining_files = NUM_FILES % size;

    // // Enviar datos a cada proceso
    // char *sub_files[files_per_process + (rank < remaining_files ? 1 : 0)];
    // MPI_Scatter(files, files_per_process, MPI_CHAR, sub_files, files_per_process, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Cada proceso recibe datos adicionales si hay archivos sobrantes
    if (numtasks == SIZE) {
        source =1;sendcount= SIZE;recvcount= SIZE;
        MPI_Scatter(setvbuf,sendcount,MPI_FLOAT, recvbuf,recvcount,MPI_FLOAT,source, MPI_COMM_WORLD);
        printf("rank = %d Resultados : %f %f %f %f\n", rank, recvbuf[0],recvbuf[1], recvbuf[2],recvbuf[3]);

    }else 
        printf("mas especifico %d processors. Terminado. \n ", SIZE);
    
    MPI_Finalize();
    return 0;
}
