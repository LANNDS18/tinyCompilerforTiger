# Make Executable file from tiger compilers

# Variables
SRC_FILE ?= test.tig
OUTPUT ?= test           # Define a base name for the output
OUTPUT_EXEC = $(OUTPUT).tigex   # Append .tigex to the base name
TIGER_EXEC = ./build/tiger
BITCODE = temp.bc
RUNTIME_SRC = src/runtime/runtime.c
RUNTIME_OBJ = runtime.o
OBJ = obj.o

# Default target
all: $(OUTPUT_EXEC)

# Compile the input file into bitcode
$(BITCODE): $(SRC_FILE)
	@echo "Compiling the Tiger source to LLVM bitcode..."
	$(TIGER_EXEC) $< $(BITCODE)

# Compile the runtime into an object file
$(RUNTIME_OBJ): $(RUNTIME_SRC)
	@echo "Compiling the runtime source..."
	clang -c $< -o $(RUNTIME_OBJ)

# Compile the bitcode into an object file
$(OBJ): $(BITCODE)
	@echo "Compiling the LLVM bitcode to object file..."
	clang++ $< -c -o $(OBJ)

# Link the object files into the final executable
$(OUTPUT_EXEC): $(OBJ) $(RUNTIME_OBJ)
	@echo "Linking to produce the final executable..."
	clang++ -o $@ $(OBJ) $(RUNTIME_OBJ)

# Clean target to remove intermediate files and the final output
clean:
	@echo "Cleaning up generated files..."
	rm -f $(BITCODE) $(RUNTIME_OBJ) $(OBJ) temp* *.tigex

# .PHONY ensures these targets always run, even if a file by that name exists
.PHONY: all clean
