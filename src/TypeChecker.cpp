#include "../include/TypeChecker.h"
#include <unordered_set>
#include <cassert>

void TypeChecker::error(int line, const std::string& errmsg) {
    std::cerr << "in line " << line << ":" << std::endl;
    std::cerr << "semantic error: " << errmsg << std::endl;
    exit(1);
}

void TypeChecker::check() {
    checkExp(ast);
}

std::string convertString(std::string s) {
    std::string convertedStr;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == '\\') {
            switch (s[i + 1]) {
                case 'n':
                    convertedStr += '\n';
                    i++;
                    break;
                case 'r':
                    convertedStr += '\r';
                    i++;
                    break;
                case 't':
                    convertedStr += '\t';
                    i++;
                    break;
                case '\\':
                    convertedStr += '\\';
                    i++;
                    break;
                default:
                    convertedStr += s[i];
                    break;
            }
        } else {
            convertedStr += s[i];
        }
    }
    return convertedStr;
}

BaseTy *TypeChecker::checkExp(A_exp *exp) {
    if (!exp) return nullptr;
    using expty = A_exp::type;
    switch (exp->ty) {
        case expty::VarExp: {
            auto e = dynamic_cast<A_VarExp *>(exp);
            return checkVar(e->var);
        }

        case expty::NilExp:
            return symbolTable.lookTy("nil");

        case expty::IntExp:
            return symbolTable.lookTy("int");

        case expty::StringExp: {
            auto e = dynamic_cast<A_StringExp *>(exp);
            e->s = convertString(e->s);
        }
            return symbolTable.lookTy("string");

        case expty::CallExp: {
            auto e = dynamic_cast<A_CallExp *>(exp);
            auto funcTy = symbolTable.lookFunc(e->func);
            if (funcTy.first == nullptr)
                error(e->pos, "there is no func named " + e->func);
            auto res = funcTy.first;
            auto list = e->args;
            for (auto argTy: funcTy.second) {
                // args less than declared
                if (!list) error(e->pos, "args num mismatch");
                if (argTy != checkExp(list->head))
                    error(e->pos, "args type mismatch");
                list = list->tail;
            }
            // args more than declared
            if (list != nullptr) error(e->pos, "args num mismatch");
            return res;
        }

        case expty::OpExp: {
            auto e = dynamic_cast<A_OpExp *>(exp);
            auto l_ty = checkExp(e->left);
            auto r_ty = checkExp(e->right);
            if (l_ty == nullptr || r_ty == nullptr)
                error(e->pos, "Fail to judge type in left or right or both");
            switch (e->aOperator) {
                case A_operator::A_eqOp:
                case A_operator::A_neqOp:
                    if (l_ty == r_ty || l_ty->ty == TIG_DTYPE::NIL && r_ty->ty == TIG_DTYPE::RECORD ||
                        r_ty->ty == TIG_DTYPE::NIL && l_ty->ty == TIG_DTYPE::RECORD);
                    else error(e->pos, "invalid types for compare operation");
                    break;
                case A_operator::A_geOp:
                case A_operator::A_gtOp:
                case A_operator::A_leOp:
                case A_operator::A_ltOP:
                    if (l_ty->ty == TIG_DTYPE::INT && r_ty->ty == TIG_DTYPE::INT ||
                        l_ty->ty == TIG_DTYPE::STRING && r_ty->ty == TIG_DTYPE::STRING);
                    else error(e->pos, "invalid types for compare operation");
                case A_operator::A_plusOp:
                case A_operator::A_minusOp:
                case A_operator::A_timesOp:
                case A_operator::A_divideOp:
                    if (l_ty->ty == TIG_DTYPE::INT && r_ty->ty == TIG_DTYPE::INT);
                    else error(e->pos, "invalid types for int operation");
                default:
                    break;
            }
        }
            return symbolTable.lookTy("int");

        case expty::RecordExp: {
            auto e = dynamic_cast<A_RecordExp *>(exp);
            auto rty = symbolTable.lookTy(e->type);
            if (!rty || rty->ty != TIG_DTYPE::RECORD)
                error(e->pos, e->type + "is not record type");
            auto &typefields = dynamic_cast<RecordTy *>(rty)->fields;
            auto list = e->fields;
            while (list != nullptr) {
                if (list->head != nullptr) {
                    if (!typefields.count(list->head->name))
                        error(e->pos, e->type + " has no fields named " + list->head->name);
                    auto initTy = checkExp(list->head->exp);
                    if (!(initTy->ty == TIG_DTYPE::NIL && typefields[list->head->name]->ty == TIG_DTYPE::RECORD)
                        && initTy != typefields[list->head->name])
                        error(e->pos, "field and parse_expression type mismatch");
                }
                list = list->tail;
            }
            return rty;
        }

        case expty::SeqExp: {
            auto list = dynamic_cast<A_SeqExp *>(exp)->seq;
            BaseTy *res = nullptr;
            while (list) {
                res = checkExp(list->head);
                list = list->tail;
            }
            return res;
        }

        case expty::AssignExp: {
            auto e = dynamic_cast<A_AssignExp *>(exp);
            auto varTy = checkVar(e->var);
            auto expTy = checkExp(e->exp);
            if (varTy != expTy && !(varTy->ty == TIG_DTYPE::RECORD && expTy->ty == TIG_DTYPE::NIL))
                error(e->pos, "The types at left & right end of the assignment expression do not match");
        }
            return symbolTable.lookTy("void");

        case expty::IfExp: {
            auto e = dynamic_cast<A_IfExp *>(exp);
            auto condition_ty = checkExp(e->test);
            if (condition_ty == nullptr || condition_ty->ty != TIG_DTYPE::INT)
                error(e->pos, "The Doc condition for the if statement must be of type int");
            auto then_ty = checkExp(e->then);
            auto else_ty = checkExp(e->elsee);
            if (else_ty == nullptr) return symbolTable.lookTy("void");
            if (then_ty != else_ty && !(then_ty->ty == TIG_DTYPE::NIL && else_ty->ty == TIG_DTYPE::RECORD ||
                                        then_ty->ty == TIG_DTYPE::RECORD && else_ty->ty == TIG_DTYPE::NIL))
                error(e->pos, "The two branches in the if statement are of inconsistent types");
            return then_ty;
        }

        case expty::WhileExp: {
            auto e = dynamic_cast<A_WhileExp *>(exp);
            auto con_ty = checkExp(e->test);
            if (con_ty == nullptr || con_ty->ty != TIG_DTYPE::INT)
                error(e->pos, "The Doc condition for the while statement must be of type int");
            checkExp(e->body);
        }
            return symbolTable.lookTy("void");

        case expty::LetExp: {
            auto e = dynamic_cast<A_LetExp *>(exp);
            auto list = e->decs;
            symbolTable.beginScope();
            while (list) {
                checkDec(list->head);
                list = list->tail;
            }
            auto res = checkExp(e->body);
            symbolTable.endScope();
            if (res != nullptr)
                return res;
            else return symbolTable.lookTy("void");
        }
        case expty::ForExp: {
            auto e = dynamic_cast<A_ForExp *>(exp);
            symbolTable.beginScope();
            symbolTable.decVar(e->var, symbolTable.lookTy("int"));
            auto lo_ty = checkExp(e->lo);
            auto hi_ty = checkExp(e->hi);
            if (lo_ty == nullptr || lo_ty->ty != TIG_DTYPE::INT || hi_ty == nullptr || hi_ty->ty != TIG_DTYPE::INT)
                error(e->pos, "The traversal range of the for statement should be of type int");
            checkExp(e->body);
            symbolTable.endScope();
        }
            return symbolTable.lookTy("void");
        case expty::ArrayExp: {
            auto e = dynamic_cast<A_ArrayExp *>(exp);
            auto arr_ty = symbolTable.lookTy(e->type);
            if (arr_ty == nullptr)
                error(e->pos, "there is no type named " + e->type);
            if (arr_ty->ty != TIG_DTYPE::ARRAY)
                error(e->pos, e->type + "is not array type");
            auto arr_ty_ = dynamic_cast<ArrayTy *>(arr_ty);
            auto sz_ty = checkExp(e->size);
            if (sz_ty == nullptr || sz_ty->ty != TIG_DTYPE::INT)
                error(e->pos, "Expr in [] are expected to be of type int");
            auto init_ty = checkExp(e->init);
            if (symbolTable.lookTy(arr_ty_->element_type) != init_ty && init_ty != symbolTable.lookTy("nil"))
                error(e->pos, "Initialization expr type mismatch ");
            return arr_ty;
        }
        case expty::BreakExp:
            return symbolTable.lookTy("void");
    }
    assert(0);
}

