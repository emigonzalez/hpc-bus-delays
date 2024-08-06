#include <mpi.h>
#include <stdio.h>

#define NUM_FILES 30

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Status status;
    
    // Inicializar MPI
    MPI_Init(&argc, &argv);
    
    // Obtener el rango del proceso y el tamaño total de los procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Los nombres de archivo a distribuir
    char filenames[NUM_FILES][11] = {
        "2024-06-01","2024-06-02","2024-06-03","2024-06-04","2024-06-05",
        "2024-06-06","2024-06-07","2024-06-08","2024-06-09","2024-06-10",
        "2024-06-11","2024-06-12","2024-06-13","2024-06-14","2024-06-15",
        "2024-06-16","2024-06-17","2024-06-18","2024-06-19","2024-06-20",
        "2024-06-21","2024-06-22","2024-06-23","2024-06-24","2024-06-25",
        "2024-06-26","2024-06-27","2024-06-28","2024-06-29","2024-06-30"
    };

    // Cada proceso recibe una porción del arreglo de nombres de archivo
    int count = NUM_FILES / size;
    char local_filenames[count][11];

    MPI_Scatter(filenames, count * 11, MPI_CHAR, local_filenames, count * 11, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Cada proceso imprime los archivos que le fueron asignados
    printf("Proceso %d:\n", rank);
    for (int i = 0; i < count; i++) {
        printf("%s\n", local_filenames[i]);
    }

    // Finalizar MPI
    MPI_Finalize();

    return 0;
}
