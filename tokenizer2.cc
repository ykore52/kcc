#include <iostream>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "tokenizer.hh"

namespace kcc
{

// Tokenizer
void Tokenizer::Tokenize(std::vector<char> &buffer, std::vector<std::string>* tokens)
{
    for (auto iter = std::begin(buffer); iter != std::end(buffer);)
    {
        auto token = FetchToken(buffer, iter);
        if (token.size() == 0)
        {
            continue;
        }

        // PrintCharVector(token);
        tokens->push_back(std::string(token.begin(), token.end()));
    }
}

Token& Tokenizer::FetchToken(std::vector<char> &buffer, std::vector<char>::iterator &iter)
{
    std::vector<char> token;
    Token token_object;

    while (iter != std::end(buffer))
    {
        // Unrecognized charachters (Nonprintable)
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
            token_object.type = TkLineFeed;
            token.push_back(TK_LF);
            ++iter;
            break;
        }

        // integer literal
        if (token.size() == 0) {

        } else {
            
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
            break;
        }

        // space and tab
        if (*iter == ' ' || *iter == '\t')
        {
            ++iter;
            break;
        }

        // other characters - e.g. [!"#$%&'()=~|;] etc...
        token.push_back(*iter);

        ++iter;
        break;
    }

    token_object.token = std::string(token.begin(), token.end());
    return token_object;
}

bool Tokenizer::ReadReservedKeyword(std::vector<std::string> &tokens,
                                    std::vector<char> &token,
                                    const char *keyword)
{
    if (std::memcmp(keyword, &token[0], std::strlen(keyword)) == 0)
    {
        tokens.push_back(&token[0]);
        return true;
    }
    return false;
}


} // namespace Tokenizer