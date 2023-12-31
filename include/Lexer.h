#pragma once

#include "Token.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>

namespace FRONTEND {
    class Lexer {
    private:
        std::unordered_map<std::string, int> tokenizerMap{
                {{"while"},    1},
                {{"for"},      2},
                {{"to"},       3},
                {{"break"},    4},
                {{"let"},      5},
                {{"in"},       6},
                {{"end"},      7},
                {{"function"}, 8},
                {{"var"},      9},
                {{"type"},     10},
                {{"array"},    11},
                {{"if"},       12},
                {{"then"},     13},
                {{"else"},     14},
                {{"do"},       15},
                {{"of"},       16},
                {{"nil"},      17},
                {{","},        18},
                {{":"},        19},
                {{";"},        20},
                {{"("},        21},
                {{")"},        22},
                {{"["},        23},
                {{"]"},        24},
                {{"{"},        25},
                {{"}"},        26},
                {{"."},        27},
                {{"+"},        28},
                {{"-"},        29},
                {{"*"},        30},
                {{"/"},        31},
                {{"="},        32},
                {{"<>"},       33},
                {{"<"},        34},
                {{"<="},       35},
                {{">"},        36},
                {{">="},       37},
                {{"&"},        38},
                {{"|"},        39},
                {{":="},       40},
        };
        std::unordered_set<char> special{',', ':', ';', '+', '-', '*', '/', '(', ')', '[', ']',
                                         '{', '}', '=', '<', '>', '&', '|', '.'};
        std::fstream src;
        int line_cnt = 1;

        inline char get_next_char() { return src.get(); };

        inline void back_char() { src.seekg(-1, std::ios::cur); };

        std::string next_word();

        void escape_comment();

        std::string get_num(char ch);

        std::string get_identifier(char ch);

        std::string get_operator(char ch);

        std::string get_string();

        char skip_whitespace();

        void handle_single_line_comment();

        void handle_multi_line_comment();

    public:
        Lexer() = delete;

        ~Lexer();

        explicit Lexer(const std::string& src_file);

        Token tokenizeNext();
    };
}