#ifndef UTIL_HH
#define UTIL_HH

#include <iostream>
#include <sstream>
#include <vector>

static int num_fn_call = 0;

static inline void PDEBUG(std::string message)
{
    std::cout << message << std::endl;
}

static int global_loop_counter = 0;
#define IF_N_RUN(n, runner)           \
    do                                \
    {                                 \
        if (global_loop_counter == n) \
        {                             \
            (runner);                 \
        }                             \
        ++global_loop_counter;        \
    } while (0)

#define DBG_IN(message) pdebug((message), (__FILE__), (__LINE__), true)
#define DBG_OUT(message) pdebug((message), (__FILE__), (__LINE__), false)
static inline void pdebug(std::string message, const char *file, int line, bool indent)
{
    if (indent)
    {
        std::string sp;
        for (int i = 0; i < num_fn_call; ++i)
            sp += " ";
        std::cout << sp << file << "(" << line << ") : " << message << std::endl;
        ++num_fn_call;
    }
    else
    {
        --num_fn_call;
        std::string sp;
        for (int i = 0; i < num_fn_call; ++i)
            sp += " ";
        std::cout << sp << file << "(" << line << ") : " << message << "-> out" << std::endl;
    }
}

static inline void PrintBool(bool condition)
{
    std::cout << (condition ? "true" : "false") << std::endl;
}

static inline void PrintCharVector(const std::vector<char> &v)
{
    std::cout << "[";

    int n = 0;
    for (auto c : v)
    {
        if (n > 0)
        {
            std::cout << ",";
        }

        if (' ' <= c && c <= '~')
            std::cout << "'" << c << "'";
        else
        {
            switch (c)
            {
            case '\n':
                std::cout << "'\\n'";
                break;
            case '\0':
                std::cout << "'\\n'";
                break;
            default:
                std::cout << "unknown";
            }
        }
        n++;
    }

    std::cout << "]" << std::endl;
}

#define FRIEND_TEST(test_case_name, test_name) \
    friend class test_case_name##_##test_name##_Test

class Exception : public std::runtime_error
{
    std::string msg;

  public:
    Exception(std::string msg, const char *file, int line) : std::runtime_error(msg)
    {
        std::ostringstream oss;
        oss << file << "(" << line << "): " << msg;
        this->msg = oss.str();
    }

    ~Exception() throw() {}
    const char *what() const throw()
    {
        return msg.c_str();
    }
};

#define throw_ln(msg) throw Exception(msg,__FILE__,__LINE__)

#endif