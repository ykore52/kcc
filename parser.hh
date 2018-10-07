#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "util.hh"
#include "assembler.hh"
#include "tokenizer.hh"

namespace kcc
{

enum NodeType
{
    kProgram,
    kDeclaration,
    kObjectType,

    kIntegerLiteral,
    kStringLiteral,
    kDeclRefExpr,

    kVariableDeclaration,

    kFuncDefinition,
    kFuncStorageClassSpecifier, // auto, register, static, extern, typedef,
                                // __decpspec(ms-specific)
    kFuncIdentifier,
    kFuncCompoundStatement,
    kFuncParamList,
    kFuncParam,
    kStatementReturn,

    kPrimaryExpr,
    kBinaryExpr,
    kAssignmentExpr
};

enum OperatorType
{
    kPlus,  // +
    kMinus, // -
    kMul,   // *
    kDiv,   // /
};

#define TOKEN(name, token) static const char *name = token
TOKEN(TKN_OPEN_PARENTHESIS, "(");
TOKEN(TKN_CLOSE_PARENTHESIS, ")");
TOKEN(TKN_OPEN_BRACE, "{");
TOKEN(TKN_CLOSE_BRACE, "}");
TOKEN(TKN_SEMICOLON, ";");

TOKEN(TKN_EQUAL, "=");
TOKEN(TKN_PLUS, "+");
TOKEN(TKN_MINUS, "-");
TOKEN(TKN_ASTERISK, "*");
TOKEN(TKN_SLASH, "/");
TOKEN(TKN_PERCENT, "%");
TOKEN(TKN_QUOTE, "'");
TOKEN(TKN_DOUBLEQUOTE, "\"");

// コンパイルエラー情報
struct CompileErrorInfo
{
    std::string module_name;
    int line_number;
    std::string message;
};

// 型情報
struct TypeInfo
{
    std::string type_name;
    bool is_pointer;
    unsigned int size; // バイト数
    std::map<std::string, std::shared_ptr<TypeInfo>> member;
};

// 識別子情報
struct IdentifierInfo
{
    std::string name;
    std::string module_name;
    std::string scope;
    unsigned int address;
};

struct DeclInfo
{
    TypeInfo type;
    IdentifierInfo identifier;

    DeclInfo() {}
    DeclInfo(const TypeInfo &type, const IdentifierInfo &identifier)
        : type(type), identifier(identifier) {}

    std::string Name() noexcept
    {
        return identifier.name;
    }

    std::string Module() noexcept
    {
        return identifier.module_name;
    }

    std::string Scope() noexcept
    {
        return identifier.scope;
    }

    unsigned int Address() noexcept
    {
        return identifier.address;
    }

    std::string TypeName() noexcept
    {
        return type.type_name;
    }

    unsigned int Size() noexcept
    {
        return type.size;
    }
};

// AST のノードとなるベースクラス
struct ASTNode
{
    ASTNode() {}
    ASTNode(const NodeType t) : node_type(t) {}
    virtual ~ASTNode() {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "dummy"; }
    virtual void Stdout() { std::cout << "ASTNode" << std::endl; }

    NodeType node_type;
};

struct LiteralBase : public ASTNode
{
    LiteralBase(NodeType t, std::string value) : ASTNode(t), value(value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "LiteralBase"; }
    virtual void Stdout() {}
    std::string value;
};

// 整数リテラル
struct IntegerLiteral : public LiteralBase
{
    IntegerLiteral(std::string value) : LiteralBase(kIntegerLiteral, value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return value; }
    virtual void Stdout() {}
};

// 文字列リテラル
struct StringLiteral : public LiteralBase
{
    StringLiteral(std::string value) : LiteralBase(kStringLiteral, value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "\"" + value + "\""; }
    virtual void Stdout() {}
};

// 変数参照
struct DeclRefExpr : public LiteralBase
{
    DeclRefExpr(std::shared_ptr<DeclInfo> &decl) : LiteralBase(kDeclRefExpr, ""), decl(decl) {}
    virtual std::string Assemble(AssemblyConfig &conf)
    {
        std::string type;
        switch (decl->Size())
        {
        case 1:
            type = "BYTE PTR";
            break;
        case 2:
            type = "WORD PTR";
            break;
        case 4:
            type = "DWORD PTR";
            break;
        case 8:
            type = "QWORD PTR";
            break;
        }

        return type + "[rbp-" + std::to_string(decl->Address()) + "]";
    }

