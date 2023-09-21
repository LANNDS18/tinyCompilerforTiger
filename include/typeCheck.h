#pragma once

#include "abstract.h"
#include "symbolTab.h"

class typeCheck {
private:
    symbolTable tbl;
    A_exp *ast;

    baseTy *check_exp(A_exp *exp);

    baseTy *check_var(A_var *var);

    void check_dec(A_dec *dec);

    static void error(int line, const std::string &errmsg);

public:
    explicit typeCheck(A_exp *ast_) : ast(ast_) {};

    void check();
};