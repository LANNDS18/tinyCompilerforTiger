# Make Executable file from tiger compilers

# Variables
SRC_FILE ?= test.tig
OUTPUT ?= test
TIGER_EXEC = ./build/tiger
BITCODE = temp.bc
RUNTIME_SRC = src/runtime/runtime.c
RUNTIME_OBJ = runtime.o
OBJ = obj.o

# Default target
all: $(OUTPUT)

# Compile the input file into bitcode
$(BITCODE): $(SRC_FILE)
	$(TIGER_EXEC) $< $(BITCODE)
	if [ $$? != 0 ]; then echo "Fail to compile, exit." && exit 1; fi

# Compile the runtime into an object file
$(RUNTIME_OBJ): $(RUNTIME_SRC)
	clang -c $< -o $(RUNTIME_OBJ)

# Compile the bitcode into an object file
$(OBJ): $(BITCODE)
	clang $< -c -o $(OBJ)

# Link the object files into the final executable
$(OUTPUT): $(OBJ) $(RUNTIME_OBJ)
	clang -o $@ $(OBJ) $(RUNTIME_OBJ)

# Clean target to remove intermediate files and the final output
clean:
	rm -f $(BITCODE) $(RUNTIME_OBJ) $(OBJ) temp*

# .PHONY ensures these targets always run, even if a file by that name exists
.PHONY: all clean
