#include <exception>
#include <fstream>
#include <iostream>
#include <vector>
#include "tokenizer.hh"
#include "parser.hh"

namespace kcc
{

struct CmdOptions
{
    std::string module_name;
};

// Compile
// Compiling a source codesource codes.
int Compile(const std::string &module_name, std::vector<char> &buffer)
{
    //  std::cout << "--- Compile start ---" << std::endl;

    // auto tokens = tokenizer.Tokenize(buffer);

    // std::cout << "--- tokenization result ---" << std::endl;
    // for (auto token : tokens)
    // {
    //     std::cout << token << ":" << token.length() << std::endl;
    // }

    std::cout << "--- syntax check begin ---" << std::endl;
    auto compiler_state = std::shared_ptr<CompilerState>(new CompilerState);
    Tokenizer tokenizer;
    tokenizer.Tokenize(buffer, &compiler_state->buf);
    compiler_state->iter = std::begin(compiler_state->buf);
    compiler_state->line_number = 0;
    std::cout << "--- syntax check end ---" << std::endl;


    // for (auto i = compiler_state->buf.begin(); i != compiler_state->buf.end(); i++) {
    //     std::cout << static_cast<const void*>(&std::end(compiler_state->buf)) << ":" << *i << ":" << std::endl;
    // }

    Parser parser(compiler_state);
    auto prog = parser.SyntaxCheck();
    PDEBUG("================= Code Generation ========================");
    //prog->Stdout();

    std::string assembly;
    parser.GenerateAssembly(prog, &assembly);

    std::cout << assembly << std::endl;
    return 0;
}

// Read command line options and stored to struct CmdOptions.
std::unique_ptr<CmdOptions> ReadOptions(int argc, char **argv)
{
    if (argc == 1)
    {
        throw std::invalid_argument("No input file");
    }

    std::unique_ptr<CmdOptions> opts(new CmdOptions);

    opts->module_name = std::string(argv[1]);

    return opts;
}

} // namespace kcc

// main function
int main(int argc, char **argv)
{
    try
    {
        auto opts = kcc::ReadOptions(argc, argv);
        std::fstream fin;
        fin.open(opts->module_name.c_str(), std::ios::in);
        if (!fin)
        {
            throw std::invalid_argument("Cannot open file.");
        }

        fin.seekg(0, std::fstream::end);
        auto eofPos = fin.tellg();
        fin.seekg(0, std::fstream::beg);
        auto beginPos = fin.tellg();

        auto size = eofPos - beginPos;

        std::vector<char> buf(size);
        fin.read(&buf[0], size);

        PrintCharVector(buf);

        // Compiling a source code
        kcc::Compile(opts->module_name, buf);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;

        return 1;
    }
    return 0;
}