#include "parser.hh"

namespace kcc
{

Node::Node(NodeType type, std::string syntax) : type(type), syntax(syntax) {}

Parser::Parser(std::string module,
               const std::vector<std::string> &tokens)
    : module_name(module),
      buf(tokens)
{
    iter = buf.begin();
    Init();
}

Parser::Parser(std::string module,
               int line_number,
               const std::vector<std::string> &tokens)
    : module_name(module),
      line_number(line_number),
      buf(tokens)
{
    iter = buf.begin();
    Init();
}

void Parser::Init()
{
    type_store_["char"] = {"char", false};
    type_store_["int"] = {"int", false};
    type_store_["long"] = {"long", false};
    type_store_["float"] = {"float", false};
    type_store_["double"] = {"double", false};
}

bool Parser::IsChar(const std::vector<std::string>::iterator &it, char c)
{
    return (*it->c_str() == c) && (it->length() == 1);
}

bool Parser::SkipSemicolon(CompilerState &state)
{

    PDEBUG(__FUNCTION__);

    if (IsChar(state.iter, ';'))
    {
        state.iter++;
        return true;
    }
    return false;
}

void Parser::SkipLF(CompilerState &state)
{

    while (state.iter != state.buf.end())
    {
        char c = (*state.iter).c_str()[0];
        PDEBUG("char is " + *state.iter + ", " + std::to_string(int(c)).c_str());

        if (IsChar(state.iter, '\n'))
        {

            state.line_number++;
            state.iter++;
        }
        else
        {
            break;
        }
    }
}

bool Parser::TypeDefinition(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(*state.iter);

    auto typeName = *(state.iter);
    state.iter++;
    PDEBUG(__FUNCTION__);
    if (state.type_store_.find(typeName) == state.type_store_.end())
    {
        state.errors.push_back({state.module_name, state.line_number, "Type name is not defined"});
        return false;
    }

    SkipLF(state);

    auto type_node = std::shared_ptr<Node>(new Node);
    type_node->type = kObjectType;
    type_node->syntax = typeName;
    node->child.push_back(type_node);

    PDEBUG("OK");
    return true;
}

bool Parser::ArgumentDeclaration(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);
    PDEBUG("OK");
    return true;
}

bool Parser::ArgumentDeclarationList(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    if (!IsChar(state.iter, '('))
    {
        state.errors.push_back({state.module_name, state.line_number, "Unexpected syntax : '('"});
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

bool Parser::FunctionIdentifier(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    auto identifier = *(state.iter);

    auto universalName = state.module_name + "::" + identifier;
    state.iter++;

    if (state.identifier_store_.find(universalName) != state.identifier_store_.end())
    {
        state.errors.push_back({state.module_name, state.line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF(state);

    node->type = kFuncIdentifier;
    node->syntax = identifier;
    state.identifier_store_[universalName] = IdentifierInfo{identifier, state.module_name};

    PDEBUG("OK");
    return true;
}

bool Parser::ReturnStatement(const std::shared_ptr<Node> &node, CompilerState &state)
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
        state.errors.push_back({state.module_name, state.line_number, "Unexpected syntax : " + *state.iter});
        return false;
    }

    ++(state.iter);
    SkipLF(state);

    if (!IsChar(state.iter, ';'))
    {
        state.errors.push_back({state.module_name, state.line_number, "Unexpected syntax : " + *state.iter});
        return false;
    }

    PDEBUG("OK");
    return true;
}

bool Parser::CompoundStatement(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    if (!IsChar(state.iter, '{'))
    {
        // single expression
    }
    else
    {
        // multiple expressions
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
            //     state.errors.push_back({ state.module_name, state.line_number, "Unexpected syntax : " + *state.iter });
            //     return false;
            // }
        };
    }

    ++(state.iter);
    SkipLF(state);
    PDEBUG("OK");
    return true;
}

bool Parser::FunctionDefinition(const std::shared_ptr<Node> &node, CompilerState &state)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> function(new Node);

    auto iter = state.iter;

    if (TypeDefinition(function, state))
        PDEBUG("OK");
    if (FunctionIdentifier(function, state))
        PDEBUG("OK");
    if (ArgumentDeclarationList(function, state))
        PDEBUG("OK");
    if (CompoundStatement(function, state))
        PDEBUG("OK");

    node->type = kFuncDefinition;

    return true;
}

bool Parser::SyntaxCheck(const std::string module_name, const std::vector<std::string> &tokens)
{
    CompilerState compiler_state(module_name, tokens);

    auto root_node = std::shared_ptr<Node>(new Node(kRoot, "/"));

    FunctionDefinition(root_node, compiler_state);

    if (compiler_state.errors.size() > 0)
    {
        for (auto e : compiler_state.errors)
        {
            std::cout << e.message << std::endl;
        }
    }

    return true;
}

} // namespace kcc