#!/bin/bash

# Define color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Define default values for arguments if not set
FROM_DAY=${FROM_DAY:-10}
NUM_DAYS=${NUM_DAYS:-1}
NUM_HOURS_PER_DAY=${NUM_HOURS_PER_DAY:-2}
PROCESSES=${PROCESSES:-1}

# Print the values being used
echo -e "${YELLOW}Running with the following settings:${NC}"
echo -e "${GREEN}FROM_DAY = $FROM_DAY${NC}"
echo -e "${GREEN}NUM_DAYS = $NUM_DAYS${NC}"
echo -e "${GREEN}NUM_HOURS_PER_DAY = $NUM_HOURS_PER_DAY${NC}"
echo -e "${GREEN}PROCESSES = $PROCESSES${NC}"

# Clean and compile
echo -e "${YELLOW}Running make clean${NC}"
make clean

echo -e "${YELLOW}Running make${NC}"
make

# Run the program with the specified number of processes
echo -e "${YELLOW}Running program with mpirun -np $PROCESSES ./main $FROM_DAY $NUM_DAYS $NUM_HOURS_PER_DAY${NC}"
mpirun -np $PROCESSES ./main $FROM_DAY $NUM_DAYS $NUM_HOURS_PER_DAY