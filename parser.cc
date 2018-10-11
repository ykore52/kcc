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

bool Parser::IsEqual(std::vector<kcc::Token>::iterator &it, char c)
{
    return (it->token.c_str()[0] == c) && (it->token.length() == 1);
}


bool Parser::IsDefinedType(const std::string &str)
{
    return compiler_state->type_store.find(str) != std::end(compiler_state->type_store);
}

bool Parser::IsDefinedVar(const std::string &uid)
{
    return compiler_state->identifier_store.find(uid) != std::end(compiler_state->identifier_store);
}

bool Parser::SkipSemicolon()
{

    DBG_IN(__FUNCTION__);

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
    DBG_IN(__FUNCTION__);
    auto type_name = GetToken().token;
    ShowTokenInfo();

    compiler_state->iter++;
    if (compiler_state->type_store.find(type_name) == std::end(compiler_state->type_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Type name is not defined"});
        return false;
    }

    SkipLF();

    type = std::make_shared<TypeInfo>(compiler_state->type_store[type_name]);

    DBG_OUT(__FUNCTION__);
    return true;
}

// 引数の宣言
bool Parser::MakeArgumentDecl(std::shared_ptr<Argument> &argument)
{
    DBG_IN(__FUNCTION__);
    DBG_OUT(__FUNCTION__);

    return true;
}

// 引数宣言のリスト
bool Parser::MakeArgumentDeclList(ArgumentList &arguments)
{
    DBG_IN(__FUNCTION__);

    if (!IsEqual(compiler_state->iter, '('))
    {
        compiler_state->errors.push_back(
            {compiler_state->module_name,
             compiler_state->line_number,
             "Unexpected syntax : " + GetToken().token});
        return false;
    }

    FwdCursor();
    SkipLF();

    while (!IsEqual(compiler_state->iter, ')'))
    {
        std::shared_ptr<Argument> arg;
        MakeArgumentDecl(arg);
        arguments.push_back(arg);
    }

    FwdCursor();
    SkipLF();
    DBG_OUT(__FUNCTION__);
    return true;
}

// 関数宣言
bool Parser::MakeFunctionIdentifier(std::string &function_identifier)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    auto identifier = GetToken().token;

    auto uid = compiler_state->module_name + "::" + identifier;
    compiler_state->iter++;

    if (compiler_state->identifier_store.find(uid) != std::end(compiler_state->identifier_store))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Function : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    function_identifier = identifier;

    PDEBUG(uid);
    compiler_state->identifier_store[uid] = IdentifierInfo{identifier, compiler_state->module_name};
    DBG_OUT(__FUNCTION__);

    return true;
}

// 変数名(識別子)
bool Parser::MakeVariableIdentifier(const std::string &scope, std::string &var_name)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    auto identifier = GetToken().token;

    auto uid = scope + "::" + identifier;
    compiler_state->iter++;

    if (IsDefinedVar(uid))
    {
        compiler_state->errors.push_back({scope + "::" + compiler_state->module_name, compiler_state->line_number, "Identifier : " + identifier + " is already defined"});
        return false;
    }

    SkipLF();

    var_name = identifier;

    PDEBUG("univ name is : " + uid);
    compiler_state->RegistID(identifier, scope);
    // compiler_state->identifier_store[uid] = IdentifierInfo{identifier, scope + "::" + compiler_state->module_name};
    DBG_OUT(__FUNCTION__);

    return true;
}

bool Parser::MakeAssignmentExpr(std::shared_ptr<AssignmentExpr> &assign_expr)
{
    DBG_IN(__FUNCTION__);

    SkipLF();
    ShowTokenInfo();

    if (GetToken().type == tkOpenParent) {
        assign_expr->expr = std::shared_ptr<PrimaryExpr>(new PrimaryExpr());
        auto prim_expr = std::dynamic_pointer_cast<PrimaryExpr>(assign_expr->expr);
        bool result = MakePrimaryExpr(prim_expr);

        if (GetToken().type != tkCloseParent) {
            compiler_state->errors.push_back({
                compiler_state->scope + "::" + compiler_state->module_name,
                compiler_state->line_number,
                "Unexpected token : " + GetToken().token
            });
        }
        FwdCursor();
    }
    // else if (GetToken()) {
        
    // }

    DBG_OUT(__FUNCTION__);
    return true;
}

