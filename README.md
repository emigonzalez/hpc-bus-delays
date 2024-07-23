# HPC parallel programming in C using MPI
## Bus delays
The idea is to calculate the delay of the buses in my city within a specific month. We have a big file that captured the bus location (latitude, longitude) of the whole month every 20 seconds. We also have a file with the bus schedule by stop. We have to map the coordinates of the bus location with the stops and the expected arrival time. The delay calculation is already done in python so no need to consider that. 

The main idea is:
 1) Split bus location file between processes.
 2) Share the bus schedules file between processes.
 3) Group locations by **#bus, scheduled time and day of month**.
 4) Group schedules by **#bus, scheduled time and day type**: weekday, saturday or sunday.
 5) Map grouped locations with grouped schedules and send it to the black box algorithm that calculates the delay.
 6) Gather the results and store them in a file.

The bus locations were made on June 2024 and are stored on a file per hour per day. That means, there are 24 files per day and 30 days.

Given that the bus location data is divided into multiple files (one per hour per day), it's indeed a good idea to assign file reading tasks to each process. This way, each process can independently read its assigned files and perform the necessary computations. Here's a refined approach that includes this aspect:

### Steps:
1. **Initialization and Setup**:
   - Initialize MPI.
   - Each process is aware of the total number of processes and its own rank.

2. **Distribute File Names**:
   - Create a list of file names.
   - Distribute the file names among the processes.

3. **File Reading**:
   - Each process independently reads its assigned files and loads the data.

4. **Broadcast Schedule Data**:
   - Broadcast the bus schedule file to all processes.

5. **Data Grouping**:
   - Group the loaded location data by bus number, scheduled time, and day of the month.
   - Group the schedule data by bus number, scheduled time, and type of day (weekday, Saturday, Sunday).

6. **Mapping Locations to Schedules**:
   - Each process maps its grouped location data to the grouped schedule data.

7. **Calculate Delays**:
   - Pass the mapping results to the Python "black box" algorithm for delay calculation.

8. **Collect Results**:
   - Gather the results from all processes and store them in a file.

### Code Structure:
Here's an outline of how you could implement this:

```c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void distribute_file_names(char** file_names, int num_files, int rank, int size);
void broadcast_schedule_data(char* schedule_filename);
void read_files_and_load_data(char** assigned_files, int num_files);
void group_data_by_bus_and_time();
void map_locations_to_schedules();
void calculate_delays();
void gather_results();

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // File names and schedule file
    char* schedule_file = "bus_schedules.txt";

    // Generate the list of file names (example for June, 24 files per day)
    int num_days = 30;
    int num_hours_per_day = 24;
    int num_files = num_days * num_hours_per_day;
    char** file_names = (char**)malloc(num_files * sizeof(char*));
    for (int day = 1; day <= num_days; day++) {
        for (int hour = 0; hour < num_hours_per_day; hour++) {
            file_names[(day-1) * num_hours_per_day + hour] = (char*)malloc(20 * sizeof(char));
            sprintf(file_names[(day-1) * num_hours_per_day + hour], "bus_locations_%02d_%02d.txt", day, hour);
        }
    }

    // Distribute file names among processes
    distribute_file_names(file_names, num_files, rank, size);

    // Broadcast schedule data to all processes
    broadcast_schedule_data(schedule_file);

    // Group data by bus number, time, and day of month/type of day
    group_data_by_bus_and_time();

    // Map grouped locations to grouped schedules
    map_locations_to_schedules();

    // Calculate delays using the Python black box
    calculate_delays();

    // Gather results from all processes
    gather_results();

    // Free allocated memory
    for (int i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);

    MPI_Finalize();
    return 0;
}

void distribute_file_names(char** file_names, int num_files, int rank, int size) {
    int files_per_process = num_files / size;
    int extra_files = num_files % size;

    int start_idx = rank * files_per_process + (rank < extra_files ? rank : extra_files);
    int end_idx = start_idx + files_per_process + (rank < extra_files);

    for (int i = start_idx; i < end_idx; i++) {
        // Read file and process data
        // read_file_and_load_data(file_names[i]);
    }
}

void broadcast_schedule_data(char* schedule_filename) {
    // Logic to broadcast the schedule data file to all processes
}

void group_data_by_bus_and_time() {
    // Logic to group the data by bus number, scheduled time, and day of month/type of day
}

void map_locations_to_schedules() {
    // Logic to map grouped locations to grouped schedules
}

void calculate_delays() {
    // Logic to call the Python script or function to calculate delays
}

void gather_results() {
    // Logic to gather results from all processes
}
```

### Key Points:

1. **File Name Distribution**: Each process gets a unique subset of file names to read and process.
2. **Independent File Reading**: Each process independently reads its assigned files, reducing the need for inter-process communication at this stage.
3. **Broadcast Schedule Data**: Use `MPI_Bcast` to ensure all processes have access to the schedule data.
4. **Grouping and Mapping**: Implement efficient data structures to handle grouping and mapping operations.
5. **Python Integration**: Ensure smooth interaction between C and Python for delay calculations.
6. **Result Gathering**: Use `MPI_Gather` or similar functions to collect results at the root process.

### Run this project

```shell
# To load mpi module
module load mpi/mpich-x86_64
# To compile all files
make
# To run the code in same machine
mpirun –np <num> ./main
# To run the code in multiple machines
mpirun –np <num> -hosts pcunix40,pcunix42 ./main
# You can also specify a hosts file
mpirun -np <num> --hostfile <file> ./main
```
