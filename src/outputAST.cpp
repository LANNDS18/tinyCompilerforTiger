#include "abstract.h"
#include "outputAST.h"
#include <iostream>

using namespace std;

void space(int n) {
    std::string str(n, ' ');
    std::cout << str;
}

void printExp(A_exp *exp, int front_space) {
    space(front_space);
    if (exp == nullptr) return;
    switch (exp->ty) {
        case A_exp::type::NilExp:
            cout << "NilExp()";
            break;
        case A_exp::type::ArrayExp: {
            auto e = dynamic_cast<A_ArrayExp *>(exp);
            cout << "ArrayExp(Symbol(" << e->type << ")," << endl;
            printExp(e->size, front_space + 4);
            cout << "," << endl;
            printExp(e->init, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::AssignExp: {
            auto e = dynamic_cast<A_AssignExp *>(exp);
            cout << "AssignmentExp(" << endl;
            printVar(e->var, front_space + 4);
            cout << "," << endl;
            printExp(e->exp, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::BreakExp:
            cout << "BreakExp()";
            break;
        case A_exp::type::CallExp: {
            auto e = dynamic_cast<A_CallExp *>(exp);
            cout << "CallExp(Symbol(" << e->func << ")," << endl;
            printExpList(e->args, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::ForExp: {
            auto e = dynamic_cast<A_ForExp *>(exp);
            cout << "ForExp(Symbol(" << e->var << ")," << endl;
            printExp(e->lo, front_space + 4);
            cout << "," << endl;
            printExp(e->hi, front_space + 4);
            cout << "," << endl;
            printExp(e->body, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::IfExp: {
            auto e = dynamic_cast<A_IfExp *>(exp);
            cout << "IfExp(" << endl;
            printExp(e->test, front_space + 4);
            cout << "," << endl;
            printExp(e->then, front_space + 4);
            cout << "," << endl;
            printExp(e->elsee, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::IntExp: {
            auto e = dynamic_cast<A_IntExp *>(exp);
            cout << "IntExp(" << e->i << ")";
        }
            break;
        case A_exp::type::LetExp: {
            auto e = dynamic_cast<A_LetExp *>(exp);
            cout << "LetExp(" << endl;
            print_decList(e->decs, front_space + 4);
            cout << "," << endl;
            printExp(e->body, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::OpExp: {
            auto e = dynamic_cast<A_OpExp *>(exp);
            cout << "OpExp(Operator(";
            switch (e->oper) {
                case A_operator::A_divideOp:
                    cout << "DIV";
                    break;
                case A_operator::A_eqOp:
                    cout << "EQ";
                    break;
                case A_operator::A_geOp:
                    cout << "GE";
                    break;
                case A_operator::A_gtOp:
                    cout << "GT";
                    break;
                case A_operator::A_leOp:
                    cout << "LE";
                    break;
                case A_operator::A_ltOP:
                    cout << "LT";
                    break;
                case A_operator::A_minusOp:
                    cout << "SUB";
                    break;
                case A_operator::A_neqOp:
                    cout << "NEQ";
                    break;
                case A_operator::A_plusOp:
                    cout << "ADD";
                    break;
                case A_operator::A_timesOp:
                    cout << "MUL";
                    break;
            }
            cout << ")," << endl;
            printExp(e->left, front_space + 4);
            cout << "," << endl;
            printExp(e->right, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::RecordExp: {
            auto e = dynamic_cast<A_RecordExp *>(exp);
            cout << "RecordExp(Symbol(" << e->type << ")," << endl;
            printExpFieldList(e->fields, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::SeqExp: {
            auto e = dynamic_cast<A_SeqExp *>(exp);
            cout << "SeqExp(" << endl;
            printExpList(e->seq, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::StringExp: {
            auto e = dynamic_cast<A_StringExp *>(exp);
            cout << "StringExp(\"" << e->s << "\")";
        }
            break;
        case A_exp::type::WhileExp: {
            auto e = dynamic_cast<A_WhileExp *>(exp);
            cout << "WhileExp(" << endl;
            printExp(e->test, front_space + 4);
            cout << "," << endl;
            printExp(e->body, front_space + 4);
            cout << ")";
        }
            break;
        case A_exp::type::VarExp: {
            auto e = dynamic_cast<A_VarExp *>(exp);
            cout << "VarExp(" << endl;
            printVar(e->var, front_space + 4);
            cout << ")";
        }
            break;
        default:
            break;
    }
}

void printExpList(A_expList *expList, int front_space) {
    space(front_space);
    cout << "ExpList(" << endl;
    while (expList != nullptr && expList->head != nullptr) {
        printExp(expList->head, front_space + 4);
        cout << "," << endl;
        expList = expList->tail;
    }
    space(front_space);
    cout << ")";
}

void printVar(A_var *var, int front_space) {
    space(front_space);
    switch (var->ty) {
        case A_var::type::SIMPLE: {
            auto v = dynamic_cast<A_SimpleVar *>(var);
            cout << "SimpleVar(Symbol(" << v->sym << "))";
        }
            break;
        case A_var::type::FIELD: {
            auto v = dynamic_cast<A_FieldVar *>(var);
            cout << "FieldVar(" << endl;
            printVar(v->var, front_space + 4);
            cout << "," << endl;
            space(front_space + 4);
            cout << "Symbol(" << v->sym << "))";
        }
            break;
        case A_var::type::SUBSCRIPT: {
            auto v = dynamic_cast<A_SubscriptVar *>(var);
            cout << "SubscriptVar(" << endl;
            printVar(v->var, front_space + 4);
            cout << "," << endl;
            printExp(v->exp, front_space + 4);
            cout << ")";
        }
    }
}

void print_dec(A_dec *dec, int front_space) {
    space(front_space);
    switch (dec->ty) {
        case A_dec::type::VARD: {
            auto d = dynamic_cast<A_VarDec *>(dec);
            cout << "VarDec(Symbol(" << d->var << "),Symbol(" << d->type << ")," << endl;
            printExp(d->init, front_space + 4);
            cout << ")";
        }
            break;
        case A_dec::type::FUNCDS: {
            cout << "FunctionDec(" << endl;
            auto d = dynamic_cast<A_FunctionDec *>(dec);
            printFunctionDecList(d->function, front_space + 4);
            cout << ")";
        }
            break;
        case A_dec::type::TYDS: {
            auto d = dynamic_cast<A_TypeDec *>(dec);
            cout << "TypeDec(" << endl;
            printNameTypeList(d->type, front_space + 4);
            cout << ")";
        }
            break;
    }
}

void print_decList(A_decList *decList, int front_space) {
    space(front_space);
    cout << "DecList(" << endl;
    while (decList != nullptr && decList->head != nullptr) {
        print_dec(decList->head, front_space + 4);
        cout << "," << endl;
        decList = decList->tail;
    }
    space(front_space);
    cout << ")";
}

void print_ty(A_ty *ty, int front_space) {
    space(front_space);
    switch (ty->ty) {
        case A_ty::type::NameTy: {
            auto t = dynamic_cast<A_NameTy *>(ty);
            cout << "NameTy(Symbol(" << t->type << "))";
        }
            break;
        case A_ty::type::ArrayTy: {
            auto t = dynamic_cast<A_ArrayTy *>(ty);
            cout << "ArrayTy(Symbol(" << t->array << "))";
        }
            break;
        case A_ty::type::RecordTy: {
            auto t = dynamic_cast<A_RecordTy *>(ty);
            cout << "RecordTy(" << endl;
            printFieldList(t->record, front_space + 4);
            cout << ")";
        }
            break;
    }
}

void printField(A_field *ty, int front_space) {
    space(front_space);
    cout << "Field(Symbol(" << ty->name << "),Symbol(" << ty->type << "))";
}

void printFieldList(A_fieldList *fieldList, int front_space) {
    space(front_space);
    cout << "FieldList(" << endl;
    while (fieldList != nullptr && fieldList->head != nullptr) {
        printField(fieldList->head, front_space + 4);
        cout << "," << endl;
        fieldList = fieldList->tail;
    }
    space(front_space);
    cout << ")";
}

void printTyDeclareName(A_TyDeclareName *ty, int front_space) {
    space(front_space);
    cout << "TypeDeclareName(Symbol(" << ty->name << ")," << endl;
    print_ty(ty->ty, front_space + 4);
    cout << ")";
}

void printNameTypeList(A_TyDeclareNameList *tyList, int front_space) {
    space(front_space);
    cout << "TypeDeclareNameList(" << endl;
    while (tyList != nullptr && tyList->head != nullptr) {
        printTyDeclareName(tyList->head, front_space + 4);
        cout << "," << endl;
        tyList = tyList->tail;
    }
    space(front_space);
    cout << ")";
}

void printFuncDec(A_funcdec *func, int front_space) {
    space(front_space);
    cout << "FunctionDec(Symbol(" << func->name << "),Symbol(" << func->result << ")," << endl;
    printFieldList(func->params, front_space + 4);
    cout << "," << endl;
    printExp(func->body, front_space + 4);
    cout << ")";
}

void printFunctionDecList(A_funcdecList *funcList, int front_space) {
    space(front_space);
    cout << "FunctionDecList(" << endl;
    while (funcList != nullptr && funcList->head != nullptr) {
        printFuncDec(funcList->head, front_space + 4);
        cout << "," << endl;
        funcList = funcList->tail;
    }
    space(front_space);
    cout << ")";
}

void printExpField(A_expField *expField, int front_space) {
    space(front_space);
    cout << "Expression-Field(Symbol(" << expField->name << ")," << endl;
    printExp(expField->exp, front_space + 4);
    cout << ")";
}

void printExpFieldList(A_expFieldList *expFieldList, int front_space) {
    space(front_space);
    cout << "Expression-Field-List(" << endl;
    while (expFieldList != nullptr && expFieldList->head != nullptr) {
        printExpField(expFieldList->head, front_space + 4);
        cout << "," << endl;
        expFieldList = expFieldList->tail;
    }
    space(front_space);
    cout << ")";
}