#include "parser.h"
#include <iostream>
#include <vector>


namespace FRONTEND {


    token parser::pop_token() {
        if(q.empty())
            return lex.next();
        auto tmp = q.front();
        q.pop();
        return tmp;
    }

    token parser::take_token(int token_ty) {
        auto t = pop_token();
        if(t.type != token_ty) {
            token exp(token_ty, -1);
            throw std::runtime_error("in line " + std::to_string(t.line) + ": parser: expected " + exp.to_str() + " but got " + t.to_str());
        }
        return t;
    }

    A_dec *parser::declaration() {
        auto t = pop_token();
        switch (t.type) {
            case VAR : {
                auto id = take_token(IDENTIFIER);
                S_symbol type;
                token _t = pop_token();
                if (_t.type == COLON) {
                    auto id = take_token(IDENTIFIER);
                    type = id.val;
                    take_token(ASSIGN);
                } else if (_t.type != ASSIGN) {
                    std::cerr << "in line " << _t.line << ":" << std::endl;
                    std::cerr << "parser: expected COLON or ASSIGN, but got " << t.to_str() << std::endl;
                    exit(1);
                }
                auto e = parse_expression();
                return new A_VarDec(_t.line, id.val, type, e);
            }
                break;
            case TYPE : {
                token id = take_token(IDENTIFIER);
                take_token(EQ);
                auto type = ty();
                auto *type_(new A_namety(id.val, type));
                auto list = new A_nametyList(type_, nullptr);
                return new A_TypeDec(id.line, list);
            }
                break;
            case FUNCTION: {
                token func_name = take_token(IDENTIFIER);
                take_token(L_SMALL);
                auto list = field_list();
                take_token(R_SMALL);
                token next = pop_token();
                if (next.type == EQ) {
                    auto e = parse_expression();
                    auto func = new A_funcdec(t.line, func_name.val, list, "", e);
                    auto _list = new A_funcdecList(func, nullptr);
                    return new A_FunctionDec(func_name.line, _list);
                } else if (next.type == COLON) {
                    auto id = take_token(IDENTIFIER);
                    take_token(EQ);
                    auto e = parse_expression();
                    auto func = new A_funcdec(t.line, func_name.val, list, id.val, e);
                    auto _list = new A_funcdecList(func, nullptr);
                    return new A_FunctionDec(func_name.line, _list);
                } else {
                    std::cerr << "in line " << next.line << ":" << std::endl;
                    std::cerr << "parser: expected EQ or COLON, but got " << next.to_str() << std::endl;
                    exit(1);
                }
            }
            default:
                unuse(t);
                break;
        }
        return nullptr;
    }

    A_exp* parser::parse_expression() {
        auto t = pop_token();
        switch (t.type) {
            case LET: {
                auto ds = decs();
                take_token(IN);
                auto es = seqexp();
                take_token(END);
                return new A_LetExp(t.line, ds, es);
            }
            case WHILE: {
                auto cond = parse_expression();
                take_token(DO);
                auto body = parse_expression();
                return new A_WhileExp(t.line, cond, body);
            }
            case FOR: {
                auto ite = take_token(IDENTIFIER);
                take_token(ASSIGN);
                auto start = parse_expression();
                take_token(TO);
                auto end = parse_expression();
                take_token(DO);
                auto body = parse_expression();
                return new A_ForExp(t.line, ite.val, start, end, body);
            }
            case IF: {
                auto cond = parse_expression();
                take_token(THEN);
                auto True = parse_expression();
                token expexted_else = pop_token();
                if (expexted_else.type != ELSE) {
                    unuse(expexted_else);
                    return new A_IfExp(t.line, cond, True, nullptr);
                }
                auto False = parse_expression();
                return new A_IfExp(t.line, cond, True, False);
            }
            case BREAK:
                return new A_BreakExp(t.line);
            case L_SMALL: {
                auto seq = seqexp();
                take_token(R_SMALL);
                return seq;
            }
            default:
                unuse(t);
                return assignexp();
        }
    }


    A_exp *parser::assignexp() {
        auto o = or_exp();
        auto a = assignexp_();
        if (a == nullptr)
            return o;
        if (o->ty != A_exp::type::VarExp) {
            std::cerr << "in line " << a->pos << std::endl;
            std::cerr << "parser: left side can not be right value in assign parse_expression." << std::endl;
            exit(1);
        }
        auto varexp = dynamic_cast<A_VarExp *>(o);
        return new A_AssignExp(varexp->pos, varexp->var, a);
    }

    A_exp *parser::assignexp_() {
        auto t = pop_token();
        if (t.type != ASSIGN) {
            unuse(t);
            return nullptr;
        }
        return parse_expression();
    }

