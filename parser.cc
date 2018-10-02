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
    compiler_state->type_store["char"] = {"char", false, 1};
    compiler_state->type_store["int"] = {"int", false, 8};
    compiler_state->type_store["long"] = {"long", false, 8};
    compiler_state->type_store["float"] = {"float", false, 8};
    compiler_state->type_store["double"] = {"double", false, 16};
}

bool Parser::IsEqual(const std::vector<std::string>::iterator &it, char c)
{
    return (*it->c_str() == c) && (it->length() == 1);
}


bool Parser::IsDefinedType(const std::string &str)
{
    return compiler_state->type_store.find(str) != std::end(compiler_state->type_store);
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
// 型定義
bool Parser::MakeTypeDefinition(std::shared_ptr<TypeInfo> &type)
{
    PDEBUG(__FUNCTION__);
    auto type_name = Token();
    compiler_state->iter++;
    if (compiler_state->type_store.find(type_name) == std::end(compiler_state->type_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Type name is not defined"});
        return false;
    }

    SkipLF();

    type = std::make_shared<TypeInfo>(compiler_state->type_store[type_name]);
    return true;
}

// 引数の宣言
bool Parser::MakeArgumentDeclaration(std::shared_ptr<Argument> &argument)
{
    PDEBUG(__FUNCTION__);
    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

// 引数宣言のリスト
bool Parser::MakeArgumentDeclarationList(ArgumentList &arguments)
{
    PDEBUG(__FUNCTION__);

    if (!IsEqual(compiler_state->iter, '('))
    {
        compiler_state->errors.push_back(
            {compiler_state->module_name,
             compiler_state->line_number,
             "Unexpected syntax : " + *compiler_state->iter});
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

// 関数宣言
bool Parser::MakeFunctionIdentifier(std::string &function_identifier)
{
    PDEBUG(__FUNCTION__);

    auto identifier = Token();

    auto universal_name = compiler_state->module_name + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(universal_name) != std::end(compiler_state->identifier_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    function_identifier = identifier;
    compiler_state->scope += identifier;

    PDEBUG(universal_name);
    compiler_state->identifier_store[universal_name] = IdentifierInfo{identifier, compiler_state->module_name};
    PDEBUG("out -> " + std::string(__FUNCTION__));

    return true;
}

// 変数名(識別子)
bool Parser::MakeVariableIdentifier(const std::string &scope, std::string &var_name)
{
    PDEBUG(__FUNCTION__);

    auto identifier = Token();

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

bool Parser::MakeAssignmentExpr(std::shared_ptr<AssignmentExpr> &assign_expr)
{
    SkipLF();

    if (Token() == TKN_OPEN_PARENTHESIS) {
        assign_expr->expr = std::shared_ptr<PrimaryExpr>(new PrimaryExpr());
        bool result = MakePrimaryExpr(std::dynamic_pointer_cast<PrimaryExpr>(assign_expr->expr));

        if (Token() != TKN_CLOSE_PARENTHESIS) {
            compiler_state->errors.push_back({
                compiler_state->scope + "::" + compiler_state->module_name,
                compiler_state->line_number,
                "Unexpected token : " + Token()
            });
        }
        FwdCursor();
    }
    // else if (Token()) {
        
    // }

    return true;
}

// 変数 (+初期化)
bool Parser::MakeInitDeclarator(const std::shared_ptr<TypeInfo> &type, std::string &var_name, std::shared_ptr<AssignmentExpr> &assign_expr)
{
    std::string scope = compiler_state->scope;
    bool result = MakeVariableIdentifier(scope, var_name);
    if (!result)
    {
        compiler_state->errors.push_back(
            {scope + "::" + compiler_state->module_name,
             compiler_state->line_number,
             "Identifier : " + var_name + " is already defined"});
    }
    SkipLF();

    if (Token() == TKN_SEMICOLON)
    {
        return true;
    }

    // 初期化式が付与されている場合は後に続く
    if (Token() != TKN_EQUAL)
    {
        compiler_state->errors.push_back(
            {scope + "::" + compiler_state->module_name,
             compiler_state->line_number,
             "Unexpected token :" + Token()});
    }

    SkipLF(); // skip "=" token
    SkipLF();

    DeclInfo decl_info;
    decl_info.identifier.name = var_name;

    assign_expr = std::shared_ptr<AssignmentExpr>(
        new AssignmentExpr());
    return MakeAssignmentExpr(assign_expr);
}

// ローカル変数宣言
bool Parser::MakeVariableDeclaration(std::vector<std::shared_ptr<VariableDeclaration>> &variables)
{
    /*
        patterns:
            int a;
            int b = 0;
            int c = 0, d;
            int e, f = 0;
            int g = 0, h = 0;
    */

    auto var_decl = std::shared_ptr<VariableDeclaration>(new VariableDeclaration(compiler_state->stack_rel_addr));

    auto assign_expr = std::shared_ptr<AssignmentExpr>(new AssignmentExpr());
    bool result = MakeTypeDefinition(var_decl->type) &&
                  MakeInitDeclarator(var_decl->type, var_decl->variable_name, assign_expr);

    // 次の変数のためにスタック相対アドレスを移動しておく
    compiler_state->stack_rel_addr += var_decl->type->size;

    SkipLF();

    if (result)
    {
        variables.push_back(var_decl);
    }

    while (Token() != ";")
    {

        // カンマ区切りで別の変数が宣言された場合の処理
        if (Token() == ",")
        {
            SkipLF();

            std::shared_ptr<VariableDeclaration> var_decl2(new VariableDeclaration(compiler_state->stack_rel_addr));
            var_decl2->type = var_decl->type;

            bool result2 = MakeInitDeclarator(var_decl->type, var_decl2->variable_name, assign_expr);
            if (!result2)
            {
                return false;
            }

            variables.push_back(var_decl);
            // 次の変数のためにスタック相対アドレスを移動しておく
            compiler_state->stack_rel_addr += var_decl2->type->size;
        }
        else
        {
            break; // 多分 false
        }
    }
    return result;
}

// return ステートメント
bool Parser::MakeReturnStatement(std::shared_ptr<ReturnStatement> &return_statement)
{
    PDEBUG(__FUNCTION__);

    if (*compiler_state->iter != "return")
    {
        return false;
    }

    ++(compiler_state->iter);
    SkipLF();

    std::shared_ptr<PrimaryExpr> primary_expr;
    bool result = MakePrimaryExpr(primary_expr);

    return_statement = std::shared_ptr<ReturnStatement>(new ReturnStatement());
    return_statement->return_expr = std::static_pointer_cast<ExprBase>(primary_expr);

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
        // single expr
    }
    else
    {
        // multiple exprs
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

            if (IsDefinedType(*compiler_state->iter))
            {
                // local variable definition
                std::vector<std::shared_ptr<VariableDeclaration>> variables;
                MakeVariableDeclaration(variables) && SkipSemicolon();
                for (auto v : variables)
                    compound_statement.push_back(v);
                SkipLF();
            }
            else
            {
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
    result &= MakeCompoundStatement(function->statements);

    PDEBUG("out -> " + std::string(__FUNCTION__));

    return result;
}

bool Parser::MakePrimaryExpr(std::shared_ptr<PrimaryExpr> &primary_expr)
{
    PDEBUG(__FUNCTION__);

    std::shared_ptr<Node> expr(new Node);
    if ((*compiler_state->iter).compare("\"") == 0)
    {
        // string literal
        ++(compiler_state->iter);

        primary_expr = std::shared_ptr<PrimaryExpr>(new PrimaryExpr());

        std::shared_ptr<StringLiteral> string_literal;
        bool result = MakeStringLiteral(string_literal);
        if (result)
        {
            auto temp = std::dynamic_pointer_cast<LiteralBase>(string_literal);
            primary_expr = std::shared_ptr<PrimaryExpr>(
                new PrimaryExpr(temp));
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
                primary_expr = std::shared_ptr<PrimaryExpr>(new PrimaryExpr(temp));
                PDEBUG("out -> " + std::string(__FUNCTION__));
                return true;
            }
        }
    }

    // error
    compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected expr : " + *compiler_state->iter});
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
    if (result)
    {
        program->decl.push_back(function);
        return true;
    }

    return false;
}

} // namespace kcc