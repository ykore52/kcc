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

    void Run() {
        Assemble_Test();
    }

    void Assemble_Test()
    {
        auto inp = PrepareInput("int main() { return 2; }");
        std::vector<kcc::Token> tokens;
        Tokenizer t;
        t.Tokenize(inp, &tokens);

        std::shared_ptr<CompilerState> c;
        Parser p(c);
        c->iter = std::begin(c->buf);
        auto ast = p.SyntaxCheck();

        std::string assembly;
        p.GenerateAssembly(ast, &assembly);

        std::cout << assembly << std::endl;
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
    kcc::test::Assemble_Assembly_Test t;
    t.Run();
    Testing::DisplaySummary();
}