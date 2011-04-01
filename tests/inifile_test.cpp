//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/inifile.h" // we test this API
#include "tinfra/test.h"    // for test infra

#include <sstream>

SUITE(tinfra) {
    TEST(inifile_test_parse)
    {
        const tinfra::tstring text = ";comment\n"
                           "[section]\n"
                           "name=value\n"
                           "name = value \n"
                           "empty=\n"
                           "empty= \n";
        std::auto_ptr<tinfra::input_stream> in(create_memory_input_stream(text.data(), text.size(), tinfra::USE_BUFFER));
        
        namespace tif = tinfra::inifile;
        
        tif::parser p(*in);
        
        tif::entry re;
        
        //";comment\n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::COMMENT, re.type );
        CHECK_EQUAL( "comment", re.value );
        
        //"[section]\n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::SECTION, re.type );
        CHECK_EQUAL( "section", re.name);
        
        //"name=value\n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "name", re.name);
        CHECK_EQUAL( "value", re.value);
        
        //"name = value \n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "name", re.name);
        CHECK_EQUAL( "value", re.value);
        
        //"empty=\n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "empty", re.name);
        CHECK_EQUAL( "", re.value);
        
        //"empty= \n"
        CHECK( p.fetch_next(re) );
        CHECK_EQUAL( tif::ENTRY, re.type );
        CHECK_EQUAL( "empty", re.name);
        CHECK_EQUAL( "", re.value);
        
        CHECK( !p.fetch_next(re) );
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

