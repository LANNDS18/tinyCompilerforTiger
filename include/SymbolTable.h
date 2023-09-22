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
    NIL,
    INT,
    STRING,
    ARRAY,
    VOID
};

struct BaseTy {
    TIG_DTYPE ty;
    S_symbol name; //S_symbol = std::string

    BaseTy(TIG_DTYPE t, S_symbol name_) : ty(t), name(std::move(name_)) {};

    virtual ~BaseTy() = default;
};

struct NilTy : public BaseTy {
    NilTy() : BaseTy(TIG_DTYPE::NIL, "nil") {};
};

struct IntTy : public BaseTy {
    IntTy() : BaseTy(TIG_DTYPE::INT, "int") {};
};

struct StringTy : public BaseTy {
    StringTy() : BaseTy(TIG_DTYPE::STRING, "string") {};
};

struct VoidTy : public BaseTy {
    VoidTy() : BaseTy(TIG_DTYPE::VOID, "void") {};
};

struct RecordTy : public BaseTy {
    std::unordered_map<S_symbol, BaseTy *> fields;

    RecordTy(decltype(fields) fs, S_symbol name_) : BaseTy(TIG_DTYPE::RECORD, std::move(name_)),
                                                    fields(std::move(fs)) {};
};

struct ArrayTy : public BaseTy {
    S_symbol element_type;

    ArrayTy(S_symbol t, S_symbol name_) : BaseTy(TIG_DTYPE::ARRAY, std::move(name_)), element_type(std::move(t)) {};
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
    std::unordered_map<S_symbol, std::vector<BaseTy *>> typeEnv;
    std::unordered_map<S_symbol, std::vector<BaseTy *>> varEnv;
    std::unordered_map<S_symbol, std::vector<std::list<BaseTy *>>> funcEnv;
    std::vector<StackOp> scopeStack;
public:
    SymbolTable();

    void beginScope();

    void endScope();

    void decType(const S_symbol &sym, BaseTy *ty);

    void decVar(const S_symbol &sym, BaseTy *ty);

    void decFunc(const S_symbol &sym, std::list<BaseTy *> &args, BaseTy *retTy);

    BaseTy *lookTy(const S_symbol &ty);

    bool ExistTy(const S_symbol &ty);

    BaseTy *lookVar(const S_symbol &name);

    std::pair<BaseTy *, std::list<BaseTy *>> lookFunc(const S_symbol &name);
};