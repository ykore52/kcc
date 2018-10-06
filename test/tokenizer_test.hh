#ifndef TOKENIZER_TEST_HH
#define TOKENIZER_TEST_HH

#include "../util.hh"
#include "../testing.hh"
#include "../tokenizer.hh"

namespace kcc
{

class Tokenize_Tokenizer_Test
{
  public:
    Tokenize_Tokenizer_Test() {
        std::cout << "=== Run tests ===" << std::endl;        
    }

    ~Tokenize_Tokenizer_Test() {
        std::cout << "=== Finish tests ===" << std::endl;
    }

    void Run()
    {
        SkipBlockComment_BasicTest();
        SkipBlockComment_ErrorTest();
        Tokenize_Test();
    }

    std::vector<char> PrepareInput(const char *input)
    {
        std::istringstream ss(input);
        std::string srcstr(ss.str());
        std::vector<char> srcbuf(std::begin(srcstr), std::end(srcstr));
        return srcbuf;
    }

    void SkipSpace_BasicTest()
    {
        // input
        Tokenizer tzr;
        tzr.Init(PrepareInput("    \n   "));
        TEST(tzr.SkipSpace());
    }

    void SkipBlockComment_BasicTest()
    {
        // input
        Tokenizer tzr;
        tzr.Init(PrepareInput("/* hoge */"));

        // asserts
        TEST(tzr.SkipBlockComment());
    }

    void SkipBlockComment_ErrorTest()
    {
        // input
        Tokenizer tzr;
        tzr.Init(PrepareInput("/* hoge"));

        // asserts
        TEST_NOT(tzr.SkipBlockComment());
    }

    void Tokenize_Test()
    {

        // output
        std::vector<Token> tokens;

        Tokenizer tzr;
        tzr.Tokenize(PrepareInput("int main() {\r\n    return 2;\n}"), &tokens);

        TEST_EQUAL(tokens[0].token, "int");
        TEST_EQUAL(tokens[1].token, "main");
        TEST_EQUAL(tokens[2].token, "(");
        TEST_EQUAL(tokens[3].token, ")");
        TEST_EQUAL(tokens[4].token, "{");
        TEST_EQUAL(tokens[5].line, 2);
        TEST_EQUAL(tokens[5].token, "return");
        TEST_EQUAL(tokens[6].token, "2");
        TEST_EQUAL(tokens[7].token, ";");
        TEST_EQUAL(tokens[8].token, "}");

        // for (auto t : tokens)
        // {
        //     std::cout << t.token << std::endl;
        // }
    }
};

} // namespace kcc2
#endif
