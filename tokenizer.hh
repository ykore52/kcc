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
    tkDecimal,
    tkHexDecimal,
    tkFloat,
    tkWord,

    // 記号
    tkOpenBrace,   // {
    tkCloseBrace,  // }
    tkOpenParent,  // (
    tkCloseParent, // )
    tkAsterisk,    // *
    tkSlash,       // /
    tkColon,       // :
    tkSemicolon,   // ;

    tkQuestion,  // ?
    tkPlus,      // +
    tkIncrement, // ++
    tkMinus,     // -
    tkDecrement, // --

    tkNot,       // !
    tkQuote,     // '
    tkDoubleQuote, // "


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

static inline bool IsNumber(char c)
{
    return ('0' <= c && c <= '9');
}

static inline bool IsHexdecimal(char c)
{
    return IsNumber(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

// C言語の識別子として使える文字を判定
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

        bool may_be_comment = false;
        bool may_be_increment = false;
        bool may_be_decrement = false;

        while (!IsEOB())
        {
            if (Ch() == '/')
            {
                may_be_comment = true;
                Fwd();
                continue;
            }

            if (may_be_comment && Ch() == '*')
            {
                // skip comment
                SkipBlockComment();
                may_be_comment = false;
                continue;
            }
            else
                may_be_comment = false;

            if (SkipSpace())
            {
                continue;
            }

            if (IsNumber(Ch()))
            {
                std::vector<char> tok;

                int n = 0;

                // 16進数認識用
                bool may_be_hex_0 = false;
                bool may_be_hex_x = false;

                while ((IsHexdecimal(Ch()) || Ch() == 'x') && it_ != std::end(buffer_))
                {
                    if (n == 0 && Ch() == '0') {
                        may_be_hex_0 = true;
                    }

                    if (n == 1 && Ch() == 'x') {
                        may_be_hex_x = true;
                    }

                    tok.push_back(Ch());
                    Fwd();
                    ++n;
                }

                TokenType tt;
                if (may_be_hex_0 && may_be_hex_x)
                    tt = tkHexDecimal;
                else
                    tt = tkDecimal;

                dest->push_back({std::string(tok.begin(), tok.end()),
                                 tt,
                                 line_,
                                 pos_});
                continue;
            }

            // 識別子を認識
            if (IsWord(Ch()))
            {
                std::vector<char> tok;
                while (IsWord(Ch()) && it_ != std::end(buffer_))
                {
                    tok.push_back(Ch());
                    Fwd();
                }

                dest->push_back({std::string(tok.begin(), tok.end()),
                                 TokenType::tkWord,
                                 line_,
                                 pos_});
                continue;
            }

            // 識別子以外の記号を認識
            if (IsPrintable(Ch()))
            {
                std::vector<char> tok;

                TokenType tt;

                switch (Ch())
                {
                case '{':
                    tt = tkOpenBrace;
                    break;
                case '}':
                    tt = tkCloseBrace;
                    break;
                case '(':
                    tt = tkOpenParent;
                    break;
                case ')':
                    tt = tkCloseParent;
                    break;
                case '*':
                    tt = tkAsterisk;
                    break;
                case '/':
                    tt = tkSlash;
                    break;
                case ':':
                    tt = tkColon;
                    break;
                case ';':
                    tt = tkSemicolon;
                    break;
                case '\'':
                    tt = tkQuote;
                    break;
                case '"':
                    tt = tkDoubleQuote;
                    break;
                case '?':
                    tt = tkQuestion;
                    break;
                case '+':

                    // 1byte 先が + だった場合、++演算子であるため
                    // 現在のループではトークンとして登録せずに
                    // 次のループで登録する.
                    if (may_be_increment)
                    {
                        tt = tkIncrement;
                        tok.push_back(Ch());
                        Fwd();
                        dest->push_back({std::string(tok.begin(), tok.end()),
                                         tt,
                                         line_,
                                         pos_});
                        may_be_increment = false; // フラグを戻しておく
                        continue;
                    }

                    // 1byte 先が + でなければ (単項または二項の) + 演算子である
                    if (Ch(1) == '+')
                    {
                        may_be_increment = true;
                        tok.push_back(Ch());
                        Fwd();
                        continue;
                    }
                    else
                    {
                        tt = tkPlus;
                    }
                    break;

                case '-':

                    // 1byte 先が - だった場合、--演算子であるため
                    // 現在のループではトークンとして登録せずに
                    // 次のループで登録する.
                    if (may_be_decrement)
                    {
                        tt = tkDecrement;
                        tok.push_back(Ch());
                        Fwd();
                        dest->push_back({std::string(tok.begin(), tok.end()),
                                         tt,
                                         line_,
                                         pos_});
                        may_be_decrement = false; // フラグを戻しておく
                        continue;
                    }

                    // 1byte 先が - でなければ (単項または二項の) - 演算子である
                    if (Ch(1) == '-')
                    {
                        may_be_decrement = true;
                        tok.push_back(Ch());
                        Fwd();
                        continue;
                    }
                    else
                    {
                        tt = tkMinus;
                    }
                    break;
                }

                tok.push_back(Ch());
                Fwd();
                dest->push_back({std::string(tok.begin(), tok.end()),
                                 tt,
                                 line_,
                                 pos_});
                continue;
            }
        }

        return 0;
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

    // 現在のポジションを 1byte 進める
    inline void Fwd(unsigned int offset = 1)
    {
        it_ += offset;
        pos_ += offset;
    }

    // 入力バッファの最後であるかの判定
    inline bool IsEOB(unsigned int offset = 0)
    {
        return (it_ + offset) == std::end(buffer_);
    }

    // ブロックコメント /* ... */ をスキップする
    bool SkipBlockComment()
    {
        bool may_be_end = false;
        for (;;)
        {
            // EOF
            if (Ch() == '\0')
            {
                ERROR("incomplete end of block comment");
                return false;
            }

            if (may_be_end && Ch() == '/')
                return true;

            may_be_end = (Ch() == '*');
            Fwd();
        }
        return true;
    }

    // SP, CR, LF, CR+LF をスキップする
    bool SkipSpace()
    {
        bool ok = false;
        bool may_be_lf = false;
        while (!IsPrintable(Ch()) && !IsEOB())
        {
            // CR+LF のスキップ
            if (Ch() == '\r')
            {
                Fwd();

                // 次の文字が eob であれば break
                if (IsEOB(1))
                {
                    break;
                }

                // 次の文字が LF でなければ改行
                if (Ch(1) != '\n')
                {
                    ++line_;
                }

                continue;
            }

            if (Ch() == '\n')
            {
                ++line_;

                Fwd();
                continue;
            }

            if (Ch() == ' ')
            {
                Fwd();
                continue;
            }
        }

        return ok;
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