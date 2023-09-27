// Ensure the header is included only once to avoid multiple definition errors.
#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Token.h"

// Enumeration representing various arithmetic and comparison operators.
enum class A_operator {
    A_plusOp,    // Addition operator
    A_minusOp,   // Subtraction operator
    A_timesOp,   // Multiplication operator
    A_divideOp,  // Division operator
    A_eqOp,      // Equal to operator
    A_neqOp,     // Not equal to operator
    A_ltOP,      // Less than operator
    A_leOp,      // Less than or equal to operator
    A_gtOp,      // Greater than operator
    A_geOp       // Greater than or equal to operator
};

using A_pos = int;               // Alias for position (typically used for debugging or error messages).
using S_symbol = std::string;    // Alias for symbols (typically variable names or identifiers).

// Forward declarations of various abstract syntax tree (AST) node types.
// These structures will represent different language constructs.
struct A_var;
struct A_exp;
struct A_dec;
struct A_type;
struct A_field;
struct A_fieldList;
struct A_expList;
struct A_funcDec;
struct A_funcDecList;
struct A_decList;
struct A_TyDeclareName;
struct A_TyDeclareNameList;
struct A_expField;
struct A_expFieldList;

// Base structure representing a variable in the AST.
struct A_var {
public:
    enum class type {
        SIMPLE,      // A simple variable
        FIELD,       // A field in a record
        SUBSCRIPT    // An element in an array
    };
    A_pos pos;      // Position in the source code
    type ty;        // Variable type (from the inner enum)

    // Constructor
    A_var(A_pos p, type t) : pos(p), ty(t) {};

    // Virtual destructor for polymorphism
    virtual ~A_var() = default;
};

// Structure representing a simple variable in the AST.
struct A_SimpleVar : public A_var {
public:
    S_symbol varSymbol;   // Name of the variable

    A_SimpleVar(A_pos p, S_symbol s) : A_var(p, type::SIMPLE), varSymbol(std::move(s)) {};
};


// Represents a field variable, which is a variable contained within a structure or class.
struct A_FieldVar : public A_var {
public:
    A_var *var;       // The containing variable or structure
    S_symbol varSymbol;     // The name of the field

    A_FieldVar(A_pos p, A_var *var_, S_symbol s) : A_var(p, type::FIELD), var(var_), varSymbol(std::move(s)) {};
};


// Represents an indexed variable, typically from arrays or lists.
struct A_SubscriptVar : public A_var {
public:
    A_var *var;       // The array or list being accessed
    A_exp *exp;       // Expression to determine the index

    A_SubscriptVar(A_pos p, A_var *var_, A_exp *exp_) :
            A_var(p, type::SUBSCRIPT), var(var_), exp(exp_) {};
};


// Base class for all expressions in the language.
struct A_exp {
public:
    enum class type {
        VarExp, NilExp, IntExp, StringExp, CallExp, OpExp, RecordExp, SeqExp, AssignExp, IfExp, WhileExp, LetExp,
        ForExp, ArrayExp, BreakExp
    };
    A_pos pos;
    type ty;

    A_exp(A_pos p, type t) : pos(p), ty(t) {};

    virtual ~A_exp() = default;
};


// Represents an expression that returns the value of a variable.
struct A_VarExp : public A_exp {
public:
    A_var *var;

    A_VarExp(A_pos p, A_var *var_) : A_exp(p, type::VarExp), var(var_) {};
};


// Represents an expression that returns a null value.
struct A_NilExp : public A_exp {
public:
    explicit A_NilExp(A_pos p) : A_exp(p, type::NilExp) {};
};


// Represents an expression that returns an integer value.
struct A_IntExp : public A_exp {
public:
    int i;

    A_IntExp(A_pos p, int i_) : A_exp(p, type::IntExp), i(i_) {};
};


// Represents an expression that returns a string value.
struct A_StringExp : public A_exp {
public:
    std::string s;

