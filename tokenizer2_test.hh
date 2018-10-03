#ifndef TOKENIZER2_TEST_HH
#define TOKENIZER2_TEST_HH

#include "util.hh"
#include "testing.hh"
#include "tokenizer2.hh"

namespace kcc2
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

    bool SkipBlockComment_BasicTest()
    {
        // input
        Tokenizer tzr;
        tzr.Init(PrepareInput("/* hoge */"));

        // asserts
        TEST(tzr.SkipBlockComment() == 0);
    }

    bool SkipBlockComment_ErrorTest()
    {
        // input
        Tokenizer tzr;
        tzr.Init(PrepareInput("/* hoge"));

        // asserts
        TEST(tzr.SkipBlockComment() == -1);
    }

    bool Tokenize_Test()
    {

        // output
        std::vector<Token> tokens;

        Tokenizer tzr;
        tzr.Tokenize(PrepareInput("int main() { return 2; }"), &tokens);

        TEST(tokens[0].token == "int");
        TEST(tokens[1].token == "main");
        TEST(tokens[2].token == "(");
        TEST(tokens[3].token == ")");
        TEST(tokens[4].token == "{");
        TEST(tokens[5].token == "return");
        TEST(tokens[6].token == "2");
        TEST(tokens[7].token == ";");
        TEST(tokens[8].token == "}");

        for (auto t : tokens)
        {
            std::cout << t.token << std::endl;
        }
    }
};

} // namespace kcc2
#endif
