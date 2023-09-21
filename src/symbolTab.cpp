#include "../include/symbolTab.h"

symbolTable::symbolTable() {
    auto int_ty = new intTy();
    auto string_ty = new stringTy();
    auto nil_ty = new nilTy();
    auto void_ty = new voidTy();
    tenv["int"].push_back(int_ty);
    tenv["string"].push_back(string_ty);
    tenv["nil"].push_back(nil_ty);
    tenv["void"].push_back(void_ty);

    fenv["print"].push_back({void_ty, string_ty});
    fenv["flush"].push_back({void_ty});
    fenv["getchar"].push_back({string_ty});
    fenv["ord"].push_back({int_ty, string_ty});
    fenv["chr"].push_back({string_ty, int_ty});
    fenv["substring"].push_back({string_ty, string_ty, int_ty, int_ty});
    fenv["concat"].push_back({string_ty, string_ty, string_ty});
    fenv["not"].push_back({int_ty, int_ty});
    fenv["exit"].push_back({void_ty, int_ty});

    fenv["puti"].push_back({void_ty, int_ty});
    fenv["getint"].push_back({int_ty});
    fenv["gets"].push_back({string_ty});
    fenv["size"].push_back({int_ty, string_ty});
}

void symbolTable::beginScope() {
    stk.push_back({operation::BEGIN, ""});
}

void symbolTable::endScope() {
    // we assume that beginScope will be called before endScope
    while(stk.back().op != operation::BEGIN) {
        auto name = stk.back().name;
        switch (stk.back().op)
        {
        case operation::VAR_DEC:
            if(!venv[name].empty()) {
                venv[name].pop_back();
                if(venv[name].empty())
                    venv.erase(name);
            }
            break;
        case operation::TYPE_DEC:
            if(!tenv[name].empty()) {
                tenv[name].pop_back();
                if(tenv[name].empty())
                    tenv.erase(name);
            }
            break;
        case operation::FUN_DEC:
            if(!fenv[name].empty()) {
                fenv[name].pop_back();
                if(fenv[name].empty())
                    fenv.erase(name);
            }
            break;
        default:
            break;
        }
        stk.pop_back();
    }
    stk.pop_back();
}

void symbolTable::decType(const S_symbol& sym, baseTy* ty) {
    tenv[sym].push_back(ty);
    stk.push_back({operation::TYPE_DEC, sym});
}

void symbolTable::decVar(const S_symbol& sym, baseTy* ty) {
    venv[sym].push_back(ty);
    stk.push_back({operation::VAR_DEC, sym});
}

void symbolTable::decFunc(const S_symbol& sym, std::list<baseTy*> &args, baseTy* retTy) {
    auto args_ = args;
    args_.push_front(retTy);
    fenv[sym].push_back(std::move(args_));
    stk.push_back({operation::FUN_DEC, sym});
}

baseTy* symbolTable::lookTy(const S_symbol& ty) {
    if(tenv.count(ty))
        return tenv[ty].back();
    return nullptr;
}

baseTy* symbolTable::lookVar(const S_symbol& name) {
    if(venv.count(name))
        return venv[name].back();
    return nullptr;
}

std::pair<baseTy*, std::list<baseTy*>> symbolTable::lookFunc(const S_symbol& name) {
    if(fenv.count(name)) {
        auto list = fenv[name].back();
        baseTy *retType = list.front();
        list.pop_front();
        return {retType, list};
    }
    return {nullptr, {}};
}

bool symbolTable::ExistTy(const S_symbol& ty) {
    return tenv.count(ty);
}