    std::shared_ptr<DeclInfo> decl;
};

// ------------------------------------------------
struct ExprBase : public ASTNode
{
    ExprBase(NodeType t) : ASTNode(t) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "ExprBase"; }
    virtual void Stdout() {}
    // child expr
    std::shared_ptr<ExprBase> expr;
    TypeInfo type_of_expr;
};

// 代入式(右辺値)
struct AssignmentExpr : public ExprBase
{
    AssignmentExpr() : ExprBase(kAssignmentExpr) {}
    AssignmentExpr(std::shared_ptr<DeclRefExpr> destination) : ExprBase(kAssignmentExpr), destination(destination) {}

    virtual std::string Assemble(AssemblyConfig &conf)
    {
        std::string result = "";
        result += conf.asm_.Asm("push", {"rax"});

        expr->Assemble(conf);

        result += conf.asm_.Asm("pop", {"rax"});

        return "";
    }

    virtual void Stdout() {}

    std::shared_ptr<DeclRefExpr> destination; // output
    std::shared_ptr<ExprBase> expr;
};

// 二項演算式
struct BinaryExpr : public ExprBase
{
    BinaryExpr() : ExprBase(kBinaryExpr) {}
    BinaryExpr(std::shared_ptr<ExprBase> &first,
               std::shared_ptr<ExprBase> &second,
               OperatorType operator_type) : ExprBase(kBinaryExpr), first(first), second(second), op_type(operator_type) {}

    virtual std::string Assemble(AssemblyConfig &conf)
    {
        return "";
    }

    virtual void Stdout()
    {
    }

    std::shared_ptr<ExprBase> first;
    std::shared_ptr<ExprBase> second;

    OperatorType op_type;
};

// 一次式
struct PrimaryExpr : public ExprBase
{
    PrimaryExpr() : ExprBase(kPrimaryExpr) {}
    PrimaryExpr(std::shared_ptr<LiteralBase> &literal)
        : ExprBase(kPrimaryExpr), literal(literal) {}
    virtual std::string Assemble(AssemblyConfig &conf)
    {
        return conf.asm_.Asm("mov", {"rax", literal->value});
    }
    virtual void Stdout() {}

    std::shared_ptr<LiteralBase> literal;

    OperatorType op_type;
};

// ------------------------------------------------
struct DeclarationAndStatement : public ASTNode
{
    DeclarationAndStatement(NodeType t) : ASTNode(t) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "DeclarationAndStatement"; }
    virtual void Stdout() {}
};

struct VariableDeclaration : public DeclarationAndStatement
{
    VariableDeclaration() : DeclarationAndStatement(kVariableDeclaration) {}
    VariableDeclaration(int stack_rel_addr) : stack_rel_addr(stack_rel_addr), DeclarationAndStatement(kVariableDeclaration) {}

    std::shared_ptr<TypeInfo> type;
    std::string variable_name;
    // std::string storage_class;
    // std::string type_qualifier;
    int stack_rel_addr = 0;

    std::string Assemble(AssemblyConfig &conf) override
    {
        std::string code = "";
        return "";
    }
};

struct ReturnStatement : public DeclarationAndStatement
{
    ReturnStatement() : DeclarationAndStatement(kStatementReturn) {}
    ReturnStatement(const std::shared_ptr<ExprBase> &e)
        : DeclarationAndStatement(kStatementReturn), return_expr(e) {}

