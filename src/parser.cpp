#include "../include/parser.h"
#include <iostream>
#include <vector>


namespace FRONTEND {


    token parser::popToken() {
        if (q.empty())
            return lex.next();
        auto tk = q.front();
        q.pop();
        return tk;
    }

    token parser::checkNextToken(int token_ty) {
        auto tk = popToken();
        if (tk.type != token_ty) {
            token exp(token_ty, -1);
            throw std::runtime_error(
                    "in line " + std::to_string(tk.line) + ": parser: expected " + exp.to_str() + " but got " +
                    tk.to_str());
        }
        return tk;
    }

    A_dec *parser::declaration() {
        auto t = popToken();
        switch (t.type) {
            case VAR : {
                auto id = checkNextToken(IDENTIFIER);
                S_symbol type;
                token _t = popToken();
                if (_t.type == COLON) {
                    type = checkNextToken(IDENTIFIER).val;
                    checkNextToken(ASSIGN);
                } else if (_t.type != ASSIGN) {
                    std::cerr << "in line " << _t.line << ":" << std::endl;
                    std::cerr << "parser: expected COLON or ASSIGN, but got " << t.to_str() << std::endl;
                    exit(1);
                }
                auto e = parse_expression();
                return new A_VarDec(_t.line, id.val, type, e);
            }
            case TYPE : {
                token id = checkNextToken(IDENTIFIER);
                checkNextToken(EQ);
                auto type = ty();
                auto *type_(new A_TyDeclareName(id.val, type));
                auto list = new A_TyDeclareNameList(type_, nullptr);
                return new A_TypeDec(id.line, list);
            }
            case FUNCTION: {
                token func_name = checkNextToken(IDENTIFIER);
                checkNextToken(L_SMALL);
                auto list = fieldList();
                checkNextToken(R_SMALL);
                token next = popToken();
                if (next.type == EQ) {
                    auto e = parse_expression();
                    auto func = new A_funcDec(t.line, func_name.val, list, "", e);
                    auto _list = new A_funcDecList(func, nullptr);
                    return new A_FunctionDec(func_name.line, _list);
                } else if (next.type == COLON) {
                    auto id = checkNextToken(IDENTIFIER);
                    checkNextToken(EQ);
                    auto e = parse_expression();
                    auto func = new A_funcDec(t.line, func_name.val, list, id.val, e);
                    auto _list = new A_funcDecList(func, nullptr);
                    return new A_FunctionDec(func_name.line, _list);
                } else {
                    std::cerr << "in line " << next.line << ":" << std::endl;
                    std::cerr << "parser: expected EQ or COLON, but got " << next.to_str() << std::endl;
                    exit(1);
                }
            }
            default:
                pushTokenBack(t);
                break;
        }
        return nullptr;
    }

    A_exp *parser::parse_expression() {
        auto t = popToken();
        switch (t.type) {
            case LET: {
                auto ds = declarationList();
                checkNextToken(IN);
                auto es = expList();
                checkNextToken(END);
                return new A_LetExp(t.line, ds, es);
            }
            case WHILE: {
                auto cond = parse_expression();
                checkNextToken(DO);
                auto body = parse_expression();
                return new A_WhileExp(t.line, cond, body);
            }
            case FOR: {
                auto ite = checkNextToken(IDENTIFIER);
                checkNextToken(ASSIGN);
                auto start = parse_expression();
                checkNextToken(TO);
                auto end = parse_expression();
                checkNextToken(DO);
                auto body = parse_expression();
                return new A_ForExp(t.line, ite.val, start, end, body);
            }
            case IF: {
                auto cond = parse_expression();
                checkNextToken(THEN);
                auto trueBranch = parse_expression();
                token isElse = popToken();
                if (isElse.type != ELSE) {
                    pushTokenBack(isElse);
                    return new A_IfExp(t.line, cond, trueBranch, nullptr);
                }
                auto falseBranch = parse_expression();
                return new A_IfExp(t.line, cond, trueBranch, falseBranch);
            }
            case BREAK:
                return new A_BreakExp(t.line);
            case L_SMALL: {
                auto seq = expList();
                checkNextToken(R_SMALL);
                return seq;
            }
            default:
                pushTokenBack(t);
                return assignmentExp();
        }
    }


