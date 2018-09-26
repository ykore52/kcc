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

namespace kcc
{

static std::string ASMSP = "    ";
static std::string ASMLF = "\n";

enum NodeType
{
    kProgram,
    kDeclaration,
    kObjectType,

    kIntegerLiteral,
    kStringLiteral,

    kFuncDefinition,
    kFuncStorageClassSpecifier, // auto, register, static, extern, typedef,
                                // __decpspec(ms-specific)
    kFuncIdentifier,
    kFuncCompoundStatement,
    kFuncParamList,
    kFuncParam,
    kStatementReturn,

    kPrimaryExpression
};

// You can change assembly syntax.
enum AssemblySyntaxMode
{
    // Intel syntax mode
    kIntel,

    // AT&T syntax mode
    kATT
};

struct AssemblyOption
{
    AssemblyOption() : mode(kIntel) {}

    AssemblySyntaxMode mode;
};


inline static std::string FillSpace(int n)
{
    std::string sp = "";
    for (int i = 0; i < n; i++)
    {
        sp += " ";
    }
    return sp;
}

inline static std::string AsmStr0(std::string instruction,
                           AssemblySyntaxMode mode = kIntel)
{
    switch (mode)
    {
    case kIntel:
        return ASMSP + instruction + ASMLF;
    case kATT:
        return ASMSP + instruction + ASMLF;
    }
}

inline static std::string AsmStr1(std::string instruction,
                           std::string src,
                           AssemblySyntaxMode mode = kIntel)
{
    if (mode == kIntel)
    {
        std::string r = ASMSP + instruction + FillSpace(8 - instruction.length()) + src + ASMLF;
        return r;
    }
    else if (mode == kATT)
    {
        std::string r = ASMSP + instruction + FillSpace(8 - instruction.length()) + src + ASMLF;
        return r;
    }
}

inline static std::string AsmStr2(std::string instruction,
                           std::string dest,
                           std::string src,
                           AssemblySyntaxMode mode = kIntel)
{
    if (mode == kIntel)
    {
        std::string r = ASMSP + instruction + FillSpace(8 - instruction.length()) + dest + "," + src + ASMLF;
        return r;
    }
    else if (mode == kATT)
    {
        std::string r = ASMSP + instruction + FillSpace(8 - instruction.length()) + src + "," + dest + ASMLF;
        return r;
    }
}

inline static std::string AsmLabel(std::string label)
{
    return label + ASMLF;
}

struct CompileErrorInfo
{
    std::string module_name;
    int line_number;
    std::string message;
};

struct TypeInfo
{
    std::string type_name;
    bool is_pointer;
    std::map<std::string, std::shared_ptr<TypeInfo>> member;
};

struct IdentifierInfo
{
    std::string id;
    std::string module_name;
};

struct ASTNode
{
    ASTNode() {}
    ASTNode(const NodeType t) : node_type(t) {}
    virtual ~ASTNode() {}
    virtual std::string Code(AssemblyOption const &opt) { return "dummy"; }
    virtual void Stdout() { std::cout << "ASTNode" << std::endl; }

    NodeType node_type;
};

// ------------------------------------------------
struct LiteralBase : public ASTNode
{
    LiteralBase(NodeType t, std::string value) : ASTNode(t), value(value) {}
    virtual std::string Code(AssemblyOption const &opt) { return "LiteralBase"; }
    virtual void Stdout() {}
    std::string value;
};

struct IntegerLiteral : public LiteralBase
{
    IntegerLiteral(std::string value) : LiteralBase(kIntegerLiteral, value) {}
    virtual std::string Code(AssemblyOption const &opt)
    {
        std::stringstream ss;
        ss << "$" << std::hex << std::stoi(value);
        std::string ret;
        ss >> ret;
        return ret;
    }
    virtual void Stdout() {}
};

struct StringLiteral : public LiteralBase
{
    StringLiteral(std::string value) : LiteralBase(kStringLiteral, value) {}
    virtual std::string Code(AssemblyOption const &opt) { return "StringLiteral"; }
    virtual void Stdout() {}
};

// ------------------------------------------------
struct ExpressionBase : public ASTNode
{
    ExpressionBase(NodeType t) : ASTNode(t) {}
    virtual std::string Code(AssemblyOption const &opt) { return "ExpressionBase"; }
    virtual void Stdout() {}
    // child expression
    std::shared_ptr<ExpressionBase> expression;
    TypeInfo type_of_expr;
};

// 一次式
struct PrimaryExpression : public ExpressionBase
{
    PrimaryExpression() : ExpressionBase(kPrimaryExpression) {}
    PrimaryExpression(std::shared_ptr<LiteralBase> &literal)
        : ExpressionBase(kPrimaryExpression), literal(literal) {}
    virtual std::string Code(AssemblyOption const &opt)
    {
        std::string asm_ = "";
        asm_ += ASMSP + "movl    %rax," + literal->Code(opt) + ASMLF;
        return asm_;
    }
    virtual void Stdout() {}