    A_StringExp(A_pos p, std::string s_) : A_exp(p, type::StringExp), s(std::move(s_)) {};
};


// Represents a function call expression.
struct A_CallExp : public A_exp {
public:
    S_symbol func;
    A_expList *args;

    A_CallExp(A_pos p, S_symbol func_, A_expList *args_) :
            A_exp(p, type::CallExp), func(std::move(func_)), args(args_) {};
};

struct A_OpExp : public A_exp {
public:
    A_operator aOperator;
    A_exp *left;
    A_exp *right;

    A_OpExp(A_pos p, A_operator anOperator, A_exp *left_, A_exp *right_) :
            A_exp(p, type::OpExp), aOperator(anOperator), left(left_), right(right_) {};
};


// Represents an expression creating a record (or struct).
struct A_RecordExp : public A_exp {
public:
    S_symbol type;
    A_expFieldList *fields;

    A_RecordExp(A_pos p, S_symbol type_, A_expFieldList *fields_) :
            A_exp(p, type::RecordExp), type(std::move(type_)), fields(fields_) {};
};


// Represents a sequence of expressions.
struct A_SeqExp : public A_exp {
public:
    A_expList *seq;

    A_SeqExp(A_pos p, A_expList *seq_) :
            A_exp(p, type::SeqExp), seq(seq_) {};
};


// Represents an assignment expression.
struct A_AssignExp : public A_exp {
public:
    A_var *var;
    A_exp *exp;

    A_AssignExp(A_pos p, A_var *var_, A_exp *exp_) :
            A_exp(p, type::AssignExp), var(var_), exp(exp_) {};
};


// Represents an if-else expression.
struct A_IfExp : public A_exp {
public:
    A_exp *test;
    A_exp *then;
    A_exp *elseExp;

    A_IfExp(A_pos p, A_exp *test_, A_exp *then_, A_exp *else_exp) :
            A_exp(p, type::IfExp), test(test_), then(then_), elseExp(else_exp) {};
};


// While exp
struct A_WhileExp : public A_exp {
public:
    A_exp *condition;
    A_exp *body;

    A_WhileExp(A_pos p, A_exp *test_, A_exp *body_) :
            A_exp(p, type::WhileExp), condition(test_), body(body_) {};
};


// Break exp
struct A_BreakExp : public A_exp {
public:
    explicit A_BreakExp(A_pos p) : A_exp(p, type::BreakExp) {};
};


struct A_ForExp : public A_exp {
public:
    S_symbol var;   //symbol
    A_exp *lo;      // low
    A_exp *hi;      // high
    A_exp *body;    // body

    A_ForExp(A_pos p, S_symbol var_, A_exp *lo_, A_exp *hi_, A_exp *body_) :
            A_exp(p, type::ForExp), var(std::move(var_)), lo(lo_), hi(hi_), body(body_) {};
};


// Represents a let-in-end expression, a kind of scoped expression.
struct A_LetExp : public A_exp {
public:
    A_decList *decList;
    A_exp *body;

    A_LetExp(A_pos p, A_decList *decs_, A_exp *body_) :
            A_exp(p, type::LetExp), decList(decs_), body(body_) {};
};


// Represents an array creation expression.
struct A_ArrayExp : public A_exp {
public:
    S_symbol type;
    A_exp *size;
    A_exp *init;

    A_ArrayExp(A_pos p, S_symbol t, A_exp *size_, A_exp *init_) :
            A_exp(p, type::ArrayExp), type(std::move(t)), size(size_), init(init_) {};
};


// Base class for all declarations in the language.
struct A_dec {
public:
    enum class type {
        FUNCDEC,
        TYDEC,
        VARDEC
    };
    A_pos pos;
    type ty;

    A_dec(A_pos p, type t) : pos(p), ty(t) {};

    virtual ~A_dec() = default;
};


// Represents a function declaration.
struct A_FunctionDec : A_dec {
public:
    A_funcDecList *function;