    A_exp *parser::or_exp() {
        auto a_exp = andexp();
        return _or_exp_helper(a_exp);
    }

    A_exp *parser::_or_exp_helper(A_exp *parent) {
        token expected_or = pop_token();
        if (expected_or.type != OR) {
            unuse(expected_or);
            return parent;
        }
        A_exp *a = andexp();
        auto one_exp = new A_IntExp(a->pos, 1);
        A_exp *cur = new A_IfExp(a->pos, parent, one_exp, a);
        return _or_exp_helper(cur);
    }

    A_exp *parser::andexp() {
        auto r = relexp();
        return andexp_(r);
    }

    A_exp *parser::andexp_(A_exp *parent) {
        token expected_and = pop_token();
        if (expected_and.type != AND) {
            unuse(expected_and);
            return parent;
        }
        auto zero_exp = new A_IntExp(parent->pos, 0);
        auto r = relexp();
        A_exp *cur = new A_IfExp(r->pos, parent, r, zero_exp);
        return andexp_(cur);
    }

    A_exp *parser::relexp() {
        auto a = addexp();
        return relexp_(a);
    }

    A_exp *parser::relexp_(A_exp *parent) {
        token t = pop_token();
        A_oper op;
        switch (t.type) {
            case EQ:
                op = A_oper::A_eqOp;
                break;
            case NEQ:
                op = A_oper::A_neqOp;
                break;
            case LE:
                op = A_oper::A_leOp;
                break;
            case LT:
                op = A_oper::A_ltOP;
                break;
            case GE:
                op = A_oper::A_geOp;
                break;
            case GT:
                op = A_oper::A_gtOp;
                break;
            default:
                unuse(t);
                return parent;
        }
        auto a = addexp();
        A_exp *cur = new A_OpExp(a->pos, op, parent, a);
        return relexp_(cur);
    }

    A_exp *parser::addexp() {
        auto m = mulexp();
        return addexp_(m);
    }

    A_exp *parser::addexp_(A_exp *parent) {
        token t = pop_token();
        if (t.type != ADD && t.type != SUB) {
            unuse(t);
            return parent;
        }
        A_oper op;
        if (t.type == ADD) op = A_oper::A_plusOp;
        else op = A_oper::A_minusOp;
        auto m = mulexp();
        A_exp *cur = new A_OpExp(t.line, op, parent, m);
        return addexp_(cur);
    }

    A_exp *parser::mulexp() {
        auto s = subexp();
        return mulexp_(s);
    }

    A_exp *parser::mulexp_(A_exp *parent) {
        token t = pop_token();
        if (t.type != MUL && t.type != DIV) {
            unuse(t);
            return parent;
        }
        A_oper op;
        if (t.type == MUL) op = A_oper::A_timesOp;
        else op = A_oper::A_divideOp;
        auto s = subexp();
        A_exp *cur = new A_OpExp(t.line, op, parent, s);
        return mulexp_(cur);
    }

    A_exp *parser::subexp() {
        token t = pop_token();
        if (t.type != SUB) {
            unuse(t);
            return valexp();
        }
        auto v = valexp();
        auto zero_exp = new A_IntExp(t.line, 0);
        return new A_OpExp(t.line, A_oper::A_minusOp, zero_exp, v);
    }

    A_exp *parser::valexp() {
        token t = pop_token();
        switch (t.type) {
            case INT_LITERAL:
                return new A_IntExp(t.line, atoi(t.val.c_str()));
            case STR_LITERAL:
                return new A_StringExp(t.line, t.val.substr(1, t.val.length() - 2));
            case NIL:
                return new A_NilExp(t.line);
            case L_SMALL: {
                auto seq = seqexp();
                take_token(R_SMALL);
                return seq;
            }
                break;
            default:
                unuse(t);
                break;
        }
        return lval();
    }

    A_exp *parser::lval() {
        token t = take_token(IDENTIFIER);
        auto var = new A_SimpleVar(t.line, t.val);
        return idexp(var);
    }

