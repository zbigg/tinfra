//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/inifile.h"

#include <sstream>

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    TEST(inifile_test_parse)
    {
        const char* text = ";comment\n"
                           "[section]\n"
                           "name=value\n"
                           "name = value \n";
        std::istringstream in(text);
        
        namespace tif = tinfra::inifile;
        
        tif::parser p(in);
        
        tif::entry re;
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::COMMENT, re.type );
        CHECK_EQUAL( "comment", re.value );
        
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::SECTION, re.type );
        CHECK_EQUAL( "section", re.name);
        
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "name", re.name);
        CHECK_EQUAL( "value", re.value);
        
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "name", re.name);
        CHECK_EQUAL( "value", re.value);
        
        CHECK( !p.fetch_next(re) );
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

