#include <exception>
#include <fstream>
#include <iostream>
#include <vector>
#include "tokenizer.hpp"
using namespace std;

struct CmdOptions
{
    string inputFile;
};

// Compile source codes.
int compile(std::vector<char> buffer)
{
    std::vector<string> tokens = tokenize(buffer);

    std::cout << "--- tokenization result ---" << std::endl;
    for (auto token : tokens)
        std::cout << token << std::endl;

    std::cout << "--- syntax check ---" << std::endl;

    return 0;
}

// Read command line options and stored to struct CmdOptions.
unique_ptr<CmdOptions> readOptions(int argc, char **argv)
{
    if (argc == 1)
    {
        throw std::invalid_argument("No input file");
    }

    unique_ptr<CmdOptions> opts(new CmdOptions);

    opts->inputFile = string(argv[1]);

    return opts;
}

// main function
int main(int argc, char **argv)
{
    try
    {
        auto opts = readOptions(argc, argv);
        std::fstream fin;
        fin.open(opts->inputFile.c_str(), ios::in | ios::binary);
        if (!fin)
        {
            throw std::invalid_argument("Cannot open file.");
        }

        fin.seekg(0, fstream::end);
        auto eofPos = fin.tellg();
        fin.seekg(0, fstream::beg);
        auto beginPos = fin.tellg();

        auto size = eofPos - beginPos;

        std::vector<char> buf(size);
        fin.read(&buf[0], size);

        compile(buf);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;

        return 1;
    }
    return 0;
}