    A_exp *parser::idexp(A_var *var) {
        token t = pop_token();
        switch (t.type) {
            case DOT: {
                token iden = take_token(IDENTIFIER);
                return idexp(new A_FieldVar(var->pos, var, iden.val));
            }
                break;
            case L_SMALL: {
                auto func = dynamic_cast<A_SimpleVar *>(var);
                token _t = pop_token();
                if (_t.type == R_SMALL) {
                    return new A_CallExp(func->pos, func->sym, nullptr);
                }
                unuse(_t);
                std::vector<A_exp *> vec;
                while (true) {
                    auto e = parse_expression();
                    vec.push_back(e);
                    token _temp_t = pop_token();
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
                take_token(R_MID);
                auto fvar = new A_SubscriptVar(var->pos, var, e);
                return idexp(fvar);
            }
                break;
            case L_BIG: {
                auto list = efield_list();
                take_token(R_BIG);
                auto v = dynamic_cast<A_SimpleVar *>(var);
                return new A_RecordExp(v->pos, v->sym, list);
            }
            case OF: {
                auto v = dynamic_cast<A_SubscriptVar *>(var);
                if (v->ty != A_var::type::SUBSCRIPT) {
                    std::cerr << "in line " << v->pos << ":" << std::endl;
                    std::cerr << "parser: array parse_expression expect format 'typename[size] of initialexp'." << std::endl;
                }
                auto e = parse_expression();
                auto primev = dynamic_cast<A_SimpleVar *>(v->var);
                if (primev->ty != A_var::type::SIMPLE) {
                    std::cerr << "in line " << v->pos << ":" << std::endl;
                    std::cerr << "parser: array parse_expression expect format 'typename[size] of initialexp'." << std::endl;
                }
                return new A_ArrayExp(v->pos, primev->sym, v->exp, e);
            }
            default:
                unuse(t);
                break;
        }
        return new A_VarExp(var->pos, var);
    }

    A_exp *parser::seqexp() {
        token t = pop_token();
        if (t.type == R_SMALL) {
            unuse(t);
            return new A_SeqExp(t.line, nullptr);
        }
        unuse(t);
        std::vector<A_exp *> vec;
        A_pos p = 0;
        while (true) {
            auto e = parse_expression();
            p = e->pos;
            vec.push_back(e);
            token _t = pop_token();
            if (_t.type == SEMICOLON)
                continue;
            unuse(_t);
            break;
        }
        A_expList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_expList(vec[i], tail);
        }
        return new A_SeqExp(p, list);
    }

    A_efieldList *parser::efield_list() {
        token t = pop_token();
        if (t.type == R_BIG) {
            unuse(t);
            return new A_efieldList(nullptr, nullptr);
        }
        unuse(t);
        std::vector<A_efield *> vec;
        while (true) {
            auto ef = efield();
            vec.push_back(std::move(ef));
            token _t = pop_token();
            if (_t.type == COMMA)
                continue;
            unuse(_t);
            break;
        }
        A_efieldList *list(nullptr);
        for (int i = vec.size() - 1; i >= 0; i--) {
            auto tail = list;
            list = new A_efieldList(vec[i], tail);
        }
        return list;
    }

    A_efield *parser::efield() {
        token id = take_token(IDENTIFIER);
        take_token(EQ);
        auto e = parse_expression();
        return new A_efield(id.val, e);
    }

    A_fieldList *parser::field_list() {
        token t = pop_token();
        if (t.type == R_BIG || t.type == R_SMALL) {
            unuse(t);
            return new A_fieldList(nullptr, nullptr);
        }
        unuse(t);
        std::vector<A_field *> vec;
        while (true) {
            auto f = field();
            vec.push_back(f);
            token _t = pop_token();
            if (_t.type == COMMA)
                continue;
            unuse(_t);
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
        token name = take_token(IDENTIFIER);
        take_token(COLON);
        token type = take_token(IDENTIFIER);
        return new A_field(name.line, type.val, name.val);
    }


    A_ty *parser::ty() {
        token t = pop_token();
        switch (t.type) {
            case IDENTIFIER:
                return new A_NameTy(t.line, t.val);
            case ARRAY: {
                take_token(OF);
                token id = take_token(IDENTIFIER);
                return new A_ArrayTy(t.line, id.val);
            }
            case L_BIG: {
                auto list = field_list();
                take_token(R_BIG);
                return new A_RecordTy(t.line, list);
            }
            default:
                std::cerr << "in line " << t.line << ":" << std::endl;
                std::cerr << "parser: expected identifier, array, or {, but got " << t.to_str() << std::endl;
                exit(1);
        }
    }

    A_decList *parser::decs() {
        std::vector<A_dec *> vec;
        while (true) {
            auto d = declaration();
            if (d == nullptr)
                break;
            if (!vec.empty()) {
                if (vec.back()->ty == A_dec::type::FUNCDS && d->ty == A_dec::type::FUNCDS) {
                    auto list = dynamic_cast<A_FunctionDec *>(vec.back())->function;
                    while (list->tail != nullptr)
                        list = list->tail;
                    list->tail = dynamic_cast<A_FunctionDec *>(d)->function;
                    continue;
                }
                if (vec.back()->ty == A_dec::type::TYDS && d->ty == A_dec::type::TYDS) {
                    auto list = dynamic_cast<A_TypeDec *>(vec.back())->type;
                    while (list->tail != nullptr)
                        list = list->tail;
                    list->tail = dynamic_cast<A_TypeDec *>(d)->type;
                    continue;
                }
            }
            vec.push_back(std::move(d));
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