    std::shared_ptr<LiteralBase> literal;
};

// ------------------------------------------------
struct StatementBase : public ASTNode
{
    StatementBase(NodeType t) : ASTNode(t) {}
    virtual std::string Code(AssemblyOption const &opt) { return "StatementBase"; }
    virtual void Stdout() {}
};

struct ReturnStatement : public StatementBase
{
    ReturnStatement() : StatementBase(kStatementReturn) {}
    ReturnStatement(const std::shared_ptr<ExpressionBase> &e)
        : StatementBase(kStatementReturn), return_expression(e) {}

    virtual std::string Code(AssemblyOption const &opt) override
    {
        std::string asm_ = "";
        asm_ += return_expression->Code(opt);
        return asm_;
    }
    virtual void Stdout() {}

    std::shared_ptr<ExpressionBase> return_expression;
};

struct IfStatement : public StatementBase
{
};
struct ForStatement : public StatementBase
{
};
struct WhileStatement : public StatementBase
{
};

// ------------------------------------------------

struct Argument : public ASTNode
{
    Argument() : ASTNode(kFuncParamList) {}
    virtual std::string Code(AssemblyOption const &opt) { return "Argument"; }
    virtual void Stdout() {}
    std::shared_ptr<TypeInfo> var_type;
    IdentifierInfo var;
};

typedef std::vector<std::shared_ptr<Argument>> ArgumentList;
typedef std::vector<std::shared_ptr<StatementBase>> CompoundStatement;

struct DeclarationBase : public ASTNode
{
    DeclarationBase(NodeType t) : ASTNode(t) {}
    virtual std::string Code(AssemblyOption const &opt) { return "DeclarationBase"; }
    virtual void Stdout() {}
};

struct Declaration : public DeclarationBase
{
    Declaration() : DeclarationBase(kDeclaration) {}
    virtual std::string Code(AssemblyOption const &opt) { return "Declaration"; }
};

struct Function : public DeclarationBase
{
    Function() : DeclarationBase(kFuncDefinition) {}
    std::shared_ptr<TypeInfo> type;
    std::string function_name;
    ArgumentList arguments;
    CompoundStatement statements;

    std::string Code(AssemblyOption const &opt)
    {
        std::string result;
        result += function_name + ":" + ASMLF;
        result += ASMSP + "pushl   %rbp" + ASMLF;
        result += ASMSP + "movl    %rbp,%rsp" + ASMLF;

        for (auto s : statements)
        {
            result += s->Code(opt);
        }

        result += ASMSP + "movl    %rsp,%rbp" + ASMLF;
        result += ASMSP + "popl    %rbp" + ASMLF;
        result += ASMSP + "ret" + ASMLF;
        return result;
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
    std::vector<std::shared_ptr<DeclarationBase>> decl;

    std::string Code(AssemblyOption const &opt)
    {
        std::string result;
        if (opt.mode == kIntel) {
            result += AsmLabel(".intel_syntax noprefix");
        }

        for (auto d : decl)
        {
            result += d->Code(opt);
        }
        return result;
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

    // assembly options
    AssemblyOption asm_option;
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

    bool SkipSemicolon();
    void SkipLF();

    // bool TypeDefinition(const std::shared_ptr<Node> &node,
    // std::shared_ptr<TypeInfo>); bool ArgumentDeclaration(const
    // std::shared_ptr<Node> &node); bool ArgumentDeclarationList(const
    // std::shared_ptr<Node> &node); bool FunctionIdentifier(const
    // std::shared_ptr<Node> &node); bool ReturnStatement(const
    // std::shared_ptr<Node> &node); bool CompoundStatement(const
    // std::shared_ptr<Node> &node); bool FunctionDefinition(const
    // std::shared_ptr<Node> &node); bool Expression(const std::shared_ptr<Node>
    // &node); bool IntegerLiteral(const std::shared_ptr<Node> &node); bool
    // StringLiteral(const std::shared_ptr<Node> &node);

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
