#ifndef __TOKENIZER_HPP__
#define __TOKENIZER_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std;

#define RESERVED_TOKEN(name, token) const char name = token
RESERVED_TOKEN(TK_BRACKET_LEFT, '(');
RESERVED_TOKEN(TK_BRACKET_RIGHT, '(');
RESERVED_TOKEN(TK_BRACE_LEFT, '{');
RESERVED_TOKEN(TK_BRACE_RIGHT, '}');
RESERVED_TOKEN(TK_SEMICOLON, ';');
RESERVED_TOKEN(TK_LF, '\n');

#define RESERVED_KW(name, keyword) const char *name = keyword
RESERVED_KW(KW_INT, "int");
RESERVED_KW(KW_RETURN, "return");

std::vector<char> fetch_token(std::vector<char>::iterator &iter)
{
    std::vector<char> token;

    while (true)
    {
        // std::cout << ";" << *iter << std::endl;

        // Unrecognized charachters
        if ((0x01 <= *iter && *iter <= 0x08) || (0x0b <= *iter && *iter <= 0x0c) ||
            (0x0e <= *iter && *iter <= 0x1f) || *iter == 0x7f)
        {
            std::cout << "unrecognized character" << std::endl;

            ++iter;
            break;
        }

        // return characters
        if (*iter == '\r')
        {
            token.push_back(TK_LF);
            ++iter;
            if (*iter == TK_LF)
            {
                ++iter;
            }
            break;
        }
        if (*iter == TK_LF)
        {
            token.push_back(TK_LF);
            ++iter;
            break;
        }

        // literals
        if (('0' <= *iter && *iter <= '9') || ('A' <= *iter && *iter <= 'Z') ||
            ('a' <= *iter && *iter <= 'z') || *iter == '_')
        {
            token.push_back(*iter);

            ++iter;
            continue;
        }

        if (token.size() > 0)
        {
            token.push_back('\0');
            break;
        }

        // space and tab
        if (*iter == ' ' || *iter == '\t')
        {
            ++iter;
            break;
        }

        // other characters;
        token.push_back(*iter);
        token.push_back('\0');

        ++iter;
        break;
    }

    return token;
}

bool read_reserved_keyword(std::vector<string> &tokens,
                           std::vector<char> token,
                           const char *keyword)
{
    if (std::memcmp(keyword, &token[0], std::strlen(keyword)) == 0)
    {
        tokens.push_back(&token[0]);
        return true;
    }
    return false;
}

// Tokenizer
std::vector<string> tokenize(std::vector<char> buffer)
{
    std::vector<string> tokens;
    auto iter = buffer.begin();
    while (iter != buffer.end())
    {
        auto token = fetch_token(iter);
        if (token.size() == 0)
        {
            continue;
        }

        if (read_reserved_keyword(tokens, token, KW_INT))
        {
            continue;
        }

        if (read_reserved_keyword(tokens, token, KW_RETURN))
        {
            continue;
        }

        tokens.push_back(&token[0]);
    }
    return tokens;
}

typedef enum _Scope {
    Global,
    Function,
    Struct,
} Scope;

struct SyntaxCheckState {
    int line_number;
    int nest;
    Scope scope;

    std::set<std::string> type_store;
    std::set<std::string> function_store; 

    SyntaxCheckState() {
        this->line_number = 0;
        this->nest = 0;
        this->scope = Scope::Global;

        this->type_store.insert("int");
    }
};

struct ArgumentInfo {
    std::string type;
    std::string name;
};

struct FunctionInfo {
    std::string type;
    std::string name;
    std::vector<ArgumentInfo> args;
};

// Syntax check worker
void syntax_check_worker(std::vector<string>::iterator &iter, SyntaxCheckState &state) {

    if (state.scope == Scope::Global) {
        auto function_type = *iter;
        if (state.type_store.find(function_type) != state.type_store.end()) {
            *iter++;
            auto function_name = *iter;
            
        } else {

        }
        auto function_name = *iter;

    }
}

// Syntax check
void syntax_check(std::vector<string> &tokens) {

    SyntaxCheckState state;

    auto iter = tokens.begin();
    while (iter != tokens.end()) {

    }
}

#endif