//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/lex.h"

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    
    using tinfra::from_string;
    using tinfra::to_string;
    
    TEST(lex_basic_api)
    {
        int ix = from_string<int>("-02");
        CHECK_EQUAL(-2, ix);
        
        std::string sx = to_string<int>(ix);
        
        CHECK_EQUAL("-2", sx);
    }
    // TODO, add rest of tests
    //   string doesn't cut on spaces
    //   char[] conversions
    //   symbol, tstring API
}