    virtual std::string Assemble(AssemblyConfig &conf) override
    {
        std::string code = "";
        code += return_expr->Assemble(conf);
        return code;
    }
    virtual void Stdout() override {}

    std::shared_ptr<ExprBase> return_expr;
};

struct IfStatement : public DeclarationAndStatement
{
};
struct ForStatement : public DeclarationAndStatement
{
};
struct WhileStatement : public DeclarationAndStatement
{
};

// ------------------------------------------------

struct Argument : public ASTNode
{
    Argument() : ASTNode(kFuncParamList) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "Argument"; }
    virtual void Stdout() {}
    std::shared_ptr<TypeInfo> var_type;
    IdentifierInfo var;
};

typedef std::vector<std::shared_ptr<Argument>> ArgumentList;
typedef std::vector<std::shared_ptr<DeclarationAndStatement>> CompoundStatement;

// ExternalDeclaration contains Function decl and Global Variable decl;
struct ExternalDeclaration : public ASTNode
{
    ExternalDeclaration(NodeType t) : ASTNode(t) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "DeclaratExternalDeclarationionAndStatement"; }
    virtual void Stdout() {}
};

struct Function : public ExternalDeclaration
{
    Function() : ExternalDeclaration(kFuncDefinition) {}
    std::shared_ptr<TypeInfo> type;
    std::string function_name;
    ArgumentList arguments;
    CompoundStatement statements;

    std::string Assemble(AssemblyConfig &conf) override
    {
        std::string code = "";
        if (function_name.compare("main") == 0)
        {
            code += conf.asm_.Directive("globl _main");
            code += conf.asm_.Label("_main");
        }
        code += conf.asm_.Asm("push", {"rbp"});
        code += conf.asm_.Asm("mov", {"rbp", "rsp"});

        for (auto s : statements)
        {
            code += s->Assemble(conf);
        }

        code += conf.asm_.Asm("mov", {"rsp", "rbp"});
        code += conf.asm_.Asm("pop", {"rbp"});
        code += conf.asm_.Asm("ret");
        return code;
    }

    void Stdout() override
    {
        std::cout << function_name << " " << type->type_name << "(" << std::endl;
        for (auto a : arguments)
        {
            a->Stdout();
        }
        std::cout << ")" << std::endl;

        for (auto s : statements)
        {
            s->Stdout();
        }
    }
};

struct Program : public ASTNode
{
    Program() : ASTNode(kProgram) {}
    std::vector<std::shared_ptr<ExternalDeclaration>> decl;

    std::string Assemble(AssemblyConfig &conf) override
    {
        std::string code;
        if (conf.mode == kIntel)
        {
            code += conf.asm_.Directive("intel_syntax noprefix");
        }

        for (auto d : decl)
        {
            code += d->Assemble(conf);
        }
        return code;
    }

    void Stdout() override
    {
        std::cout << "Program" << std::endl;
        for (auto d : decl)
        {
            std::cout << d->node_type << std::endl;
            if (d->node_type == kFuncDefinition)
            {
                std::shared_ptr<Function> f = std::dynamic_pointer_cast<Function>(d);

                std::cout << f->function_name << std::endl;
                std::cout << f->type->type_name << std::endl;

                f->Stdout();
            }
        }
    }
};

struct CompilerState
{
    // 識別子が登録済みであるかを判定
    //   true  : 登録済み
    //   false : 未登録
    bool IsDefinedVar(const std::string &univ_name)
    {
        return identifier_store.find(univ_name) != std::end(identifier_store);
    }

    // 識別子ストアへの登録
    //   true  : 登録成功
    //   false : 登録失敗
    bool RegistID(const std::string &id, const std::string &scope)
    {
        auto uid = module_name + "::" + scope + "::" + id;
        if (IsDefinedVar(uid))
            return false;

        identifier_store[uid] = IdentifierInfo{id, module_name + "::" + scope};

        return true;
    }

