# # Compiler and compilation flags
# CXX = g++
# CXXFLAGS = -std=c++11

# # Source files
# SRCS = $(wildcard ./src/*.cpp)

# # Executable name
# PROGRAM = asembler.exe

# # Default target
# all: $(PROGRAM)

# # Build the executable
# $(PROGRAM): $(SRCS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# # Clean up object and metafiles
# clean:
# 	del $(PROGRAM) ./tests/*.o ./tests/*.hex




# Compiler and compilation flags
CXX = g++
CXXFLAGS = -std=c++11

# Source files for assembler, linker, and emulator
ASSEMBLER_SRCS = ./src/Assembler.cpp ./src/RelocationTable.cpp ./src/Section.cpp ./src/SymbolTable.cpp
LINKER_SRCS = ./src/Linker.cpp ./src/RelocationTable.cpp ./src/Section.cpp ./src/SymbolTable.cpp
EMULATOR_SRCS = ./src/Emulator.cpp

# Executable names for assembler, linker, and emulator
ASSEMBLER_PROGRAM = assembler.exe
LINKER_PROGRAM = linker.exe
EMULATOR_PROGRAM = emulator.exe

# Default target
all: $(ASSEMBLER_PROGRAM) $(LINKER_PROGRAM) $(EMULATOR_PROGRAM)

# Build the assembler executable
$(ASSEMBLER_PROGRAM): $(ASSEMBLER_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the linker executable
$(LINKER_PROGRAM): $(LINKER_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the emulator executable
$(EMULATOR_PROGRAM): $(EMULATOR_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up object and metafiles
clean:
	del $(ASSEMBLER_PROGRAM) $(LINKER_PROGRAM) $(EMULATOR_PROGRAM) ./tests/*.o ./tests/*.hex


