#ifndef UTIL_HH
#define UTIL_HH

#include <iostream>
#include <vector>

static inline void PDEBUG(std::string message)
{
    std::cout << message << std::endl;
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
#endif