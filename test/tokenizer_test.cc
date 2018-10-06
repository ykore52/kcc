#include "tokenizer_test.hh"

using namespace kcc;

int main()
{
    Tokenize_Tokenizer_Test t;

    t.Run();

    ::Testing::DisplaySummary();
}
