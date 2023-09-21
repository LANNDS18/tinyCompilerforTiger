cmake_minimum_required(VERSION 3.10)

project(Tiger)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g2 -ggdb")
SET(LLVM_DIR "/opt/homebrew/Cellar/llvm@11/11.1.0_4/lib/cmake/llvm")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(include)
set(LEXER_SRC src/lexer.cpp)
set(PARSER_SRC src/parser.cpp)
set(CHECKER_SRC src/symbolTab.cpp src/typeCheck.cpp)
set(GENERATOR_SRC src/codeGenerator.cpp)
set(AST_SRC src/ASTPrinter.cpp)
set(MAIN_SRC src/main.cpp)
set(RUNTIME_SRC src/runtime/runtime.c)

add_executable(
        tiger
        ${MAIN_SRC}
        ${LEXER_SRC}
        ${PARSER_SRC}
        ${CHECKER_SRC}
        ${GENERATOR_SRC}
        ${AST_SRC}
        ${RUNTIME_SRC}
)

llvm_map_components_to_libnames(llvm_libs support core BitWriter)
message(STATUS "Using LLVM libs: ${llvm_libs}")
target_link_libraries(tiger ${llvm_libs})