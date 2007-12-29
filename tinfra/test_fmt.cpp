#include "tinfra/fmt.h"

#include <unittest++/UnitTest++.h>

using tinfra::simple_fmt;

TEST(simple_fmt)
{
    CHECK( simple_fmt("").str() == "");
    CHECK( simple_fmt("a").str() == "a");
    CHECK( simple_fmt("%%").str() == "%");
    CHECK( simple_fmt("a%%b").str() == "a%b");
}

TEST(simple_fmt2)
{
    CHECK_EQUAL( "TEST", (simple_fmt("%s") % "TEST").c_str() );
    CHECK_EQUAL( "TEST", (simple_fmt("T%sT") % "ES").c_str() );
}