# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# Source files
SRC = inputbuf.cc lexer.cc parser.cc resolution.cc

# Object files (replace .cc with .o)
OBJ = $(SRC:.cc=.o)

# Output executable
TARGET = a.out

# Default rule to build the program
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Compile each .cc file into .o
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove generated files
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

