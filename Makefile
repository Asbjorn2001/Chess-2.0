# Compiler and flags
CXX = g++-14
CXXFLAGS = -Iinclude -std=c++23 -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion

# Directories
SRCDIR = src
OBJDIR = obj

# Source and object files
SOURCES = main.cpp $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Output binary
BIN = chess-2.0

# Default rule
all: $(BIN)

# Link objects into binary
$(BIN): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(CXXFLAGS)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Ensure object folder exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Build shortcut
build: $(BIN)

# Run the program
run: $(BIN)
	./$(BIN)

# Clean up
clean:
	rm -f $(BIN) $(OBJECTS)

