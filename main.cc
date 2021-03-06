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
    std::string assembly_filename;
};

// Compile
// Compiling a source codesource codes.
int Compile(std::string &assembly, const std::string &module_name, std::vector<char> &buffer)
{
    PDEBUG("======= Tokenization =======");

    auto compiler_state = std::shared_ptr<CompilerState>(new CompilerState);

    Tokenizer tokenizer;
    tokenizer.Tokenize(buffer, &(compiler_state->buf));
    
    compiler_state->iter = std::end(compiler_state->buf);
 
    Parser parser(compiler_state);
    auto prog = parser.SyntaxCheck();

    PDEBUG("======= Code Generation ========");

    parser.GenerateAssembly(prog, &assembly);
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

    std::vector<std::string> opts_array;
    for (int i = 0; i < argc; i++) {
        opts_array.push_back(argv[i]);
    }

    for (auto o = opts_array.begin(); o != opts_array.end(); ++o) {
        if (o->compare("-S") == 0) {
            ++o;
            if (o == opts_array.end()) {
                throw std::invalid_argument("No specific assembly(*.S) file");
            }
            opts->assembly_filename = std::string(*o);
            continue;
        }

        if (o->compare("-o") == 0) {
            continue;
        }

        opts->module_name = std::string(argv[1]);
    }

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

        //PrintCharVector(buf);

        // Compiling a source code
        std::string assembly_code = "";
        kcc::Compile(assembly_code, opts->module_name, buf);

        // Output file
        if (!opts->assembly_filename.empty())
        {
            std::ofstream fs_asm(opts->assembly_filename);
            fs_asm << assembly_code.c_str();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;

        return 1;
    }
    return 0;
}