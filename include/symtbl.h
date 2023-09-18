#pragma once

#include <unordered_map>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <iostream>
#include "abstract.h"

enum class TIGTY {
    RECORD,
    NILTY,
    INT,
    STRING,
    NAME,
    ARRAYTY,
    VOID
};

struct tgrTy {
    TIGTY ty;
    S_symbol name;

    tgrTy(TIGTY t, S_symbol name_) : ty(t), name(std::move(name_)) {};

    virtual ~tgrTy() = default;
};

struct nilTy : public tgrTy {
    nilTy() : tgrTy(TIGTY::NILTY, "nil") {};
};

struct intTy : public tgrTy {
    intTy() : tgrTy(TIGTY::INT, "int") {};
};

struct stringTy : public tgrTy {
    stringTy() : tgrTy(TIGTY::STRING, "string") {};
};

struct voidTy : public tgrTy {
    voidTy() : tgrTy(TIGTY::VOID, "void") {};
};

struct recordTy : public tgrTy {
    std::unordered_map<S_symbol, tgrTy *> fields;

    recordTy(decltype(fields) fs, S_symbol name_) : tgrTy(TIGTY::RECORD, std::move(name_)), fields(std::move(fs)) {};
};

struct arrayTy : public tgrTy {
    S_symbol element_type;

    arrayTy(S_symbol t, S_symbol name_) : tgrTy(TIGTY::ARRAYTY, std::move(name_)), element_type(std::move(t)) {};
};

class symtbl {
    enum class operation {
        BEGIN, VARDEC, TYDEC, FUNDEC
    };
    struct stkop {
        operation op;
        S_symbol name;
    };
private:
    // here we use vector as stack
    std::unordered_map<S_symbol, std::vector<tgrTy *>> tenv;
    std::unordered_map<S_symbol, std::vector<tgrTy *>> venv;
    // first element in list is the type of return value
    std::unordered_map<S_symbol, std::vector<std::list<tgrTy *>>> fenv;
    std::vector<stkop> stk;
public:
    symtbl();

    void beginScope();

    void endScope();

    void decType(const S_symbol &sym, tgrTy *ty);

    void decVar(const S_symbol &sym, tgrTy *ty);

    void decFunc(const S_symbol &sym, std::list<tgrTy *> &args, tgrTy *retTy);

    tgrTy *lookTy(const S_symbol &ty);

    bool ExistTy(const S_symbol &ty);

    tgrTy *lookVar(const S_symbol &name);

    std::pair<tgrTy *, std::list<tgrTy *>> lookFunc(const S_symbol &name);
};