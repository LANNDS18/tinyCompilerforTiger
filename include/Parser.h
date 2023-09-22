#pragma once

#include <queue>
#include "Lexer.h"
#include "abstract.h"

namespace FRONTEND {
    class Parser {
    private:
        Lexer lex;
        std::queue<Token> q;

        Token checkNextToken(int token_ty);

        Token popToken();

        void pushTokenBack(const Token& t) { q.push(t); };

        A_dec *declaration();

        A_exp *parse_expression();

        A_exp *assignmentExp();

        A_exp *assignmentExpHelper();

        A_exp *orExp();

        A_exp *orExpHelper(A_exp *parent);

        A_exp *andExp();

        A_exp *andExpHelper(A_exp *parent);

        A_exp *relationExp();

        A_exp *relationExpHelper(A_exp *parent);

        A_exp *addExp();

        A_exp *addExpHelper(A_exp *parent);

        A_exp *mulExp();

        A_exp *mulExpHelper(A_exp *parent);

        A_exp *subExp();

        A_exp *valExp();

        A_exp *leftVal();

        A_exp *identifierExp(A_var *var);

        A_expFieldList *expFieldList();

        A_expField *expField();

        A_fieldList *fieldList();

        A_field *field();

        A_type *ty();

        A_decList *declarationList();

        A_exp *expList();

    public:
        Parser() = delete;

        explicit Parser(const std::string& src_file) : lex(src_file) {};

        A_exp *parse();
    };
}
