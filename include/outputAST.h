#pragma once

#include "abstract.h"

void space(int n);

void printExp(A_exp *exp, int front_space);

void printExpList(A_expList *expList, int front_space);

void printVar(A_var *var, int front_space);

void print_dec(A_dec *dec, int front_space);

void print_decList(A_decList *decList, int front_space);

void print_ty(A_ty *ty, int front_space);

void printField(A_field *ty, int front_space);

void printFieldList(A_fieldList *fieldList, int front_space);

void printTyDeclareName(A_TyDeclareName *ty, int front_space);

void printNameTypeList(A_TyDeclareNameList *tyList, int front_space);

void printFuncDec(A_funcdec *func, int front_space);

void printFunctionDecList(A_funcdecList *funcList, int front_space);

void printExpField(A_expField *expField, int front_space);

void printExpFieldList(A_expFieldList *expFieldList, int front_space);