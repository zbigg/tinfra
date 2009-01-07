//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>

#include "tinfra/regexp.h"

//
// sample program, proof of concept
//

using tinfra::regexp;
using tinfra::scanner;
using tinfra::matcher;

SUITE(tinfra)  {
    TEST(regexp_basic)
    {
        // TODO: write basic regexp test
    }
    TEST(regexp_scanner)
    {
        std::string name;
        int h,m,s;
        bool matches = scanner("^(\\w+) (\\d+):(\\d+):(\\d+)$", "Week 1:22:333") % name % h % m % s;
        
        CHECK(matches);
        CHECK_EQUAL(name, "Week");
        CHECK_EQUAL(h, 1);
        CHECK_EQUAL(m, 22);
        CHECK_EQUAL(s, 333);
    }
    TEST(regexp_matcher)
    {
        // TODO: write matcher test
    }
    TEST(regexp_static_tstring_match_result)
    {
        using tinfra::static_tstring_match_result;
        static_tstring_match_result<2> foo;
        const char* seached_text = "abcdef";
        CHECK( regexp("abc(.*)").matches(seached_text, foo) );
        CHECK_EQUAL( foo.groups[0], "abcdef");
        CHECK_EQUAL( foo.groups[1], "def");
        
        // tstring should reuse original string, so 
        // it should point to original contents
        CHECK_EQUAL( foo.groups[0].data(), seached_text);
        CHECK_EQUAL( foo.groups[1].data(), seached_text+3);
    }
}
