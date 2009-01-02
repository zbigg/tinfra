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

SUITE(tinfra_tstring)
{
    TEST(find_first_of_char)
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
    
    TEST(find_first_not_of_str)
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
    
    TEST(find)
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
}
