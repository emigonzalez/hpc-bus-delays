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
#define SHM_SIZE 1024*1024

HashMap *vfd_map = NULL;

void copy_hash_map_to_shared_memory(HashMap *vfd_map, void *shared_memory) {
    // Puntero para recorrer la memoria compartida
    void *current_position = shared_memory;
    // Copiar HashMap
    HashMap *shm_hash_map = (HashMap *)current_position;
    memcpy(shm_hash_map, vfd_map, sizeof(HashMap));
    current_position += sizeof(HashMap);

    // Copiar campos_capturas
    char **src_campos_capturas = vfd_map->campos_capturas;
    char **dst_campos_capturas = (char **)current_position;
    size_t num_campos_capturas = 15; //vfd_map->size; // Asegúrate de usar el número correcto
    current_position += num_campos_capturas * sizeof(char *);

    for (size_t i = 0; i < num_campos_capturas; ++i) {
        size_t str_len = strlen(src_campos_capturas[i]) + 1;
        dst_campos_capturas[i] = (char *)current_position;
        memcpy(dst_campos_capturas[i], src_campos_capturas[i], str_len);
        current_position += str_len;
    }
    
    shm_hash_map->campos_capturas = (char **)shared_memory + sizeof(HashMap);

    
    // Copiar campos_horarios de forma similar
    char **src_campos_horarios = vfd_map->campos_horarios;
    char **dst_campos_horarios = (char **)current_position;
    size_t num_campos_horarios = 9; //vfd_map->size; // Ajusta según sea necesario
    current_position += num_campos_horarios * sizeof(char *);

    for (size_t i = 0; i < num_campos_horarios; ++i) {
        size_t str_len = strlen(src_campos_horarios[i]) + 1;
        dst_campos_horarios[i] = (char *)current_position;
        memcpy(dst_campos_horarios[i], src_campos_horarios[i], str_len);
        current_position += str_len;
    }
    
    shm_hash_map->campos_horarios = (char **)shared_memory + sizeof(HashMap) + num_campos_capturas * sizeof(char *);

    // Copiar las entradas del HashMap
    Entry **src_entries = vfd_map->buckets;
    Entry **dst_entries = (Entry **)current_position;
    current_position += vfd_map->size * sizeof(Entry *);

    for (size_t i = 0; i < vfd_map->size; ++i) {
        if (src_entries[i] != NULL) {
            size_t entry_size = sizeof(Entry) + src_entries[i]->vft_count * sizeof(VFT *);
            dst_entries[i] = (Entry *)current_position;
            memcpy(dst_entries[i], src_entries[i], entry_size);
            current_position += entry_size;

            // Copiar VFTs y VFDs si están presentes
            VFT **src_vfts = src_entries[i]->vfts;
            VFT **dst_vfts = dst_entries[i]->vfts;
            for (size_t j = 0; j < src_entries[i]->vft_count; ++j) {
                size_t vft_size = sizeof(VFT);
                dst_vfts[j] = (VFT *)current_position;
                memcpy(dst_vfts[j], src_vfts[j], vft_size);
                current_position += vft_size;
            }

            VFD **src_vfds = src_entries[i]->vfds;
            VFD **dst_vfds = dst_entries[i]->vfds;
            for (size_t j = 0; j < src_entries[i]->vfd_count; ++j) {
                size_t vfd_size = sizeof(VFD);
                dst_vfds[j] = (VFD *)current_position;
                memcpy(dst_vfds[j], src_vfds[j], vfd_size);
                current_position += vfd_size;
            }
        }
    }
}


size_t calculate_hash_map_size(HashMap *map) {
    size_t size = sizeof(HashMap);

    // Suponiendo que map->buckets es un array de punteros a Entry
    size += sizeof(Entry*) * map->size;

    // Calcular tamaño de campos_capturas
    if (map->campos_capturas) {
        size += sizeof(char*) * map->count;
        for (size_t i = 0; i < map->count; ++i) {
            size += strlen(map->campos_capturas[i]) + 1; // +1 para el carácter null
        }
    }

    // Calcular tamaño de campos_horarios
    if (map->campos_horarios) {
        size += sizeof(char*) * map->count;
        for (size_t i = 0; i < map->count; ++i) {
            size += strlen(map->campos_horarios[i]) + 1; // +1 para el carácter null
        }
    }

    // Añadir tamaño de cada Entry
    for (size_t i = 0; i < map->size; ++i) {
        Entry *entry = map->buckets[i];
        while (entry) {
            // Añadir tamaño de cada Entry
            size += sizeof(Entry);

            // Añadir tamaño de VFT* y VFD* en Entry
            size += sizeof(VFT*) * entry->vft_capacity;
            for (size_t j = 0; j < entry->vft_count; ++j) {
                size += sizeof(VFT);
                // Añadir tamaño de los campos en VFT
                size += strlen(entry->vfts[j]->cod_variante) + 1;
                size += strlen(entry->vfts[j]->frecuencia) + 1;
                // ... Hacer lo mismo para otros campos en VFT
            }

            size += sizeof(VFD*) * entry->vfd_capacity;
            for (size_t j = 0; j < entry->vfd_count; ++j) {
                size += sizeof(VFD);
                // Añadir tamaño de los campos en VFD
                size += strlen(entry->vfds[j]->id) + 1;
                size += strlen(entry->vfds[j]->codigoEmpresa) + 1;
                // ... Hacer lo mismo para otros campos en VFD
            }

            entry = entry->next;
        }
    }

    return size;
}


void run_python_script(const char *script_name) {
    // Open the shared memory object
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
    void* shm_base = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Copy vfd_map to the shared memory
    // memcpy(shm_base, vfd_map, sizeof(HashMap));
    deep_copy_hashmap_to_shared_memory(vfd_map,shm_base);

    // Assume we have a pointer to the end of the shared memory
    void *shm_end = shm_base + SHM_SIZE;  // Adjust based on your actual end pointer logic

    // Calculate the size to synchronize
    size_t sync_size = shm_end - shm_base;

    // Synchronize the shared memory
    if (msync(shm_base, sync_size, MS_SYNC) == -1) {
        perror("msync");
        exit(1);
    }

    // Call the Python script
    char command[256];
    snprintf(command, sizeof(command), "python3 access_vfd_map.py");
    // snprintf(command, sizeof(command), "python3.10 calcular-retrasos_conC.py");
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

    // The shared memory now contains a deep copy of the HashMap
    // printf("Deep copy to shared memory completed successfully\n");

    // Cleanup
    munmap(shm_base, SHM_SIZE);
    close(shm_fd);

    // Unlink the shared memory object and semaphore only after Python script finishes
    shm_unlink(SHM_NAME);
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

    char** campos = (get_campos_capturas(vfd_map));
    for (size_t i = 0; i < 15; ++i) {
        char*  campo = campos[i];
        printf("%s", campo);
        printf("\n");
        
    }

    printf("HashMap size: %zu\n", vfd_map->size);
    printf("HashMap count: %zu\n", vfd_map->count);

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
