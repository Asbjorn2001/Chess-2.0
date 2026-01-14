# Compiler and flags
CXX = g++-14

OPT ?=
CXXFLAGS = -Iinclude -std=c++23 -Wall -Weffc++ -Wextra -mbmi2 -MMD -MP $(OPT)

LIB = -lSDL2 -lSDL2_image
GTEST_LIBS = -lgtest -lgtest_main -pthread

# Directories
SRCDIR = src
TESTDIR = tests
OBJDIR = obj
BINDIR = bin

# Source and object files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/%.test.o,$(TEST_SOURCES))

# Output binaries
BIN = $(BINDIR)/chess-2.0
TESTBIN = $(BINDIR)/chess-tests

# Default rule
all: $(BIN)

# Link objects into binary
$(BIN): main.cpp $(OBJECTS) | $(BINDIR)
	$(CXX) -o $@ $^ $(LIB) $(CXXFLAGS)

# Link objects into test binary
$(TESTBIN): $(TEST_OBJECTS) $(OBJECTS) | $(BINDIR)
	$(CXX) -o $@ $^ $(GTEST_LIBS) $(LIB) $(CXXFLAGS)

# Compile engine source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Compile test source files
$(OBJDIR)/%.test.o: $(TESTDIR)/%.cpp | $(OBJDIR)
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

# Clean up
clean:
	rm -f $(BIN) $(TESTBIN) $(OBJECTS) $(TEST_OBJECTS)

-include $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

