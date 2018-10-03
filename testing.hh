#ifndef TESTING_HH
#define TESTING_HH

#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <cassert>

class Testing
{
  public:
    static void Assert(bool condition, std::string file, int line)
    {
        std::cout << file << "(" << line << ") : ";
        if (condition)
        {
            std::cout << "OK" << std::endl;
        }
        else
        {
            std::cout << "FAIL" << std::endl;
        }
    }
};

#define TEST(condition) ::Testing::Assert((condition), __FILE__, __LINE__)

#endif