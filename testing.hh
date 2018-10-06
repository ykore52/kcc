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
    static void Assert(bool condition, std::string file, int line) noexcept
    {
        std::cout << file << "(" << line << ") : ";
        if (condition) {
            std::cout << "OK" << std::endl;
            ++ok_;
        }
        else {
            std::cout << "FAIL" << std::endl;
            ++fail_;
        }
        ++num_test_;
    }

    static bool DisplaySummary() noexcept
    {
        std::cout << "====================" << std::endl;
        std::cout << "[Test Summary]" << std::endl;
        std::cout << "  OK   : " << ok_ << std::endl;
        std::cout << "  FAIL : " << fail_ << std::endl;
    }

  private:
    static int num_test_;
    static int ok_;
    static int fail_;
};

int Testing::num_test_ = 0;
int Testing::ok_ = 0;
int Testing::fail_ = 0;

#define TEST(condition) ::Testing::Assert((condition), __FILE__, __LINE__)
#define TEST_NOT(condition) ::Testing::Assert(!(condition), __FILE__, __LINE__)
#define TEST_EQUAL(value1, value2) ::Testing::Assert((value1) == (value2), __FILE__, __LINE__)
#define TEST_NOT_EQUAL(value1, value2) ::Testing::Assert((value1) != (value2), __FILE__, __LINE__)

#endif