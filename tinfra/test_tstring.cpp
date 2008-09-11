#include <unittest++/UnitTest++.h>
#include "tinfra/string.h"
#include "tinfra/fmt.h"

using tinfra::fmt;
using tinfra::tstring;

static const size_t N = tstring::npos;

SUITE(tinfra_tstring)
{
    TEST(find_first_of_char)
    {
        CHECK_EQUAL(0, tstring("abc").find_first_of('a'));
        CHECK_EQUAL(1, tstring("abc").find_first_of('b'));
        CHECK_EQUAL(2, tstring("abc").find_first_of('c'));
        CHECK_EQUAL(tstring::npos, tstring("abc").find_first_of('d'));
        
        CHECK_EQUAL(0, tstring("aaa").find_first_of('a'));
        CHECK_EQUAL(1, tstring("aaa").find_first_of('a',1));
        CHECK_EQUAL(2, tstring("aaa").find_first_of('a',2));
        CHECK_EQUAL(tstring::npos, tstring("aaa").find_first_of('a',3));
    }
    
    TEST(find_first_of_str)
    {
        CHECK_EQUAL(tstring::npos, tstring("").find_first_of(""));
        CHECK_EQUAL(tstring::npos, tstring("a").find_first_of(""));
        CHECK_EQUAL(tstring::npos, tstring("").find_first_of("ab"));
        
        CHECK_EQUAL(1, tstring(" abc ").find_first_of("ab"));
        CHECK_EQUAL(1, tstring(" abc ").find_first_of("ba"));
        
        CHECK_EQUAL(1, tstring(" abc ").find_first_of("ab"));
        CHECK_EQUAL(3, tstring(" abc ").find_first_of("cd"));
        CHECK_EQUAL(tstring::npos, tstring("abc").find_first_of("def"));
    }
}
