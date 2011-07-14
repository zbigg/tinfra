//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/text.h" // API under test
#include "tinfra/test.h" // test infra
#include "tinfra/tstring.h"
#include "tinfra/stream.h"

SUITE(tinfra)
{
    TEST(text_line_reader_empty_file)
    	// returns 0 entries!!!
    {
	const tinfra::tstring text = "";
        std::auto_ptr<tinfra::input_stream> in(create_memory_input_stream(text.data(), text.size(), tinfra::USE_BUFFER));
        tinfra::line_reader r(*in);
        std::string result;
        
        CHECK_EQUAL(false, r.fetch_next(result));
        //CHECK_EQUAL("", result); 
    }
    
    TEST(text_line_reader_onechar)
    	// returns 1 entry even without a line!
    {
	const tinfra::tstring text = "a";
        std::auto_ptr<tinfra::input_stream> in(create_memory_input_stream(text.data(), text.size(), tinfra::USE_BUFFER));
        tinfra::line_reader r(*in);
        std::string result;
        
        CHECK_EQUAL(true, r.fetch_next(result));
        CHECK_EQUAL("a", result);
        
        CHECK_EQUAL(false, r.fetch_next(result));
    }
    
    TEST(text_line_reader_normal)
    	// returns 1 entries!!!
    	// or 1 entry !?
    {
	const tinfra::tstring text = "\nabc\ndef\n";
        std::auto_ptr<tinfra::input_stream> in(create_memory_input_stream(text.data(), text.size(), tinfra::USE_BUFFER));
        tinfra::line_reader r(*in);
        std::string result;
        
        CHECK_EQUAL(true, r.fetch_next(result));
        CHECK_EQUAL("\n", result);
        
        CHECK_EQUAL(true, r.fetch_next(result));
        CHECK_EQUAL("abc\n", result);
        
        CHECK_EQUAL(true, r.fetch_next(result));
        CHECK_EQUAL("def\n", result);
        
        CHECK_EQUAL(false, r.fetch_next(result));
    }
}