    A_exp *parser::assignmentExp() {
        auto o = orExp();
        auto a = assignmentExpHelper();
        if (a == nullptr)
            return o;
        if (o->ty != A_exp::type::VarExp) {
            std::cerr << "in line " << a->pos << std::endl;
            std::cerr << "parser: left side can not be right value in assign parse_expression." << std::endl;
            exit(1);
        }
        auto varExp = dynamic_cast<A_VarExp *>(o);
        return new A_AssignExp(varExp->pos, varExp->var, a);
    }

    A_exp *parser::assignmentExpHelper() {
        auto t = popToken();
        if (t.type != ASSIGN) {
            pushTokenBack(t);
            return nullptr;
        }
        return parse_expression();
    }

    A_exp *parser::orExp() {
        auto a_exp = andExp();
        return orExpHelper(a_exp);
    }

    A_exp *parser::orExpHelper(A_exp *parent) {
        token expected_or = popToken();
        if (expected_or.type != OR) {
            pushTokenBack(expected_or);
            return parent;
        }
        A_exp *a = andExp();
        auto one_exp = new A_IntExp(a->pos, 1);
        A_exp *cur = new A_IfExp(a->pos, parent, one_exp, a);
        return orExpHelper(cur);
    }

    A_exp *parser::andExp() {
        auto r = relationExp();
        return andExpHelper(r);
    }

    A_exp *parser::andExpHelper(A_exp *parent) {
        token expected_and = popToken();
        if (expected_and.type != AND) {
            pushTokenBack(expected_and);
            return parent;
        }
        auto zero_exp = new A_IntExp(parent->pos, 0);
        auto r = relationExp();
        A_exp *cur = new A_IfExp(r->pos, parent, r, zero_exp);
        return andExpHelper(cur);
    }

    A_exp *parser::relationExp() {
        auto a = addExp();
        return relationExpHelper(a);
    }

    A_exp *parser::relationExpHelper(A_exp *parent) {
        token t = popToken();
        A_operator op;
        switch (t.type) {
            case EQ:
                op = A_operator::A_eqOp;
                break;
            case NEQ:
                op = A_operator::A_neqOp;
                break;
            case LE:
                op = A_operator::A_leOp;
                break;
            case LT:
                op = A_operator::A_ltOP;
                break;
            case GE:
                op = A_operator::A_geOp;
                break;
            case GT:
                op = A_operator::A_gtOp;
                break;
            default:
                pushTokenBack(t);
                return parent;
        }
        auto a = addExp();
        A_exp *cur = new A_OpExp(a->pos, op, parent, a);
        return relationExpHelper(cur);
    }

    A_exp *parser::addExp() {
        auto m = mulExp();
        return addExpHelper(m);
    }

    A_exp *parser::addExpHelper(A_exp *parent) {
        token t = popToken();
        if (t.type != ADD && t.type != SUB) {
            pushTokenBack(t);
            return parent;
        }
        A_operator op;
        if (t.type == ADD) op = A_operator::A_plusOp;
        else op = A_operator::A_minusOp;
        auto m = mulExp();
        A_exp *cur = new A_OpExp(t.line, op, parent, m);
        return addExpHelper(cur);
    }

    A_exp *parser::mulExp() {
        auto s = subExp();
        return mulExpHelper(s);
    }

    A_exp *parser::mulExpHelper(A_exp *parent) {
        token t = popToken();
        if (t.type != MUL && t.type != DIV) {
            pushTokenBack(t);
            return parent;
        }
        A_operator op;
        if (t.type == MUL) op = A_operator::A_timesOp;
        else op = A_operator::A_divideOp;
        auto s = subExp();
        A_exp *cur = new A_OpExp(t.line, op, parent, s);
        return mulExpHelper(cur);
    }

