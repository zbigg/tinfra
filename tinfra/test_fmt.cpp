//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fmt.h"

#include <unittest++/UnitTest++.h>
#include <sstream>

using tinfra::fmt;
using tinfra::simple_fmt;

SUITE(tinfra_fmt) {
    TEST(fmt_basic)
    {
        CHECK( fmt("").str() == "");
        CHECK( fmt("a").str() == "a");
        CHECK( fmt("%%").str() == "%");
        CHECK( fmt("a%%b").str() == "a%b");
    }

    TEST(fmt_str)
    {
        CHECK_EQUAL( "TEST", (const char*)(fmt("%s") % "TEST" ) );
        CHECK_EQUAL( "TEST", (const char*)(fmt("T%sT") % "ES" ) );    
    }

    TEST(fmt_number)
    {
        CHECK_EQUAL( "0", (const char*)(fmt("%i") % 0) );
        CHECK_EQUAL( "1", (const char*)(fmt("%i") % 1) );
        CHECK_EQUAL( "-1", (const char*)(fmt("%i") % -1) );
    }

    TEST(fmt_complex)
    {
        CHECK_EQUAL( "AB", (const char*)(fmt("%s%s") % "A" % "B") );
        CHECK_EQUAL( "ABCDEF", (const char*)(fmt("A%sC%sE%s") % 'B' % 'D' % 'F') );
    }

    TEST(fmt_errors)
    {
        CHECK_THROW( simple_fmt("BOBO%").push(1).str(), tinfra::format_exception); // % at the end
        CHECK_THROW( simple_fmt("%s%s").push(1).str(), tinfra::format_exception); // not all arguments realized
        CHECK_THROW( simple_fmt("%s").push(1).push(2).str(), tinfra::format_exception); // to many actual arguments
    }

    TEST(simple_fmt_errors)
    {
        CHECK_THROW( simple_fmt("%x").push(1).str(), tinfra::format_exception); // bad format command
        CHECK_THROW( simple_fmt("%l").push(1).str(), tinfra::format_exception); // bad format command
    }
    
    TEST(fmt_type_str)
    {
        const char* str = "zero";
        CHECK_EQUAL( str, (const char*)(fmt("%s") % str));
    }
    
    TEST(fmt_iostream)
    {
	std::ostringstream tst;
	tst << fmt("%s %s %s") % "a" % 1 % 'c';
	CHECK_EQUAL("a 1 c", tst.str().c_str());
    }
} // end SUITE(fmt)
