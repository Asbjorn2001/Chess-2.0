# Compiler and flags
CXX = g++-14
# Build mode: debug (default), verbose or release
BUILD ?= normal
OPT ?=

COMMON_FLAGS = -Iinclude -std=c++23 -Wall -Wextra -Weffc++ -mbmi2 -MMD -MP $(OPT)
DEBUG_FLAGS  = -O0 -g -fsanitize=address,undefined
RELEASE_FLAGS = -O3 -DNDEBUG

ifeq ($(BUILD),release)
    CXXFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
else ifeq ($(BUILD),debug)
	CXXFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)
else
    CXXFLAGS = $(COMMON_FLAGS)
endif

LIB = -lSDL2 -lSDL2_image -lSDL2_ttf
GTEST_LIB = -lgtest -lgtest_main -pthread
GBENCH_LIB = -lbenchmark -lpthread

# Directories
SRCDIR = src
TESTDIR = tests
BENCHDIR = benchmarks
OBJDIR = obj
BINDIR = bin

# Source and object files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/%.test.o,$(TEST_SOURCES))

BENCH_SOURCES = $(wildcard $(BENCHDIR)/*.cpp)
BENCH_OBJECTS = $(patsubst $(BENCHDIR)/%.cpp,$(OBJDIR)/%.bench.o,$(BENCH_SOURCES))

# Dependency files
DEPS = $(OBJECTS:.o=.d)
TEST_DEPS = $(TEST_OBJECTS:.o=.d)
BENCH_DEPS = $(BENCH_OBJECTS:.o=.d)

# Output binaries
BIN = $(BINDIR)/chess-2.0
TESTBIN = $(BINDIR)/chess-tests
BENCHBIN = $(BINDIR)/chess-bench

# Default rule
all: $(BIN)

# Link objects into binary
$(BIN): main.cpp $(OBJECTS) | $(BINDIR)
	$(CXX) -o $@ $^ $(LIB) $(CXXFLAGS)

# Link objects into test binary
$(TESTBIN): $(TEST_OBJECTS) $(OBJECTS) | $(BINDIR)
	$(CXX) -o $@ $^ $(GTEST_LIB) $(LIB) $(CXXFLAGS)

$(BENCHBIN): $(BENCH_OBJECTS) $(OBJECTS) | $(BINDIR)
	$(CXX) -o $@ $^ $(GBENCH_LIB) $(LIB) $(CXXFLAGS)

# Compile engine source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Compile test source files
$(OBJDIR)/%.test.o: $(TESTDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJDIR)/%.bench.o: $(BENCHDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Ensure object folder exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Build shortcuts
build: $(BIN)
tests: $(TESTBIN)

# Run binaries
run: $(BIN)
	./$(BIN)

test: $(TESTBIN)
	./$(TESTBIN) $(if $(FILTER), --gtest_filter=$(FILTER))

bench: $(BENCHBIN)
	./$(BENCHBIN) --benchmark_counters_tabular=true

# Clean up
clean:
	rm -f $(BIN) $(TESTBIN) $(BENCHBIN) \
	      $(OBJECTS) $(TEST_OBJECTS) $(BENCH_OBJECTS) \
	      $(DEPS) $(TEST_DEPS) $(BENCH_DEPS)

-include $(DEPS) $(TEST_DEPS) $(BENCH_DEPS)

