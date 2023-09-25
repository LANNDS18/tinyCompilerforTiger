#include "../include/CodeGenerator.h"
#include <cassert>
#include <utility>
#include <vector>
#include <iostream>


llvm::Value *CodeGenerator::genExp(A_exp *exp) {
    if (!exp)
        return nullptr;
    switch (exp->ty) {
        case A_exp::type::VarExp:
            return genVarExp(dynamic_cast<A_VarExp *>(exp));
        case A_exp::type::NilExp:
            return genNilExp();
        case A_exp::type::IntExp:
            return genIntExp(dynamic_cast<A_IntExp *>(exp));
        case A_exp::type::StringExp:
            return genStringExp(dynamic_cast<A_StringExp *>(exp));
        case A_exp::type::CallExp:
            return genCallExp(dynamic_cast<A_CallExp *>(exp));
        case A_exp::type::OpExp:
            return genOpExp(dynamic_cast<A_OpExp *>(exp));
        case A_exp::type::RecordExp:
            return genRecordExp(dynamic_cast<A_RecordExp *>(exp));
        case A_exp::type::SeqExp:
            return genSeqExp(dynamic_cast<A_SeqExp *>(exp));
        case A_exp::type::AssignExp:
            return genAssignExp(dynamic_cast<A_AssignExp *>(exp));
        case A_exp::type::IfExp:
            return genIfExp(dynamic_cast<A_IfExp *>(exp));
        case A_exp::type::WhileExp:
            return genWhileExp(dynamic_cast<A_WhileExp *>(exp));
        case A_exp::type::LetExp:
            return genLetExp(dynamic_cast<A_LetExp *>(exp));
        case A_exp::type::ForExp:
            return genForExp(dynamic_cast<A_ForExp *>(exp));
        case A_exp::type::ArrayExp:
            return genArrayExp(dynamic_cast<A_ArrayExp *>(exp));
        case A_exp::type::BreakExp:
            return genBreakExp(dynamic_cast<A_BreakExp *>(exp));
    }
    assert(0);
}

llvm::Value *CodeGenerator::genVarExp(A_VarExp *exp) {
    auto ptr = genLeftValue(exp->var);
    return convertRightValue(ptr.first);
}

std::pair<llvm::Value *, A_type *> CodeGenerator::genLeftValue(A_var *var) {
    switch (var->ty) {
        case A_var::type::SIMPLE: {
            auto v = dynamic_cast<A_SimpleVar *>(var);
            auto vdec = decsValue.get(v->varSymbol);
            return {getNamedValue(v->varSymbol), decsType.get(vdec->type)};
        }
        case A_var::type::FIELD: {
            auto v = dynamic_cast<A_FieldVar *>(var);
            auto parent = genLeftValue(v->var);
            llvm::Value *parentValue = convertRightValue(parent.first);
            auto *parentTypeDec = dynamic_cast<A_RecordTy *>(parent.second);
            assert(parentTypeDec && parentTypeDec->ty == A_type::type::RecordTy);
            int idx = getIdxInRecordTy(v->varSymbol, parentTypeDec);
            A_type *fieldTypeDec = getFieldTypeDec(v->varSymbol, parentTypeDec);
            auto fieldPtr = builder.CreateGEP(
                    parentValue->getType()->getPointerElementType(),
                    parentValue,
                    genIndice({0, idx})
            );
            return {fieldPtr, fieldTypeDec};
        }
        case A_var::type::SUBSCRIPT: {
            auto v = dynamic_cast<A_SubscriptVar *>(var);
            auto parent = genLeftValue(v->var);
            llvm::Value *parentValue = convertRightValue(parent.first);
            auto *parentTypeDec = dynamic_cast<A_ArrayTy *>(parent.second);
            assert(parentTypeDec && parentTypeDec->ty == A_type::type::ArrayTy);
            llvm::Value *offset = genExp(v->exp);
            auto elementTyDec = decsType.get(parentTypeDec->array);
            assert(elementTyDec || (parentTypeDec->array == "int") || (parentTypeDec->array == "string"));
            auto elementPtr = builder.CreateGEP(
                    parentValue->getType()->getPointerElementType(),
                    parentValue,
                    builder.CreateTrunc(offset, builder.getInt32Ty())
            );
            return {elementPtr, elementTyDec};
        }
    }
    assert(0);
}

