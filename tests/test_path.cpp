//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/path.h" // API under test
#include "tinfra/fs.h"

#include "tinfra/test.h" // test infra

using std::string;

SUITE(tinfra)
{
    TEST(path_basename)
    {
	using tinfra::path::basename;
        CHECK_EQUAL( "",  basename(""));
        CHECK_EQUAL( "a", basename("a"));
        CHECK_EQUAL( "a", basename("/a"));
        CHECK_EQUAL( "a", basename("b/a"));
        CHECK_EQUAL( "a", basename("b\\a"));
        CHECK_EQUAL( ".", basename("."));
    }
    
    TEST(path_dirname)
    {
	using tinfra::path::dirname;
        CHECK_EQUAL( ".", dirname(""));
        CHECK_EQUAL( ".", dirname("a"));
        CHECK_EQUAL( "/", dirname("/a"));
        CHECK_EQUAL( "b", dirname("b/a"));
        CHECK_EQUAL( "b", dirname("b\\a"));
        CHECK_EQUAL( ".", dirname("."));
    }
    
    TEST(path_join)
    {
	using namespace tinfra::path;
        CHECK_EQUAL( "", join("",""));
        CHECK_EQUAL( "a", join("a",""));
        CHECK_EQUAL( "a", join("","a"));
        CHECK_EQUAL( "a/b", join("a","b"));
        CHECK_EQUAL( "a/b/c", join("a/b","c"));
        CHECK_EQUAL( "a/b/c", join("a","b/c"));
        
        CHECK_EQUAL( "/b", join("/","b"));
        CHECK_EQUAL( "/a/b", join("/a/","b"));
        CHECK_EQUAL( "/a/b", join("/a","/b"));
        CHECK_EQUAL( "a/b", join("a","/b"));
    }
    
    TEST(path_is_absolute)
    {
        using tinfra::path::is_absolute;
        
        CHECK(  is_absolute("/") );
        CHECK(  is_absolute("\\") );
        CHECK(  is_absolute("/abc/") );
        CHECK(  is_absolute("/abc/def") );
        CHECK( !is_absolute("abc/def") );
        CHECK( !is_absolute("a") );
        CHECK( !is_absolute("") );

#ifdef _WIN32
        CHECK( is_absolute("C:") );
        CHECK( is_absolute("d:") );
        CHECK( is_absolute("e:a") );
        CHECK( is_absolute("f:d") );
        CHECK( is_absolute("g:\abc") );
        CHECK( is_absolute("h://abc") );

        CHECK( !is_absolute("abc") );
        CHECK( !is_absolute("./a") );
        CHECK( !is_absolute("c/a") );
#endif
    }
    
    TEST(path_has_extension)
    {
        using tinfra::path::has_extension;
        
        CHECK(  has_extension("file.exe") );
        CHECK(  has_extension("file.a") );
        
        CHECK(  has_extension("abc/file.exe") );
        CHECK(  has_extension("/abc/file.a") );
        
        CHECK(  has_extension("abc\\file.exe") );
        CHECK(  has_extension("\\abc\\file.a") );
        
        CHECK(  !has_extension(".") );
        
        CHECK(  !has_extension("a") );
        CHECK(  !has_extension("./a") );
        CHECK(  !has_extension("akuku/dir.b/a") );
        CHECK(  !has_extension("akuku\\dir.b\\a") );
        
        // TODO: specify what to do with questionnable case
        //CHECK(  !has_extension("file.") );
        
    }

    TEST(path_search_executable)
    {
        using tinfra::path::search_executable;
        using tinfra::path::is_executable;
        using tinfra::fs::exists;
#ifdef _WIN32
        std::string rp = search_executable("regedit");
        CHECK( rp != "");
        CHECK( exists(rp) );
        CHECK( is_executable(rp) );
        
        CHECK( search_executable("regedit.foo") == "");
        CHECK( search_executable("dupa\\regedit.foo") == "");
#else
        std::string rp = search_executable("uname");
        CHECK( exists(rp) );
        CHECK( is_executable(rp) );
                
        CHECK( search_executable("uname-surelydontexists") == "");
        
        CHECK( search_executable("/bin/sh") == "/bin/sh");
#endif
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

