//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/fs.h"
#include "tinfra/test.h"

#include "tinfra/vfs.h"
#include "tinfra/path.h"
#include "tinfra/io/stream.h"

#include <iostream>
#include <stdexcept>


using namespace tinfra;

SUITE(tinfra)
{
    using tinfra::test::test_fs_sandbox;
    
    TEST(test_is_dir)
    {
        using tinfra::fs::is_dir;
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

    TEST(fs_copy)
    {
        tinfra::test::test_fs_sandbox sandbox("testtest_file");
	
        fs::copy("testtest_file", "boo.test");
        CHECK( fs::is_file("boo.test") );
        
        // check if source doesn't exist
        CHECK_THROW( fs::copy("testtest_fileXX", "foo"), std::runtime_error);
        
        // check if dest directory doesn't exist
        CHECK_THROW( fs::copy("testtest_file", "fooFOOfoo/foo"), std::runtime_error);
    }
    
    TEST(fs_list_files)
    {
        test_fs_sandbox tmp_location("testtest_dir");
        std::vector<std::string> files = fs::list_files(".");
		CHECK_EQUAL(1, int(files.size()));
        CHECK_EQUAL("testtest_dir", files[0]);        
    }
    
    TEST(fs_mkdir_rmdir)
    {
        test_fs_sandbox tmp_location;
        {
            const char* name = "kukkuryku";
            CHECK( !fs::exists(name));
            fs::mkdir(name);
            CHECK( fs::exists(name));
            CHECK( fs::is_dir(name));
            fs::rmdir(name);
            CHECK( !fs::is_dir(name));
        }
        
        {
            const char* name = "huzia/c/f/e";
            CHECK( !fs::exists(name));
            fs::mkdir(name,true);
            CHECK( fs::exists(name));
            CHECK( fs::is_dir(name));
            fs::rmdir("huzia/c/f/e");
            fs::rmdir("huzia/c/f");
            fs::rmdir("huzia/c");
            fs::rmdir("huzia");
            CHECK( !fs::exists("a"));
        }
    }
    TEST(fs_recursive)
    {
        test_fs_sandbox tmp_location("testtest_dir");
        fs::recursive_copy("testtest_dir", "boo");
        fs::recursive_rm("boo");
    }
    
    TEST(fs_walk)
    {
        test_fs_sandbox tmp_location("testtest_dir");
        struct foo_walker: public fs::walker {
            virtual bool accept(tstring const& name, tstring const& parent, bool is_dir)
            {
                return true;
            }            
        };
        foo_walker foo;
        fs::walk(".", foo);
    }
    
    void touch(tstring const& name)
    {
        
        using tinfra::io::stream;
        using tinfra::io::open_file;
        std::auto_ptr<stream> f(open_file(name.str().c_str(), std::ios_base::out));
        f->close();
    }
    TEST(fs_localized_name_create)
    {
        test_fs_sandbox tmp_location;
        
        tstring const POLISH_NAME = "\x0c5\x082\xc3\xb3\x64\x6b\x61"; // "lodka", polish boat
        
        touch(POLISH_NAME);
        
        std::vector<std::string> files = fs::list_files(".");
        CHECK_EQUAL(1, int(files.size()));
        CHECK_EQUAL(POLISH_NAME, files[0]);
    }
    
    void test_vfs(tinfra::vfs& fs)
    {
        using tinfra::fs::file_name_list;
        // check if the roots are available
        file_name_list roots = fs.roots();
        for( file_name_list::const_iterator r = roots.begin(); r != roots.end(); ++r)
            CHECK( fs.is_dir(r->c_str()) );
        
        
    }
    
    TEST(test_local_vfs)
    {
        test_vfs(tinfra::local_fs());
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

