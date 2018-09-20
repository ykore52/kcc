#include "parser.hh"

namespace kcc
{

Node::Node(NodeType type, std::string syntax) : type(type), syntax(syntax) {}

Parser::Parser(const std::shared_ptr<CompilerState> &compiler_state) : compiler_state(compiler_state)
{
    Init();
}

bool Parser::SyntaxCheck(std::shared_ptr<Node> &root_node)
{
    auto root_node = std::shared_ptr<Node>(new Node(kRoot, "/"));

    bool result = FunctionDefinition(root_node);

    if (!result || compiler_state->errors.size() > 0)
    {
        for (auto e : compiler_state->errors)
        {
            std::cout << e.message << std::endl;
        }

        return false;
    }

    return true;
}

int Parser::GenerateAssembly(const std::shared_ptr<Node> &ast_root, std::string *assembly)
{
    auto node = ast_root;
    while (true) {

    }
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

    auto child = std::shared_ptr<Node>(new Node);
    child->type = kObjectType;
    child->syntax = typeName;
    node->child.push_back(child);

    
    return true;
}

bool Parser::ArgumentDeclaration(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);
    
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

    // TODO: To evaluate any expressions
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
    
    return true;
}

bool Parser::FunctionDefinition(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> func_node(new Node);

    bool result = TypeDefinition(func_node) &&
                  FunctionIdentifier(func_node) &&
                  ArgumentDeclarationList(func_node) &&
                  CompoundStatement(func_node);

    node->type = kFuncDefinition;

    return result;
}

} // namespace kcc