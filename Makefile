CC = mpicc
CFLAGS = -Iinclude -Wall -fPIC  # Added -fPIC for position-independent code
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

clean:
	rm -rf $(OBJDIR) $(EXEC) $(EXEC).80s*

.PHONY: all clean
