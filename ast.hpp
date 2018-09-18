#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <map>

#define PDEBUG(message)                    \
    do                                     \
    {                                      \
        std::cout << message << std::endl; \
    } while (0)
namespace AST
{

enum NodeType
{
    eRoot,

    eObjectType,
    eFuncDefinition,
    eFuncStorageClassSpecifier, // auto, register, static, extern, typedef, __decpspec(ms-specific)
    eFuncIdentifier,
};

struct CompileErrorInfo
{
    std::string moduleName;
    int lineNumber;
    std::string message;
};

struct TypeInfo
{
    std::string name;
    bool isPointer;
    std::map<std::string, TypeInfo> member;
};

struct IdentifierInfo
{
    std::string name;
    std::string moduleName;
};

struct CompilerState
{
    CompilerState()
    {
        this->lineNumber = 0;
        this->iter = buf.begin();

        typeStore["char"] = {"char", false};
        typeStore["int"] = {"int", false};
        typeStore["long"] = {"long", false};
        typeStore["float"] = {"float", false};
        typeStore["double"] = {"double", false};
    }

    CompilerState(std::string moduleName)
    {
        CompilerState();
        this->moduleName = moduleName;
    }

    CompilerState(std::string moduleName, int lineNumber, const std::vector<std::string> &tokens)
    {
        this->moduleName = moduleName;
        this->lineNumber = lineNumber;
        this->buf = tokens;
        this->iter = this->buf.begin();

        typeStore["char"] = {"char", false};
        typeStore["int"] = {"int", false};
        typeStore["long"] = {"long", false};
        typeStore["float"] = {"float", false};
        typeStore["double"] = {"double", false};
    }

    std::string moduleName;
    int lineNumber;

    std::vector<std::string> buf;
    std::vector<std::string>::iterator iter;

    std::map<std::string, TypeInfo> typeStore;
    std::map<std::string, IdentifierInfo> identifierStore;

    std::vector<CompileErrorInfo> errors;
};

struct Node
{
    Node() {}
    Node(NodeType type, std::string syntax)
    {
        this->type = type;
        this->syntax = syntax;
    }

    NodeType type;
    std::string syntax;

    std::shared_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> child;
};

bool IsChar(const std::vector<std::string>::iterator &it, char c)
{
    return (*it->c_str() == c) && (it->length() == 1);
}

bool SkipSemicolon(CompilerState &state)
{

    PDEBUG(__FUNCTION__);

    if (IsChar(state.iter, ';'))
    {
        state.iter++;
        return true;
    }
    return false;
}

void SkipLF(CompilerState &state)
{

    while (state.iter != state.buf.end())
    {
        PDEBUG( "char is " + *state.iter );
        if (IsChar(state.iter, '\n'))
        {
            PDEBUG(__FUNCTION__);
            state.lineNumber++;

    PDEBUG(std::string("fuga") + *state.iter);

            if (state.iter == state.buf.end()) break;
            state.iter++;
        }
        else
        {
            break;
        }
    }
}

bool TypeDefinition(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    auto typeName = *(state.iter);
    state.iter++;
    if (state.typeStore.find(typeName) == state.typeStore.end())
    {
        state.errors.push_back({state.moduleName, state.lineNumber, "Type name is not defined"});
        return false;
    }

    SkipLF(state);

    auto type = std::shared_ptr<Node>(new Node);
    type->type = eObjectType;
    type->syntax = typeName;
    node->child.push_back(type);
    PDEBUG("OK");
    return true;
}

bool ArgumentDeclaration(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);
    PDEBUG("OK");
    return true;
}

bool ArgumentDeclarationList(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);
    if (!IsChar(state.iter, '('))
    {
        state.errors.push_back({state.moduleName, state.lineNumber, "Unexpected syntax : '('"});
        return false;
    }

    ++(state.iter);
    SkipLF(state);

    while (!IsChar(state.iter, ')'))
    {
        ArgumentDeclaration(node, state);
    }

    ++(state.iter);
    SkipLF(state);

    PDEBUG("OK");
    return true;
}

bool FunctionIdentifier(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    auto identifier = *(state.iter);

    auto universalName = state.moduleName + "::" + identifier;
    state.iter++;

    if (state.identifierStore.find(universalName) != state.identifierStore.end())
    {
        state.errors.push_back({state.moduleName, state.lineNumber, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF(state);

    node->type = eFuncIdentifier;
    node->syntax = identifier;
    auto id = IdentifierInfo{identifier, state.moduleName};
    state.identifierStore[universalName] = id;
    PDEBUG("OK");
    return true;
}

bool ReturnStatement(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);
    PDEBUG(*state.iter);
    if (*state.iter != "return")
    {
        return false;
    }

    ++(state.iter);
    SkipLF(state);

    // TODO: overwrite to evaluate any expressions
    if (*state.iter != "2")
    {
        state.errors.push_back({state.moduleName, state.lineNumber, "Unexpected syntax : " + *state.iter});
        return false;
    }

    ++(state.iter);
    SkipLF(state);

    if (!IsChar(state.iter, ';'))
    {
        state.errors.push_back({state.moduleName, state.lineNumber, "Unexpected syntax : " + *state.iter});
        return false;
    }
    PDEBUG("OK");
    return true;
}

bool CompoundStatement(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    if (!IsChar(state.iter, '{'))
    {
        // single expression
    }
    else
    {
        // 2 or more expressions
        ++(state.iter);
        SkipLF(state);

        while (true)
        {

            ReturnStatement(node, state) && SkipSemicolon(state);

            SkipLF(state);

            if (IsChar(state.iter, '}'))
            {
                ++(state.iter);
                SkipLF(state);
                break;
            }

            // if (IsChar(state.iter, ',')) {
            //     ++(state.iter);
            //     SkipLF(state);
            // } else {
            //     state.errors.push_back({ state.moduleName, state.lineNumber, "Unexpected syntax : " + *state.iter });
            //     return false;
            // }
        };
    }

    ++(state.iter);
    SkipLF(state);
    PDEBUG("OK");
    return true;
}

int FunctionDefinition(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> function(new Node);

    auto iter = state.iter;

    TypeDefinition(function, state);
    FunctionIdentifier(function, state);
    ArgumentDeclarationList(function, state);
    CompoundStatement(function, state);

    node->type = eFuncDefinition;

    return true;
}

} // namespace AST

#endif
