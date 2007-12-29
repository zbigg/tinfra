#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

using tinfra::symbol;

SUITE(tinfra_symbol)
{
    TEST(symbol_basic)
    {
        CHECK_EQUAL( symbol("a"), symbol("a") );
        CHECK(symbol("c") != symbol("a") );
        
        CHECK_EQUAL("a", symbol("a").c_str());       
    }
}
