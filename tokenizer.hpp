#ifndef __TOKENIZER_HPP__
#define __TOKENIZER_HPP__

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

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

bool read_reserved_keyword(std::vector<string> &tokens, std::vector<char> token,
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

typedef enum _Scope
{
    Global,
    Function,
    Struct,
} Scope;

struct CompileError
{
    int line_number;
    std::string message;

    CompileError()
    {
        this->line_number = 0;
        this->message = "";
    }

    CompileError(int line_number, std::string message)
    {
        this->line_number = line_number;
        this->message = message;
    }
};

struct ObjectInfo
{
    std::string type;
    std::string name;
};

struct FunctionInfo : public ObjectInfo
{
    std::vector<ObjectInfo> args;
};

struct SyntaxCheckState
{
    std::string filename;
    int line_number;
    int nest;
    Scope scope;

    std::set<std::string> type_store;
    std::map<std::string, std::shared_ptr<ObjectInfo>> object_store;

    std::vector<CompileError> error;

    SyntaxCheckState()
    {
        this->line_number = 1;
        this->nest = 0;
        this->scope = Scope::Global;

        this->type_store.insert("int");
    }
};

///////////////////////////////////////////////////////////////////////////////

void skip_line_feed(std::vector<string>::iterator &iter,
                    SyntaxCheckState &state)
{
    // skip line feeds
    while ((*iter)[0] == TK_LF)
    {
        ++iter;
        ++state.line_number;
    }
}

bool semicolon(std::vector<string>::iterator &iter,
               SyntaxCheckState &state)
{
    std::cout << "semicolon" << std::endl;
    auto semicolon = *iter;
    ++iter;

    skip_line_feed(iter, state);

    return semicolon[0] == TK_SEMICOLON;
}

bool constant_integer(int &number,
                      std::vector<string>::iterator &iter,
                      SyntaxCheckState &state)
{
    std::cout << "constant_integer" << std::endl;
    auto constant_integer = *iter;
    ++iter;

    skip_line_feed(iter, state);

    number = std::atoi(constant_integer.c_str());
    return true;
}

bool expression(std::vector<string>::iterator &iter,
                SyntaxCheckState &state)
{
    std::cout << "expression" << std::endl;
    // constant number;
    int number;
    bool result = constant_integer(number, iter, state) && semicolon(iter, state);

    std::cout << "constant number is : " << number << endl;
    return result;
}

bool return_statement(std::vector<string>::iterator &iter,
                      SyntaxCheckState &state)
{
    std::cout << "return_statement: next is " << *iter << std::endl;
    if (*iter != KW_RETURN)
    {
        return false;
    }

    ++iter;

    skip_line_feed(iter, state);

    return expression(iter, state);
}

bool type_specifier(const std::shared_ptr<ObjectInfo> &objInfo,
                    std::vector<string>::iterator &iter,
                    SyntaxCheckState &state)
{
    std::cout << "type_specifier" << std::endl;
    std::string typeName = *iter;
    ++iter;
    if (state.type_store.find(typeName) == state.type_store.end())
    {
        state.error.push_back(
            CompileError(state.line_number,
                         "ERROR: type name '" + typeName + "' is not defined."));
        return false;
    }

    skip_line_feed(iter, state);

    objInfo->type = typeName;

    std::cout << " -> type name : " << typeName << std::endl;
    return true;
}

bool function_identifier(const std::shared_ptr<FunctionInfo> &funcInfo,
                         std::vector<string>::iterator &iter,
                         SyntaxCheckState &state)
{
    std::cout << "function_identifier" << std::endl;

    std::string functionName = *iter;
    ++iter;
    if (state.object_store.find(functionName) != state.object_store.end())
    {
        state.error.push_back(
            CompileError(state.line_number, "ERROR: function name '" +
                                                functionName + "' is duplicated."));
        return false;
    }

    skip_line_feed(iter, state);

    funcInfo->name = functionName;
    std::cout << " -> function name : " << functionName << std::endl;
    return true;
}

bool declaration_list(std::vector<string>::iterator &iter,
                      SyntaxCheckState &state)
{
    std::cout << "declaration_list" << std::endl;
    // TBD
    return true;
}

bool open_parenthesis(std::vector<string>::iterator &iter,
                      SyntaxCheckState &state)
{
    std::cout << "open_parenthesis" << std::endl;
    auto token = *iter;
    *iter++;
    if (token[0] != TK_OPEN_PARENTHESIS)
    {
        state.error.push_back(CompileError(
            state.line_number, "ERROR: unexpected identifier: '" + token + "'"));

        return false;
    }

    skip_line_feed(iter, state);

    return true;
}

bool close_parenthesis(std::vector<string>::iterator &iter,
                       SyntaxCheckState &state)
{
    std::cout << "close_parenthesis" << std::endl;

    auto token = *iter;
    *iter++;
    if (token[0] != TK_CLOSE_PARENTHESIS)
    {
        state.error.push_back(CompileError(
            state.line_number, "ERROR: unexpected identifier: '" + token + "'"));

        return false;
    }

    skip_line_feed(iter, state);

    return true;
}

bool open_brace(std::vector<string>::iterator &iter,
                      SyntaxCheckState &state)
{
    std::cout << "open_brace" << std::endl;
    std::cout << *iter << std::endl;
    auto token = *iter;
    *iter++;
    if (token[0] != TK_OPEN_BRACE)
    {
        state.error.push_back(CompileError(
            state.line_number, "ERROR: unexpected identifier: '" + token + "'"));

        return false;
    }

    std::cout << *iter << std::endl;
    skip_line_feed(iter, state);

    return true;
}

bool close_brace(std::vector<string>::iterator &iter,
                       SyntaxCheckState &state)
{
    std::cout << "close_brace" << std::endl;

    auto token = *iter;
    *iter++;
    if (token[0] != TK_CLOSE_BRACE)
    {
        state.error.push_back(CompileError(
            state.line_number, "ERROR: unexpected identifier: '" + token + "'"));

        return false;
    }

    skip_line_feed(iter, state);

    return true;
}

bool function_declaration_list(const std::shared_ptr<FunctionInfo> &funcInfo,
                               std::vector<string>::iterator &iter,
                               SyntaxCheckState &state)
{
    std::cout << "function_declaration_list" << std::endl;

    return open_parenthesis(iter, state) &&
           declaration_list(iter, state) &&
           close_parenthesis(iter, state);
}

bool compound_statement(std::vector<string>::iterator &iter,
                        SyntaxCheckState &state)
{
    std::cout << "compound_statement" << std::endl;

    bool result = open_brace(iter, state) &&
                  return_statement(iter, state) &&
                  close_brace(iter, state);
    if (!result)
    {
        state.error.push_back(CompileError(
            state.line_number, "ERROR: unexpected statement : " + (*iter)));

        return false;
    }

    return result;
}

// function_definition
bool function_definition(std::vector<string>::iterator &iter,
                         SyntaxCheckState &state)
{
    std::cout << "function_definition" << std::endl;

    std::shared_ptr<FunctionInfo> funcInfo(new FunctionInfo);

    bool result = type_specifier(funcInfo, iter, state) &&
                  function_identifier(funcInfo, iter, state) &&
                  function_declaration_list(funcInfo, iter, state);

    if (result)
    {
        state.object_store[funcInfo->name] = funcInfo;
        std::cout << " -> function_definition is successfully" << std::endl;
    }

    result = result && compound_statement(iter, state);

    return result;
}

// Syntax check worker
bool syntax_check_worker(std::vector<string>::iterator &iter,
                         SyntaxCheckState &state)
{

    if (state.scope == Scope::Global)
    {
        bool result = function_definition(iter, state);
    }
    return true;
}

// Syntax check
bool syntax_check(std::vector<string> &tokens)
{

    SyntaxCheckState state;

    auto iter = tokens.begin();
    static int n = 0;
    while (iter != tokens.end())
    {
        std::cout << "============== " << n++ << " ===============" << std::endl;
        syntax_check_worker(iter, state);
        if (state.error.size() > 0)
        {
            for (auto e : state.error)
            {
                std::cout << "line " << e.line_number << ": " << e.message << std::endl;
            }
            return false;
        }
    }

    return true;
}

#endif