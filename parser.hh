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
    kRoot,

    kObjectType,
    kFuncDefinition,
    kFuncStorageClassSpecifier, // auto, register, static, extern, typedef,
                                // __decpspec(ms-specific)
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

    bool SyntaxCheck();

  private:
    void Init();

    bool IsEqual(const std::vector<std::string>::iterator &it, char c);

    bool SkipSemicolon();
    void SkipLF();

    bool TypeDefinition(const std::shared_ptr<Node> &node);
    bool ArgumentDeclaration(const std::shared_ptr<Node> &node);
    bool ArgumentDeclarationList(const std::shared_ptr<Node> &node);
    bool FunctionIdentifier(const std::shared_ptr<Node> &node);
    bool ReturnStatement(const std::shared_ptr<Node> &node);
    bool CompoundStatement(const std::shared_ptr<Node> &node);
    bool FunctionDefinition(const std::shared_ptr<Node> &node);

    std::shared_ptr<CompilerState> compiler_state;
};

} // namespace kcc

#endif
