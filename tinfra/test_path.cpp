//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/path.h"

#include <unittest++/UnitTest++.h>

using std::string;

SUITE(tinfra_path)
{
    TEST(test_is_dir)
    {
        using tinfra::path::is_dir;
        CHECK( is_dir("/") );
        CHECK( is_dir("\\") );
        
        CHECK( is_dir(".") );
        CHECK( is_dir("./") );
        
        #ifdef _WIN32
        
        CHECK( is_dir(".\\") );
        CHECK( is_dir("C:") );
        CHECK( is_dir("C:/") );
        CHECK( is_dir("C:\\") );
        
        #endif
    }
    TEST(test_basename)
    {
	using tinfra::path::basename;
        CHECK_EQUAL( "",  basename(""));
        CHECK_EQUAL( "a", basename("a"));
        CHECK_EQUAL( "a", basename("/a"));
        CHECK_EQUAL( "a", basename("b/a"));
        CHECK_EQUAL( "a", basename("b\\a"));
        CHECK_EQUAL( ".", basename("."));
    }
    
    TEST(test_dirname)
    {
	using tinfra::path::dirname;
        CHECK_EQUAL( ".", dirname(""));
        CHECK_EQUAL( ".", dirname("a"));
        CHECK_EQUAL( "/", dirname("/a"));
        CHECK_EQUAL( "b", dirname("b/a"));
        CHECK_EQUAL( "b", dirname("b\\a"));
        CHECK_EQUAL( ".", dirname("."));
    }
    
    TEST(test_join)
    {
	using namespace tinfra::path;
        CHECK_EQUAL( "", join("",""));
        CHECK_EQUAL( "a", join("a",""));
        CHECK_EQUAL( "a", join("","a"));
        CHECK_EQUAL( "a/b", join("a","b"));
        CHECK_EQUAL( "a/b/c", join("a/b","c"));
        CHECK_EQUAL( "a/b/c", join("a","b/c"));
        
    }
}
