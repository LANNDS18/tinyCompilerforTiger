#pragma once

#include "abstract.h"
#include "symbolTab.h"

class typeCheck {
private:
    symbolTable tbl;
    A_exp *ast;

    baseTy *checkExp(A_exp *exp);

    baseTy *checkVar(A_var *var);

    void checkDec(A_dec *dec);

    static void error(int line, const std::string &errmsg);

public:
    explicit typeCheck(A_exp *ast_) : ast(ast_) {};

    void check();
};