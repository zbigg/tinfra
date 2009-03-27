//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>
#include "tinfra/string.h"
#include "tinfra/fmt.h"

using tinfra::fmt;

SUITE(tinfra)
{
    std::string strip(std::string const& s)
    {
        std::string r1 = tinfra::strip(s);
        std::string r2(s);
        tinfra::strip_inplace(r2);
        if( r1 != r2 ) 
            throw UnitTest::AssertException("strip vs strip_inplace difference",__FILE__, __LINE__);
        
        return r1;
    }
    
    TEST(string_strip)
    {
        CHECK_EQUAL("abc", strip("abc"));
        CHECK_EQUAL("abc", strip(" abc"));
        CHECK_EQUAL("abc", strip("abc "));
        CHECK_EQUAL("a b c", strip("a b c"));
        CHECK_EQUAL("a b c", strip("  a b c  "));
    }
    
    std::string escape_c(std::string const& s)
    {
        std::string r1 = tinfra::escape_c(s);
        std::string r2(s);
        tinfra::escape_c_inplace(r2);
        if( r1 != r2 ) 
            throw UnitTest::AssertException("escape_c vs escape_c_inplace difference",__FILE__, __LINE__);
        
        return r1;
    }
    
    TEST(string_escape_c)
    {
        CHECK_EQUAL("abc", escape_c("abc"));
        CHECK_EQUAL("a\\nc", escape_c("a\nc"));
        CHECK_EQUAL("abc\\r\\n", escape_c("abc\r\n"));
        CHECK_EQUAL("a\\x1c", escape_c("a\x01c"));
        CHECK_EQUAL("abc\\xff", escape_c("abc\xff"));        
    }
    
    std::string chop(std::string const& s)
    {
        std::string r1 = tinfra::chop(s);
        std::string r2(s);
        tinfra::chop_inplace(r2);
        if( r1 != r2 ) 
            throw UnitTest::AssertException(fmt("chop vs chop_inplace difference for: %s") % tinfra::escape_c(s),__FILE__, __LINE__);
        
        return r1;
    }
    
    TEST(string_chop)
    {
        CHECK_EQUAL(" ", chop(" "));
        CHECK_EQUAL("", chop(""));
        CHECK_EQUAL("abc", chop("abc"));
        CHECK_EQUAL("abc", chop("abc\n"));
        CHECK_EQUAL("abc", chop("abc\r"));
        CHECK_EQUAL("abc", chop("abc\r\n"));
        CHECK_EQUAL("abc", chop("abc\n\r"));
        CHECK_EQUAL("abc", chop("abc\n\r"));
        CHECK_EQUAL("", chop("\r"));
        CHECK_EQUAL("", chop("\r\n"));
    }
    
    TEST(string_split_lines)
    {
        using tinfra::split_lines;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_lines("");
        CHECK_EQUAL(1u, r.size() );
        
        r = split_lines("a");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\n");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\n");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\nb");
        CHECK_EQUAL(2u, r.size() );
        CHECK_EQUAL("a", r[0] );
        CHECK_EQUAL("b", r[1] );
    }
}
