#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "util.hh"

namespace kcc
{

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
  NodeType node_type;
};

// ------------------------------------------------
struct LiteralBase : public ASTNode
{
  LiteralBase(NodeType t, std::string value) : ASTNode(t) {}
  std::string value;
};

struct IntegerLiteral : public LiteralBase
{
  IntegerLiteral(std::string value) : LiteralBase(kIntegerLiteral, value) {}
};

struct StringLiteral : public LiteralBase
{
  StringLiteral(std::string value) : LiteralBase(kStringLiteral, value) {}
};

// ------------------------------------------------
struct ExpressionBase : public ASTNode
{
  ExpressionBase(NodeType t) : ASTNode(t) {}
  // child expression
  std::shared_ptr<ExpressionBase> expression;
};

// 一次式
struct PrimaryExpression : public ExpressionBase
{
  PrimaryExpression() : ExpressionBase(kPrimaryExpression) {}
  PrimaryExpression(std::shared_ptr<LiteralBase> &literal) : ExpressionBase(kPrimaryExpression), literal(literal) {}
  std::shared_ptr<LiteralBase> literal;
};

// ------------------------------------------------
struct StatementBase : public ASTNode
{
  StatementBase(NodeType t) : ASTNode(t) {}
};

struct ReturnStatement : public StatementBase
{
  ReturnStatement() : StatementBase(kStatementReturn) {}
  ReturnStatement(const std::shared_ptr<ExpressionBase> &e) : StatementBase(kStatementReturn), return_expression(e) {}
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
  std::shared_ptr<TypeInfo> var_type;
  IdentifierInfo var;
};

typedef std::vector<Argument> ArgumentList;
typedef std::vector<StatementBase> CompoundStatement;

struct DeclarationBase : public ASTNode
{
  DeclarationBase(NodeType t) : ASTNode(t) {}
};

struct Declaration : public DeclarationBase
{
  Declaration() : DeclarationBase(kDeclaration) {}
};

struct Function : public DeclarationBase
{
  Function() : DeclarationBase(kFuncDefinition) {}
  std::shared_ptr<TypeInfo> type;
  std::string function_name;
  ArgumentList arguments;
  CompoundStatement statements;
};

struct Program : public ASTNode
{
  Program() : ASTNode(kProgram) {}
  std::vector<DeclarationBase> decl;
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
  int GenerateAssembly(std::shared_ptr<Node> &node, std::string *assembly);

private:
  void Init();

  bool IsEqual(const std::vector<std::string>::iterator &it, char c);

  bool SkipSemicolon();
  void SkipLF();

  // bool TypeDefinition(const std::shared_ptr<Node> &node, std::shared_ptr<TypeInfo>);
  // bool ArgumentDeclaration(const std::shared_ptr<Node> &node);
  // bool ArgumentDeclarationList(const std::shared_ptr<Node> &node);
  // bool FunctionIdentifier(const std::shared_ptr<Node> &node);
  // bool ReturnStatement(const std::shared_ptr<Node> &node);
  // bool CompoundStatement(const std::shared_ptr<Node> &node);
  // bool FunctionDefinition(const std::shared_ptr<Node> &node);
  // bool Expression(const std::shared_ptr<Node> &node);
  // bool IntegerLiteral(const std::shared_ptr<Node> &node);
  // bool StringLiteral(const std::shared_ptr<Node> &node);

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
