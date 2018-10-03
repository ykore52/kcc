#ifndef TOKENIZER2_HH
#define TOKENIZER2_HH

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "util.hh"

#define ERROR(str) std::cerr << __FILE__ << " (" << __LINE__ << "): " << str << std::endl;

namespace kcc2
{

enum TokenType
{
    tkInteger,
    tkWord,

    // 記号
    tkOpenParent,  // (
    tkCloseParent, // )
    tkOpenBrace,   // {
    tkCloseBrace,  // }

    tkNull
};

struct Token
{
    std::string token;
    TokenType type;
    int line;
    int pos;
};

static inline bool IsPrintable(char c)
{
    if (0x21 <= c && c <= 0x7e)
        return true;
    return false;
}

static inline bool IsWord(char c)
{
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
        ('0' <= c && c <= '9') || c == '_')
        return true;
    else
        return false;
}

class Tokenizer
{
    FRIEND_TEST(Tokenize, Tokenizer);

  public:
    Tokenizer() : line_(0), pos_(0) {}

    int Tokenize(const std::vector<char> &buf, std::vector<Token> *dest)
    {
        Init(buf);

        while (!IsEOB())
        {
            // skip comment
            if (Ch() == '/' && Ch(1) == '*')
            {
            }

            if (SkipSpace())
                continue;

            if (IsWord(Ch()))
            {
                std::vector<char> tok(256);
                while (it_ != std::end(buffer_))
                {
                    if (SkipSpace() || !IsWord(Ch()))
                        break;
                    tok.push_back(Ch());
                    Fwd();
                }
                tok.push_back('\0');

                dest->push_back({std::string(tok.begin(), tok.end()),
                                 TokenType::tkWord,
                                 line_,
                                 pos_});
            }
        }
    }

  private:
    void Init(const std::vector<char> &buf)
    {
        buffer_ = buf;
        it_ = std::begin(buffer_);
    }

    // 現在の位置の文字を取得.
    // 引数指定することで先読み可能
    // eob の場合 '\0' を返す
    inline char Ch(unsigned int offset = 0)
    {
        if ((pos_ + offset) >= buffer_.size())
            return '\0';
        return *(it_ + offset);
    }

    inline void Fwd(unsigned int offset = 1)
    {
        it_ += offset;
        pos_ += offset;
    }

    inline bool IsEOB(unsigned int offset = 0)
    {
        return (it_ + offset) == std::end(buffer_);
    }

    // コメント /* ... */ をスキップする
    int SkipBlockComment()
    {
        bool asterisk = false;
        for (;;)
        {
            // EOF
            if (Ch() == '\0') {
                ERROR("incomplete end of block comment");
                return -1;
            }

            if (asterisk && Ch() == '/')
                return 0;

            asterisk = (Ch() == '*');
            Fwd();
        }
    }

    // SP, LF, CR+LF をスキップする
    // CR のみは未サポート
    bool SkipSpace()
    {
        bool is_skip = false;
        while (!IsEOB())
        {
            // CR+LF のスキップ
            if (Ch() == '\r')
            {

                // 次の文字が eob であれば break
                if (IsEOB(1))
                {
                    Fwd();
                    break;
                }

                if (Ch(1) == '\n')
                {
                    Fwd(2);
                    line_++;
                }

                continue;
            }

            if (Ch() == '\n')
            {
                Fwd();
                continue;
            }

            if (Ch() == ' ')
            {
                Fwd();
                continue;
            }
        }

        return is_skip;
    }

    // buffer_ から 1行切り出して返す
    int GetLine(std::vector<char> &dst_buf)
    {
        auto it = std::begin(buffer_);
        while (it != std::end(buffer_))
        {
            if (Ch() == '\r')
            {
                if (Ch(1) == '\n')
                {
                    break;
                }
            }

            if (Ch() == '\n')
            {
                break;
            }

            dst_buf.push_back(*it);
        }

        return 0;
    }

    std::vector<char> buffer_;
    std::vector<char>::const_iterator it_;
    int line_;
    int pos_;
};

} // namespace kcc2
#endif