# Makefile

# Default value for the number of processes
PROCESSES ?= 1

CC = mpicc
CFLAGS = -Iinclude -Wall -fPIC
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
	mpirun -np $(PROCESSES) ./main

# Debug targets
valgrind: main
	mpirun -np $(PROCESSES) valgrind --leak-check=full ./main

clean:
	rm -rf $(OBJDIR) $(EXEC) $(EXEC).80s*

.PHONY: all clean