BaseTy *TypeChecker::checkVar(A_var *var) {
    assert(var != nullptr);
    using t = A_var::type;
    switch (var->ty) {
        case t::SIMPLE: {
            auto v = dynamic_cast<A_SimpleVar *>(var);
            auto res = symbolTable.lookVar(v->sym);
            if (res == nullptr)
                error(v->pos, "undeclared var " + v->sym);
            return res;
        }
        case t::FIELD: {
            auto v = dynamic_cast<A_FieldVar *>(var);
            auto parTy = checkVar(v->var);
            if (!parTy || parTy->ty != TIG_DTYPE::RECORD)
                error(v->pos, "parent type not exist or is not record type in field var");
            auto par = dynamic_cast<RecordTy *>(parTy);
            if (!par->fields.count(v->sym))
                error(v->pos, "parent type has no field named " + v->sym);
            return par->fields[v->sym];
        }
        case t::SUBSCRIPT: {
            auto v = dynamic_cast<A_SubscriptVar *>(var);
            auto parTy = checkVar(v->var);
            if (!parTy || parTy->ty != TIG_DTYPE::ARRAY)
                error(v->pos, "parent type not exist or is not array type in subscript var");
            auto expTy = checkExp(v->exp);
            if (!expTy || expTy->ty != TIG_DTYPE::INT)
                error(v->pos, "Expr in [] are expected to be of type int");
            return symbolTable.lookTy(dynamic_cast<ArrayTy *>(parTy)->element_type);
        }
    }
    assert(0);
}

