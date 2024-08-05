CXX = mpic++
CXXFLAGS = -Iinclude -Wall -fPIC -std=c++11  # Added -fPIC for position-independent code
OBJDIR = objects
SRCDIR = src
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/%.o,$(TEST_SOURCES))
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJECTS) $(OBJDIR)/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/main.o: main.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(EXEC) $(EXEC).80s*

.PHONY: all clean
