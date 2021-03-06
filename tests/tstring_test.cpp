//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/tstring.h"
#include "tinfra/fmt.h"

#include "tinfra/test.h" // test infra

using tinfra::fmt;
using tinfra::tstring;

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

    TEST(tstring_find_last_of_str)
    {
		const size_t npos = tstring::npos;
        struct {
            size_t      expected;
            const char* victim;
            const char* param;
			size_t pos;
        } dataset[] = { 
            { npos, "",    "", npos},
            { npos, "abc", "d", npos},
            { npos, "",    "a", npos},
			{ 0,    "abc", "a", npos},
			{ 2,    "abc", "abc", npos},
            { 2,    "abc", "c", npos},
			{ 2,    "abc", "c", 2},
			{ npos,    "abc", "c", 1},
            { 2,    "abc", "xbc", npos},
			{ 2,    "acb", "xcb", npos}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
        
        for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find_last_of(dataset[i].param, dataset[i].pos));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find_last_of(dataset[i].param, dataset[i].pos));
        }        
    }

	TEST(tstring_find_last_of_char)
    {
		const size_t npos = tstring::npos;
        struct {
            size_t      expected;
            const char* victim;
            char   param;
			size_t pos;
        } dataset[] = { 
            { npos, "",    'X', npos},
            { npos, "abc", 'X', npos},
            { npos, "",    'a', npos},
			{ 0,    "abc", 'a', npos},
            { 2,    "abc", 'c', npos},
			{ 2,    "abc", 'c', 2},
			{ npos,    "abc", 'c', 1},
			{ npos,    "abc", 'c', 0}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
        
        for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find_last_of(dataset[i].param, dataset[i].pos));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find_last_of(dataset[i].param, dataset[i].pos));
        }        
    }

	TEST(tstring_find_last_not_of_str)
    {
		const size_t npos = tstring::npos;
        struct {
            size_t      expected;
            const char* victim;
            const char* param;
			size_t pos;
        } dataset[] = { 
            { npos, "",    "", npos},
            { 2,    "abc", "d", npos},
            { npos, "",    "a", npos},
			{ 2,    "abc", "a", npos},
			{ npos,    "abc", "abc", npos},
            { 1,    "abc", "c", npos},
			{ 1,    "abc", "c", 2},
			{ 1,    "abc", "c", 1},
            { 0,    "abc", "xbc", npos},
			{ 0,    "acb", "xcb", npos}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
        
        for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find_last_not_of(dataset[i].param, dataset[i].pos));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find_last_not_of(dataset[i].param, dataset[i].pos));
        }        
    }

	TEST(tstring_find_last_not_of_char)
    {
		const size_t npos = tstring::npos;
        struct {
            size_t      expected;
            const char* victim;
            char   param;
			size_t pos;
        } dataset[] = { 
            { npos, "",    'X', npos},
            { 2,    "abc", 'X', npos},
            { npos, "",    'a', npos},
			{ 2,    "abc", 'a', npos},
            { 1,    "abc", 'c', npos},
			{ 1,    "abc", 'c', 2},
			{ 1,    "abc", 'c', 1},
			{ 0,    "abc", 'c', 0}
        };
        const int N = sizeof(dataset)/sizeof(dataset[0]);
        
        for( int i = 0; i < N; ++i ) {
            CHECK_EQUAL(dataset[i].expected, tstring(dataset[i].victim).find_last_not_of(dataset[i].param, dataset[i].pos));
            CHECK_EQUAL(dataset[i].expected, std::string(dataset[i].victim).find_last_not_of(dataset[i].param, dataset[i].pos));
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

    TEST(tstring_starts_with) {
        CHECK( tstring("abc123").starts_with('a') );
        CHECK( tstring("abc123").starts_with("a") );
        CHECK( tstring("abc123").starts_with("abc") );
        CHECK( tstring("abc123").starts_with("abc123") );

        CHECK( tstring("abc123").starts_with(tstring("ab")) );

        CHECK( !tstring("abc123").starts_with("abc12X") );
        CHECK( !tstring("abc123").starts_with(tstring("abckukuku")) );
    }

    TEST(tstring_ends_with) {
        CHECK( tstring("abc123").ends_with('3') );
        CHECK( tstring("abc123").ends_with("3") );
        CHECK( tstring("abc123").ends_with("123") );
        CHECK( tstring("abc123").ends_with("abc123") );

        CHECK( tstring("abc123").ends_with(tstring("23")) );

        CHECK( !tstring("abc123").ends_with("abc12X") );
        CHECK( !tstring("abc123").ends_with(tstring("abckukuku")) );
    }

    TEST(tstring_string_view_compatibility)
    {
        tstring ts("abc");
        tstring::iterator c = ts.begin();
        tstring::iterator e = ts.end();

        CHECK(c != e);
        CHECK(c + 3 == e);
    }
}