    A_exp *parser::subExp() {
        token t = popToken();
        if (t.type != SUB) {
            pushTokenBack(t);
            return valExp();
        }
        auto v = valExp();
        auto zero_exp = new A_IntExp(t.line, 0);
        return new A_OpExp(t.line, A_operator::A_minusOp, zero_exp, v);
    }

    A_exp *parser::valExp() {
        token t = popToken();
        switch (t.type) {
            case INT_LITERAL:
                return new A_IntExp(t.line, atoi(t.val.c_str()));
            case STR_LITERAL:
                return new A_StringExp(t.line, t.val.substr(1, t.val.length() - 2));
            case NIL:
                return new A_NilExp(t.line);
            case L_SMALL: {
                auto seq = expList();
                checkNextToken(R_SMALL);
                return seq;
            }
                break;
            default:
                pushTokenBack(t);
                break;
        }
        return leftVal();
    }

    A_exp *parser::leftVal() {
        token t = checkNextToken(IDENTIFIER);
        auto var = new A_SimpleVar(t.line, t.val);
        return identifierExp(var);
    }

    A_exp *parser::identifierExp(A_var *var) {
        token t = popToken();
        switch (t.type) {
            case DOT: {
                token id = checkNextToken(IDENTIFIER);
                return identifierExp(new A_FieldVar(var->pos, var, id.val));
            }
                break;
            case L_SMALL: {
                auto func = dynamic_cast<A_SimpleVar *>(var);
                token _t = popToken();
                if (_t.type == R_SMALL) {
                    return new A_CallExp(func->pos, func->sym, nullptr);
                }
                pushTokenBack(_t);
                std::vector<A_exp *> vec;
                while (true) {
                    auto e = parse_expression();
                    vec.push_back(e);
                    token _temp_t = popToken();
                    if (_temp_t.type == R_SMALL)
                        break;
                    if (_temp_t.type != COMMA) {
                        std::cerr << "in line " << _temp_t.line << ": " << std::endl;
                        std::cerr << "parser: expected ')' or ',' but got " << _temp_t.to_str() << std::endl;
                        exit(1);
                    }
                }
                A_expList *list = nullptr;
                for (int i = vec.size() - 1; i >= 0; i--) {
                    auto tail = list;
                    list = new A_expList(vec[i], tail);
                }
                return new A_CallExp(func->pos, func->sym, list);
            }
                break;
            case L_MID: {
                auto e = parse_expression();
                checkNextToken(R_MID);
                auto fvar = new A_SubscriptVar(var->pos, var, e);
                return identifierExp(fvar);
            }
                break;
            case L_BIG: {
                auto list = expFieldList();
                checkNextToken(R_BIG);
                auto v = dynamic_cast<A_SimpleVar *>(var);
                return new A_RecordExp(v->pos, v->sym, list);
            }
            case OF: {
                auto v = dynamic_cast<A_SubscriptVar *>(var);
                if (v->ty != A_var::type::SUBSCRIPT) {
                    std::cerr << "in line " << v->pos << ":" << std::endl;
                    std::cerr << "parser: array parse_expression expect format 'typename[size] of initialexp'."
                              << std::endl;
                }
                auto e = parse_expression();
                auto primev = dynamic_cast<A_SimpleVar *>(v->var);
                if (primev->ty != A_var::type::SIMPLE) {
                    std::cerr << "in line " << v->pos << ":" << std::endl;
                    std::cerr << "parser: array parse_expression expect format 'typename[size] of initialexp'."
                              << std::endl;
                }
                return new A_ArrayExp(v->pos, primev->sym, v->exp, e);
            }
            default:
                pushTokenBack(t);
                break;
        }
        return new A_VarExp(var->pos, var);
    }

