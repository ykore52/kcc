#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace kcc
{

enum NodeType
{
    kRoot,

    kObjectType,
    kFuncDefinition,
    kFuncStorageClassSpecifier, // auto, register, static, extern, typedef, __decpspec(ms-specific)
    kFuncIdentifier,
};

struct CompileErrorInfo
{
    std::string module_name;
    int line_number;
    std::string message;
};

struct TypeInfo
{
    std::string name;
    bool is_pointer;
    std::map<std::string, TypeInfo> member;
};

struct IdentifierInfo
{
    std::string name;
    std::string module_name;
};

struct CompilerState
{

    CompilerState(std::string module,
                  const std::vector<std::string> &tokens);
    CompilerState(std::string module,
                  int line_number,
                  const std::vector<std::string> &tokens);

    std::string module_name;
    int line_number;

    std::vector<std::string> buf;
    std::vector<std::string>::iterator iter;

    std::map<std::string, TypeInfo> type_store_;
    std::map<std::string, IdentifierInfo> identifier_store_;

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
    Parser(std::string module,
           const std::vector<std::string> &tokens);
    Parser(std::string module,
           int line_number,
           const std::vector<std::string> &tokens);

  private:
    void Init();

    bool IsChar(const std::vector<std::string>::iterator &it, char c);

    bool SkipSemicolon(CompilerState &state);
    void SkipLF(CompilerState &state);

    bool TypeDefinition(const std::shared_ptr<Node> &node, CompilerState &state);
    bool ArgumentDeclaration(const std::shared_ptr<Node> &node, CompilerState &state);
    bool ArgumentDeclarationList(const std::shared_ptr<Node> &node, CompilerState &state);
    bool FunctionIdentifier(const std::shared_ptr<Node> &node, CompilerState &state);
    bool ReturnStatement(const std::shared_ptr<Node> &node, CompilerState &state);
    bool CompoundStatement(const std::shared_ptr<Node> &node, CompilerState &state);
    bool FunctionDefinition(const std::shared_ptr<Node> &node, CompilerState &state);
    bool SyntaxCheck(const std::string module_name, const std::vector<std::string> &tokens);
};

} // namespace kcc

#endif
