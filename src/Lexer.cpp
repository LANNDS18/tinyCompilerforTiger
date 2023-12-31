#include "../include/Lexer.h"
#include <iostream>

namespace FRONTEND {
    Lexer::Lexer(const std::string &src_file) {
        src.open(src_file, std::ios::in);
        if (!src.is_open()) {
            std::cerr << "Fail to open file " << src_file << std::endl;
            exit(1);
        }
    }

    Lexer::~Lexer() {
        src.close();
    }

    char Lexer::skip_whitespace() {
        char ch = get_next_char();
        while (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
            if (ch == '\n') line_cnt++;
            ch = get_next_char();
        }
        return ch;
    }

    void Lexer::handle_single_line_comment() {
        char ch;
        do {
            ch = get_next_char();
            if (ch == '\n') line_cnt++;
        } while (ch != '\n' && ch != EOF);
    }

    void Lexer::handle_multi_line_comment() {
        char last, cur;
        last = get_next_char();
        if (last == '\n') line_cnt++;
        if (last == EOF) {
            std::cerr << "in line " << line_cnt << ": Lexer: incomplete comment, expected */" << std::endl;
            exit(1);
        }
        do {
            cur = last;
            last = get_next_char();
            if (last == '\n') line_cnt++;
            if (last == EOF) {
                std::cerr << "in line " << line_cnt << ": Lexer: incomplete comment, expected */" << std::endl;
                exit(1);
            }
        } while (!(cur == '*' && last == '/'));
    }

    void Lexer::escape_comment() {
        while (true) { // Keep looping until there are no more comments to escape
            char ch = get_next_char();
            if (ch == '/') {
                handle_single_line_comment();
            } else if (ch == '*') {
                handle_multi_line_comment();
            } else {
                back_char();
                back_char();
                return;
            }
            ch = skip_whitespace();
            if (ch != '/') {
                back_char();
                return; // Exit the loop and the method if the next character isn't a '/'
            }
            // If ch == '/', the loop continues and handles the next comment
        }
    }

    std::string Lexer::get_num(char ch) {
        std::string ret;
        ret += ch;
        while (true) {
            ch = get_next_char();
            if (ch >= '0' && ch <= '9')
                ret += ch;
            else if (ch == EOF)
                return ret;
            else if (special.count(ch)) {
                back_char();
                return ret;
            } else if (ch == '/') {
                escape_comment();
                return ret;
            } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                if (ch == '\n') line_cnt++;
                return ret;
            } else {
                ret += ch;
                std::cerr << "in line " << line_cnt << ":" << std::endl;
                std::cerr << "Lexer: invalid num " << ret << "..." << std::endl;
                exit(1);
            }
        }
    }

    std::string Lexer::get_identifier(char ch) {
        std::string ret;
        ret += ch;
        while (true) {
            ch = get_next_char();
            if (ch == EOF)
                return ret;
            else if (special.count(ch)) {
                back_char();
                return ret;
            } else if (ch == '/') {
                escape_comment();
                return ret;
            } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                if (ch == '\n') line_cnt++;
                return ret;
            }
                // valid char [0-9a-zA-Z]|_
            else if (ch >= '0' && ch <= '9' || ch == '_' ||
                     ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z') {
                ret += ch;
            } else {
                std::cerr << "in line " << line_cnt << ":" << std::endl;
                std::cerr << "Lexer: char " << ch << " are not expected to occur in identifier." << std::endl;
                exit(1);
            }
        }
    }

    std::string Lexer::get_operator(char ch) {
        // only operator start with [:<>] may contain more than one chars.
        if (ch != ':' && ch != '<' && ch != '>') {
            return {ch};
        }
        char next = get_next_char();
        if (ch == ':' && next == '=' || ch == '<' && next == '>'
            || ch == '<' && next == '=' || ch == '>' && next == '=') {
            std::string ret{ch};
            return ret + next;
        }
        back_char();
        return {ch};
    }

    std::string Lexer::get_string() {
        char last = '"', ch;
        std::string ret;
        ret += '"';
        while (true) {
            ch = get_next_char();
            if (ch == EOF) {
                std::cerr << "in line " << line_cnt << ":" << std::endl;
                std::cerr << "Lexer: fail to get complete string." << std::endl;
                exit(1);
            }
            if (ch != '"') {
                ret += ch;
            } else if (last != '\\') {
                break;
            }
            last = ch;
        }
        ret += '"';
        return ret;
    }


    std::string Lexer::next_word() {
        char ch = skip_whitespace();
        if (ch == EOF) return {};

        if (ch == '/') {
            escape_comment();
            ch = get_next_char();
        }
        // escape printSpace after escape comment
        while (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
            if (ch == '\n') line_cnt++;
            ch = get_next_char();
        }
        if (ch == EOF) return {};

        // parse string
        if (ch == '"')
            return get_string();
            // parse number
        else if (ch >= '0' && ch <= '9')
            return get_num(ch);
            // parse operator
        else if (special.count(ch))
            return get_operator(ch);
            // parse identifier or other reserved word.
        else
            return get_identifier(ch);
    }


    Token Lexer::tokenizeNext() {
        std::string next_ = next_word();
        if (tokenizerMap.count(next_))
            return {tokenizerMap[next_], line_cnt};
        if (next_.length() == 0)
            return {EOF, line_cnt};
        if ('0' <= next_[0] && next_[0] <= '9')
            return {INT_LITERAL, next_, line_cnt};
        if (next_[0] == '"')
            return {STR_LITERAL, next_, line_cnt};
        return {IDENTIFIER, next_, line_cnt};
    }
}