CC = mpic++
CFLAGS = -Iinclude -Wall -fPIC -std=c++11
OBJDIR = objects
SRCDIR = src
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/%.o,$(TEST_SOURCES))
EXEC = main

all: $(EXEC) 

$(EXEC): $(OBJECTS) main.cpp
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(EXEC) $(EXEC).80s*

.PHONY: all clean
