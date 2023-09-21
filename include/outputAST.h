#pragma once

#include "abstract.h"

class ASTPrinter {
public:
    static void printExp(A_exp *exp, int front_space);

private:
    static void space(int n);

    static void printExpList(A_expList *expList, int front_space);

    static void printVar(A_var *var, int front_space);

    static void print_dec(A_dec *dec, int front_space);

    static void print_decList(A_decList *decList, int front_space);

    static void print_ty(A_ty *ty, int front_space);

    static void printField(A_field *ty, int front_space);

    static void printFieldList(A_fieldList *fieldList, int front_space);

    static void printTyDeclareName(A_TyDeclareName *ty, int front_space);

    static void printNameTypeList(A_TyDeclareNameList *tyList, int front_space);

    static void printFuncDec(A_funcdec *func, int front_space);

    static void printFunctionDecList(A_funcdecList *funcList, int front_space);

    static void printExpField(A_expField *expField, int front_space);

    static void printExpFieldList(A_expFieldList *expFieldList, int front_space);

};