llvm::Value *CodeGenerator::genNilExp() {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(NilTy));
}

llvm::Value *CodeGenerator::genIntExp(A_IntExp *exp) {
    return llvm::ConstantInt::get(context, llvm::APInt(64, exp->i, true));
}

llvm::Value *CodeGenerator::genStringExp(A_StringExp *exp) {
    return getStrConstant(exp->s);
}

llvm::Value *CodeGenerator::genCallExp(A_CallExp *exp) {
    llvm::Function *callee = envFunc.get(exp->func);
    assert(callee != nullptr);
    std::vector<llvm::Value *> args;
    for (auto l = exp->args; l != nullptr && l->head != nullptr; l = l->tail) {
        args.push_back(genExp(l->head));
    }
    return builder.CreateCall(callee, args);
}

llvm::Value *CodeGenerator::genOpExp(A_OpExp *exp) {
    auto lhs = genExp(exp->left);
    auto rhs = genExp(exp->right);
    if (!lhs || !rhs)
        return nullptr;
    switch (exp->aOperator) {
        case A_operator::A_plusOp:
            return builder.CreateAdd(lhs, rhs);
        case A_operator::A_minusOp:
            return builder.CreateSub(lhs, rhs);
        case A_operator::A_timesOp:
            return builder.CreateMul(lhs, rhs);
        case A_operator::A_divideOp:
            return builder.CreateSDiv(lhs, rhs);
        case A_operator::A_eqOp:
            return builder.CreateZExt(builder.CreateICmpEQ(lhs, rhs), builder.getInt64Ty());
        case A_operator::A_neqOp:
            return builder.CreateZExt(builder.CreateICmpNE(lhs, rhs), builder.getInt64Ty());
        case A_operator::A_ltOP:
            return builder.CreateZExt(builder.CreateICmpSLT(lhs, rhs), builder.getInt64Ty());
        case A_operator::A_leOp:
            return builder.CreateZExt(builder.CreateICmpSLE(lhs, rhs), builder.getInt64Ty());
        case A_operator::A_gtOp:
            return builder.CreateZExt(builder.CreateICmpSGT(lhs, rhs), builder.getInt64Ty());
        case A_operator::A_geOp:
            return builder.CreateZExt(builder.CreateICmpSGE(lhs, rhs), builder.getInt64Ty());
    }
    assert(0);
}

int CodeGenerator::getIdxInRecordTy(const std::string &name, A_RecordTy *ty) {
    int idx = 0;
    auto list = ty->record;
    for (; list != nullptr && list->head != nullptr; list = list->tail) {
        if (list->head->name == name)
            break;
        idx++;
    }
    return idx;
}

llvm::Type *CodeGenerator::getFieldType(const std::string &name, A_RecordTy *ty) {
    auto list = ty->record;
    for (; list != nullptr && list->head != nullptr; list = list->tail) {
        if (list->head->name == name)
            return envType.get(list->head->type);
    }
    return nullptr;
}

A_type *CodeGenerator::getFieldTypeDec(const std::string &name, A_RecordTy *ty) {
    auto list = ty->record;
    for (; list != nullptr && list->head != nullptr; list = list->tail) {
        if (list->head->name == name)
            return decsType.get(list->head->type);
    }
    return nullptr;
}


