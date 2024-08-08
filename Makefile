# Makefile

# Define default arguments
PROCESSES ?= 1
FROM_DAY ?= 10
NUM_DAYS ?= 1
NUM_HOURS_PER_DAY ?= 2

CC = mpicc
CFLAGS = -Iinclude -Wall -fPIC -std=gnu17 -g
#  -fsanitize=address -g
# LDFLAGS = -fsanitize=address

OBJDIR = objects
SRCDIR = src
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c,$(OBJDIR)/%.o,$(TEST_SOURCES))
EXEC = main

all: $(EXEC) 

$(EXEC): $(OBJECTS) main.c
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Target for running the MPI program
run: main
	mpirun -np $(PROCESSES) ./main $(FROM_DAY) $(NUM_DAYS) $(NUM_HOURS_PER_DAY)

# Debug targets
valgrind: main
	mpirun -np $(PROCESSES) valgrind --leak-check=full ./main $(FROM_DAY) $(NUM_DAYS) $(NUM_HOURS_PER_DAY)

clean:
	rm -rf $(OBJDIR) $(EXEC) $(EXEC).80s*

.PHONY: all clean
