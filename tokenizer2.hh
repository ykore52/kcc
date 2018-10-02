#ifndef TOKENIZER2_HH
#define TOKENIZER2_HH

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace kcc2
{

enum TokenType
{
    tkInteger,

    // 記号
    tkOpenParent, // (
    tkCloseParent, // )
    tkOpenBrace, // {
    tkCloseBrace, // }


    tkNull
};

struct Token
{
    std::string token;
    TokenType type;
};

class Tokenizer
{
  public:
    Tokenizer() : pos(0) {}

    int SkipSpace()
    {
        while (*it == ' ' || it != std::end(buffer)) {
            it++;
            pos++;
        }
    }

    int Tokenize(const std::vector<char> &buf, std::vector<Token> *dest)
    {
        buffer = buf;
        it = std::begin(buf);
        while (it != std::end(buf))
        {
            if (('a' <= *it && *it <= 'z') || ('A' <= *it && *it <= 'Z') || ('0' <= *it && *it <= '9') || *it == '_')
            {
                
            }
        }
    }

  private:
    std::vector<char> buffer;
    std::vector<char>::const_iterator it;
    int pos;
};

} // namespace kcc2
#endif