    A_FunctionDec(A_pos p, A_funcDecList *function_) :
            A_dec(p, type::FUNCDEC), function(function_) {};
};


// Represents a variable declaration.
struct A_VarDec : A_dec {
public:
    S_symbol var;
    S_symbol type;
    A_exp *init;

    A_VarDec(A_pos p, S_symbol var_, S_symbol type_, A_exp *init_) :
            A_dec(p, type::VARDEC), var(std::move(var_)), type(std::move(type_)), init(init_) {};
};


// Represents a type declaration.
struct A_TypeDec : A_dec {
public:
    A_TyDeclareNameList *type;

    A_TypeDec(A_pos p, A_TyDeclareNameList *type_) :
            A_dec(p, type::TYDEC), type(type_) {};
};


// Base class for all types in the language.
struct A_type {
public:
    enum class type {
        NameTy,
        RecordTy,
        ArrayTy
    };
    A_pos pos;
    type ty;

    A_type(A_pos p, type t) : pos(p), ty(t) {};

    virtual ~A_type() = default;
};


struct A_NameTy : public A_type {
public:
    S_symbol type;

    A_NameTy(A_pos p, S_symbol type_) : A_type(p, type::NameTy), type(std::move(type_)) {};
};


struct A_RecordTy : public A_type {
public:
    A_fieldList *record;

    A_RecordTy(A_pos p, A_fieldList *record_) :
            A_type(p, type::RecordTy), record(record_) {};
};


struct A_ArrayTy : public A_type {
public:
    S_symbol array;

    A_ArrayTy(A_pos p, S_symbol array_) : A_type(p, type::ArrayTy), array(std::move(array_)) {};
};


struct A_field {
public:
    A_pos pos;
    S_symbol type;
    S_symbol name;

    A_field(A_pos pos_, S_symbol type_, S_symbol name_) : pos(pos_), type(std::move(type_)), name(std::move(name_)) {};
};


struct A_fieldList {
public:
    A_field *head;
    A_fieldList *tail;

    A_fieldList(A_field *head_, A_fieldList *tail_) : head(head_), tail(tail_) {};
};


struct A_expList {
public:
    A_exp *head;
    A_expList *tail;

    A_expList(A_exp *head_, A_expList *tail_) :
            head(head_), tail(tail_) {};
};


struct A_funcDec {
public:
    A_pos pos;
    S_symbol name;
    A_fieldList *params;
    S_symbol result;
    A_exp *body;

    A_funcDec(A_pos p, S_symbol name_, A_fieldList *params_, S_symbol result_, A_exp *body_) :
            pos(p), name(std::move(name_)), params(params_), result(std::move(result_)), body(body_) {};
};


struct A_funcDecList {
public:
    A_funcDec *head;
    A_funcDecList *tail;

    A_funcDecList(A_funcDec *head_, A_funcDecList *tail_) :
            head(head_), tail(tail_) {};
};


struct A_decList {
public:
    A_dec *head;
    A_decList *tail;

    A_decList(A_dec *head_, A_decList *tail_) :
            head(head_), tail(tail_) {};
};


struct A_TyDeclareName {
public:
    S_symbol name;
    A_type *ty;

    A_TyDeclareName(S_symbol name_, A_type *ty_) : name(std::move(name_)), ty(ty_) {};
};

// linked list
struct A_TyDeclareNameList {
public:
    A_TyDeclareName *head;
    A_TyDeclareNameList *tail;

    A_TyDeclareNameList(A_TyDeclareName *head_, A_TyDeclareNameList *tail_) :
            head(head_), tail(tail_) {};
};


struct A_expField {
public:
    S_symbol name;
    A_exp *exp;

    A_expField(S_symbol name_, A_exp *exp_) : name(std::move(name_)), exp(exp_) {};
};


struct A_expFieldList {
    A_expField *head;
    A_expFieldList *tail;

    A_expFieldList(A_expField *head_, A_expFieldList *tail_) :
            head(head_), tail(tail_) {};
};