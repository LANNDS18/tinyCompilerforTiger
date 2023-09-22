#pragma once

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#include "abstract.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>

template<typename T>
class tbl {
private:
    std::unordered_map<std::string, std::vector<T *>> store;
    std::vector<std::string> q;
public:
    void put(std::string key, T *value) {
        assert(!key.empty());
        store[key].push_back(value);
        q.push_back(key);
    };

    T *get(std::string &key) {
        if (store.count(key))
            return store[key].back();
        return nullptr;
    };

    void pop() {
        while (!q.empty() && !q.back().empty()) {
            store[q.back()].pop_back();
            if (store[q.back()].empty())
                store.erase(q.back());
            q.pop_back();
        }
        if (!q.empty())
            q.pop_back();
    };

    void begin() { q.emplace_back(""); };
};


using Type = llvm::Type;
using Function = llvm::Function;
using BasicBlock = llvm::BasicBlock;

class CodeGenerator {
private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::vector<BasicBlock *> loop_stack;

    tbl<llvm::Value> venv;
    tbl<Type> tenv;
    tbl<A_type> tdecs;
    tbl<A_VarDec> vdecs;
    tbl<Function> fenv;

    llvm::Value *genExp(A_exp *exp);

    llvm::Value *genVarExp(A_VarExp *exp);

    llvm::Value *genNilExp();

    llvm::Value *genIntExp(A_IntExp *exp);

    llvm::Value *genStringExp(A_StringExp *exp);

    llvm::Value *genCallExp(A_CallExp *exp);

    llvm::Value *genOpExp(A_OpExp *exp);

    llvm::Value *genRecordExp(A_RecordExp *exp);

    llvm::Value *genSeqExp(A_SeqExp *exp);

    llvm::Value *genAssignExp(A_AssignExp *exp);

    llvm::Value *genIfExp(A_IfExp *exp);

    llvm::Value *genWhileExp(A_WhileExp *exp);

    llvm::Value *genLetExp(A_LetExp *exp);

    llvm::Value *genForExp(A_ForExp *exp);

    llvm::Value *genArrayExp(A_ArrayExp *exp);

    llvm::Value *genBreakExp(A_BreakExp *exp);

    void genVarDec(A_VarDec *dec);

    void genTypeDec(A_TypeDec *dec);

    void genFuncDec(A_FunctionDec *dec);

    void genDec(A_dec *dec);

    // aux funcs
    void beginScope();

    void endScope();

    void initFenv();

    llvm::Value *getStrConstant(std::string &str);

    void createNamedValue(std::string name, llvm::Value *value, Type *type);

    llvm::Value *getNamedValue(std::string name);

    static int getIdxInRecordTy(const std::string &name, A_RecordTy *ty);

    Type *getFieldType(const std::string &name, A_RecordTy *ty);

    A_type *getFieldTypeDec(const std::string &name, A_RecordTy *ty);

    static llvm::Value *convertTypedNil(Type *type);

    llvm::Value *convertRightValue(llvm::Value *leftValue);

    Function *createIntrinsicFunction(const std::string &name, std::vector<Type *> const &arg_tys, Type *ret_ty);

    std::pair<llvm::Value *, A_type *> genLeftValue(A_var *vare);

    Type *NilTy = llvm::PointerType::getUnqual(Type::getVoidTy(context));

    std::vector<llvm::Value *> genIndice(const std::vector<int> &ids);

public:
    CodeGenerator() : builder(context), module(new llvm::Module("The Module", context)) {
        initFenv();
        tenv.put("int", Type::getInt64Ty(context));
        tenv.put("string", llvm::Type::getInt8PtrTy(context));
        tenv.put("void", Type::getVoidTy(context));
    };

    void generate(A_exp *syntax_tree, const std::string &filename, int task);
};