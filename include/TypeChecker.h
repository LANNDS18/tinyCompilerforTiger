#pragma once

#include "abstract.h"
#include "SymbolTable.h"

class TypeChecker {
private:
    SymbolTable symbolTable;
    A_exp *ast;

    BaseTy *checkExp(A_exp *exp);

    BaseTy *checkVar(A_var *var);

    void checkDec(A_dec *dec);

    static void error(int line, const std::string &errmsg);

public:
    explicit TypeChecker(A_exp *ast_) : ast(ast_) {};

    void check();
};