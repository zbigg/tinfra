#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"
#include <iostream>

#include <unittest++/UnitTest++.h>

using namespace tinfra::fs;
using namespace tinfra::path;
using namespace tinfra::test;

SUITE(tinfra_fs)
{
    TEST(test_copy)
    {
        TempTestLocation testLocation("testtest_file");
        copy("testtest_file", "boo.test");
        CHECK( is_file("boo.test") );
        // TODO: check file contents
    }
    
    TEST(test_list_files)
    {
        TempTestLocation tmp_location("testtest_dir");
        std::vector<std::string> files = list_files(".");
        CHECK_EQUAL(1, files.size());
        CHECK_EQUAL("testtest_dir", files[0]);        
    }
    
    TEST(test_mkdir_rmdir)
    {
        TempTestLocation tmp_location;
        {
            const char* name = "kukkuryku";
            CHECK( !exists(name));
            mkdir(name);
            CHECK( exists(name));
            CHECK( is_dir(name));
            rmdir(name);
            CHECK( !is_dir(name));
        }
        
        {
            const char* name = "huzia/c/f/e";
            CHECK( !exists(name));
            mkdir(name,true);
            CHECK( exists(name));
            CHECK( is_dir(name));
            rmdir("huzia/c/f/e");
            rmdir("huzia/c/f");
            rmdir("huzia/c");
            rmdir("huzia");
            CHECK( !exists("a"));
        }
    }
    TEST(test_recursive)
    {
        TempTestLocation tmp_location("testtest_dir");
        recursive_copy("testtest_dir", "boo");
        recursive_rm("boo");
    }
}
