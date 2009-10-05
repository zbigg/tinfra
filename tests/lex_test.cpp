//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
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
