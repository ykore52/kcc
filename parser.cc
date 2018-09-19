#include "parser.hh"

namespace kcc
{

Node::Node(NodeType type, std::string syntax) : type(type), syntax(syntax) {}

Parser::Parser(const std::shared_ptr<CompilerState> &compiler_state) : compiler_state(compiler_state)
{
    Init();
}

void Parser::Init()
{
    compiler_state->type_store["char"] = {"char", false};
    compiler_state->type_store["int"] = {"int", false};
    compiler_state->type_store["long"] = {"long", false};
    compiler_state->type_store["float"] = {"float", false};
    compiler_state->type_store["double"] = {"double", false};
}

bool Parser::IsEqual(const std::vector<std::string>::iterator &it, char c)
{
    return (*it->c_str() == c) && (it->length() == 1);
}

bool Parser::SkipSemicolon()
{

    PDEBUG(__FUNCTION__);

    if (IsEqual(compiler_state->iter, ';'))
    {
        compiler_state->iter++;
        return true;
    }
    return false;
}

void Parser::SkipLF()
{
    while (compiler_state->iter != compiler_state->buf.end())
    {
        if (IsEqual(compiler_state->iter, '\n'))
        {
            compiler_state->line_number++;
            compiler_state->iter++;
        }
        else
        {
            break;
        }
    }
}

bool Parser::TypeDefinition(const std::shared_ptr<Node> &node)
{
    PDEBUG(*compiler_state->iter);

    auto typeName = *(compiler_state->iter);
    compiler_state->iter++;
    PDEBUG(__FUNCTION__);
    if (compiler_state->type_store.find(typeName) == compiler_state->type_store.end())
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Type name is not defined"});
        return false;
    }

    SkipLF();

    auto type_node = std::shared_ptr<Node>(new Node);
    type_node->type = kObjectType;
    type_node->syntax = typeName;
    node->child.push_back(type_node);

    // PDEBUG("OK");
    return true;
}

bool Parser::ArgumentDeclaration(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);
    // PDEBUG("OK");
    return true;
}

bool Parser::ArgumentDeclarationList(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    if (!IsEqual(compiler_state->iter, '('))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : '('"});
        return false;
    }

    ++(compiler_state->iter);
    SkipLF();

    while (!IsEqual(compiler_state->iter, ')'))
    {
        ArgumentDeclaration(node);
    }

    ++(compiler_state->iter);
    SkipLF();

    // PDEBUG("OK");
    return true;
}

bool Parser::FunctionIdentifier(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    auto identifier = *(compiler_state->iter);

    auto universalName = compiler_state->module_name + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(universalName) != compiler_state->identifier_store.end())
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    node->type = kFuncIdentifier;
    node->syntax = identifier;
    compiler_state->identifier_store[universalName] = IdentifierInfo{identifier, compiler_state->module_name};

    // PDEBUG("OK");
    return true;
}

bool Parser::ReturnStatement(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    if (*compiler_state->iter != "return")
    {
        return false;
    }

    ++(compiler_state->iter);
    SkipLF();

    // TODO: overwrite to evaluate any expressions
    if (*compiler_state->iter != "2")
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter});
        return false;
    }

    ++(compiler_state->iter);
    SkipLF();

    if (!IsEqual(compiler_state->iter, ';'))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter});
        return false;
    }

    // PDEBUG("OK");
    return true;
}

bool Parser::CompoundStatement(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    if (!IsEqual(compiler_state->iter, '{'))
    {
        // single expression
    }
    else
    {
        // multiple expressions
        ++(compiler_state->iter);
        SkipLF();

        while (true)
        {

            ReturnStatement(node) && SkipSemicolon();

            SkipLF();

            if (IsEqual(compiler_state->iter, '}'))
            {
                ++(compiler_state->iter);
                SkipLF();
                break;
            }

            // if (IsEqual(compiler_state->iter, ',')) {
            //     ++(compiler_state->iter);
            //     SkipLF();
            // } else {
            //     compiler_state->errors.push_back({ compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter });
            //     return false;
            // }
        };
    }

    ++(compiler_state->iter);
    SkipLF();
    // PDEBUG("OK");
    return true;
}

bool Parser::FunctionDefinition(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> func_node(new Node);

    if (TypeDefinition(func_node))
        PDEBUG("OK");
    if (FunctionIdentifier(func_node))
        PDEBUG("OK");
    if (ArgumentDeclarationList(func_node))
        PDEBUG("OK");
    if (CompoundStatement(func_node))
        PDEBUG("OK");

    node->type = kFuncDefinition;

    return true;
}

bool Parser::SyntaxCheck()
{
    auto root_node = std::shared_ptr<Node>(new Node(kRoot, "/"));

    FunctionDefinition(root_node);

    if (compiler_state->errors.size() > 0)
    {
        for (auto e : compiler_state->errors)
        {
            std::cout << e.message << std::endl;
        }
    }

    return true;
}

} // namespace kcc