// 変数 (+初期化)
bool Parser::MakeInitDecl(const std::shared_ptr<TypeInfo> &type, std::string &var_name, std::shared_ptr<AssignmentExpr> &assign_expr)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

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

    if (GetToken().token == TKN_SEMICOLON)
    {
        return true;
    }

    // 初期化式が付与されている場合は後に続く
    if (GetToken().token != TKN_EQUAL)
    {
        compiler_state->errors.push_back(
            {scope + "::" + compiler_state->module_name,
             compiler_state->line_number,
             "Unexpected token :" + GetToken().token});
    }

    SkipLF(); // skip "=" token
    SkipLF();

    DeclInfo decl_info;
    decl_info.identifier.name = var_name;

    assign_expr = std::shared_ptr<AssignmentExpr>(
        new AssignmentExpr());

    DBG_OUT(__FUNCTION__);
    return MakeAssignmentExpr(assign_expr);
}

// ローカル変数宣言
bool Parser::MakeVariableDecl(std::vector<std::shared_ptr<VariableDecl>> &variables)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    /*
        patterns:
            int a;
            int b = 0;
            int c = 0, d;
            int e, f = 0;
            int g = 0, h = 0;
    */

    auto var_decl = std::shared_ptr<VariableDecl>(new VariableDecl(compiler_state->stack_rel_addr));

    auto assign_expr = std::shared_ptr<AssignmentExpr>(new AssignmentExpr());
    bool ok = MakeTypeDefinition(var_decl->type) &&
                  MakeInitDecl(var_decl->type, var_decl->variable_name, assign_expr);

    // 次の変数のためにスタック相対アドレスを移動しておく
    compiler_state->stack_rel_addr += var_decl->type->size;

    SkipLF();

    if (ok)
    {
        variables.push_back(var_decl);
    }

    while (GetToken().token != ";")
    {

        // カンマ区切りで別の変数が宣言された場合の処理
        if (GetToken().token == ",")
        {
            SkipLF();

            std::shared_ptr<VariableDecl> var_decl2(new VariableDecl(compiler_state->stack_rel_addr));
            var_decl2->type = var_decl->type;

            bool result2 = MakeInitDecl(var_decl->type, var_decl2->variable_name, assign_expr);
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

    DBG_OUT(__FUNCTION__);

    return ok;
}

// return ステートメント
bool Parser::MakeReturnStmt(std::shared_ptr<ReturnStmt> &return_stmt)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    if (GetToken().token != "return")
    {
        return false;
    }

    FwdCursor();
    SkipLF();

    std::shared_ptr<PrimaryExpr> primary_expr;
    bool result = MakePrimaryExpr(primary_expr);

    return_stmt = std::shared_ptr<ReturnStmt>(new ReturnStmt());
    return_stmt->return_expr = std::static_pointer_cast<ExprBase>(primary_expr);

    SkipLF();

    if (!IsEqual(compiler_state->iter, ';'))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + GetToken().token});
        return false;
    }

    DBG_OUT(__FUNCTION__);

    return true;
}

bool MakeExprStmt(std::shared_ptr<ExprBase> &expr)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();
    DBG_OUT(__FUNCTION__);
    return true;
}

bool Parser::MakeCompoundStmt(CompoundStmt &compound_stmt)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    if (!IsEqual(compiler_state->iter, '{'))
    {
        // single expr
    }
    else
    {
        // multiple exprs
        FwdCursor();
        SkipLF();

        while (true)
        {
            ShowTokenInfo();

            // end of compound stmts
            if (GetToken().type == tkCloseBrace)
            {
                FwdCursor();
                SkipLF();
                break;
            }

            if (IsDefinedType(GetToken().token))
            {
                // local variable definition
                std::vector<std::shared_ptr<VariableDecl>> variables;
                MakeVariableDecl(variables) && SkipSemicolon();
                for (auto v : variables)
                    compound_stmt.push_back(v);
                SkipLF();
                continue;
            }

            if (GetToken().type == tkWord)
            {
            }

            if (GetToken().type == tkReturn)
            {
                std::shared_ptr<ReturnStmt> return_stmt;
                MakeReturnStmt(return_stmt) && SkipSemicolon();
                compound_stmt.push_back(return_stmt);
                SkipLF();
                // if (IsEqual(compiler_state->iter, ',')) {
                //     FwdCursor();
                //     SkipLF();
                // } else {
                //     compiler_state->errors.push_back({ compiler_state->module_name, compiler_state->line_number, "Unexpected syntax : " + GetToken().token });
                //     return false;
                // }
            }

            IF_N_RUN(3, { PDEBUG("fuga"); throw_ln("THROW!!");});
        };
    }

    SkipLF();

    DBG_OUT(__FUNCTION__);

    return true;
}

