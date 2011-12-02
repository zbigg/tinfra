//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/stream.h"  // we test this API
#include "tinfra/test.h"    // for test infra

#include <sstream>

SUITE(tinfra) {
    TEST(memory_input_stream_basic)
    {
        using tinfra::create_memory_input_stream;
        const tinfra::tstring text = "0123456789";
        std::auto_ptr<tinfra::input_stream> in(create_memory_input_stream(text.data(), text.size(), tinfra::USE_BUFFER));
        
        char buf[128];
        CHECK_EQUAL( 5, in->read(buf, 5));
        CHECK_EQUAL( "01234", std::string(buf,5));
        
        CHECK_EQUAL( 5, in->read(buf, sizeof(buf)));
        CHECK_EQUAL( "56789", std::string(buf,5));
                    
        CHECK_EQUAL( 0, in->read(buf, sizeof(buf)));
    }
    
    TEST(memory_output_stream_basic)
    {
        using tinfra::create_memory_output_stream;
        std::string result;
        std::auto_ptr<tinfra::output_stream> out(create_memory_output_stream(result));
        
        char buf[128];
        CHECK_EQUAL( 5, out->write(std::string("01234",5)));
        CHECK_EQUAL( "01234", result);
        
        CHECK_EQUAL( 0, out->write(std::string("",0)));
        CHECK_EQUAL( "01234", result);
                    
        CHECK_EQUAL( 5, out->write(std::string("56789",5)));
        CHECK_EQUAL( "0123456789", result);
                    
        CHECK_EQUAL( 5, out->write(std::string("AB\0CD",5)));
        CHECK_EQUAL( std::string("0123456789AB\0CD",15), result);
        
        CHECK_EQUAL( 15, result.size());
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