void TypeChecker::checkDec(A_dec *dec) {
    assert(dec != nullptr);
    using declarationTypeEnum = A_dec::type;
    switch (dec->ty) {
        case declarationTypeEnum::VARDEC: {
            auto d = dynamic_cast<A_VarDec *>(dec);
            if (d->type.empty()) {
                auto ty = checkExp(d->init);
                if (ty == nullptr || ty->ty == TIG_DTYPE::VOID)
                    error(d->pos, "invalid type declaration.");
                if (ty->ty == TIG_DTYPE::NIL)
                    error(d->pos, "initializing nil expressions not constrained by record type");
                symbolTable.decVar(d->var, ty);
                d->type = ty->name;
            } else {
                auto expected_ty = symbolTable.lookTy(d->type);
                auto init_ty = checkExp(d->init);
                if (!expected_ty || !init_ty ||
                    expected_ty != init_ty && !(expected_ty->ty == TIG_DTYPE::RECORD && init_ty->ty == TIG_DTYPE::NIL)) {
                    error(d->pos, "types are not exist or mismatch in var declaration");
                }
                symbolTable.decVar(d->var, expected_ty);
                // this will erase all the named types, the CodeGenerator can just ignore named type declarations.
                d->type = expected_ty->name;
            }
        }
            break;
        case declarationTypeEnum::TYDEC: {
            auto d = dynamic_cast<A_TypeDec *>(dec);
            auto list = d->type;
            std::unordered_set<S_symbol> ty_names;
            std::unordered_map<S_symbol, A_NameTy *> nameTys;  // store named types
            for (; list != nullptr; list = list->tail) {
                auto cur = list->head;
                if (ty_names.count(cur->name))
                    error(list->head->ty->pos, "multiple definition on type " + cur->name);
                ty_names.insert(cur->name);
                switch (cur->ty->ty) {
                    case A_type::type::ArrayTy:
                        symbolTable.decType(cur->name, new ArrayTy("", cur->name));
                        break;
                    case A_type::type::NameTy:
                        nameTys[cur->name] = dynamic_cast<A_NameTy *>(cur->ty);
                        break;
                    case A_type::type::RecordTy: {
                        std::unordered_map<S_symbol, BaseTy *> fields;
                        symbolTable.decType(cur->name, new RecordTy(fields, cur->name));
                    }
                        break;
                }
            }
            // add defination for named type
            for (auto [name, ty]: nameTys) {
                S_symbol parent_name = ty->type;
                std::unordered_set<S_symbol> path;
                path.insert(name);
                while (true) {
                    if (path.count(parent_name))
                        error(ty->pos, "loop recursive type definition on " + parent_name);
                    path.insert(parent_name);
                    if (symbolTable.ExistTy(parent_name)) {
                        symbolTable.decType(name, symbolTable.lookTy(parent_name));
                        break;
                    }
                    if (nameTys.count(parent_name))
                        parent_name = nameTys[parent_name]->type;
                    else error(ty->pos, "there is no type named " + parent_name);
                }
            }
            // add internal defination for array type & record type
            list = d->type;
            for (; list != nullptr; list = list->tail) {
                auto cur = list->head;
                switch (cur->ty->ty) {
                    case A_type::type::ArrayTy: {
                        auto arrTy = dynamic_cast<A_ArrayTy *>(cur->ty);
                        auto eleTy = symbolTable.lookTy(dynamic_cast<A_ArrayTy *>(cur->ty)->array);
                        if (!symbolTable.lookTy(arrTy->array))
                            error(cur->ty->pos, "there is no type named " + arrTy->array);
                        auto ty = dynamic_cast<ArrayTy *>(symbolTable.lookTy(cur->name));
                        assert(ty != nullptr);
                        // erase named types
                        arrTy->array = eleTy->name;
                        ty->element_type = eleTy->name;
                    }
                        break;
                    case A_type::type::RecordTy: {
                        auto fieldList = dynamic_cast<A_RecordTy *>(cur->ty)->record;
                        auto ty = dynamic_cast<RecordTy *>(symbolTable.lookTy(cur->name));
                        assert(ty != nullptr);
                        for (; fieldList != nullptr; fieldList = fieldList->tail) {
                            auto fieldTy = symbolTable.lookTy(fieldList->head->type);
                            if (fieldTy == nullptr)
                                error(cur->ty->pos, "there is no type named " + fieldList->head->type);
                            ty->fields[fieldList->head->name] = fieldTy;
                            // erase named type
                            fieldList->head->type = fieldTy->name;
                        }
                    }
                        break;
                    case A_type::type::NameTy:
                        break;
                }
            }
        }
            break;
        case declarationTypeEnum::FUNCDEC: {
            auto d = dynamic_cast<A_FunctionDec *>(dec);
            // first, add func declaration to environment
            auto list = d->function;
            std::unordered_set<S_symbol> func_names;
            for (; list != nullptr; list = list->tail) {
                if (func_names.count(list->head->name))
                    error(list->head->pos, "multiple defination of function " + list->head->name);
                func_names.insert(list->head->name);
                auto cur = list->head;
                BaseTy *resTy = symbolTable.lookTy("void");
                assert(resTy != nullptr);
                if (cur->result.length() != 0) {
                    resTy = symbolTable.lookTy(cur->result);
                    if (resTy == nullptr)
                        error(cur->pos, "there is no type named " + cur->result);
                    cur->result = resTy->name;
                } else cur->result = "void";
                std::list<BaseTy *> argTys;
                for (auto argnode = cur->params;
                     argnode != nullptr && argnode->head != nullptr; argnode = argnode->tail) {
                    auto argTy = symbolTable.lookTy(argnode->head->type);
                    if (argTy == nullptr)
                        error(cur->pos, "there is no type named " + argnode->head->type);
                    // erase named types.
                    argnode->head->type = argTy->name;
                    argTys.push_back(argTy);
                }
                symbolTable.decFunc(cur->name, argTys, resTy);
            }
            // then, check the function bodys.
            list = d->function;
            for (; list != nullptr; list = list->tail) {
                auto cur = list->head;
                symbolTable.beginScope();
                for (auto argnode = cur->params;
                     argnode != nullptr && argnode->head != nullptr; argnode = argnode->tail) {
                    symbolTable.decVar(argnode->head->name, symbolTable.lookTy(argnode->head->type));
                }
                checkExp(cur->body);
                symbolTable.endScope();
            }
        }
            break;
        default:
            break;
    }
}