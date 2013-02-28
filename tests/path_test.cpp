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
	CHECK_EQUAL( "a/b/c", join("a","b","c"));
        CHECK_EQUAL( "a/b/c", join("a/b","c"));
        CHECK_EQUAL( "a/b/c", join("a","b/c"));
        CHECK_EQUAL( "x/y/z/w", join("x","y", "z", "w"));
        CHECK_EQUAL( "/x/y/z/w/", join("/x","y", "z", "w/"));
        
	// check if spurious componens are properly normalized
        CHECK_EQUAL( "x/y/z/w", join("x//","//y", "//z//", "//w"));
        // but we're not too greedy 
        CHECK_EQUAL( "/x/y/z/w/", join("///x//","//y", "//z//", "//w///"));
        
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
    
    /* 
      not sure if this is needed
    TEST(path_sanitize_removes_unnecessary_dots)
    {
        using tinfra::path::sanitize;
        
        // canonical example
        CHECK_EQUAL("a/b/c", sanitize("./a/./b./c"));
        
        // some more tricky cases
        CHECK_EQUAL("foo/a", sanitize("foo/././a"));
        CHECK_EQUAL("foo/a", sanitize("foo/./a"));
        
        CHECK_EQUAL("foo/a", sanitize("foo/./a"));
    }

    TEST(path_sanitize_removes_unnecessary_slashes)
    {
        using tinfra::path::sanitize;
        // canonical example
        CHECK_EQUAL("foo/a", sanitize(".//foo//a"));
        
        // corner cases
        CHECK_EQUAL("foo/a", sanitize("foo//.//a"));
        CHECK_EQUAL("x/a", sanitize("//.//x//a"));
        CHECK_EQUAL("foo/", sanitize("foo//")); // not sure about this
    }
    
    TEST(path_sanitize_removes_unnecessary_dosts) 
    { 
        using tinfra::path::sanitize;
        
        // canonical example
        CHECK_EQUAL("a/b", sanitize("a/b/c/.."));
        
        CHECK_EQUAL(".", sanitize("a/b/../.."));
        
        // tricky ones
        CHECK_EQUAL("..", sanitize("./.."));
        CHECK_EQUAL("c", sanitize("b/../c/d/.."));
    }
    */
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
    
    TEST(path_extension)
    {
        using tinfra::path::extension;
        
        CHECK_EQUAL( "exe", extension("file.exe") );
        CHECK_EQUAL(  "a", extension("file.a") );
        
        CHECK_EQUAL(  "exe", extension("abc/file.exe") );
        CHECK_EQUAL(  "a", extension("/abc/file.a") );
        
        CHECK_EQUAL(  "exe", extension("abc.a\\file.exe") );
        CHECK_EQUAL(  "a", extension("\\abc.b\\file.a") );
        
        CHECK_EQUAL(  "", extension(".") );
        CHECK_EQUAL(  "", extension("/") );
        CHECK_EQUAL(  "", extension("a") );
        CHECK_EQUAL(  "", extension("./a") );
        CHECK_EQUAL(  "", extension("akuku/dir.b/a") );
        CHECK_EQUAL(  "", extension("akuku\\dir.b\\a") );
        
        // TODO: specify what to do with questionnable case
        //CHECK(  !has_extension("file.") );
        
    }

    TEST(path_remove_extension)
    {
        using tinfra::path::remove_extension;
        
		// no op without extension
        CHECK_EQUAL( "file", remove_extension("file") );
        CHECK_EQUAL( "a", remove_extension("a") );

		// no op for other names
		CHECK_EQUAL(  ".", remove_extension(".") );
        CHECK_EQUAL(  "/", remove_extension("/") );
        CHECK_EQUAL(  "./a", remove_extension("./a") );

		// noop for files without extension but in folders with extension
        CHECK_EQUAL(  "akuku/dir.b/a", remove_extension("akuku/dir.b/a") );
        CHECK_EQUAL(  "akuku\\dir.b\\a", remove_extension("akuku\\dir.b\\a") );
        
        CHECK_EQUAL(  "abc/file", remove_extension("abc/file.exe") );
        CHECK_EQUAL(  "/abc/file", remove_extension("/abc/file.a") );
        
        CHECK_EQUAL(  "abc.a\\file", remove_extension("abc.a\\file.exe") );
        CHECK_EQUAL(  "\\abc.b\\file", remove_extension("\\abc.b\\file.a") );
        
    }

    TEST(path_remove_all_extensions)
    {
        using tinfra::path::remove_all_extensions;
        
		// no op without extension
        CHECK_EQUAL( "file", remove_all_extensions("file") );
        CHECK_EQUAL( "a", remove_all_extensions("a") );
        
		// noop for files without extension but in folders with extension
        CHECK_EQUAL(  "akuku/dir.b.c.d/xxx", remove_all_extensions("akuku/dir.b.c.d/xxx") );
        CHECK_EQUAL(  "akuku.foo.bar\\dir.spam\\a", remove_all_extensions("akuku.foo.bar\\dir.spam\\a") );

		// no op for other names
		CHECK_EQUAL(  ".", remove_all_extensions(".") );
        CHECK_EQUAL(  "/", remove_all_extensions("/") );
        CHECK_EQUAL(  "./a", remove_all_extensions("./a") );

		// actual work
        CHECK_EQUAL(  "abc/file", remove_all_extensions("abc/file.exe.zip") );
        CHECK_EQUAL(  "/abc/file", remove_all_extensions("/abc/file.a.gz.bzip2") );
        
        CHECK_EQUAL(  "abc.a\\file", remove_all_extensions("abc.a\\file.exe.spam") );
        CHECK_EQUAL(  "\\abc.b\\file", remove_all_extensions("\\abc.b\\file.a.gz") );
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