llvm::Value *CodeGenerator::genRecordExp(A_RecordExp *exp) {
    auto type = envType.get(exp->type);
    assert(type != nullptr && type->isPointerTy());
    auto elementType = llvm::cast<llvm::PointerType>(type)->getPointerElementType();
    assert(elementType->isStructTy());
    // auto structType = type;
    auto structType = llvm::cast<llvm::StructType>(elementType);
    auto size = module->getDataLayout().getTypeAllocSize(structType);
    llvm::Value *sz = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), llvm::APInt(64, size));
    std::string allocator{"alloc"};
    llvm::Value *ptr = builder.CreateCall(envFunc.get(allocator), sz);
    ptr = builder.CreateBitCast(ptr, llvm::cast<llvm::PointerType>(type));
    auto list = exp->fields;
    auto tydec = dynamic_cast<A_RecordTy *>(decsType.get(exp->type));
    assert(tydec != nullptr);
    for (; list != nullptr && list->head != nullptr; list = list->tail) {
        auto idx = getIdxInRecordTy(list->head->name, tydec);
        auto initVal = genExp(list->head->exp);
        auto fieldTy = getFieldType(list->head->name, tydec);
        assert(fieldTy != nullptr);
        if (initVal->getType() == NilTy) {
            initVal = convertTypedNil(fieldTy);
        }
        auto fieldPtr = builder.CreateGEP(
                ptr->getType()->getPointerElementType(),
                ptr,
                genIndice({0, idx})
                );
        builder.CreateStore(initVal, fieldPtr);
    }
    return ptr;
}

llvm::Value *CodeGenerator::genSeqExp(A_SeqExp *exp) {
    auto list = exp->seq;
    llvm::Value *res = nullptr;
    while (list != nullptr && list->head != nullptr) {
        res = genExp(list->head);
        list = list->tail;
    }
    return res;
}

llvm::Value *CodeGenerator::genAssignExp(A_AssignExp *exp) {
    assert(exp->var != nullptr);
    auto dst = genLeftValue(exp->var);
    auto val = genExp(exp->exp);
    if (val->getType() == NilTy) {
        // need to deduction type
        if (dst.second == nullptr) { // string type has no dec
            val = convertTypedNil(llvm::PointerType::getUnqual(llvm::Type::getInt8PtrTy(context)));
        }
            // record type
        else if (dst.second->ty == A_type::type::RecordTy) {
            val = convertTypedNil(
                    getFieldType(
                            dynamic_cast<A_FieldVar *>(exp->var)->varSymbol,
                            dynamic_cast<A_RecordTy *>(dst.second)));
        }
            // array type
        else if (dst.second->ty == A_type::type::ArrayTy) {
            val = convertTypedNil(envType.get(dynamic_cast<A_ArrayTy *>(dst.second)->array));
        }
    }
    builder.CreateStore(val, dst.first);
    return nullptr;
}

