#include <unittest++/UnitTest++.h>

#include "tinfra/regexp.h"

//
// sample program, proof of concept
//

using tinfra::regexp;
using tinfra::scanner;
using tinfra::matcher;

SUITE(tinfra_regexp)  {
    TEST(regexp_basic)
    {
        // TODO: write basic regexp test
    }
    TEST(scanner)
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
    TEST(matcher)
    {
        // TODO: write matcher test
    }
}
