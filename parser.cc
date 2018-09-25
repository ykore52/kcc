#include "parser.hh"

namespace kcc
{

Node::Node(NodeType type, std::string syntax) : type(type), syntax(syntax) {}

Parser::Parser(const std::shared_ptr<CompilerState> &compiler_state) : compiler_state(compiler_state)
{
    Init();
}

std::shared_ptr<Node> Parser::SyntaxCheck()
{
    auto root_node = std::shared_ptr<Node>(new Node(kProgram, "/"));

    bool result = FunctionDefinition(root_node);

    if (!result || compiler_state->errors.size() > 0)
    {
        for (auto e : compiler_state->errors)
        {
            std::cout << e.message << std::endl;
        }

        return nullptr;
    }

    return root_node;
}

int Parser::GenerateAssembly(std::shared_ptr<Node> &node, std::string *assembly)
{
    PDEBUG(node->syntax);
    if (node->type == kProgram)
    {
        for (auto child : node->child)
        {
            GenerateAssembly(child, assembly);
        }
    }

    if (node->type == kFuncDefinition)
    {
        for (auto child : node->child)
        {
            PDEBUG(std::to_string(child->type));
            GenerateAssembly(child, assembly);
        }
    }

    if (node->type == kFuncIdentifier)
    {
        if (node->syntax == "main")
        {
            *assembly += ".globl main\n\nmain:\n";
        }
    }

    if (node->type == kFuncCompoundStatement)
    {
        for (auto child : node->child)
        {
            GenerateAssembly(child, assembly);
        }
    }

    if (node->type == kStatementReturn)
    {
        if (node->child[0]->type == kIntegerLiteral) {
            *assembly += "movl    $" + node->child[0]->syntax + " eax\n";
            *assembly += "ret\n";
        }
    }

    return 0;
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
    while (compiler_state->iter != std::end(compiler_state->buf))
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
    PDEBUG(__FUNCTION__);
    auto typeName = *(compiler_state->iter);
    compiler_state->iter++;
    if (compiler_state->type_store.find(typeName) == std::end(compiler_state->type_store))
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
    PDEBUG("out -> " + std::string(__FUNCTION__ ));
    
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
    PDEBUG("out -> " + std::string(__FUNCTION__ ));
    return true;
}

bool Parser::FunctionIdentifier(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    auto identifier = *(compiler_state->iter);

    auto universalName = compiler_state->module_name + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(universalName) != std::end(compiler_state->identifier_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    node->type = kFuncIdentifier;
    node->syntax = identifier;
    compiler_state->identifier_store[universalName] = IdentifierInfo{identifier, compiler_state->module_name};
    PDEBUG("out -> " + std::string(__FUNCTION__ ));

    return true;
}

bool Parser::ReturnStatement(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    if (*compiler_state->iter != "return")
    {
        return false;
    }

    std::shared_ptr<Node> return_statement(new Node(kStatementReturn, "return"));
    return_statement->parent = node;

    ++(compiler_state->iter);
    SkipLF();

    Expression(return_statement);

    SkipLF();

    if (!IsEqual(compiler_state->iter, ';'))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter});
        return false;
    }

    return_statement->type = kStatementReturn;
    return_statement->syntax = "return";
    node->child.push_back(return_statement);
    PDEBUG("out -> " + std::string(__FUNCTION__ ));

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
            if (IsEqual(compiler_state->iter, '}'))
            {               
                ++(compiler_state->iter);
                SkipLF();
                break;
            }

            ReturnStatement(node) && SkipSemicolon();

            SkipLF();


            // if (IsEqual(compiler_state->iter, ',')) {
            //     ++(compiler_state->iter);
            //     SkipLF();
            // } else {
            //     compiler_state->errors.push_back({ compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter });
            //     return false;
            // }
        };
    }

    SkipLF();

    node->type = kFuncCompoundStatement;
    node->syntax = "compound statement";
    PDEBUG("out -> " + std::string(__FUNCTION__ ));

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

    func_node->type = kFuncDefinition;
    func_node->syntax = "function";
    func_node->parent = node;
    node->child.push_back(func_node);
    PDEBUG("out -> " + std::string(__FUNCTION__ ));

    return result;
}

bool Parser::Expression(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);
    
    std::shared_ptr<Node> expression(new Node);
    if ((*compiler_state->iter).compare("\"") == 0)
    {
        // string literal
        node->child.push_back(expression);
        expression->parent = node;
        expression->type = kPrimaryExpression;

        ++(compiler_state->iter);
        return StringLiteral(expression);
    }
    else
    {
        // number literal
        bool is_numeric;
        try
        {
            std::stoi(*compiler_state->iter);
            is_numeric = true;
        }
        catch (const std::invalid_argument &e)
        {
            is_numeric = false;
        }
        catch (const std::out_of_range &e)
        {
            is_numeric = false;
        }


        if (is_numeric)
        {
            node->child.push_back(expression);
            expression->parent = node;
            expression->type = kPrimaryExpression;
            return IntegerLiteral(expression);
        }
    }

    // error
    compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected expression : " + *compiler_state->iter});
    return false;
}

bool Parser::StringLiteral(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);
    std::string strings = *compiler_state->iter;
    ++(compiler_state->iter);

    if (!(*compiler_state->iter).compare("\""))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "The end of '\"' is not found : " + *compiler_state->iter});
        ++(compiler_state->iter);
        return false;
    }

    std::shared_ptr<Node> literal(new Node);
    literal->parent = node;
    literal->type = kStringLiteral;
    literal->syntax = strings;

    node->child.push_back(literal);

    ++(compiler_state->iter);
    PDEBUG("out -> " + std::string(__FUNCTION__ ));
    return true;
}

bool Parser::IntegerLiteral(const std::shared_ptr<Node> &node)
{
    PDEBUG(__FUNCTION__);

    try
    {
        std::stoi(*compiler_state->iter);        
    }
    catch (const std::invalid_argument &e)
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "invalid argument: " + *compiler_state->iter});
        return false;
    }
    catch (const std::out_of_range &e)
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "out of range: " + *compiler_state->iter});
        return false;
    }

    std::shared_ptr<Node> integer(new Node);
    integer->parent = node;
    integer->type = kIntegerLiteral;
    integer->syntax = *compiler_state->iter;

    node->child.push_back(integer);

    ++(compiler_state->iter);
    PDEBUG("out -> " + std::string(__FUNCTION__ ));
    return true;
}

bool Parser::Program(const std::shared_ptr<Node> &node), std::shared_ptr<Program> &program) {

    if (!program) {
        return false;
    }

    program = std::shared_ptr<Program>(new Program);
}

} // namespace kcc