    // module name
    std::string module_name;

    // line number in module
    int line_number = 0;

    // input data
    std::vector<Token> buf;

    // current position
    std::vector<Token>::iterator iter;

    // type information store
    std::map<std::string, TypeInfo> type_store;

    // identifier information store
    std::map<std::string, IdentifierInfo> identifier_store;

    // error information
    std::vector<CompileErrorInfo> errors;

    // current scope
    std::string scope;

    // assembly config
    AssemblyConfig asm_config;

    // relative address of base stack pointer
    int stack_rel_addr = 0;
};

struct Node
{
    Node() {}
    Node(NodeType type, std::string syntax);

    NodeType type;
    std::string syntax;

    std::shared_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> child;
};

class Parser
{
  public:
    Parser(const std::shared_ptr<CompilerState> &compiler_state);

    std::shared_ptr<Program> SyntaxCheck();
    int GenerateAssembly(std::shared_ptr<Program> &node, std::string *assembly);

  private:
    void Init();

    bool IsEqual(std::vector<kcc::Token>::iterator &it, char c);
    bool IsDefinedType(const std::string &str);
    bool IsDefinedVar(const std::string &var);

    bool SkipSemicolon();
    void SkipLF();

    bool MakeVariableDeclaration(std::vector<std::shared_ptr<VariableDeclaration>> &variables);
    bool MakeVariableIdentifier(const std::string &scope, std::string &var_name);
    bool MakeInitDeclarator(const std::shared_ptr<TypeInfo> &type, std::string &var_name, std::shared_ptr<AssignmentExpr> &assign_expr);
    bool MakeAssignmentExpr(std::shared_ptr<AssignmentExpr> &assign_expr);
    bool MakeExpr(std::shared_ptr<ExprBase> &expr);

    bool MakeTypeDefinition(std::shared_ptr<TypeInfo> &type);
    bool MakeArgumentDeclaration(std::shared_ptr<Argument> &argument);
    bool MakeArgumentDeclarationList(ArgumentList &arguments);
    bool MakeFunctionIdentifier(std::string &function_identifier);

    bool MakeReturnStatement(std::shared_ptr<ReturnStatement> &return_statement);
    bool MakeCompoundStatement(CompoundStatement &compound_statement);
    bool MakeFunctionDefinition(std::shared_ptr<Function> &function);

    bool MakeBinaryExpr(std::shared_ptr<BinaryExpr> &primary_expr);
    bool MakePrimaryExpr(std::shared_ptr<PrimaryExpr> &primary_expr);

    bool MakeStringLiteral(std::shared_ptr<StringLiteral> &string_literal);
    bool MakeIntegerLiteral(std::shared_ptr<IntegerLiteral> &integer_literal);

    bool MakeProgram(std::shared_ptr<Program> &program);

    inline const Token Token(int n = 0)
    {
        if (compiler_state->iter + n >= std::end(compiler_state->buf))
        {
            throw_ln("tokens : out of range");
        }
        return *(compiler_state->iter + n);
    }

    inline const std::vector<kcc::Token>::iterator FwdCursor(int n = 1)
    {
        if (compiler_state->iter + n > std::end(compiler_state->buf))
        {
            throw_ln("tokens : out of range");
        }
        return (compiler_state->iter += n);
    }

    inline const std::vector<kcc::Token>::iterator BwdCursor(int n = 1)
    {
        if (compiler_state->iter - n <= std::begin(compiler_state->buf) - 1)
        {
            throw_ln("tokens : out of range");
        }
        return (compiler_state->iter -= n);
    }

    inline void ShowTokenInfo()
    {
        std::cout << "-- TokenInfo ----" << std::endl;
        std::cout << " token :" << Token().token << std::endl;
        std::cout << " type  :" << Token().type << std::endl;
    }

    std::shared_ptr<CompilerState> compiler_state;
};

} // namespace kcc

#endif