llvm::Value *CodeGenerator::genIfExp(A_IfExp *exp) {
    llvm::Value *CondV = genExp(exp->test);
    CondV = builder.CreateICmpNE(CondV, builder.getInt64(0));
    assert(CondV != nullptr);

    llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
    BasicBlock *ThenBB = BasicBlock::Create(context, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(context, "else");
    BasicBlock *MergeBB = BasicBlock::Create(context, "ifcond");
    builder.CreateCondBr(CondV, ThenBB, ElseBB);

    // generate then cond
    builder.SetInsertPoint(ThenBB);
    llvm::Value *ThenV = genExp(exp->then);
    builder.CreateBr(MergeBB);
    ThenBB = builder.GetInsertBlock();
    TheFunction->getBasicBlockList().push_back(ElseBB);

    // generate else cond
    builder.SetInsertPoint(ElseBB);
    llvm::Value *ElseV = genExp(exp->elseExp);
    builder.CreateBr(MergeBB);
    ElseBB = builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    builder.SetInsertPoint(MergeBB);

    if (exp->elseExp == nullptr)
        return nullptr;

    if (ThenV == nullptr || ElseV == nullptr)
        return nullptr;

    if (ThenV->getType() == builder.getVoidTy() || ElseV->getType() == builder.getVoidTy()) {
        return nullptr;
    }

    llvm::PHINode *node = builder.CreatePHI(ThenV->getType(), 2);
    node->addIncoming(ThenV, ThenBB);
    node->addIncoming(ElseV, ElseBB);
    return node;
}

llvm::Value *CodeGenerator::genWhileExp(A_WhileExp *exp) {
    llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
    BasicBlock *CondBB = BasicBlock::Create(context, "wcond", TheFunction);
    BasicBlock *WhileBodyBB = BasicBlock::Create(context, "wbody", TheFunction);
    BasicBlock *EndBB = BasicBlock::Create(context, "wend", TheFunction);
    loop_stack.push_back(EndBB);

    builder.CreateBr(CondBB);
    builder.SetInsertPoint(CondBB);
    llvm::Value *cond = genExp(exp->test);
    cond = builder.CreateICmpNE(cond, builder.getInt64(0));
    assert(cond != nullptr);
    builder.CreateCondBr(cond, WhileBodyBB, EndBB);

    builder.SetInsertPoint(WhileBodyBB);
    genExp(exp->body);
    builder.CreateBr(CondBB);

    builder.SetInsertPoint(EndBB);
    loop_stack.pop_back();

    return nullptr;
}

llvm::Value *CodeGenerator::genLetExp(A_LetExp *exp) {
    beginScope();
    for (auto l = exp->decList; l != nullptr; l = l->tail) {
        if (l->head != nullptr)
            genDec(l->head);
    }
    genExp(exp->body);
    endScope();
    return nullptr;
}

llvm::Value *CodeGenerator::genForExp(A_ForExp *exp) {
    beginScope();
    llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
    BasicBlock *InitBB = BasicBlock::Create(context, "finit", TheFunction);
    BasicBlock *CondBB = BasicBlock::Create(context, "fcond", TheFunction);
    BasicBlock *ForBodyBB = BasicBlock::Create(context, "fbody", TheFunction);
    BasicBlock *EndBB = BasicBlock::Create(context, "fend", TheFunction);
    loop_stack.push_back(EndBB);

    // init "i"
    builder.CreateBr(InitBB);
    builder.SetInsertPoint(InitBB);
    llvm::Value *low = genExp(exp->lo);
    assert(low != nullptr);
    createNamedValue(exp->var, low, llvm::Type::getInt64Ty(context));
    decsValue.put(exp->var, new A_VarDec(exp->pos, exp->var, "int", exp->body));
    builder.CreateBr(CondBB);

    builder.SetInsertPoint(CondBB);
    llvm::Value *high = genExp(exp->hi);
    assert(high != nullptr);
    auto namedValue = getNamedValue(exp->var);
    llvm::Value *CondV = builder.CreateICmpSLE(
            builder.CreateLoad(namedValue->getType()->getPointerElementType(),namedValue), high);
    builder.CreateCondBr(CondV, ForBodyBB, EndBB);

    builder.SetInsertPoint(ForBodyBB);
    genExp(exp->body);
    // "i++"

    namedValue = getNamedValue(exp->var);
    builder.CreateStore(
            builder.CreateAdd(
                    builder.CreateLoad(namedValue->getType()->getPointerElementType(),namedValue),
                    llvm::ConstantInt::get(context, llvm::APInt(64, 1, true))),
            getNamedValue(exp->var));
    builder.CreateBr(CondBB);

    builder.SetInsertPoint(EndBB);
    endScope();
    loop_stack.pop_back();
    return nullptr;
}

llvm::Value *CodeGenerator::genArrayExp(A_ArrayExp *exp) {
    auto type = envType.get(exp->type);
    assert(type != nullptr && type->isPointerTy());
    auto arrayLength = genExp(exp->size);
    auto initValue = genExp(exp->init);

    // allocate space for the array, todo: migrate to opaque type
    auto elementType = llvm::cast<llvm::PointerType>(type)->getPointerElementType();
    // std::cout << "debug " << elementType->isSingleValueType() << exp->type << "\n";

    if (initValue->getType() == NilTy) {
        initValue = convertTypedNil(elementType);
    }
    auto size = module->getDataLayout().getTypeAllocSize(elementType);

    llvm::Value *sz = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), llvm::APInt(64, size));
    std::string allocator{"alloc"};
    llvm::Value *ptr = builder.CreateCall(envFunc.get(allocator), builder.CreateMul(arrayLength, sz));

    ptr = builder.CreateBitCast(ptr, type, "array");

    // init the array in a for loop style
    llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
    BasicBlock *InitBB = BasicBlock::Create(context, "finit", TheFunction);
    BasicBlock *CondBB = BasicBlock::Create(context, "fcond", TheFunction);
    BasicBlock *ForBodyBB = BasicBlock::Create(context, "fbody", TheFunction);
    BasicBlock *EndBB = BasicBlock::Create(context, "fend", TheFunction);

    // init "i"
    builder.CreateBr(InitBB);
    builder.SetInsertPoint(InitBB);
    llvm::Value *index = builder.CreateAlloca(builder.getInt32Ty(), nullptr);
    builder.CreateStore(builder.getInt32(0), index);
    builder.CreateBr(CondBB);

    // "i < len"
    builder.SetInsertPoint(CondBB);
    llvm::Value *CondV = builder.CreateICmpSLT(builder.CreateLoad(index->getType()->getPointerElementType(), index),
                                               builder.CreateTrunc(arrayLength, builder.getInt32Ty()));
    builder.CreateCondBr(CondV, ForBodyBB, EndBB);

    // "arr[i] = init_val"
    builder.SetInsertPoint(ForBodyBB);
    auto element = builder.CreateGEP(ptr->getType()->getPointerElementType(), ptr, builder.CreateLoad(index->getType()->getPointerElementType(), index));
    builder.CreateStore(initValue, element);

    // "index++"
    builder.CreateStore(
            builder.CreateAdd(
                    builder.CreateLoad(
                            index->getType()->getPointerElementType(),
                            index
                            ),
                    llvm::ConstantInt::get(context, llvm::APInt(32, 1, true))),
            index);
    builder.CreateBr(CondBB);
    builder.SetInsertPoint(EndBB);
    return ptr;
}


