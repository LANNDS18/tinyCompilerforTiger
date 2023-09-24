#pragma once

#include "abstract.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/BitcodeWriter.h>


template<typename T>
class table {
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

using BasicBlock = llvm::BasicBlock;

class CodeGenerator {
private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::vector<BasicBlock *> loop_stack;

    table<llvm::Value> envValue;
    table<llvm::Type> envType;
    table<llvm::Function> envFunc;

    table<A_type> decsType;
    table<A_VarDec> decsValue;

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

    void beginScope();

    void endScope();

    void initFenv();

    llvm::Value *getStrConstant(std::string &str);

    void createNamedValue(std::string name, llvm::Value *value, llvm::Type *type);

    llvm::Value *getNamedValue(std::string name);

    static int getIdxInRecordTy(const std::string &name, A_RecordTy *ty);

    llvm::Type *getFieldType(const std::string &name, A_RecordTy *ty);

    A_type *getFieldTypeDec(const std::string &name, A_RecordTy *ty);

    static llvm::Value *convertTypedNil(llvm::Type *type);

    llvm::Value *convertRightValue(llvm::Value *leftValue);

    llvm::Function *
    createIntrinsicFunction(const std::string &name, std::vector<llvm::Type *> const &arg_tys, llvm::Type *ret_ty);

    std::pair<llvm::Value *, A_type *> genLeftValue(A_var *vare);

    llvm::Type *NilTy = llvm::PointerType::getUnqual(llvm::Type::getVoidTy(context));

    std::vector<llvm::Value *> genIndice(const std::vector<int> &ids);

public:
    CodeGenerator() : builder(context), module(new llvm::Module("The Module", context)) {
        initFenv();
        envType.put("int", llvm::Type::getInt64Ty(context));
        envType.put("string", llvm::Type::getInt8PtrTy(context));
        envType.put("void", llvm::Type::getVoidTy(context));
    };

    void generate(A_exp *syntax_tree, const std::string &filename, int task);
};