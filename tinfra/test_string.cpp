//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>
#include "tinfra/string.h"

SUITE(tinfra_string)
{
    TEST(strip)
    {
        using tinfra::strip;
        CHECK_EQUAL("abc", strip("abc"));
        CHECK_EQUAL("abc", strip(" abc"));
        CHECK_EQUAL("abc", strip("abc "));
        CHECK_EQUAL("a b c", strip("a b c"));
        CHECK_EQUAL("a b c", strip("  a b c  "));
    }
    
    TEST(escape_c)
    {
        using tinfra::escape_c;
        CHECK_EQUAL("abc", escape_c("abc"));
        CHECK_EQUAL("a\\nc", escape_c("a\nc"));
        CHECK_EQUAL("abc\\r\\n", escape_c("abc\r\n"));
        CHECK_EQUAL("a\\x1c", escape_c("a\x01c"));
        CHECK_EQUAL("abc\\xff", escape_c("abc\xff"));        
    }
    
    TEST(split_lines)
    {
        using tinfra::split_lines;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_lines("");
        CHECK_EQUAL(1, r.size() );
        
        r = split_lines("a");
        CHECK_EQUAL(1, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r");
        CHECK_EQUAL(1, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\n");
        CHECK_EQUAL(1, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\n");
        CHECK_EQUAL(1, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\nb");
        CHECK_EQUAL(2, r.size() );
        CHECK_EQUAL("a", r[0] );
        CHECK_EQUAL("b", r[1] );
    }
}
