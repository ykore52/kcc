#ifndef __TOKENIZER_HPP__
#define __TOKENIZER_HPP__

#include <iostream>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>

#include "util.hh"

namespace kcc
{


#define RESERVED_TOKEN(name, token) static const char name = token
RESERVED_TOKEN(TK_OPEN_PARENTHESIS, '(');
RESERVED_TOKEN(TK_CLOSE_PARENTHESIS, ')');
RESERVED_TOKEN(TK_OPEN_BRACE, '{');
RESERVED_TOKEN(TK_CLOSE_BRACE, '}');
RESERVED_TOKEN(TK_SEMICOLON, ';');

RESERVED_TOKEN(TK_EQUAL, '=');
RESERVED_TOKEN(TK_PLUS, '+');
RESERVED_TOKEN(TK_MINUS, '-');
RESERVED_TOKEN(TK_ASTERISK, '*');
RESERVED_TOKEN(TK_SLASH, '/');
RESERVED_TOKEN(TK_PERCENT, '%');

RESERVED_TOKEN(TK_LF, '\n');

#define RESERVED_KW(name, keyword) static const char *name = keyword
RESERVED_KW(KW_INT, "int");
RESERVED_KW(KW_RETURN, "return");

enum TokenType
{
    TkWord,
    TkInteger,

    TkOpenParent,
    TkCloseParent,
    TkOpenBrace,
    TkCloseBrace,
    TkSemicolon,
    TkEqual,
    TkPlus,
    TkMinus,
    TkAsterisk,
    TkSlash,
    TkPercent,

    TkLineFeed,

    TkNull
};

struct Token
{
    const std::string& operator()()
    {
        return token;
    }

    std::string token,
    TokenType type
};

class Tokenizer
{
  public:
    void Tokenize(std::vector<char> &buffer, std::vector<std::string> *tokens);

  private:
    Token& FetchToken(std::vector<char> &buffer, std::vector<char>::iterator &iter);
    bool ReadReservedKeyword(std::vector<std::string> &tokens,
                             std::vector<char> &token,
                             const char *keyword);
};

} // namespace Tokenizer

#endif