bool Parser::MakeFunctionDefinition(std::shared_ptr<Function> &function)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    function = std::shared_ptr<Function>(new Function());

    bool result = MakeTypeDefinition(function->type) &&
                  MakeFunctionIdentifier(function->function_name) &&
                  MakeArgumentDeclList(function->arguments);

    compiler_state->scope += "::" + function->function_name;

    result &= MakeCompoundStmt(function->stmts);

    DBG_OUT(__FUNCTION__);

    return result;
}

bool Parser::MakePrimaryExpr(std::shared_ptr<PrimaryExpr>& primary_expr)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    std::shared_ptr<Node> expr(new Node);
    if (GetToken().type == tkWord) {
        if (compiler_state->IsDefinedVar(compiler_state->scope + "::" + GetToken().token))
        {
        }
    }
    else if (GetToken().type == tkDoubleQuote)
    {
        // string literal
        FwdCursor();

        primary_expr = std::shared_ptr<PrimaryExpr>(new PrimaryExpr());

        std::shared_ptr<StringLiteral> string_literal;
        bool result = MakeStringLiteral(string_literal);
        if (result)
        {
            auto temp = std::dynamic_pointer_cast<LiteralBase>(string_literal);
            primary_expr = std::shared_ptr<PrimaryExpr>(
                new PrimaryExpr(temp));
            DBG_OUT(__FUNCTION__);
            return true;
        }
    }
    else if (GetToken().type == tkDecimal)
    {
        // number literal
        bool is_numeric;
        try
        {
            std::stoi(GetToken().token);
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
                DBG_OUT(__FUNCTION__);
                return true;
            }
        }
    }

    // error
    compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "Unexpected expr : " + GetToken().token});
    return false;
}

bool Parser::MakeStringLiteral(std::shared_ptr<StringLiteral> &string_literal)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    std::string strings = GetToken().token;
    FwdCursor();

    if (!(GetToken().token).compare("\""))
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "The end of '\"' is not found : " + GetToken().token});
        FwdCursor();
        return false;
    }

    string_literal = std::shared_ptr<StringLiteral>(new StringLiteral(strings));

    FwdCursor();
    DBG_OUT(__FUNCTION__);
    return true;
}

bool Parser::MakeIntegerLiteral(std::shared_ptr<IntegerLiteral> &integer_literal)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    try
    {
        std::stoi(GetToken().token);
    }
    catch (const std::invalid_argument &e)
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "invalid argument: " + GetToken().token});
        return false;
    }
    catch (const std::out_of_range &e)
    {
        compiler_state->errors.push_back({compiler_state->module_name, compiler_state->line_number, "out of range: " + GetToken().token});
        return false;
    }

    integer_literal = std::shared_ptr<IntegerLiteral>(new IntegerLiteral(GetToken().token));

    FwdCursor();
    DBG_OUT(__FUNCTION__);
    return true;
}

bool Parser::MakeProgram(std::shared_ptr<Program> &program)
{
    DBG_IN(__FUNCTION__);
    ShowTokenInfo();

    if (program)
    {
        return false;
    }

    compiler_state->scope = compiler_state->module_name;

    program = std::shared_ptr<Program>(new Program());

    std::shared_ptr<Function> function;

    bool result = MakeFunctionDefinition(function);
    if (result)
    {
        program->decl.push_back(function);
        return true;
    }

    DBG_OUT(__FUNCTION__);
    return false;
}

} // namespace kcc