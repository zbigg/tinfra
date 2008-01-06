#include "tinfra/path.h"

#include <unittest++/UnitTest++.h>

using std::string;

SUITE(tinfra_path)
{

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
