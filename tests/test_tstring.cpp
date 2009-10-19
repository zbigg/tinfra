//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>
#include "tinfra/string.h"
#include "tinfra/fmt.h"

using tinfra::fmt;
using tinfra::tstring;

static const size_t N = tstring::npos;

SUITE(tinfra)
{
    TEST(tstring_find_first_of_char)
    {
        /*struct {
            size_t expected;
            const char* victim;
            char param;
        } dataset = { 
            { 0, "abc", 'a'},
            { 1, "abc", 'b'},
            { 2, "abc", 'c'},
        */    
            
        CHECK_EQUAL(0u, tstring("abc").find_first_of('a'));
        CHECK_EQUAL(1u, tstring("abc").find_first_of('b'));
        CHECK_EQUAL(2u, tstring("abc").find_first_of('c'));
        CHECK_EQUAL(tstring::npos, tstring("abc").find_first_of('d'));
        
        CHECK_EQUAL(0u, tstring("aaa").find_first_of('a'));
        CHECK_EQUAL(1u, tstring("aaa").find_first_of('a',1));
        CHECK_EQUAL(2u, tstring("aaa").find_first_of('a',2));
        CHECK_EQUAL(tstring::npos, tstring("aaa").find_first_of('a',3));
    }
    
    TEST(tstring_find_first_of_str)
    {
        CHECK_EQUAL(tstring::npos, tstring("").find_first_of(""));
        CHECK_EQUAL(tstring::npos, tstring("a").find_first_of(""));
        CHECK_EQUAL(tstring::npos, tstring("").find_first_of("ab"));
        
        CHECK_EQUAL(1u, tstring(" abc ").find_first_of("ab"));
        CHECK_EQUAL(1u, tstring(" abc ").find_first_of("ba"));
        
        CHECK_EQUAL(1u, tstring(" abc ").find_first_of("ab"));
        CHECK_EQUAL(3u, tstring(" abc ").find_first_of("cd"));
        CHECK_EQUAL(tstring::npos, tstring("abc").find_first_of("def"));
    }
    
    TEST(tstring_find_first_not_of_str)
    {
        struct {
            size_t      expected;
            const char* victim;
            const char* param;
        } dataset[] = { 
            { tstring::npos, "", ""},
            { 0,             "abc", ""},
            { tstring::npos, "", "a"},
            { 1,             "abc", "a"},
            { 2,             "abc", "ab"},
            { tstring::npos, "abc", "abc"},
            { 0,             "abc", "x"}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
        
        for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find_first_not_of(dataset[i].param));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find_first_not_of(dataset[i].param));
        }        
    }
    
    TEST(tstring_find)
    {
	struct {
            size_t      expected;
            const char* victim;
            const char* param;
        } dataset[] = { 
            { 0,             "", ""},
            { 0,             "abc", ""},
            { tstring::npos, "", "a"},
            { 0,             "abc", "a"},
            { 0,             "abc", "ab"},
            { 0,             "abc", "abc"},
            { tstring::npos, "abc", "x"}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
	
	for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find(dataset[i].param));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find(dataset[i].param));
        }
    }
    
    TEST(tstring_null_termination)
    {
        tinfra::string_pool pool;
        
        {   // check that literal based tstring is null terminated
            // and don't duplicate data using_c_str
            tstring literal_based("abc");
            CHECK(  literal_based.c_str(pool) == literal_based.data() );
            CHECK(  literal_based.is_null_terminated());
        }
        
        {
            // same as above for std::string based tstring
            std::string akuku("abc");
            tstring  basic_string_based(akuku);
            CHECK( basic_string_based.is_null_terminated());
            CHECK( basic_string_based.c_str(pool) == akuku.c_str());
        }
        
        {
            // buffer based string (eg. slice of another)
            // usually is not null_terminated, so
            // it should use pool to alloc copy of string
            char buf[] = { 'a', 'b', 'c' };
                
            tstring buffer_based(buf, sizeof(buf));
            CHECK( !buffer_based.is_null_terminated());
            
            CHECK( buffer_based.c_str(pool) != buffer_based.data() );
        }
    }
    
    static void foo(tstring const& a)
    {
        std::string x = a.str();
    }
    
    // @compilation-test
    // @valgrind-test
    TEST(tstring_conversions)
    {
        // checks that tstrin const& parameters are captured
        // in most use cases
        foo( "abc" );
        
        std::string a("abc");
        foo( a );
        foo( std::string("abc") );
        
        using tinfra::fmt;
        foo( fmt("abc%s") % a );
    }
}
