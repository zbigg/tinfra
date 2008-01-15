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
}
