#include "../include/SymbolTable.h"

SymbolTable::SymbolTable() {
    auto int_ty = new IntTy();
    auto string_ty = new StringTy();
    auto nil_ty = new NilTy();
    auto void_ty = new VoidTy();
    typeEnv["int"].push_back(int_ty);
    typeEnv["string"].push_back(string_ty);
    typeEnv["nil"].push_back(nil_ty);
    typeEnv["void"].push_back(void_ty);

    funcEnv["print"].push_back({void_ty, string_ty});
    funcEnv["flush"].push_back({void_ty});
    funcEnv["getchar"].push_back({string_ty});
    funcEnv["ord"].push_back({int_ty, string_ty});
    funcEnv["chr"].push_back({string_ty, int_ty});
    funcEnv["substring"].push_back({string_ty, string_ty, int_ty, int_ty});
    funcEnv["concat"].push_back({string_ty, string_ty, string_ty});
    funcEnv["not"].push_back({int_ty, int_ty});
    funcEnv["exit"].push_back({void_ty, int_ty});
    funcEnv["puti"].push_back({void_ty, int_ty});
    funcEnv["getint"].push_back({int_ty});
    funcEnv["gets"].push_back({string_ty});
    funcEnv["size"].push_back({int_ty, string_ty});
}

void SymbolTable::beginScope() {
    scopeStack.push_back({operation::BEGIN, ""});
}

void SymbolTable::endScope() {
    // we assume that beginScope will be called before endScope
    while(scopeStack.back().op != operation::BEGIN) {
        auto name = scopeStack.back().name;
        switch (scopeStack.back().op)
        {
        case operation::VAR_DEC:
            if(!varEnv[name].empty()) {
                varEnv[name].pop_back();
                if(varEnv[name].empty())
                    varEnv.erase(name);
            }
            break;
        case operation::TYPE_DEC:
            if(!typeEnv[name].empty()) {
                typeEnv[name].pop_back();
                if(typeEnv[name].empty())
                    typeEnv.erase(name);
            }
            break;
        case operation::FUN_DEC:
            if(!funcEnv[name].empty()) {
                funcEnv[name].pop_back();
                if(funcEnv[name].empty())
                    funcEnv.erase(name);
            }
            break;
        default:
            break;
        }
        scopeStack.pop_back();
    }
    scopeStack.pop_back();
}

void SymbolTable::decType(const S_symbol& sym, BaseTy* ty) {
    typeEnv[sym].push_back(ty);
    scopeStack.push_back({operation::TYPE_DEC, sym});
}

void SymbolTable::decVar(const S_symbol& sym, BaseTy* ty) {
    varEnv[sym].push_back(ty);
    scopeStack.push_back({operation::VAR_DEC, sym});
}

void SymbolTable::decFunc(const S_symbol& sym, std::list<BaseTy*> &args, BaseTy* retTy) {
    auto args_ = args;
    args_.push_front(retTy);
    funcEnv[sym].push_back(std::move(args_));
    scopeStack.push_back({operation::FUN_DEC, sym});
}

BaseTy* SymbolTable::lookTy(const S_symbol& ty) {
    if(typeEnv.count(ty))
        return typeEnv[ty].back();
    return nullptr;
}

BaseTy* SymbolTable::lookVar(const S_symbol& name) {
    if(varEnv.count(name))
        return varEnv[name].back();
    return nullptr;
}

std::pair<BaseTy*, std::list<BaseTy*>> SymbolTable::lookFunc(const S_symbol& name) {
    if(funcEnv.count(name)) {
        auto list = funcEnv[name].back();
        BaseTy *retType = list.front();
        list.pop_front();
        return {retType, list};
    }
    return {nullptr, {}};
}

bool SymbolTable::ExistTy(const S_symbol& ty) {
    return typeEnv.count(ty);
}