#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "x64.hh"

namespace kcc
{

static std::string ASMSP = "    ";
static std::string ASMLF = "\n";

// You can change assembly syntax.
enum AssemblySyntaxMode
{
    // Intel syntax mode
    kIntel,

    // AT&T syntax mode
    kATT
};


// アセンブリ/バイナリコードの1命令を格納するオブジェクト
struct Instruction
{
    Instruction() {}
    Instruction(const Mnemonic mnemonic) : mnemonic(mnemonic) {}

    Mnemonic mnemonic;
    std::vector<Instruction> arguments;
    std::string label;
};

// struct MOV : public Instruction
// {
//     MOV(Mnemonic::RegisterX64 dest, ) : Instruction(Mnemonic::MOV);
// };


class Assembler
{
  public:

    // ディレクティブを出力します.
    // 先頭の "." は自動的に挿入します.
    static std::string Directive(const std::string &directive)
    {
        std::string code;
        code = "." + directive + ASMLF;
        return code;
    }

    // ラベルを出力します.
    // 行末のセミコロンは自動的に挿入します.
    static std::string Label(const std::string &label)
    {
        std::string code;
        code = label + ":" + ASMLF;
        return code;
    }

    // オペコードとオペランドを出力します.
    // 行頭のスペースは自動的に挿入します.
    static std::string Asm(const std::string &opcode, const std::vector<std::string> &args = {})
    {
        std::string code;
        std::ostringstream oss;
        std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(oss, ","));

        code = ASMSP + opcode + " " + oss.str();
        code.erase(code.size() - std::char_traits<char>::length(","));
        code += ASMLF;
        return code;
    }
};

struct AssemblyConfig
{
    AssemblyConfig() : mode(kIntel) {}

    AssemblySyntaxMode mode;
    Assembler asm_;
};

}

#endif
