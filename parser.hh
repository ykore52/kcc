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

namespace kcc
{

enum NodeType
{
    kProgram,
    kDeclaration,
    kObjectType,

    kIntegerLiteral,
    kStringLiteral,

    kVariableDeclaration,

    kFuncDefinition,
    kFuncStorageClassSpecifier, // auto, register, static, extern, typedef,
                                // __decpspec(ms-specific)
    kFuncIdentifier,
    kFuncCompoundStatement,
    kFuncParamList,
    kFuncParam,
    kStatementReturn,

    kPrimaryExpression,
    kAssignmentExpression
};

// 記述言語をアセンブリ言語に変換するための中間オブジェクト
// ジャンプ先を記憶するために利用
struct Instruction
{
    Instruction() {}
    Instruction(const Mnemonic mnemonic) : mnemonic(mnemonic) {}
    Instruction(const Mnemonic &mnemonic, const std::shared_ptr<Instruction> &jump_dest, const std::string &line)
        : mnemonic(mnemonic), destination(jump_dest), line(line) {}

    void operator=(const Instruction &src)
    {
        mnemonic = src.mnemonic;
        destination = src.destination;
        line = src.line;
    }

    Mnemonic mnemonic;
    std::shared_ptr<Instruction> destination;
    std::string line;
};

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
    int size; // バイト数
    std::map<std::string, std::shared_ptr<TypeInfo>> member;
};

// 識別子情報
struct IdentifierInfo
{
    std::string id;
    std::string module_name;
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

// ------------------------------------------------
struct LiteralBase : public ASTNode
{
    LiteralBase(NodeType t, std::string value) : ASTNode(t), value(value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "LiteralBase"; }
    virtual void Stdout() {}
    std::string value;
};

struct IntegerLiteral : public LiteralBase
{
    IntegerLiteral(std::string value) : LiteralBase(kIntegerLiteral, value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return value; }
    virtual void Stdout() {}
};

struct StringLiteral : public LiteralBase
{
    StringLiteral(std::string value) : LiteralBase(kStringLiteral, value) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "\"" + value + "\""; }
    virtual void Stdout() {}
};

// ------------------------------------------------
struct ExpressionBase : public ASTNode
{
    ExpressionBase(NodeType t) : ASTNode(t) {}
    virtual std::string Assemble(AssemblyConfig &conf) { return "ExpressionBase"; }
    virtual void Stdout() {}
    // child expression
    std::shared_ptr<ExpressionBase> expression;
    TypeInfo type_of_expr;
};

// 代入式(右辺値)
struct AssignmentExpression : public ExpressionBase
{
    AssignmentExpression() : ExpressionBase(kAssignmentExpression) {}
    
    virtual std::string Assembly(AssemblyConfig &conf)
    {
        return "";
    }

    virtual void Stdout() {}

    std::shared_ptr<ExpressionBase> expression;
};

// 一次式
struct PrimaryExpression : public ExpressionBase
{
    PrimaryExpression() : ExpressionBase(kPrimaryExpression) {}
    PrimaryExpression(std::shared_ptr<LiteralBase> &literal)
        : ExpressionBase(kPrimaryExpression), literal(literal) {}
    virtual std::string Assemble(AssemblyConfig &conf)
    {
        return conf.asm_.Asm("mov", {"rax", literal->value});
    }
    virtual void Stdout() {}

    std::shared_ptr<LiteralBase> literal;
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
    }
};

struct ReturnStatement : public DeclarationAndStatement
{
    ReturnStatement() : DeclarationAndStatement(kStatementReturn) {}
    ReturnStatement(const std::shared_ptr<ExpressionBase> &e)
        : DeclarationAndStatement(kStatementReturn), return_expression(e) {}

    virtual std::string Assemble(AssemblyConfig &conf) override
    {
        std::string code = "";
        code += return_expression->Assemble(conf);
        return code;
    }
    virtual void Stdout() override {}

    std::shared_ptr<ExpressionBase> return_expression;
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
struct ExternalDeclaration : public ASTNode {
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
        if (function_name.compare("main") == 0) {
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
    // module name
    std::string module_name;

    // line number in module
    int line_number;

    // input data
    std::vector<std::string> buf;

    // current position
    std::vector<std::string>::iterator iter;

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

    bool IsEqual(const std::vector<std::string>::iterator &it, char c);
    bool IsDefinedType(const std::string &str);
    NodeType Which(std::vector<std::string>::iterator &it);

    bool SkipSemicolon();
    void SkipLF();

    bool MakeVariableDeclaration(std::vector<std::shared_ptr<VariableDeclaration>> &variables);
    bool MakeVariableIdentifier(const std::string &scope, std::string &var_name);
    bool MakeInitDeclarator(std::string &var_name, std::shared_ptr<AssignmentExpression> &assign_expr);

    bool MakeTypeDefinition(std::shared_ptr<TypeInfo> &type);
    bool MakeArgumentDeclaration(std::shared_ptr<Argument> &argument);
    bool MakeArgumentDeclarationList(ArgumentList &arguments);
    bool MakeFunctionIdentifier(std::string &function_identifier);

    bool MakeReturnStatement(std::shared_ptr<ReturnStatement> &return_statement);
    bool MakeCompoundStatement(CompoundStatement &compound_statement);
    bool MakeFunctionDefinition(std::shared_ptr<Function> &function);
    
    bool MakeExpression(std::shared_ptr<PrimaryExpression> &primary_expression);

    bool MakeStringLiteral(std::shared_ptr<StringLiteral> &string_literal);
    bool MakeIntegerLiteral(std::shared_ptr<IntegerLiteral> &integer_literal);

    bool MakeProgram(std::shared_ptr<Program> &program);

    std::shared_ptr<CompilerState> compiler_state;
};

} // namespace kcc

#endif