llvm::Value *CodeGenerator::genBreakExp(A_BreakExp *exp) {
    // ignore invalid break
    if (!loop_stack.empty()) {
        builder.CreateBr(loop_stack.back());
        llvm::Function *TheFunction = builder.GetInsertBlock()->getParent();
        BasicBlock *BreakCondBB = BasicBlock::Create(context, "break", TheFunction);
        builder.SetInsertPoint(BreakCondBB);
    }
    return nullptr;
}

llvm::Value *CodeGenerator::getStrConstant(std::string &str) {
    llvm::Constant *strConstant = builder.CreateGlobalStringPtr(str);
    return strConstant;
}

void CodeGenerator::genVarDec(A_VarDec *dec) {
    llvm::Value *initValue = genExp(dec->init);
    createNamedValue(dec->var, initValue, envType.get(dec->type));
    decsValue.put(dec->var, dec);
}

void CodeGenerator::genTypeDec(A_TypeDec *dec) {
    auto l = dec->type;
    // At first, we just focus on record type, because it supports pre declare
    for (; l != nullptr && l->head != nullptr; l = l->tail) {
        auto cur = l->head;
        if (cur->ty->ty == A_type::type::RecordTy) {
            envType.put(cur->name, llvm::PointerType::getUnqual(llvm::StructType::create(context, cur->name)));
            decsType.put(cur->name, cur->ty);
        }
    }

    // Then, we gen array types, which cannot be pre declared
    for (l = dec->type; l != nullptr && l->head != nullptr; l = l->tail) {
        auto cur = l->head;
        if (cur->ty->ty == A_type::type::ArrayTy) {
            auto t = dynamic_cast<A_ArrayTy *>(cur->ty);
            auto elementType = envType.get(t->array);
            auto pointerType = llvm::PointerType::getUnqual(elementType);
            envType.put(cur->name, pointerType);
            decsType.put(cur->name, cur->ty);
        }
    }

    // In the end, we set body of record type.
    for (l = dec->type; l != nullptr && l->head != nullptr; l = l->tail) {
        auto cur = l->head;
        if (cur->ty->ty == A_type::type::RecordTy) {
            auto t = dynamic_cast<A_RecordTy *>(cur->ty);
            std::vector<llvm::Type *> fields;
            for (auto _l = t->record; _l != nullptr; _l = _l->tail) {
                if (_l->head == nullptr)
                    break;
                if (_l->head->type == "int")
                    fields.push_back(llvm::Type::getInt64Ty(context));
                else if (_l->head->type == "string")
                    fields.push_back(llvm::Type::getInt8PtrTy(context));
                else
                    fields.push_back(envType.get(_l->head->type));
            }
            auto structPointerType = llvm::cast<llvm::PointerType>(envType.get(cur->name));
            assert(structPointerType != nullptr && structPointerType->isPointerTy());
            auto structType = llvm::cast<llvm::StructType>(structPointerType->getPointerElementType());
            assert(structType != nullptr && structType->isStructTy());
            structType->setBody(fields);
        }
    }
}

