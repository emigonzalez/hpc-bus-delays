// #include <mpi.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <signal.h>

#include "file_distribute.h"
#include "schedule_distribute.h"
#include "data_grouping.h"
#include "location_mapping.h"
#include "delay_calculation.h"
#include "result_gathering.h"

/*
#define NUM_DAYS 30
#define NUM_HOURS_PER_DAY 24

char** file_names = NULL;
char** assigned_files = NULL;

void handle_signal(int signal);
void free_memory();

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Error handling setup
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Register signal handler
    signal(SIGINT, handle_signal);  // Handle Ctrl+C (interrupt signal)
    signal(SIGTERM, handle_signal); // Handle termination signal

    // File names and schedule file
    char* schedule_file = "data/bus_schedules.csv";

    // Generate the list of file names (example for June, 24 files per day)
    file_names = generate_file_names(NUM_DAYS, NUM_HOURS_PER_DAY);

    // Calculate total number of files
    int num_files = NUM_DAYS * NUM_HOURS_PER_DAY;

    // Distribute file names among processes
    assigned_files = distribute_file_names(file_names, num_files, rank, size);

    printf("BUS LOCATIONS \n");
    // Each process reads its assigned location files
    for (int i = 0; assigned_files[i] != NULL; i++) {
        printf("Process %d reading file %s\n", rank, assigned_files[i]);
        // read_file_and_process_data(assigned_files[i]);
    }

    printf("BUS SCHEDULES \n");
    // Each process reads the entire schedule file
    printf("Process %d reading schedule file %s\n", rank, schedule_file);
    read_schedule_data(schedule_file, rank);

    // Group data by bus variant, time, and day of month/type of day
    group_data_by_bus_and_time(assigned_files);

    // Map grouped locations to grouped schedules
    map_locations_to_schedules();

    // Calculate delays using the Python black box
    calculate_delays();

    // Gather results from all processes
    gather_results();

    // Free allocated memory
    free_memory();

    MPI_Finalize();
    return 0;
}

void free_memory() {
    // Perform any necessary cleanup here
    if (file_names != NULL) {
        for (int i = 0; i < NUM_DAYS * NUM_HOURS_PER_DAY; i++) {
            free(file_names[i]);
        }
        free(file_names);
    }

    if (assigned_files != NULL) {
        for (int i = 0; assigned_files[i] != NULL; i++) {
            free(assigned_files[i]);
        }
        free(assigned_files);
    }
}

void handle_signal(int signal) {
    printf("Received signal %d, performing cleanup...\n", signal);

    // Perform cleanup
    free_memory();

    MPI_Finalize();

    printf("Cleanup done, exiting...\n");
    exit(EXIT_SUCCESS);
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NAME "/vfd_map_shm"
#define SEM_NAME "/vfd_map_sem"
#define SHM_SIZE sizeof(HashMap)

HashMap *vfd_map = NULL;

void run_python_script(const char *script_name) {
    // Create a shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Map the shared memory object
    HashMap *shm_map = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_map == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Copy vfd_map to the shared memory
    memcpy(shm_map, vfd_map, SHM_SIZE);

    // Synchronize the shared memory
    msync(shm_map, SHM_SIZE, MS_SYNC);

    // Create a semaphore for synchronization
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Call the Python script
    char command[256];
    snprintf(command, sizeof(command), "python3 access_vfd_map.py");
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        exit(1);
    }

    // Read the Python script output (if any)
    char output[256];
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("%s", output);
    }

    // Wait for the Python script to finish
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
    }

    // Clean up
    munmap(shm_map, SHM_SIZE);
    close(shm_fd);

    // Unlink the shared memory object and semaphore only after Python script finishes
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

}

int main() {
    printf("BEGIN in C \n");

    char** capturas = (char**)malloc(2 * sizeof(char*));
    capturas[0] = "data/stm-buses-2024-06-09_09_min.csv";
    capturas[1] = NULL; // Terminate the list

    char** horarios = (char**)malloc(2 * sizeof(char*));
    horarios[0] = "data/uptu_pasada_variante_min.csv";
    horarios[1] = NULL; // Terminate the list

    printf("GENERATING VFT...\n");
    HashMap* vft_map = group_data_by_vft(horarios);

    printf("VFT GENERATED.\n");
    printf("GENERATING VFD...\n");
    vfd_map = group_data_by_vfd(capturas, vft_map);

    printf("VFD GENERATED.\n");
    printf("PRINTING MAP...\n");
    // Example: Print grouped data
    print_hash_map(vfd_map);

    // Path to the Python script
    const char *script_name = "access_vfd_map.py";

    printf("CALLING PYTHON SCRIPT %s \n", script_name);

    // Call the function to run the Python script with the hash map pointer
    run_python_script(script_name);

    printf("END PYTHON :) \n");    

    // Free the hash map
    free_hash_map(vft_map);
    free_hash_map(vfd_map);

    free(capturas);
    free(horarios);
    printf("END in C\n");
    return EXIT_SUCCESS;
}
