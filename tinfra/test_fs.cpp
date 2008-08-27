#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"
#include <iostream>

#include <stdexcept>

#include <unittest++/UnitTest++.h>

using namespace tinfra;

SUITE(tinfra_fs)
{
    TEST(test_copy)
    {
        test::TempTestLocation testLocation("testtest_file");
        fs::copy("testtest_file", "boo.test");
        CHECK( path::is_file("boo.test") );
        
        // check if source doesn't exist
        CHECK_THROW( fs::copy("testtest_fileXX", "foo"), std::logic_error);
        
        // check if dest directory doesn't exist
        CHECK_THROW( fs::copy("testtest_file", "fooFOOfoo/foo"), std::logic_error);
    }
    
    TEST(test_list_files)
    {
        test::TempTestLocation tmp_location("testtest_dir");
        std::vector<std::string> files = fs::list_files(".");
		CHECK_EQUAL(1, int(files.size()));
        CHECK_EQUAL("testtest_dir", files[0]);        
    }
    
    TEST(test_mkdir_rmdir)
    {
        test::TempTestLocation tmp_location;
        {
            const char* name = "kukkuryku";
            CHECK( !path::exists(name));
            fs::mkdir(name);
            CHECK( path::exists(name));
            CHECK( path::is_dir(name));
            fs::rmdir(name);
            CHECK( !path::is_dir(name));
        }
        
        {
            const char* name = "huzia/c/f/e";
            CHECK( !path::exists(name));
            fs::mkdir(name,true);
            CHECK( path::exists(name));
            CHECK( path::is_dir(name));
            fs::rmdir("huzia/c/f/e");
            fs::rmdir("huzia/c/f");
            fs::rmdir("huzia/c");
            fs::rmdir("huzia");
            CHECK( !path::exists("a"));
        }
    }
    TEST(test_recursive)
    {
        test::TempTestLocation tmp_location("testtest_dir");
        fs::recursive_copy("testtest_dir", "boo");
        fs::recursive_rm("boo");
    }
    
    TEST(test_walk)
    {
        test::TempTestLocation tmp_location("testtest_dir");
        struct foo_walker: public fs::walker {
            virtual bool accept(const char* name, const char* parent, bool is_dir)
            {
                return true;
            }            
        };
        foo_walker foo;
        fs::walk(".", foo);
    }
}