void CodeGenerator::genFuncDec(A_FunctionDec *dec) {
    auto l = dec->function;
    for (; l != nullptr && l->head != nullptr; l = l->tail) {
        auto cur = l->head;
        llvm::Type *retTy;
        if (cur->result.length() == 0)
            retTy = llvm::Type::getVoidTy(context);
        else
            retTy = envType.get(cur->result);
        assert(retTy != nullptr);

        std::vector<llvm::Type *> paramTys;
        for (auto params = cur->params; params != nullptr && params->head != nullptr; params = params->tail) {
            llvm::Type *t = envType.get(params->head->type);
            assert(t != nullptr);
            paramTys.push_back(t);
        }
        auto functionType = llvm::FunctionType::get(retTy, paramTys, false);
        auto func = llvm::Function::Create(functionType, llvm::Function::InternalLinkage, cur->name, module.get());
        envFunc.put(cur->name, func);
    }

    for (l = dec->function; l != nullptr && l->head != nullptr; l = l->tail) {
        auto cur = l->head;
        beginScope();
        llvm::Function *TheFunction = envFunc.get(cur->name);
        assert(TheFunction != nullptr);
        BasicBlock *Body = BasicBlock::Create(context, "entry", TheFunction);
        auto originalPoint = builder.GetInsertPoint();
        auto originalBlock = builder.GetInsertBlock();
        builder.SetInsertPoint(Body);

        auto params = cur->params;
        for (auto &Arg: TheFunction->args()) {
            assert(params && params->head);
            createNamedValue(params->head->name, &Arg, envType.get(params->head->type));
            decsValue.put(params->head->name,
                          new A_VarDec(params->head->pos, params->head->name, params->head->type, nullptr));
            params = params->tail;
        }
        llvm::Value *retVal = genExp(cur->body);
        if (TheFunction->getFunctionType()->getReturnType() != llvm::Type::getVoidTy(context))
            builder.CreateRet(retVal);
        else builder.CreateRet(nullptr);
        // if(verifyFunction(*TheFunction, &llvm::outs())) {
        //     error("Generator: Fail to generate function " + cur->name);
        // }
        endScope();
        builder.SetInsertPoint(originalBlock, originalPoint);
    }
}

void CodeGenerator::genDec(A_dec *dec) {
    switch (dec->ty) {
        case A_dec::type::VARDEC:
            genVarDec(dynamic_cast<A_VarDec *>(dec));
            break;
        case A_dec::type::FUNCDEC:
            genFuncDec(dynamic_cast<A_FunctionDec *>(dec));
            break;
        case A_dec::type::TYDEC:
            genTypeDec(dynamic_cast<A_TypeDec *>(dec));
            break;
    }
}

