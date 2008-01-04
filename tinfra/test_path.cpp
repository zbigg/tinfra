#include "tinfra/path.h"

#include <unittest++/UnitTest++.h>

using namespace tinfra::path;
using std::string;


SUITE(tinfra_path)
{
    TEST(test_basename)
    {
        CHECK_EQUAL( "", basename(""));
        CHECK_EQUAL( "a", basename("a"));
        CHECK_EQUAL( "a", basename("/a"));
        CHECK_EQUAL( "a", basename("a/a"));
        CHECK_EQUAL( "a", basename("a\\a"));
        CHECK_EQUAL( ".", basename("."));
    }
    
    TEST(test_dirname)
    {
        CHECK_EQUAL( ".", dirname(""));
        CHECK_EQUAL( ".", dirname("a"));
        CHECK_EQUAL( "/", dirname("/a"));
        CHECK_EQUAL( "a", dirname("a/a"));
        CHECK_EQUAL( "a", dirname("a\\a"));
        CHECK_EQUAL( ".", dirname("."));
    }
    
    TEST(test_join)
    {
        CHECK_EQUAL( "", join("",""));
        CHECK_EQUAL( "a", join("a",""));
        CHECK_EQUAL( "a", join("","a"));
        CHECK_EQUAL( "a/b", join("a","b"));
        CHECK_EQUAL( "a/b/c", join("a/b","c"));
        CHECK_EQUAL( "a/b/c", join("a","b/c"));
        
    }
}
