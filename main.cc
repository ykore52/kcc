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
int Compile(const std::string &module_name, std::vector<char> buffer)
{
    std::vector<std::string> tokens = Tokenizer::Tokenize(buffer);

    std::cout << "--- tokenization result ---" << std::endl;
    for (auto token : tokens)
    {
        std::cout << token << ":" << token.length() << std::endl;
    }

    std::cout << "--- syntax check ---" << std::endl;
    Parser::SyntaxCheck(module_name, tokens);

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
        fin.open(opts->module_name.c_str(), std::ios::in | std::ios::binary);
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