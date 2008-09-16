//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
