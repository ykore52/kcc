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

namespace kcc
{

#define RESERVED_TOKEN(name, token) const char name = token
RESERVED_TOKEN(TK_OPEN_PARENTHESIS, '(');
RESERVED_TOKEN(TK_CLOSE_PARENTHESIS, ')');
RESERVED_TOKEN(TK_OPEN_BRACE, '{');
RESERVED_TOKEN(TK_CLOSE_BRACE, '}');
RESERVED_TOKEN(TK_SEMICOLON, ';');
RESERVED_TOKEN(TK_LF, '\n');

#define RESERVED_KW(name, keyword) const char *name = keyword
RESERVED_KW(KW_INT, "int");
RESERVED_KW(KW_RETURN, "return");

static inline void PDEBUG(std::string message)
{
    std::cout << message << std::endl;
}

class Tokenizer
{
  public:
    Tokenizer() {}
    ~Tokenizer() {}

    std::vector<std::string> Tokenize(std::vector<char> buffer);

  private:
    std::vector<char> FetchToken(std::vector<char>::iterator &iter);
    bool ReadReservedKeyword(std::vector<std::string> &tokens,
                             std::vector<char> token,
                             const char *keyword);
}

} // namespace Tokenizer

#endif
