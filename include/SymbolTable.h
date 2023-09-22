#pragma once

#include <unordered_map>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <iostream>
#include "abstract.h"

enum class TIG_DTYPE {
    RECORD,
    NIL_TY,
    INT,
    STRING,
    ARRAY_TY,
    VOID
};

struct baseTy {
    TIG_DTYPE ty;
    S_symbol name; //S_symbol = std::string

    baseTy(TIG_DTYPE t, S_symbol name_) : ty(t), name(std::move(name_)) {};

    virtual ~baseTy() = default;
};

struct nilTy : public baseTy {
    nilTy() : baseTy(TIG_DTYPE::NIL_TY, "nil") {};
};

struct intTy : public baseTy {
    intTy() : baseTy(TIG_DTYPE::INT, "int") {};
};

struct stringTy : public baseTy {
    stringTy() : baseTy(TIG_DTYPE::STRING, "string") {};
};

struct voidTy : public baseTy {
    voidTy() : baseTy(TIG_DTYPE::VOID, "void") {};
};

struct recordTy : public baseTy {
    std::unordered_map<S_symbol, baseTy *> fields;

    recordTy(decltype(fields) fs, S_symbol name_) : baseTy(TIG_DTYPE::RECORD, std::move(name_)),
                                                    fields(std::move(fs)) {};
};

struct arrayTy : public baseTy {
    S_symbol element_type;

    arrayTy(S_symbol t, S_symbol name_) : baseTy(TIG_DTYPE::ARRAY_TY, std::move(name_)), element_type(std::move(t)) {};
};

class SymbolTable {
    enum class operation {
        BEGIN, VAR_DEC, TYPE_DEC, FUN_DEC
    };
    struct StackOp {
        operation op;
        S_symbol name;
    };
private:
    std::unordered_map<S_symbol, std::vector<baseTy *>> typeEnv;
    std::unordered_map<S_symbol, std::vector<baseTy *>> varEnv;
    std::unordered_map<S_symbol, std::vector<std::list<baseTy *>>> funcEnv;
    std::vector<StackOp> scopeStack;
public:
    SymbolTable();

    void beginScope();

    void endScope();

    void decType(const S_symbol &sym, baseTy *ty);

    void decVar(const S_symbol &sym, baseTy *ty);

    void decFunc(const S_symbol &sym, std::list<baseTy *> &args, baseTy *retTy);

    baseTy *lookTy(const S_symbol &ty);

    bool ExistTy(const S_symbol &ty);

    baseTy *lookVar(const S_symbol &name);

    std::pair<baseTy *, std::list<baseTy *>> lookFunc(const S_symbol &name);
};