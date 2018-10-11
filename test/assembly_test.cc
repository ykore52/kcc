#include "../parser.hh"
#include "../testing.hh"
#include "../tokenizer.hh"
#include "../util.hh"

namespace kcc
{

namespace test
{

class Assemble_Assembly_Test
{
  public:
    Assemble_Assembly_Test() { std::cout << "=== Run tests ===" << std::endl; }

    ~Assemble_Assembly_Test() { std::cout << "=== Finish tests ===" << std::endl; }

    void Run()
    {
        Assemble_BasicTest();
        Assemble_Var_Test();
    }

    void Assemble_BasicTest()
    {
        auto inp = PrepareInput("int main() { return 2; }");

        // output following assembly
        auto answer = R"(.intel_syntax noprefix
.globl _main
_main:
    push rbp
    mov rbp,rsp
    mov rax,2
    mov rsp,rbp
    pop rbp
    ret
)";

        std::vector<kcc::Token> tokens;
        Tokenizer t;
        t.Tokenize(inp, &tokens);

        std::shared_ptr<CompilerState> c(new CompilerState);
        Parser p(c);
        c->buf = tokens;
        c->iter = std::begin(c->buf);
        c->module_name = "Assemble_BasicTest";
        auto ast = p.SyntaxCheck();

        std::string assembly;
        p.GenerateAssembly(ast, &assembly);
        // std::cout << "=========== output assembly code =================" << std::endl;
        // std::cout << assembly << std::endl;

        TEST_EQUAL(answer, assembly);
    }

    void Assemble_Var_Test()
    {
        auto inp = PrepareInput("int main() { int a; a = 1; return a; }");

        // output following assembly
        auto answer = R"(".intel_syntax noprefix
.globl _main
_main:
    push    rbp
    mov     rbp, rsp
    mov     QWORD PTR [rbp-8], 1
    mov     rax, QWORD PTR [rbp-8]
    pop     rbp
    ret
)";

        std::vector<kcc::Token> tokens;
        Tokenizer t;
        t.Tokenize(inp, &tokens);

        std::shared_ptr<CompilerState> c(new CompilerState);
        Parser p(c);
        c->buf = tokens;
        c->iter = std::begin(c->buf);
        c->module_name = "Assemble_Var_Test";
        auto ast = p.SyntaxCheck();

        std::string assembly;
        p.GenerateAssembly(ast, &assembly);
        // std::cout << "=========== output assembly code =================" << std::endl;
        // std::cout << assembly << std::endl;

        TEST_EQUAL(answer, assembly);
    }

    std::vector<char> PrepareInput(const char *input)
    {
        std::istringstream ss(input);
        std::string srcstr(ss.str());
        std::vector<char> srcbuf(std::begin(srcstr), std::end(srcstr));
        return srcbuf;
    }
};

} // namespace test
} // namespace kcc

int main()
{
    try
    {
        kcc::test::Assemble_Assembly_Test t;
        t.Run();
        Testing::DisplaySummary();
    }
    catch (Exception e)
    {
        std::cerr << e.what() << std::endl;
    }
}