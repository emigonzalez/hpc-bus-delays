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
SHARED_LIB = hash_map.so

all: $(EXEC) $(SHARED_LIB)

$(EXEC): $(OBJECTS) main.c
	$(CC) $(CFLAGS) -o $@ $^

$(SHARED_LIB): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(EXEC) $(SHARED_LIB) $(EXEC).80s*

.PHONY: all clean