llvm::Function *
CodeGenerator::createIntrinsicFunction(const std::string &name, std::vector<llvm::Type *> const &arg_tys,
                                       llvm::Type *ret_ty) {
    auto functionType = llvm::FunctionType::get(ret_ty, arg_tys, false);
    auto func = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module.get());
    return func;
}

llvm::Value *CodeGenerator::convertTypedNil(llvm::Type *type) {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(type));
}

void CodeGenerator::beginScope() {
    envType.begin();
    envValue.begin();
    envFunc.begin();
    decsType.begin();
    decsValue.begin();
}

void CodeGenerator::endScope() {
    envType.pop();
    envValue.pop();
    envFunc.pop();
    decsType.pop();
    decsValue.pop();
}

void CodeGenerator::createNamedValue(std::string name, llvm::Value *value, llvm::Type *type) {
    if (value->getType() == NilTy) {
        value = convertTypedNil(type);
    }
    llvm::Value *namedValue = builder.CreateAlloca(value->getType());
    builder.CreateStore(value, namedValue);
    envValue.put(std::move(name), namedValue);
}

llvm::Value *CodeGenerator::getNamedValue(std::string name) {
    return envValue.get(name);
}

llvm::Value *CodeGenerator::convertRightValue(llvm::Value *leftValue) {
    return builder.CreateLoad(
            leftValue->getType()->getPointerElementType(),
            leftValue
            );
}

void CodeGenerator::initFenv() {
    llvm::Type *intType{llvm::Type::getInt64Ty(context)};
    llvm::Type *voidType{llvm::Type::getVoidTy(context)};
    llvm::Type *stringType{llvm::Type::getInt8PtrTy(context)};

    envFunc.put("print", createIntrinsicFunction("__print__", {stringType}, voidType));
    envFunc.put("puti", createIntrinsicFunction("__puti__", {intType}, voidType));
    envFunc.put("flush", createIntrinsicFunction("__flush__", {}, voidType));
    envFunc.put("getchar", createIntrinsicFunction("__getchar__", {}, stringType));
    envFunc.put("getint", createIntrinsicFunction("__getint__", {}, intType));
    envFunc.put("ord", createIntrinsicFunction("__ord__", {stringType}, intType));
    envFunc.put("size", createIntrinsicFunction("__size__", {stringType}, intType));
    envFunc.put("substring", createIntrinsicFunction("__substring__", {stringType, intType, intType}, stringType));
    envFunc.put("concat", createIntrinsicFunction("__concat__", {stringType, stringType}, stringType));
    envFunc.put("not", createIntrinsicFunction("__not__", {intType}, intType));
    envFunc.put("exit", createIntrinsicFunction("__exit__", {intType}, voidType));
    envFunc.put("alloc", createIntrinsicFunction("alloc", {intType}, stringType));
    envFunc.put("gets", createIntrinsicFunction("__gets__", {}, stringType));
    envFunc.put("chr", createIntrinsicFunction("alloc", {intType}, stringType));
}

void CodeGenerator::generate(A_exp *syntax_tree, const std::string &filename, int task) {
    auto mainFunctionType = llvm::FunctionType::get(llvm::Type::getInt64Ty(context), false);
    auto mainFunction =
            llvm::Function::Create(mainFunctionType, llvm::GlobalValue::ExternalLinkage,
                                   "main", module.get());
    auto block = BasicBlock::Create(context, "entry", mainFunction);
    builder.SetInsertPoint(block);
    genExp(syntax_tree);
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), llvm::APInt(64, 0)));
    std::cout << "success to build, writing......\n";
    std::error_code EC;
    llvm::raw_fd_ostream OS{filename, EC};
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return;
    }
    if (task == 1)
        module->print(OS, nullptr);
    else
        llvm::WriteBitcodeToFile(*module, OS);
    std::cout << "Done.\n";
}

std::vector<llvm::Value *> CodeGenerator::genIndice(const std::vector<int> &ids) {
    std::vector<llvm::Value *> ret;
    ret.reserve(ids.size());
    for (auto id: ids)
        ret.push_back(builder.getInt32(id));
    return ret;
}