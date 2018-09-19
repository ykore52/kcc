#include "ast.hh"

int main() {

    auto rootNode = std::shared_ptr<AST::Node>(new AST::Node(AST::eRoot, "root"));

    auto state = AST::CompilerState (
        "return2.c",
        0,
        { "int", "main", "(", ")", "{", "\n", "return", "2", ";", "\n", "}", "\n" }
    );


    AST::FunctionDefinition(rootNode, state);
}