    A_exp *parser::expList() {
        token t = popToken();
        if (t.type == R_SMALL) {
            pushTokenBack(t);
            return new A_SeqExp(t.line, nullptr);
        }
        pushTokenBack(t);
        std::vector<A_exp *> vec;
        A_pos p = 0;
        while (true) {
            auto e = parse_expression();
            p = e->pos;
            vec.push_back(e);
            token _t = popToken();
            if (_t.type == SEMICOLON)
                continue;
            pushTokenBack(_t);
            break;
        }
        A_expList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_expList(vec[i], tail);
        }
        return new A_SeqExp(p, list);
    }

    A_expFieldList *parser::expFieldList() {
        token t = popToken();
        if (t.type == R_BIG) {
            pushTokenBack(t);
            return new A_expFieldList(nullptr, nullptr);
        }
        pushTokenBack(t);
        std::vector<A_expField *> vec;
        while (true) {
            auto ef = expField();
            vec.push_back(ef);
            token _t = popToken();
            if (_t.type == COMMA)
                continue;
            pushTokenBack(_t);
            break;
        }
        A_expFieldList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_expFieldList(vec[i], tail);
        }
        return list;
    }

    A_expField *parser::expField() {
        token id = checkNextToken(IDENTIFIER);
        checkNextToken(EQ);
        auto e = parse_expression();
        return new A_expField(id.val, e);
    }

    A_fieldList *parser::fieldList() {
        token t = popToken();
        if (t.type == R_BIG || t.type == R_SMALL) {
            pushTokenBack(t);
            return new A_fieldList(nullptr, nullptr);
        }
        pushTokenBack(t);
        std::vector<A_field *> vec;
        while (true) {
            auto f = field();
            vec.push_back(f);
            token _t = popToken();
            if (_t.type == COMMA)
                continue;
            pushTokenBack(_t);
            break;
        }
        A_fieldList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_fieldList(vec[i], tail);
        }
        return list;
    }


    A_field *parser::field() {
        token name = checkNextToken(IDENTIFIER);
        checkNextToken(COLON);
        token type = checkNextToken(IDENTIFIER);
        return new A_field(name.line, type.val, name.val);
    }


    A_ty *parser::ty() {
        token t = popToken();
        switch (t.type) {
            case IDENTIFIER:
                return new A_NameTy(t.line, t.val);
            case ARRAY: {
                checkNextToken(OF);
                token id = checkNextToken(IDENTIFIER);
                return new A_ArrayTy(t.line, id.val);
            }
            case L_BIG: {
                auto list = fieldList();
                checkNextToken(R_BIG);
                return new A_RecordTy(t.line, list);
            }
            default:
                std::cerr << "in line " << t.line << ":" << std::endl;
                std::cerr << "parser: expected identifier, array, or {, but got " << t.to_str() << std::endl;
                exit(1);
        }
    }

    A_decList *parser::declarationList() {
        std::vector<A_dec *> vec;
        while (true) {
            auto dec = declaration();
            if (dec == nullptr)
                break;
            if (!vec.empty()) {
                if (vec.back()->ty == A_dec::type::FUNCDEC && dec->ty == A_dec::type::FUNCDEC) {
                    auto list = dynamic_cast<A_FunctionDec *>(vec.back())->function;
                    while (list->tail != nullptr)
                        list = list->tail;
                    list->tail = dynamic_cast<A_FunctionDec *>(dec)->function;
                    continue;
                }
                if (vec.back()->ty == A_dec::type::TYDEC && dec->ty == A_dec::type::TYDEC) {
                    auto list = dynamic_cast<A_TypeDec *>(vec.back())->type;
                    while (list->tail != nullptr)
                        list = list->tail;
                    list->tail = dynamic_cast<A_TypeDec *>(dec)->type;
                    continue;
                }
            }
            vec.push_back(dec);
        }
        A_decList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_decList(vec[i], tail);
        }
        return list;
    }

    A_exp *parser::parse() {
        return parse_expression();
    }
}