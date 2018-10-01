#include "parser.hh"

namespace kcc
{

Node::Node(NodeType type, std::string syntax) : type(type), syntax(syntax) {}

Parser::Parser(const std::shared_ptr<CompilerState> &compiler_state) : compiler_state(compiler_state)
{
    Init();
}

std::shared_ptr<Program> Parser::SyntaxCheck()
{
    std::shared_ptr<Program> prog;
    bool result = MakeProgram(prog);

    if (!result || compiler_state->errors.size() > 0)
    {
        for (auto e : compiler_state->errors)
        {
            std::cout << e.message << std::endl;
        }

        return nullptr;
    }

    return prog;
}

int Parser::GenerateAssembly(std::shared_ptr<Program> &node, std::string *assembly)
{
    *assembly = node->Assemble(compiler_state->asm_config);
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

bool Parser::IsDefinedType(const std::string &str)
{
    return compiler_state->type_store[str] != compiler_state->type_store.end();
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

// -------------------------------------------------------------------------------

bool Parser::MakeTypeDefinition(std::shared_ptr<TypeInfo> &type)
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

    type = std::shared_ptr<TypeInfo>(new TypeInfo{typeName, false, std::map<std::string, std::shared_ptr<TypeInfo>>()});
    return true;
}

bool Parser::MakeArgumentDeclaration(std::shared_ptr<Argument> &argument)
{
    PDEBUG(__FUNCTION__);
    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

bool Parser::MakeArgumentDeclarationList(ArgumentList &arguments)
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
        std::shared_ptr<Argument> arg;
        MakeArgumentDeclaration(arg);
        arguments.push_back(arg);
    }

    ++(compiler_state->iter);
    SkipLF();
    PDEBUG("out -> " + std::string(__FUNCTION__));
    return true;
}

bool Parser::MakeFunctionIdentifier(std::string &function_identifier)
{
    PDEBUG(__FUNCTION__);

    compiler_state->scope += 

    auto identifier = *(compiler_state->iter);

    auto universal_name = compiler_state->module_name + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(universal_name) != std::end(compiler_state->identifier_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    function_identifier = identifier;

    PDEBUG(universal_name);
    compiler_state->identifier_store[universal_name] = IdentifierInfo{identifier, compiler_state->module_name};
    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

bool Parser::MakeVariableIdentifier(const std::string &scope, std::string &var_name)
{
    PDEBUG(__FUNCTION__);

    auto identifier = *(compiler_state->iter);

    auto universal_name = compiler_state->module_name + "::" + scope + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(universal_name) != std::end(compiler_state->identifier_store))
    {
        compiler_state->errors.push_back({scope + "::" + compiler_state->module_name, compiler_state->line_number, "Identifier : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    var_name = identifier;

    PDEBUG(universal_name);
    compiler_state->identifier_store[universal_name] = IdentifierInfo{identifier, scope + "::" + compiler_state->module_name};
    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

bool Parser::MakeInitDeclarator(std::string &var_name, std::shared_ptr<AssignmentExpression> &assign_expr)
{
    std::string scope = compiler_state->scope;
    bool result = MakeVariableIdentifier(scope, var_name);
    if (!result) {
        compiler_state->errors.push_back({scope + "::" + compiler_state->module_name, compiler_state->line_number, "Identifier : " + identifier + " is already defined"});
    }
    SkipLF();
    
}

bool Parser::MakeVariableDeclaration(std::shared_ptr<VariableDeclaration> &var_decl)
{
    /*
        patterns:
            int a;
            int b = 0;
            int c = 0, d;
            int e, f = 0;
            int g = 0, h = 0;
    */

    
    bool result = MakeTypeDefinition(var_decl->type) &&
                  MakeInitDeclarator(var_decl->variable_name);
    SkipLF();
    if(*compiler_state->iter.compare(",")) {
        std::shared_ptr<VariableDeclaration> var(new VariableDeclaration());
        var->type = var_decl->type;
        bool result2 = MakeInitDeclarator(var);
    }
    
    variable = std::shared_ptr<VariableDeclaration> variable(new VariableDeclaration());
    return true;
}


bool Parser::MakeReturnStatement(std::shared_ptr<ReturnStatement> &return_statement)
{
    PDEBUG(__FUNCTION__);

    if (*compiler_state->iter != "return")
    {
        return false;
    }

    ++(compiler_state->iter);
    SkipLF();

    std::shared_ptr<PrimaryExpression> primary_expression;
    bool result = MakeExpression(primary_expression);

    return_statement = std::shared_ptr<ReturnStatement>(new ReturnStatement());
    return_statement->return_expression = std::static_pointer_cast<ExpressionBase>(primary_expression);

    SkipLF();

    if (!IsEqual(compiler_state->iter, ';'))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter});
        return false;
    }

    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

bool Parser::MakeCompoundStatement(CompoundStatement &compound_statement)
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

            if (IsDefinedType(*compiler_state->iter)) {

                // local variable definition
                std::shared_ptr<VariableDeclaration> variable_decl;
                MakeVariableDeclaration(variable_decl) && SkipSemicolon();
                compound_statement.push_back(variable_decl);
                SkipLF();

            } else {
                std::shared_ptr<ReturnStatement> return_statement;
                MakeReturnStatement(return_statement) && SkipSemicolon();
                compound_statement.push_back(return_statement);
                SkipLF();

                // if (IsEqual(compiler_state->iter, ',')) {
                //     ++(compiler_state->iter);
                //     SkipLF();
                // } else {
                //     compiler_state->errors.push_back({ compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + *compiler_state->iter });
                //     return false;
                // }
            }
        };
    }

    SkipLF();

    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

bool Parser::MakeFunctionDefinition(std::shared_ptr<Function> &function)
{
    PDEBUG(__FUNCTION__);

    function = std::shared_ptr<Function>(new Function());

    bool result = MakeTypeDefinition(function->type) &&
                  MakeFunctionIdentifier(function->function_name) &&
                  MakeArgumentDeclarationList(function->arguments);

    compiler_state->scope += function->function_name;
    result &&= MakeCompoundStatement(function->statements);

    PDEBUG("out -> " + std::string(__FUNCTION__));

    return result;
}

bool Parser::MakeExpression(std::shared_ptr<PrimaryExpression> &primary_expression)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> expression(new Node);
    if ((*compiler_state->iter).compare("\"") == 0)
    {
        // string literal
        ++(compiler_state->iter);

        primary_expression = std::shared_ptr<PrimaryExpression>(new PrimaryExpression());

        std::shared_ptr<StringLiteral> string_literal;
        bool result = MakeStringLiteral(string_literal);
        if (result)
        {
            auto temp = std::dynamic_pointer_cast<LiteralBase>(string_literal);
            primary_expression = std::shared_ptr<PrimaryExpression>(
                new PrimaryExpression(temp)
            );
            PDEBUG("out -> " + std::string(__FUNCTION__));
            return true;
        }
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
            std::shared_ptr<IntegerLiteral> integer_literal;
            bool result = MakeIntegerLiteral(integer_literal);
            if (result)
            {               
                auto temp = std::dynamic_pointer_cast<LiteralBase>(integer_literal);
                primary_expression = std::shared_ptr<PrimaryExpression>(new PrimaryExpression(temp));
                PDEBUG("out -> " + std::string(__FUNCTION__));
                return true;
            }
        }
    }

    // error
    compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected expression : " + *compiler_state->iter});
    return false;
}

bool Parser::MakeStringLiteral(std::shared_ptr<StringLiteral> &string_literal)
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

    string_literal = std::shared_ptr<StringLiteral>(new StringLiteral(strings));

    ++(compiler_state->iter);
    PDEBUG("out -> " + std::string(__FUNCTION__));
    return true;
}

bool Parser::MakeIntegerLiteral(std::shared_ptr<IntegerLiteral> &integer_literal)
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

    integer_literal = std::shared_ptr<IntegerLiteral>(new IntegerLiteral(*compiler_state->iter));

    ++(compiler_state->iter);
    PDEBUG("out -> " + std::string(__FUNCTION__));
    return true;
}

bool Parser::MakeProgram(std::shared_ptr<Program> &program)
{
    if (program)
    {
        return false;
    }

    compiler_state->scope = compiler_state->module_name + "::";

    program = std::shared_ptr<Program>(new Program());

    std::shared_ptr<Function> function;

    bool result = MakeFunctionDefinition(function);
    if (result) {
        program->decl.push_back(function);
        return true;
    }
    
    return false;
}

} // namespace kcc