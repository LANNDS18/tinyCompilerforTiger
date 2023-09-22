#include "../include/Parser.h"
#include "../include/TypeChecker.h"
#include "../include/CodeGenerator.h"
#include "../include/ASTPrinter.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "expected input file name!";
        exit(1);
    }
    std::string input_file{argv[1]}, output_file{"out.bc"};
    if (argc >= 3) {
        output_file = std::string{argv[2]};
    }
    FRONTEND::Parser p(input_file);
    auto tree = p.parse();
    if (argc == 3 || strcmp(argv[3], "-bc") == 0) {
        TypeChecker c(tree);
        c.check();
        CodeGenerator g;
        g.generate(tree, output_file, 0);
    } else if (strcmp(argv[3], "-ir") == 0) {
        TypeChecker c(tree);
        c.check();
        CodeGenerator g;
        g.generate(tree, output_file, 1);
    } else if (strcmp(argv[3], "-ast") == 0) {
        std::ofstream outfile(output_file);
        std::streambuf *outBuffer = std::cout.rdbuf();
        std::cout.rdbuf(outfile.rdbuf());
        ASTPrinter::printExp(tree, 0);
        std::cout.rdbuf(outBuffer);
    } else if (strcmp(argv[3], "-ast+") == 0) {
        std::ofstream outfile(output_file);
        std::streambuf *outBuffer = std::cout.rdbuf();
        std::cout.rdbuf(outfile.rdbuf());
        TypeChecker c(tree);
        c.check();
        ASTPrinter::printExp(tree, 0);
        std::cout.rdbuf(outBuffer);
    } else {
        std::cout << "Unknown cmd arg. Expected args are:" << std::endl;
        std::cout << "-bc : generate llvm bytecode." << std::endl;
        std::cout << "-ir : generate llvm ir." << std::endl;
        std::cout << "-ast : generate abstract syntax tree." << std::endl;
        std::cout << "-ast+ : generate converted abstract syntax tree after semantic analyse." << std::endl;
    }
    return 0;
}