#include <unittest++/UnitTest++.h>
#include <string>

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"

using namespace tinfra;
    
SUITE(test_test) {
    
    TEST(test_TempTestLocation_empty)
    {
        std::string cwd = fs::pwd();
        {
            test::TempTestLocation testLocation;
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(0, files.size());
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
    
    TEST(test_TempTestLocation_file)
    {
        std::string cwd = fs::pwd();
        {
            test::TempTestLocation testLocation("testtest_file");
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(1, files.size());
            CHECK_EQUAL("testtest_file", files[0]);
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
    
    TEST(test_TempTestLocation_dir)
    {
        std::string cwd = fs::pwd();
        {
            test::TempTestLocation testLocation("testtest_dir");
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(1, files.size());
            CHECK_EQUAL("testtest_dir", files[0]);
            CHECK( path::is_dir("testtest_dir"));
            CHECK( path::is_dir("testtest_dir/a"));        
            CHECK( path::is_file("testtest_dir/file1"));
            CHECK( path::is_file("testtest_dir/a/file2"));
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
}
