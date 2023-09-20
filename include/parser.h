#pragma once

#include <queue>
#include "lexer.h"
#include "abstract.h"

namespace FRONTEND {
    class parser {
    private:
        lexer lex;
        std::queue<token> q;

        token checkNextToken(int token_ty);

        token popToken();

        void pushTokenBack(const token& t) { q.push(t); };

        A_dec *declaration();

        A_exp *parse_expression();

        A_exp *assignmentExp();

        A_exp *assignmentExpHelper();

        A_exp *orExp();

        A_exp *orExpHelper(A_exp *parent);

        A_exp *andExp();

        A_exp *andExpHelper(A_exp *parent);

        A_exp *relexp();

        A_exp *relexp_(A_exp *parent);

        A_exp *addExp();

        A_exp *addExpHelper(A_exp *parent);

        A_exp *mulExp();

        A_exp *mulExpHelper(A_exp *parent);

        A_exp *subExp();

        A_exp *valExp();

        A_exp *lval();

        A_exp *identifierExp(A_var *var);

        A_efieldList *eFieldList();

        A_efield *eField();

        A_fieldList *fieldList();

        A_field *field();

        A_ty *ty();

        A_decList *declarationList();

        A_exp *expList();

    public:
        parser() = delete;

        explicit parser(const std::string& src_file) : lex(src_file) {};

        A_exp *parse();
    };
}
